// Copyright 2014 Hottinger Baldwin Messtechnik
// Distributed under MIT license
// See file LICENSE provided

#include <stdint.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <MSTcpIP.h>
#undef max
#undef min

#define __time_t long
#define ssize_t int
#define snprintf sprintf_s



#include "hbm/communication/socketnonblocking.h"


/// Maximum time to wait for connecting
const time_t TIMEOUT_CONNECT_S = 5;


hbm::communication::SocketNonblocking::SocketNonblocking()
	: m_fd(-1)
	, m_bufferedReader()
{
	WSADATA wsaData;
	WSAStartup(2, &wsaData);
	m_event = WSACreateEvent();
}

hbm::communication::SocketNonblocking::SocketNonblocking(int fd)
	: m_fd(fd)
	, m_bufferedReader()
{
	WSADATA wsaData;
	WSAStartup(2, &wsaData);
	m_event = WSACreateEvent();
}

hbm::communication::SocketNonblocking::~SocketNonblocking()
{
	stop();
	WSACloseEvent(m_event);
}

int hbm::communication::SocketNonblocking::setSocketOptions()
{
	bool opt = true;

	// switch to non blocking
	u_long value = 1;
	::ioctlsocket(m_fd, FIONBIO, &value);

	// turn off nagle algorithm
	setsockopt(m_fd, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<char*>(&opt), sizeof(opt));


	// configure keep alive
	DWORD len;
	tcp_keepalive ka;
	ka.keepaliveinterval = 1000; // probe interval in ms
	ka.keepalivetime = 1000; // time of inactivity until first keep alive probe is being send in ms
	// from MSDN: on windows vista and later, the number of probes is set to 10 and can not be changed
	// time until recognition: keepaliveinterval + (keepalivetime*number of probes)
	ka.onoff = 1;
	WSAIoctl(m_fd, SIO_KEEPALIVE_VALS, &ka, sizeof(ka), NULL, 0, &len, NULL, NULL);

	return 0;
}

int hbm::communication::SocketNonblocking::init(int domain)
{
	int retVal = 0;

	m_fd = static_cast < int > (::socket(domain, SOCK_STREAM, 0));
	if(m_fd==-1) {
		retVal=-1;
	} else {
		setSocketOptions();		
	}

	WSAEventSelect(m_fd, m_event, FD_READ);

	return retVal;
}

int hbm::communication::SocketNonblocking::connect(const std::string &address, const std::string& port)
{
	struct addrinfo hints;
	struct addrinfo* pResult = NULL;

	memset(&hints, 0, sizeof(hints));

	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;


	if( getaddrinfo(address.c_str(), port.c_str(), &hints, &pResult)!=0 ) {
		return -1;
	}
	
	int retVal = connect(pResult->ai_family, pResult->ai_addr, sizeof(sockaddr_in));

	freeaddrinfo( pResult );

	return retVal;
}

int hbm::communication::SocketNonblocking::connect(int domain, const struct sockaddr* pSockAddr, socklen_t len)
{
	int err = init(domain);

	if(err<0) {
		return err;
	}

	err = ::connect(m_fd,pSockAddr, len);
	// success if WSAGetLastError returns WSAEWOULDBLOCK
	if((err == SOCKET_ERROR) && (WSAGetLastError() == WSAEWOULDBLOCK))
	{

		fd_set fdWrite;

		struct timeval timeout;

		timeout.tv_sec = TIMEOUT_CONNECT_S;
		timeout.tv_usec = 0;


		FD_ZERO(&fdWrite);
		FD_SET(m_fd,&fdWrite);

		err = select(m_fd + 1, NULL, &fdWrite, NULL, &timeout);
		if(err==1) {
			int value;
			socklen_t len = sizeof(value);
			getsockopt(m_fd, SOL_SOCKET, SO_ERROR, reinterpret_cast < char* > (&value), &len);
			if(value!=0) {
				err = -1;
			}
		} else {
			err = -1;
		}
	} else {
		err = -1;
	}

	return err;
}


event hbm::communication::SocketNonblocking::getFd() const
{
	return m_event;
}

int hbm::communication::SocketNonblocking::bind(uint16_t Port)
{
	//ipv6 does work for ipv4 too!
	sockaddr_in6 address;

	memset(&address, 0, sizeof(address));
	address.sin6_family = AF_INET;
	address.sin6_addr = in6addr_any;
	address.sin6_port = htons(Port);

	int retVal = init(address.sin6_family);
	if (retVal == -1) {
		return retVal;
	}
	retVal = ::bind(m_fd, reinterpret_cast<sockaddr*>(&address), sizeof(address));
	return retVal;
}

std::unique_ptr < hbm::communication::SocketNonblocking > hbm::communication::SocketNonblocking::acceptClient()
{
	std::unique_ptr < SocketNonblocking > retSocket;

	sockaddr_in SockAddr;
	// the length of the client's address
	socklen_t socketAddressLen = sizeof(SockAddr);

	int err;

	fd_set fds;

	FD_ZERO(&fds);
	FD_SET(m_fd,&fds);

	// wir warten ohne timeout, bis etwas zu lesen ist
	do {
		err = select(static_cast < int >(m_fd) + 1, &fds, NULL, NULL, NULL);
	} while((err==-1)&&(errno==WSAEINTR));

	if(err!=1) {
		return std::unique_ptr < SocketNonblocking >();		
	} else if(FD_ISSET(m_fd, &fds)) {
		// the new socket file descriptor returned by the accept system call
		SOCKET clientFd;

		clientFd = accept(m_fd, reinterpret_cast<sockaddr*>(&SockAddr), &socketAddressLen);
		if (clientFd >= 0) {
			std::unique_ptr < SocketNonblocking > p( new SocketNonblocking(static_cast < int > (clientFd)));
			p->setSocketOptions();
			return p;
		}
	}
	return std::unique_ptr < SocketNonblocking >();
}

