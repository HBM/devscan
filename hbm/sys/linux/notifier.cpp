// Copyright 2014 Hottinger Baldwin Messtechnik
// Distributed under MIT license
// See file LICENSE provided


#include <cstring>

#include <sys/eventfd.h>
#include <sys/poll.h>
#include <unistd.h>

#include "hbm/exception/exception.hpp"
#include "hbm/sys/notifier.h"
#include "hbm/sys/eventloop.h"

namespace hbm {
	namespace sys {
		Notifier::Notifier(EventLoop& eventLoop, EventHandler_t eventHandler)
			: m_fd(eventfd(0, EFD_NONBLOCK))
			, m_eventLoop(eventLoop)
			, m_eventHandler(eventHandler)
		{
			if (m_fd<0) {
				throw hbm::exception::exception("could not create event fd");
			}
			m_eventLoop.addEvent(m_fd, std::bind(&Notifier::process, this));
		}

		Notifier::Notifier(Notifier&& source)
			: m_fd(source.m_fd)
			, m_eventLoop(source.m_eventLoop)
			, m_eventHandler(source.m_eventHandler)
		{
			m_eventLoop.addEvent(m_fd, std::bind(&Notifier::process, this));
		}

		Notifier::~Notifier()
		{
			m_eventLoop.eraseEvent(m_fd);
			close(m_fd);
		}

		int Notifier::notify()
		{
			static const uint64_t value = 1;
			return write(m_fd, &value, sizeof(value));
		}

		int Notifier::process()
		{
			uint64_t value;
			int readStatus = ::read(m_fd, &value, sizeof(value));
			if(readStatus>0) {
				for (uint64_t i=0; i<value; i++) {
					if (m_eventHandler) {
						m_eventHandler();
					}
				}
			}
			return readStatus;
		}

		int Notifier::read()
		{
			uint64_t value;
			ssize_t readStatus = ::read(m_fd, &value, sizeof(value));
			if (readStatus<0) {
				return 0;
			} else {
				// to be compatible between windows and linux, we return 1 even if timer expired timerEventCount times.
				return 1;
			}

		}
	}
}
