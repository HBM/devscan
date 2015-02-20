// Copyright 2014 Hottinger Baldwin Messtechnik
// Distributed under MIT license
// See file LICENSE provided

#ifndef __HBM__SOCKETNONBLOCKING_H
#define __HBM__SOCKETNONBLOCKING_H

#include <string>
#include <memory>

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
		/// the socke uses keep-alive in order to detect broken connection.
		class SocketNonblocking
		{
		public:
			SocketNonblocking();
			virtual ~SocketNonblocking();

			/// \return 0: success; -1: error
			int connect(const std::string& address, const std::string& port);
			int connect(int domain, const struct sockaddr* pSockAddr, socklen_t len);

			/// for server side:bind socket to a port
			int bind(uint16_t Port);

			/// Listens to connecting clients, a server call
			/// @param numPorts   Maximum length of the queue of pending connections.
			int listenToClient(int numPorts = 5);

			/// accepts a new connecting client.
			/// \return On success, the worker socket for the new connected client is returned. NULL on error.
			std::unique_ptr < SocketNonblocking > acceptClient();

			ssize_t sendBlock(const void* pBlock, size_t len, bool more);

			/// might return with less bytes the requested
			ssize_t receive(void* pBlock, size_t len);

			/// might return with less bytes then requested if connection is being closed before completion
			/// @param @msTimeout -1 for infinite
			ssize_t receiveComplete(void* pBlock, size_t len, int msTimeout = -1);

			event getFd() const;

			bool isFirewire() const;

			bool checkSockAddr(const struct sockaddr* pCheckSockAddr, socklen_t checkSockAddrLen) const;

			void stop();

		protected:
			SocketNonblocking(int fd);

			/// should not be copied
			SocketNonblocking(const SocketNonblocking& op);

			/// should not be assigned
			SocketNonblocking& operator= (const SocketNonblocking& op);

			/// \return 0 on success; -1 on error
			int init(int domain);
			int setSocketOptions();

			int m_fd;
			#ifdef _WIN32
					WSAEVENT m_event;
			#endif

			BufferedReader m_bufferedReader;
		};
	}
}
#endif
