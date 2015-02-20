// Copyright 2014 Hottinger Baldwin Messtechnik
// Distributed under MIT license
// See file LICENSE provided


#include <cstring>

#include <sys/eventfd.h>
#include <sys/poll.h>
#include <unistd.h>

#include "hbm/exception/exception.hpp"
#include "hbm/sys/notifier.h"

namespace hbm {
	namespace sys {
		Notifier::Notifier()
			: m_fd(eventfd(0, EFD_NONBLOCK))
		{
			if (m_fd<0) {
				throw hbm::exception::exception("could not create event fd");
			}
		}

		Notifier::Notifier(Notifier&& source)
			: m_fd(source.m_fd)
		{
			source.m_fd = -1;
		}

		Notifier::~Notifier()
		{
			close(m_fd);
		}

		int Notifier::notify()
		{
			static const uint64_t value = 1;
			return write(m_fd, &value, sizeof(value));
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

		int Notifier::wait()
		{
			return wait_for(-1);
		}

		int Notifier::wait_for(int period_ms)
		{
			struct pollfd pfd;

			pfd.fd = m_fd;
			pfd.events = POLLIN;

			int retval = poll(&pfd, 1, period_ms);
			if (retval!=1) {
				return -1;
			}
			return read();
		}

		event Notifier::getFd() const
		{
			return m_fd;
		}
	}
}
