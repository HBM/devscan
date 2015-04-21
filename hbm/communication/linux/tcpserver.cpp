// Copyright 2014 Hottinger Baldwin Messtechnik
// Distributed under MIT license
// See file LICENSE provided

#include <memory>
#include <cstring>
#include <arpa/inet.h>
#include <netinet/tcp.h>

#include <unistd.h>

#include <errno.h>
#include <syslog.h>

#include "hbm/communication/socketnonblocking.h"
#include "hbm/communication/tcpserver.h"
#include "hbm/sys/eventloop.h"


namespace hbm {
	namespace communication {
		TcpServer::TcpServer(sys::EventLoop &eventLoop)
			: m_listeningSocket(-1)
			, m_eventLoop(eventLoop)
			, m_acceptCb()
		{
		}

		TcpServer::~TcpServer()
		{
			stop();
		}

		int TcpServer::start(uint16_t port, int backlog, Cb_t acceptCb)
		{
			//ipv6 does work for ipv4 too!
			sockaddr_in6 address;

			memset(&address, 0, sizeof(address));
			address.sin6_family = AF_INET6;
			address.sin6_addr = in6addr_any;
			address.sin6_port = htons(port);

			m_listeningSocket = ::socket(address.sin6_family, SOCK_STREAM | SOCK_NONBLOCK, 0);
			if (m_listeningSocket==-1) {
				syslog(LOG_ERR, "%s: Socket initialization failed '%s'", __FUNCTION__ , strerror(errno));
				return -1;
			}
			if (::bind(m_listeningSocket, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == -1) {
				syslog(LOG_ERR, "%s: Binding socket to port initialization failed '%s'", __FUNCTION__ , strerror(errno));
				return -1;
			}

			if (listen(m_listeningSocket, backlog)==-1) {
				return -1;
			}

			m_acceptCb = acceptCb;
			if (acceptCb) {
				m_eventLoop.addEvent(m_listeningSocket, std::bind(&TcpServer::process, this));
			}


			return 0;
		}

		void TcpServer::stop()
		{
			m_eventLoop.eraseEvent(m_listeningSocket);
			close(m_listeningSocket);
		}



		workerSocket_t TcpServer::acceptClient()
		{
			int clientFd = accept(m_listeningSocket, NULL, NULL);

			if (clientFd<0) {
				return workerSocket_t();
			}

			return workerSocket_t(new SocketNonblocking(clientFd, m_eventLoop));
		}


		int TcpServer::process()
		{
			workerSocket_t worker = acceptClient();
			if (!worker) {
				return 0;
			}

			if (m_acceptCb) {
				m_acceptCb(std::move(worker));
			}
			return 0;
		}
	}
}
