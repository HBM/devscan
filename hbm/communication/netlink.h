// Copyright 2014 Hottinger Baldwin Messtechnik
// Distributed under MIT license
// See file LICENSE provided

#ifndef _NETLINK_H
#define _NETLINK_H

#include "hbm/exception/exception.hpp"
#include "hbm/communication/netadapterlist.h"
#include "hbm/communication/multicastserver.h"

namespace hbm {
	class Netlink {
	public:
		/// \throws hbm::exception
		Netlink();
		virtual ~Netlink();

		/// receive events from netlink. Adapt netadapter list and mulicast server accordingly
		ssize_t receiveAndProcess(communication::NetadapterList &netadapterlist, communication::MulticastServer &mcs) const;

		/// to poll
		int getFd() const
		{
			return m_fd;
		}

		int stop();

	private:
		ssize_t receive(char* pReadBuffer, size_t bufferSize) const;
		/// \param[in, out] netadapterlist will be adapted when processing netlink events
		/// \param[in, out] mcs will be adapted when processing netlink events
		void process(char *pReadBuffer, size_t bufferSize, communication::NetadapterList &netadapterlist, communication::MulticastServer &mcs) const;
		int m_fd;
	};
}

#endif
