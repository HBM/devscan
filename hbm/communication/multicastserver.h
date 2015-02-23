// Copyright 2014 Hottinger Baldwin Messtechnik
// Distributed under MIT license
// See file LICENSE provided

#ifndef _MULTICASTSERVER_H
#define _MULTICASTSERVER_H


#include <string>
#include <chrono>



#ifdef _WIN32
#include <WinSock2.h>
#undef max
#undef min
#else
#include <arpa/inet.h>
#endif

#include "netadapter.h"
#include "netadapterlist.h"

#include "hbm/sys/defines.h"
#include "hbm/sys/eventloop.h"

#ifdef _WIN32
#ifndef ssize_t
typedef int ssize_t;
#endif
#else
#ifndef SOCKET
typedef int SOCKET;
#endif
#endif

namespace hbm {
	namespace communication {
		/// The maximum datagram size supported by UDP
		const unsigned int MAX_DATAGRAM_SIZE = 65536;

		class Netadapter;

		/// for receiving/sending UDP packets from/to multicast groups
		class MulticastServer
		{
		public:
			typedef std::function < int (MulticastServer* mcs) > DataHandler_t;

			/// @param address the multicast group
			MulticastServer(const std::string& address, unsigned int port, const NetadapterList& netadapterList, sys::EventLoop &eventLoop, DataHandler_t dataHandler);

			virtual ~MulticastServer();

			int addInterface(const std::string& interfaceAddress);

			/// all interfaces known to the internal netadapter list are added as receiving interfaces.
			void addAllInterfaces();

			int dropInterface(const std::string& interfaceAddress);

			/// all interfaces known to the internal netadapter list are dropped as receiving interfaces.
			void dropAllInterfaces();

			int start();

			void stop();

			/// Send over all interfaces
			int send(const std::string& data, unsigned int ttl=1) const;

			int send(const void *pData, size_t length, unsigned int ttl=1) const;

			/// send over specific interface
			int sendOverInterface(const Netadapter& adapter, const std::string& data, unsigned int ttl=1) const;
			int sendOverInterface(const Netadapter &adapter, const void* pData, size_t length, unsigned int ttl=1) const;

			int sendOverInterface(int interfaceIndex, const std::string& data, unsigned int ttl=1) const;
			int sendOverInterface(int interfaceIndex, const void* pData, size_t length, unsigned int ttl=1) const;


			/// send over specific interface.
			/// @param interfaceIp IP address of the interface to use
			int sendOverInterfaceByAddress(const std::string& interfaceIp, const std::string& data, unsigned int ttl=1) const;
			int sendOverInterfaceByAddress(const std::string& interfaceIp, const void* pData, size_t length, unsigned int ttl=1) const;

//			ssize_t receiveTelegramBlocking(void* msgbuf, size_t len, int& adapterIndex);

//			/// @param[in,out] waitTime maximum time to wait.
//			ssize_t receiveTelegramBlocking(void* msgbuf, size_t len, int& adapterIndex, std::chrono::milliseconds timeout);

			ssize_t receiveTelegram(void* msgbuf, size_t len, Netadapter& adapter, int &ttl);

			/// @param[out] ttl ttl in the ip header (the value set by the last sender(router))
			ssize_t receiveTelegram(void* msgbuf, size_t len, int& adapterIndex, int &ttl);
		private:

			MulticastServer(const MulticastServer&);
			MulticastServer& operator=(const MulticastServer&);

			int setupSendSocket();

			/// setup the receiver socket and the address structure.
			int setupReceiveSocket();

			int dropOrAddInterface(const std::string& interfaceAddress, bool add);

			/// called by eventloop
			int process();

			/// The All Hosts multicast group addresses all hosts on the same network segment.
			const std::string m_address;

			const unsigned int m_port;

			static const SOCKET NO_SOCKET = static_cast<SOCKET>(-1);

			SOCKET m_ReceiveSocket;
			SOCKET m_SendSocket;
	#ifdef _WIN32
			WSAEVENT m_event;
	#endif

			struct sockaddr_in m_receiveAddr;

			const NetadapterList& m_netadapterList;

			uint8_t m_recvBuffer[65536];
			sys::EventLoop& m_eventLoop;
			DataHandler_t m_dataHandler;
		};
	}
}
#endif