int hbm::communication::SocketNonblocking::listenToClient(int numPorts)
{
	return listen(m_fd, numPorts);
}

ssize_t hbm::communication::SocketNonblocking::receive(void* pBlock, size_t size)
{
  return m_bufferedReader.recv(m_fd, pBlock, size, 0);
}

ssize_t hbm::communication::SocketNonblocking::receiveComplete(void* pBlock, size_t len, int msTimeout)
{
	size_t DataToGet = len;
	unsigned char* pDat = static_cast<unsigned char*>(pBlock);
	ssize_t numBytes = 0;


	struct timeval timeVal;
	struct timeval* pTimeVal;
  fd_set recvFds;

	FD_ZERO(&recvFds);
  FD_SET(m_fd,&recvFds);
  int err;

	if(msTimeout>=0) {
		timeVal.tv_sec = msTimeout / 1000;
		int rest = msTimeout % 1000;
		timeVal.tv_usec = rest*1000;
		pTimeVal = &timeVal;
	} else {
		pTimeVal = NULL;
	}



  while (DataToGet > 0) {
    numBytes = m_bufferedReader.recv(m_fd, reinterpret_cast<char*>(pDat), static_cast < int >(DataToGet), 0);
    if(numBytes>0) {
      pDat += numBytes;
      DataToGet -= numBytes;
    } else if(numBytes==0) {
      // the peer has performed an orderly shutdown!
      DataToGet = 0;
      return -1;
    } else {
      if(WSAGetLastError()==WSAEWOULDBLOCK) {
	// wait until there is something to read or something bad happened
	do {
	  err = select(static_cast < int >(m_fd) + 1, &recvFds, NULL, NULL, pTimeVal);
	} while((err==-1) && (WSAGetLastError()!=WSAEINTR));
	if(err!=1) {
	  return -1;
	}
      } else {
	return -1;
      }
    }
  }
  return static_cast < ssize_t > (len);
}


ssize_t hbm::communication::SocketNonblocking::sendBlock(const void* pBlock, size_t size, bool more)
{
	const uint8_t* pDat = reinterpret_cast<const uint8_t*>(pBlock);
	size_t BytesLeft = size;
	int numBytes;
	ssize_t retVal = static_cast < ssize_t > (size);

	fd_set recvFds;

	FD_ZERO(&recvFds);
	FD_SET(m_fd,&recvFds);
	int err;

	while (BytesLeft > 0) {
		// wir warten ohne timeout, bis wir schreiben koennen
		err = select(static_cast < int >(m_fd) + 1, NULL, &recvFds, NULL, NULL);

		if(err==1) {
			if(FD_ISSET(m_fd, &recvFds)) {
				numBytes = send(m_fd, reinterpret_cast < const char* >(pDat), static_cast < int >(BytesLeft), 0);
				if (numBytes > 0) {
					pDat += numBytes;
					BytesLeft -= numBytes;
				} else if(numBytes==0){
					BytesLeft = 0;
					retVal = -1;
				} else {
					// -1: error
					// ignore "would block"
					if(WSAGetLastError()!=WSAEWOULDBLOCK) {
						BytesLeft = 0;
						retVal = -1;
					}
				}
			}
		} else if(err==-1) {
			// ignore EINTR
			if( WSAGetLastError()!=WSAEINTR) {
				retVal = -1;
				break;
			}
		} else {
			// 0: timeout, should not happen here...
			retVal = -1;
			break;
		}
	}
	return retVal;
}


bool hbm::communication::SocketNonblocking::checkSockAddr(const struct ::sockaddr* pCheckSockAddr, socklen_t checkSockAddrLen) const
{
	struct sockaddr sockAddr;
	socklen_t addrLen = sizeof(sockaddr_in);

	char checkHost[256];
	char ckeckService[8];

	char host[256];
	char service[8];
	int err = getnameinfo(pCheckSockAddr, checkSockAddrLen, checkHost, sizeof(checkHost), ckeckService, sizeof(ckeckService), NI_NUMERICHOST | NI_NUMERICSERV);
	if (err != 0) {
		return false;
	}

	getpeername(m_fd, &sockAddr, &addrLen);

	getnameinfo(&sockAddr, addrLen, host, sizeof(host), service, sizeof(service), NI_NUMERICHOST | NI_NUMERICSERV);

	if(
		(strcmp(host, checkHost)==0) &&
		(strcmp(service, ckeckService)==0)
		)
	{
		return true;
	}
	return false;
}


void hbm::communication::SocketNonblocking::stop()
{
	::shutdown(m_fd, SD_BOTH);
	::closesocket(m_fd);
	m_fd = -1;
}

bool hbm::communication::SocketNonblocking::isFirewire() const
{
	return false;
}

