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
		Notifier::Notifier(EventLoop& eventLoop)
			: m_fd(eventfd(0, EFD_NONBLOCK))
			, m_eventLoop(eventLoop)
			, m_eventHandler()
		{
			if (m_fd<0) {
				throw hbm::exception::exception("could not create event fd");
			}
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

		int Notifier::set(Cb_t eventHandler)
		{
			m_eventHandler = eventHandler;
			if (eventHandler) {
				m_eventLoop.addEvent(m_fd, std::bind(&Notifier::process, this));
			} else {
				m_eventLoop.eraseEvent(m_fd);
			}
			return 0;
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
	}
}
