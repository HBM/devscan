// Copyright 2014 Hottinger Baldwin Messtechnik
// Distributed under MIT license
// See file LICENSE provided

#ifndef _NETLINK_H
#define _NETLINK_H

#include "hbm/exception/exception.hpp"
#include "hbm/communication/netadapterlist.h"
#include "hbm/communication/multicastserver.h"
#include "hbm/sys/defines.h"
#include "hbm/sys/eventloop.h"

namespace hbm {
	class Netlink {
	public:
		/// \throws hbm::exception
		Netlink(communication::NetadapterList &netadapterlist, communication::MulticastServer &mcs, sys::EventLoop &eventLoop);
		virtual ~Netlink();

		int start(sys::EventHandler_t eventHandler);

		int stop();

	private:
		ssize_t process() const;

		ssize_t receive(char* pReadBuffer, size_t bufferSize) const;

		/// receive events from netlink. Adapt netadapter list and mulicast server accordingly
		/// \param[in, out] netadapterlist will be adapted when processing netlink events
		/// \param[in, out] mcs will be adapted when processing netlink events
		void processNetlinkTelegram(char *pReadBuffer, size_t bufferSize) const;
		int m_fd;

		communication::NetadapterList &m_netadapterlist;
		communication::MulticastServer &m_mcs;

		sys::EventLoop& m_eventloop;
		sys::EventHandler_t m_eventHandler;
	};
}

#endif
