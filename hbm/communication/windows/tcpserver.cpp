// Copyright 2014 Hottinger Baldwin Messtechnik
// Distributed under MIT license
// See file LICENSE provided

#ifdef _WIN32
#include <WinSock2.h>
#include <WS2tcpip.h>
#undef max
#undef min
#endif

#include <memory>
#include <cstring>

#include <errno.h>

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
			WSADATA wsaData;
			WSAStartup(2, &wsaData);
			m_event = WSACreateEvent();
		}

		TcpServer::~TcpServer()
		{
			stop();
			WSACloseEvent(m_event);
		}

		int TcpServer::start(uint16_t port, int backlog, Cb_t acceptCb)
		{
			m_listeningSocket = socket(PF_INET, SOCK_STREAM, 0);
			SOCKADDR_IN address;
			memset(&address, 0, sizeof(address));
			address.sin_family = AF_INET;
			address.sin_addr.s_addr = htonl(INADDR_ANY);
			address.sin_port = htons(port);			

			////ipv6 does work for ipv4 too!
			//m_listeningSocket = ::socket(AF_INET6, SOCK_STREAM, 0);
			//sockaddr_in6 address;
			//memset(&address, 0, sizeof(address));
			//address.sin6_family = AF_INET6;
			//address.sin6_addr = in6addr_any;
			//address.sin6_port = htons(port);

			if (::bind(m_listeningSocket, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == -1) {
				printf("%s: Binding socket to port initialization failed '%s'", __FUNCTION__, strerror(errno));
				return -1;
			}

			// switch to non blocking
			u_long value = 1;
			::ioctlsocket(m_listeningSocket, FIONBIO, &value);

			m_acceptCb = acceptCb;
			m_eventLoop.addEvent(m_event, std::bind(&TcpServer::process, this));

			if (WSAEventSelect(m_listeningSocket, m_event, FD_ACCEPT | FD_CLOSE) == -1) {
				return -1;
			}



			if (listen(m_listeningSocket, backlog) == -1) {
				return -1;
			}


			return 0;
		}

		void TcpServer::stop()
		{
			m_eventLoop.eraseEvent(m_event);
			closesocket(m_listeningSocket);
		}

		workerSocket_t TcpServer::acceptClient()
		{
			sockaddr_in SockAddr;
			// the length of the client's address
			socklen_t socketAddressLen = sizeof(SockAddr);
			int clientFd = accept(m_listeningSocket, reinterpret_cast<sockaddr*>(&SockAddr), &socketAddressLen);

			if (clientFd<0) {
				printf_s("%s: Accept failed!", __FUNCTION__);
				return workerSocket_t();
			}


			return workerSocket_t(new SocketNonblocking(clientFd, m_eventLoop));
		}


		int TcpServer::process()
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
