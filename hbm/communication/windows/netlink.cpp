// Copyright 2014 Hottinger Baldwin Messtechnik
// Distributed under MIT license
// See file LICENSE provided


#include <WinSock2.h>
#include <IPHlpApi.h>

#include <cstring>

#include "hbm/communication/netlink.h"
#include "hbm/exception/exception.hpp"


namespace hbm {
	Netlink::Netlink(communication::NetadapterList &netadapterlist, sys::EventLoop &eventLoop)
		: m_netadapterlist(netadapterlist)
		, m_eventloop(eventLoop)
	{
		HANDLE handle = NULL;
		m_overlap.hEvent = WSACreateEvent();
		DWORD ret = NotifyAddrChange(&handle, &m_overlap);
		if (ret!=NO_ERROR) {
			int error = WSAGetLastError();
			if (error != WSA_IO_PENDING) {
				throw hbm::exception::exception("NotifyAddrChange failed: " + WSAGetLastError());
			}
		}
	}

	Netlink::~Netlink()
	{
		stop();
	}

	ssize_t Netlink::process()
	{
		m_netadapterlist.update();
		if (m_eventHandler) {
			m_eventHandler(COMPLETE, 0, "");
		}

		HANDLE handle = NULL;
		DWORD ret = NotifyAddrChange(&handle, &m_overlap);
		if (ret != NO_ERROR) {
			int error = WSAGetLastError();
			if (error != WSA_IO_PENDING) {
				throw hbm::exception::exception("NotifyAddrChange failed: Error " + WSAGetLastError());
			}
		}
		return 0;
	}

	int Netlink::start(cb_t eventHandler)
	{
		m_eventHandler = eventHandler;
		if (m_eventHandler) {
			m_eventHandler(COMPLETE, 0, "");
		}
		m_eventloop.addEvent(m_overlap.hEvent, std::bind(&Netlink::process, this));
		return 0;
	}

	int Netlink::stop()
	{
		m_eventloop.eraseEvent(m_overlap.hEvent);
		return CloseHandle(m_overlap.hEvent);
	}
}
