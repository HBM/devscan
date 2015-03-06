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
		Notifier::Notifier(EventLoop& eventLoop)
			: m_fd(CreateEvent(NULL, false, false, NULL))
			, m_eventLoop(eventLoop)
			, m_eventHandler()
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
			if (m_eventHandler) {
				m_eventHandler();
			}
			return 0;
		}

		int Notifier::set(Cb_t eventHandler)
		{
			m_eventHandler = eventHandler;
			m_eventLoop.addEvent(m_fd, std::bind(&Notifier::process, this));
			return 0;
		}
	}
}
