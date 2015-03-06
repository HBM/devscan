// Copyright 2014 Hottinger Baldwin Messtechnik
// Distributed under MIT license
// See file LICENSE provided

#ifndef __HBM__SOCKETNONBLOCKING_H
#define __HBM__SOCKETNONBLOCKING_H

#include <memory>
#include <string>

#ifdef _WIN32
#include <WinSock2.h>
#include <WS2tcpip.h>
#undef max
#undef min
#else
#include <sys/socket.h>
#endif



#include "hbm/communication/bufferedreader.h"
#include "hbm/sys/eventloop.h"

namespace hbm
{
	namespace communication {
		class SocketNonblocking;
#ifdef _MSC_VER
		typedef std::shared_ptr <SocketNonblocking > workerSocket_t;
#else
		typedef std::unique_ptr <SocketNonblocking > workerSocket_t;
#endif

		/// the socke uses keep-alive in order to detect broken connection.
		class SocketNonblocking
		{
		public:
			/// called on the arrival of data
			typedef std::function < int (SocketNonblocking* pSocket) > DataCb_t;
			SocketNonblocking(sys::EventLoop &eventLoop);

			/// \throw std::runtime_error on error
			SocketNonblocking(int fd, sys::EventLoop &eventLoop);
			virtual ~SocketNonblocking();

			/// \return 0: success; -1: error
			int connect(const std::string& address, const std::string& port);
			int connect(int domain, const struct sockaddr* pSockAddr, socklen_t len);

			/// if setting a callback function, data receiption is done via event loop.
			/// if setting an empty callback function DataCb_t(), the event is taken out of the eventloop.
			void setDataCb(DataCb_t dataCb);

			ssize_t sendBlock(const void* pBlock, size_t len, bool more);

			/// might return with less bytes the requested
			ssize_t receive(void* pBlock, size_t len);

			/// might return with less bytes then requested if connection is being closed before completion
			/// @param @msTimeout -1 for infinite
			ssize_t receiveComplete(void* pBlock, size_t len, int msTimeout = -1);

			bool isFirewire() const;

			bool checkSockAddr(const struct sockaddr* pCheckSockAddr, socklen_t checkSockAddrLen) const;

			void disconnect();

		protected:
			/// should not be copied
			SocketNonblocking(const SocketNonblocking& op);

			/// should not be assigned
			SocketNonblocking& operator= (const SocketNonblocking& op);

			int setSocketOptions();

			/// called by eventloop
			int process();

			int m_fd;
			#ifdef _WIN32
			WSAEVENT m_event;
			#endif

			BufferedReader m_bufferedReader;

			sys::EventLoop& m_eventLoop;
			DataCb_t m_dataHandler;
		};
	}
}
#endif
