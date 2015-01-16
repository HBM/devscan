// Copyright 2014 Hottinger Baldwin Messtechnik
// Distributed under MIT license
// See file LICENSE provided


#include <cstring>

#include <sys/eventfd.h>
#include <unistd.h>

#include "hbm/exception/exception.hpp"
#include "hbm/sys/notifier.h"

namespace hbm {
	namespace sys {
		Notifier::Notifier()
			: m_fd(eventfd(0, EFD_SEMAPHORE))
		{
			if (m_fd<0) {
				throw hbm::exception::exception("could not create event fd");
			}
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

		int Notifier::wait()
		{
			uint64_t value;
			return read(m_fd, &value, sizeof(value));
		}

		int Notifier::cancel()
		{
			return ::close(m_fd);
		}

		event Notifier::getFd() const
		{
			return m_fd;
		}
	}
}
