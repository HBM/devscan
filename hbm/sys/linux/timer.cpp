// Copyright 2014 Hottinger Baldwin Messtechnik
// Distributed under MIT license
// See file LICENSE provided


#include <cstring>

#include <sys/timerfd.h>
#include <sys/poll.h>
#include <unistd.h>

#include "hbm/exception/exception.hpp"

#include "hbm/sys/timer.h"

namespace hbm {
	namespace sys {
		Timer::Timer()
			: m_fd(timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK))
		{
			if (m_fd<0) {
				throw hbm::exception::exception("could not create timer fd");
			}
		}

		Timer::Timer(unsigned int period_ms)
			: m_fd(timerfd_create(CLOCK_MONOTONIC, 0))
		{
			if (m_fd<0) {
				throw hbm::exception::exception("could not create timer fd");
			}

			set(period_ms);
		}

		Timer::~Timer()
		{
			close(m_fd);
		}

		int Timer::set(unsigned int period_ms)
		{
			if (period_ms==0) {
				return -1;
			}
			struct itimerspec timespec;
			memset (&timespec, 0, sizeof(timespec));
			unsigned int period_s = period_ms / 1000;
			unsigned int rest = period_ms % 1000;

			timespec.it_value.tv_sec = period_s;
			timespec.it_value.tv_nsec = rest * 1000 * 1000;
			timespec.it_interval.tv_sec = period_s;
			timespec.it_interval.tv_nsec = rest * 1000 * 1000;

			return timerfd_settime(m_fd, 0, &timespec, nullptr);
		}

		int Timer::read()
		{
			struct itimerspec currValue;
			timerfd_gettime(m_fd, &currValue);
			if(currValue.it_value.tv_sec==0 && currValue.it_value.tv_nsec==0) {
				// not started!
				return -1;
			}

			uint64_t timerEventCount;
			if (::read(m_fd, &timerEventCount, sizeof(timerEventCount))<0) {
				// timer was stopped!
				return 0;
			} else {
				// to be compatible between windows and linux, we return 1 even if timer expired timerEventCount times.
				return 1;
			}
		}

		int Timer::wait()
		{
			struct itimerspec currValue;
			timerfd_gettime(m_fd, &currValue);
			if(currValue.it_value.tv_sec==0 && currValue.it_value.tv_nsec==0) {
				// not started!
				return -1;
			}

			struct pollfd pfd;

			pfd.fd = m_fd;
			pfd.events = POLLIN;

			int retval = poll(&pfd, 1, -1);
			if (retval!=1) {
				return -1;
			}
			uint64_t timerEventCount;
			if (::read(m_fd, &timerEventCount, sizeof(timerEventCount))<0) {
				// timer was stopped!
				return 0;
			} else {
				// to be compatible between windows and linux, we return 1 even if timer expired timerEventCount times.
				return 1;
			}
		}

		int Timer::cancel()
		{
			return ::close(m_fd);
		}

		event Timer::getFd() const
		{
			return m_fd;
		}
	}
}
