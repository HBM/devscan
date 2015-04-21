// Copyright 2014 Hottinger Baldwin Messtechnik
// Distributed under MIT license
// See file LICENSE provided

#include <cstring>
#include <stdint.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>

#include <unistd.h>
#include <fcntl.h>
#include <vector>

#include <errno.h>
#include <syslog.h>
#include <poll.h>

#include "hbm/communication/socketnonblocking.h"


/// Maximum time to wait for connecting
const time_t TIMEOUT_CONNECT_S = 5;


hbm::communication::SocketNonblocking::SocketNonblocking(sys::EventLoop &eventLoop)
	: m_fd(-1)
	, m_bufferedReader()
	, m_eventLoop(eventLoop)
	, m_dataHandler()
{
}

hbm::communication::SocketNonblocking::SocketNonblocking(int fd, sys::EventLoop &eventLoop)
	: m_fd(fd)
	, m_bufferedReader()
	, m_eventLoop(eventLoop)
	, m_dataHandler()
{
	if (fcntl(m_fd, F_SETFL, O_NONBLOCK)==-1) {
		throw std::runtime_error("error setting socket to non-blocking");
	}
	if (setSocketOptions()<0) {
		throw std::runtime_error("error setting socket options");
	}
}

hbm::communication::SocketNonblocking::~SocketNonblocking()
{
	disconnect();
}

void hbm::communication::SocketNonblocking::setDataCb(DataCb_t dataCb)
{
	m_dataHandler = dataCb;
	if (dataCb) {
		m_eventLoop.addEvent(m_fd, std::bind(&SocketNonblocking::process, this));
	} else {
		m_eventLoop.eraseEvent(m_fd);
	}
}

int hbm::communication::SocketNonblocking::setSocketOptions()
{
	int opt = 1;

	// turn off Nagle algorithm
	if (setsockopt(m_fd, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<char*>(&opt), sizeof(opt))==-1) {
		syslog(LOG_ERR, "error turning off nagle algorithm");
		return -1;
	}

	opt = 12;
	// the interval between the last data packet sent (simple ACKs are not considered data) and the first keepalive probe;
	// after the connection is marked to need keepalive, this counter is not used any further
	if (setsockopt(m_fd, SOL_TCP, TCP_KEEPIDLE, reinterpret_cast<char*>(&opt), sizeof(opt))==-1) {
		syslog(LOG_ERR, "error setting socket option TCP_KEEPIDLE");
		return -1;
}


	opt = 3;
	// the interval between subsequential keepalive probes, regardless of what the connection has exchanged in the meantime
	if (setsockopt(m_fd, SOL_TCP, TCP_KEEPINTVL, reinterpret_cast<char*>(&opt), sizeof(opt))==-1) {
		syslog(LOG_ERR, "error setting socket option TCP_KEEPINTVL");
		return -1;
	}


	opt = 2;
	// the number of unacknowledged probes to send before considering the connection dead and notifying the application layer
	if (setsockopt(m_fd, SOL_TCP, TCP_KEEPCNT, reinterpret_cast<char*>(&opt), sizeof(opt))==-1) {
		syslog(LOG_ERR, "error setting socket option TCP_KEEPCNT");
		return -1;
	}


	opt = 1;
	if (setsockopt(m_fd, SOL_SOCKET, SO_KEEPALIVE, reinterpret_cast<char*>(&opt), sizeof(opt))==-1) {
		syslog(LOG_ERR, "error setting socket option SO_KEEPALIVE");
		return -1;
	}

	return 0;
}

int hbm::communication::SocketNonblocking::connect(const std::string &address, const std::string& port)
{
	struct addrinfo hints;
	struct addrinfo* pResult = NULL;

	memset(&hints, 0, sizeof(hints));

	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = 6; // Ip V6!

	if( getaddrinfo(address.c_str(), port.c_str(), &hints, &pResult)!=0 ) {
		return -1;
	}
	int retVal = connect(pResult->ai_family, pResult->ai_addr, pResult->ai_addrlen);

	freeaddrinfo( pResult );

	return retVal;
}

int hbm::communication::SocketNonblocking::connect(int domain, const struct sockaddr* pSockAddr, socklen_t len)
{
	m_fd = ::socket(domain, SOCK_STREAM | SOCK_NONBLOCK, 0);
	if (m_fd==-1) {
		return -1;
	}

	if (setSocketOptions()<0) {
		return -1;
	}

	int err = ::connect(m_fd, pSockAddr, len);
	if (err==-1) {
		// success if errno equals EINPROGRESS
		if(errno != EINPROGRESS) {
			syslog(LOG_ERR, "failed to connect errno=%d '%s'", errno, strerror(errno));
			return -1;
		}
		struct pollfd pfd;
		pfd.fd = m_fd;
		pfd.events = POLLOUT;
		do {
			err = poll(&pfd, 1, TIMEOUT_CONNECT_S*1000);
		} while((err==-1) && (errno==EINTR) );
		if(err!=1) {
			return -1;
		}

		int value;
		len = sizeof(value);
		getsockopt(m_fd, SOL_SOCKET, SO_ERROR, &value, &len);
		if(value!=0) {
			return -1;
		}
	}
	return 0;
}

int hbm::communication::SocketNonblocking::process()
{
	if (m_dataHandler) {
		return m_dataHandler(this);
	} else {
		return -1;
	}
}

ssize_t hbm::communication::SocketNonblocking::receive(void* pBlock, size_t size)
{
	return m_bufferedReader.recv(m_fd, pBlock, size, 0);
}

