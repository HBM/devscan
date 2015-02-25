// Copyright 2014 Hottinger Baldwin Messtechnik
// Distributed under MIT license
// See file LICENSE provided


#include <iostream>
#include <cstring>

#include <WinSock2.h>

#include "hbm/sys/notifier.h"
#include "hbm/sys/eventloop.h"

namespace hbm {
	namespace sys {
		Notifier::Notifier(EventLoop& eventLoop, EventHandler_t eventHandler)
			: m_fd(NULL)
			, m_eventLoop(eventLoop)
			, m_eventHandler(eventHandler)
		{
		}

		Notifier::~Notifier()
		{
			m_eventLoop.eraseEvent(m_fd);
			CloseHandle(m_fd);
		}

		int Notifier::notify()
		{
			if (SetEvent(m_fd)==0) {
				return -1;
			}
			return 0;
		}

		int Notifier::process()
		{
			int result = read();
			if (result > 0) {
				if (m_eventHandler) {
					m_eventHandler();
				}
			}
			return result;
		}

		int Notifier::set(uEventHandler_t eventHandler)
		{
			m_fd = CreateEvent(NULL, false, false, NULL);
			m_eventHandler = eventHandler;
			m_eventLoop.addEvent(m_fd, std::bind(&Notifier::process, this));
		}

		int Notifier::read()
		{
			DWORD result = WaitForSingleObject(m_fd, 0);
			switch (result) {
			case WAIT_OBJECT_0:
				return 1;
				break;
			case WAIT_TIMEOUT:
				return 0;
				break;
			default:
				return -1;
				break;
			}
			return 0;
		}
	}
}
