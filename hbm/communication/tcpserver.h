// Copyright 2014 Hottinger Baldwin Messtechnik
// Distributed under MIT license
// See file LICENSE provided

#ifndef __HBM__COMMUNICATION_TCPACCEPTOR_H
#define __HBM__COMMUNICATION_TCPACCEPTOR_H

#include <memory>
#include <string>

#ifdef _WIN32
#include <WinSock2.h>
#include <WS2tcpip.h>
#undef max
#undef min
#endif



#include "hbm/communication/socketnonblocking.h"
#include "hbm/sys/eventloop.h"

namespace hbm {
	namespace communication {
		class TcpServer {
		public:
			/// deliveres the worker socket for an accepted client
			typedef std::function < void (workerSocket_t) > Cb_t;

			TcpServer(sys::EventLoop &eventLoop);
			virtual ~TcpServer();

			/// @param numPorts Maximum length of the queue of pending connections
			/// \param acceptCb called when accepting a new tcp client
			int start(uint16_t port, int backlog, Cb_t acceptCb);

			void stop();

		private:

			/// should not be copied
			TcpServer(const TcpServer& op);

			/// should not be assigned
			TcpServer& operator= (const TcpServer& op);

			/// \return 0 on success; -1 on error
			int init(int domain);

			/// Listens to connecting clients, a server call
			/// @param numPorts Maximum length of the queue of pending connections
			int listenToClient(int numPorts);

			/// called by eventloop
			/// accepts a new connection creates new worker socket anf calls acceptCb
			int process();

			/// accepts a new connecting client.
			/// \return On success, the worker socket for the new connected client is returned. Empty worker socket on error
			workerSocket_t acceptClient();

			int m_listeningSocket;
#ifdef _WIN32
			WSAEVENT m_event;
#endif
			sys::EventLoop& m_eventLoop;
			Cb_t m_acceptCb;
		};
	}
}
#endif
