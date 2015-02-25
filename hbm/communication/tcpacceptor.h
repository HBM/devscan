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
			/// deliveres the worker socket for an accepted client
			typedef std::unique_ptr <SocketNonblocking > workerSocket_t;
			typedef std::function < void (workerSocket_t) > Cb_t;

			TcpAcceptor(sys::EventLoop &eventLoop);
			virtual ~TcpAcceptor();

			/// @param numPorts Maximum length of the queue of pending connections
			/// \param acceptCb called when accepting a new tcp client
			/// \param workerDataHandler provided to the worker socket that is created for the connecting client
			int start(uint16_t port, int backlog, Cb_t acceptCb, SocketNonblocking::DataCb_t workerDataHandler);

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
			/// @param numPorts Maximum length of the queue of pending connections
			int listenToClient(int numPorts);

			/// called by eventloop
			/// accepts a new connection creates new worker socket anf calls acceptCb
			int process();

			/// accepts a new connecting client.
			/// \return On success, the worker socket for the new connected client is returned. -1 on error.
			workerSocket_t acceptClient();

			int m_listeningSocket;
			sys::EventLoop& m_eventLoop;
			Cb_t m_acceptCb;
			SocketNonblocking::DataCb_t m_workerDataHandler;
		};
	}
}
#endif