ssize_t hbm::communication::SocketNonblocking::receiveComplete(void* pBlock, size_t size, int msTimeout)
{
	ssize_t retVal;
	unsigned char* pPos = reinterpret_cast < unsigned char* > (pBlock);
	size_t sizeLeft = size;
	while(sizeLeft) {
		retVal = m_bufferedReader.recv(m_fd, pPos, sizeLeft, 0);
		if(retVal>0) {
			sizeLeft -= retVal;
			pPos += retVal;
		} else if(retVal==0) {
			return size-sizeLeft;
		} else {
			if(errno==EWOULDBLOCK || errno==EAGAIN) {
				// wait for socket to become readable.
				struct pollfd pfd;
				pfd.fd = m_fd;
				pfd.events = POLLIN;
				int nfds;
				do {
					nfds = poll(&pfd, 1, msTimeout);
				} while((nfds==-1) && (errno==EINTR));
				if(nfds!=1) {
					return -1;
				}
			} else {
				syslog(LOG_ERR, "%s: recv failed '%s'", __FUNCTION__, strerror(errno));
				return -1;
			}
		}
	}
	return size;
}


ssize_t hbm::communication::SocketNonblocking::sendBlocks(const dataBlocks_t &blocks)
{
	std::vector < iovec > iovs(6); // reserve a reasonable number of entries!

	size_t completeLength = 0;
	iovec newIovec;

	for(dataBlocks_t::const_iterator iter=blocks.begin(); iter!=blocks.end(); ++iter) {
		const dataBlock_t& item = *iter;
		newIovec.iov_base = const_cast < void* > (item.pData);
		newIovec.iov_len = item.size;
		iovs.push_back(newIovec);
		completeLength += item.size;
	}


	ssize_t retVal = writev(m_fd, &iovs[0], iovs.size());
	if (retVal==0) {
		return retVal;
	} else if (retVal==-1) {
		if((errno!=EWOULDBLOCK) && (errno!=EAGAIN) && (errno!=EINTR) ) {
			return retVal;
		}
	}

	size_t bytesWritten = retVal;
	if(bytesWritten==completeLength) {
		// we are done!
		return bytesWritten;
	} else {
		size_t blockSum = 0;

		for(size_t index=0; index<iovs.size(); ++index) {
			blockSum += iovs[index].iov_len;
			if(bytesWritten<blockSum) {
				// this block was not send completely
				size_t bytesRemaining = blockSum - bytesWritten;
				size_t start = iovs[index].iov_len-bytesRemaining;
				retVal = sendBlock(static_cast < unsigned char* > (iovs[index].iov_base)+start, bytesRemaining, false);
				if(retVal>0) {
					bytesWritten += retVal;
				} else {
					return -1;
				}
			}
		}
	}

	return bytesWritten;
}

ssize_t hbm::communication::SocketNonblocking::sendBlock(const void* pBlock, size_t size, bool more)
{
	const uint8_t* pDat = reinterpret_cast<const uint8_t*>(pBlock);
	size_t BytesLeft = size;
	ssize_t numBytes;
	ssize_t retVal = size;

	struct pollfd pfd;
	pfd.fd = m_fd;
	pfd.events = POLLOUT;

	int flags = 0;
	if(more) {
		flags |= MSG_MORE;
	}
	int err;

	while (BytesLeft > 0) {
		numBytes = send(m_fd, pDat, BytesLeft, flags);
		if(numBytes>0) {
			pDat += numBytes;
			BytesLeft -= numBytes;
		} else if(numBytes==0) {
			// connection lost...
			BytesLeft = 0;
			retVal = -1;
		} else {
			// <0
			if(errno==EWOULDBLOCK || errno==EAGAIN) {
				// wait for socket to become writable.
				do {
					err = poll(&pfd, 1, -1);
				} while((err==-1) && (errno==EINTR));
				if(err!=1) {
					BytesLeft = 0;
					retVal = -1;
				}
			} else if (errno!=EINTR) {
				// a real error happened!
				BytesLeft = 0;
				retVal = -1;
			}
		}
	}
	return retVal;
}


bool hbm::communication::SocketNonblocking::checkSockAddr(const struct ::sockaddr* pCheckSockAddr, socklen_t checkSockAddrLen) const
{
	struct sockaddr sockAddr;
	socklen_t addrLen = sizeof(sockaddr_in);

	char checkHost[256] = "";
	char ckeckService[8] = "";

	char host[256] = "";
	char service[8] = "";
	int err = getnameinfo(pCheckSockAddr, checkSockAddrLen, checkHost, sizeof(checkHost), ckeckService, sizeof(ckeckService), NI_NUMERICHOST | NI_NUMERICSERV);
	if (err != 0) {
		syslog(LOG_ERR, "%s: error from getnameinfo", __FUNCTION__);
		return false;
	}

	if (getpeername(m_fd, &sockAddr, &addrLen)!=0) {
		return false;
	}

	if (getnameinfo(&sockAddr, addrLen, host, sizeof(host), service, sizeof(service), NI_NUMERICHOST | NI_NUMERICSERV)!=0) {
		return false;
	}

	if(
		 (strcmp(host, checkHost)==0) &&
		 (strcmp(service, ckeckService)==0)
		 )
	{
		return true;
	}
	return false;
}

void hbm::communication::SocketNonblocking::disconnect()
{
	m_eventLoop.eraseEvent(m_fd);
	shutdown(m_fd, SHUT_RDWR);
	::close(m_fd);
	m_fd = -1;
}

bool hbm::communication::SocketNonblocking::isFirewire() const
{
	bool retVal = false;

	struct ifreq ifr;
	memset(&ifr, 0, sizeof(struct ifreq));

	if ((::ioctl(m_fd, SIOCGIFHWADDR, (caddr_t)&ifr, sizeof(struct ifreq))) >= 0) {
		if (ifr.ifr_hwaddr.sa_family == ARPHRD_IEEE1394) {
			retVal = true;
		}
	}
	return retVal;
}
