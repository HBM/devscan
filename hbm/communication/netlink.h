// Copyright 2014 Hottinger Baldwin Messtechnik
// Distributed under MIT license
// See file LICENSE provided

#ifndef _NETLINK_H
#define _NETLINK_H

#include <functional>

#include "hbm/exception/exception.hpp"
#include "hbm/communication/netadapterlist.h"
#include "hbm/sys/defines.h"
#include "hbm/sys/eventloop.h"

namespace hbm {
	class Netlink {
	public:
		enum event_t {
			/// not supported under Windows
			NEW,
			/// not supported under Windows
			DEL,
			/// happens on initial start.
			/// windows version does not tell what happened it always sends COMPLETE to tell that something has happened.
			COMPLETE
		};

		typedef std::function < void(event_t event, unsigned int adapterIndex, const std::string& ipv4Address) > cb_t;

		/// \throws hbm::exception
		Netlink(communication::NetadapterList &netadapterlist, sys::EventLoop &eventLoop);
		virtual ~Netlink();

		/// on execution of start, the callback function is being called with event = COMPLETE.
		int start(cb_t eventHandler);

		int stop();

	private:
		ssize_t process();

#ifdef _WIN32
		OVERLAPPED m_overlap;
#else
		ssize_t receive(void *pReadBuffer, size_t bufferSize) const;

		/// receive events from netlink. Adapt netadapter list and mulicast server accordingly
		/// \param[in, out] netadapterlist will be adapted when processing netlink events
		/// \param[in, out] mcs will be adapted when processing netlink events
		void processNetlinkTelegram(void *pReadBuffer, size_t bufferSize) const;

		event m_fd;
#endif
		communication::NetadapterList &m_netadapterlist;

		sys::EventLoop& m_eventloop;
		cb_t m_eventHandler;
	};
}
#endif
