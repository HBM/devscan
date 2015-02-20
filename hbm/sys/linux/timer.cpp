// Copyright 2014 Hottinger Baldwin Messtechnik
// Distributed under MIT license
// See file LICENSE provided


#include <cstring>
#include <iostream>

#include <sys/timerfd.h>
#include <sys/poll.h>
#include <unistd.h>
#include <string.h> // for memset()


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

		Timer::Timer(Timer&& source)
			: m_fd(source.m_fd)
#ifdef _WIN32
			, m_isRunning(source.m_isRunning)
#endif
		{
			source.m_fd = -1;
#ifdef _WIN32
			source.m_isRunning;
#endif
		}

		Timer::Timer(unsigned int period_ms, bool repeated)
			: m_fd(timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK))
		{
			if (m_fd<0) {
				throw hbm::exception::exception("could not create timer fd");
			}

			set(period_ms, repeated);
		}

		Timer::~Timer()
		{
			close(m_fd);
		}

		int Timer::set(unsigned int period_ms, bool repeated)
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
			if (repeated) {
				timespec.it_interval.tv_sec = period_s;
				timespec.it_interval.tv_nsec = rest * 1000 * 1000;
			}

			return timerfd_settime(m_fd, 0, &timespec, nullptr);
		}

		int Timer::read()
		{
			uint64_t timerEventCount;
			ssize_t readStatus = ::read(m_fd, &timerEventCount, sizeof(timerEventCount));
			if (readStatus<0) {
				return 0;
			} else {
				// to be compatible between windows and linux, we return 1 even if timer expired timerEventCount times.
				return 1;
			}
		}

		int Timer::wait_for(int period_ms)
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

		int Timer::wait()
		{
			return wait_for(-1);
		}

		int Timer::cancel()
		{
			int retval = 0;
			struct itimerspec timespec;

			timerfd_gettime(m_fd, &timespec);
			if ( (timespec.it_value.tv_sec != 0) || (timespec.it_value.tv_nsec != 0) ) {
				// timer is running
				retval = 1;
			}

			memset (&timespec, 0, sizeof(timespec));
			timerfd_settime(m_fd, 0, &timespec, nullptr);

			return retval;
		}

		event Timer::getFd() const
		{
			return m_fd;
		}
	}
}
