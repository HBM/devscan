// Copyright 2014 Hottinger Baldwin Messtechnik
// Distributed under MIT license
// See file LICENSE provided

#ifndef __HBM__TCPACCEPTOR_H
#define __HBM__TCPACCEPTOR_H

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
		class TcpAcceptor {
		public:
			typedef std::function < void (std::unique_ptr < SocketNonblocking >) > Cb_t;
			typedef std::unique_ptr <SocketNonblocking > worker_t;

			TcpAcceptor(sys::EventLoop &eventLoop, Cb_t acceptCb, SocketNonblocking::DataHandler_t workerDataHandler);
			virtual ~TcpAcceptor();

			int start(uint16_t port, int backlog);

			void stop();

		private:

			/// should not be copied
			TcpAcceptor(const TcpAcceptor& op);

			/// should not be assigned
			TcpAcceptor& operator= (const TcpAcceptor& op);

			/// \return 0 on success; -1 on error
			int init(int domain);

			/// for server side:bind socket to a port
			int bind(uint16_t Port);

			/// Listens to connecting clients, a server call
			/// @param numPorts Maximum length of the queue of pending connections.
			int listenToClient(int numPorts);

			/// called by eventloop
			int process();

			/// accepts a new connecting client.
			/// \return On success, the worker socket for the new connected client is returned. -1 on error.
			worker_t acceptClient();

			int m_listeningSocket;
			sys::EventLoop& m_eventLoop;
			Cb_t m_acceptCb;
			SocketNonblocking::DataHandler_t m_workerDataHandler;
		};
	}
}
#endif
