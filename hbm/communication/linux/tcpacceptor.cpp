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
#include "hbm/communication/tcpacceptor.h"
#include "hbm/sys/eventloop.h"


namespace hbm {
	namespace communication {
		TcpAcceptor::TcpAcceptor(sys::EventLoop &eventLoop, Cb_t acceptCb, SocketNonblocking::DataCb_t workerDataHandler)
			: m_listeningSocket(-1)
			, m_eventLoop(eventLoop)
			, m_acceptCb(acceptCb)
			, m_workerDataHandler(workerDataHandler)
		{
		}

		TcpAcceptor::~TcpAcceptor()
		{
			stop();
		}

		int TcpAcceptor::start(uint16_t port, int backlog)
		{
			int result = bind(port);
			if (result<0) {
				return result;
			}
			result = listen(m_listeningSocket, backlog);
			if (result<0) {
				return result;
			}
			m_eventLoop.addEvent(m_listeningSocket, std::bind(&TcpAcceptor::process, this));
		}

		void TcpAcceptor::stop()
		{
			m_eventLoop.eraseEvent(m_listeningSocket);
			close(m_listeningSocket);
		}

		int TcpAcceptor::init(int domain)
		{
			m_listeningSocket = ::socket(domain, SOCK_STREAM | SOCK_NONBLOCK, 0);
			if (m_listeningSocket==-1) {
				return -1;
			}
		}

		int TcpAcceptor::bind(uint16_t Port)
		{
			//ipv6 does work for ipv4 too!
			sockaddr_in6 address;

			memset(&address, 0, sizeof(address));
			address.sin6_family = AF_INET6;
			address.sin6_addr = in6addr_any;
			address.sin6_port = htons(Port);

			m_listeningSocket = ::socket(address.sin6_family, SOCK_STREAM | SOCK_NONBLOCK, 0);
			if (m_listeningSocket==-1) {
				syslog(LOG_ERR, "%s: Socket initialization failed '%s'", __FUNCTION__ , strerror(errno));
				return -1;
			}
			if (::bind(m_listeningSocket, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == -1) {
				syslog(LOG_ERR, "%s: Binding socket to port initialization failed '%s'", __FUNCTION__ , strerror(errno));
				return -1;
			}
			m_eventLoop.addEvent(m_listeningSocket, std::bind(&TcpAcceptor::process, this));

			return 0;
		}

		TcpAcceptor::workerSocket_t TcpAcceptor::acceptClient()
		{
			sockaddr_in SockAddr;
			// the length of the client's address
			socklen_t socketAddressLen = sizeof(SockAddr);
			int clientFd = accept(m_listeningSocket, reinterpret_cast<sockaddr*>(&SockAddr), &socketAddressLen);

			if (clientFd<0) {
				syslog(LOG_ERR, "%s: Accept failed!", __FUNCTION__);
				return workerSocket_t();
			}


			return workerSocket_t(new SocketNonblocking(clientFd, m_eventLoop, m_workerDataHandler));
		}


		int TcpAcceptor::process()
		{
			workerSocket_t worker = acceptClient();
			if (!worker) {
				return -1;
			}

			if (m_acceptCb) {
				m_acceptCb(std::move(worker));
			}
			return 0;
		}
	}
}
