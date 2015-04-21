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
		Timer::Timer(EventLoop &eventLoop)
			: m_fd(timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK))
			, m_eventLoop(eventLoop)
			, m_eventHandler()
		{
			if (m_fd<0) {
				throw hbm::exception::exception("could not create timer fd");
			}
			m_eventLoop.addEvent(m_fd, std::bind(&Timer::process, this));
		}

		Timer::Timer(Timer&& source)
			: m_fd(source.m_fd)
			, m_eventLoop(source.m_eventLoop)
			, m_eventHandler(source.m_eventHandler)
#ifdef _WIN32
			, m_isRunning(source.m_isRunning)
#endif
		{
			m_eventLoop.addEvent(m_fd, std::bind(&Timer::process, this));
		}

		Timer::~Timer()
		{
			m_eventLoop.eraseEvent(m_fd);
			cancel();
			close(m_fd);
		}

		int Timer::set(std::chrono::milliseconds period, bool repeated, Cb_t eventHandler)
		{
			return set(period.count(), repeated, eventHandler);
		}


		int Timer::set(unsigned int period_ms, bool repeated, Cb_t eventHandler)
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
			m_eventHandler = eventHandler;
			return timerfd_settime(m_fd, 0, &timespec, nullptr);
		}

		int Timer::cancel()
		{
			int retval = 0;
			struct itimerspec timespec;

			// Before calling callback function with fired=false, we need to clear the callback routine. Otherwise a recursive call might happen
			Cb_t originalEventHandler = m_eventHandler;
			m_eventHandler = Cb_t();

			timerfd_gettime(m_fd, &timespec);
			if ( (timespec.it_value.tv_sec != 0) || (timespec.it_value.tv_nsec != 0) ) {
				// timer is running
				if (originalEventHandler) {
					originalEventHandler(false);
				}
				retval = 1;
			}

			memset (&timespec, 0, sizeof(timespec));
			timerfd_settime(m_fd, 0, &timespec, nullptr);

			return retval;
		}

		int Timer::process()
		{
			uint64_t timerEventCount;
			ssize_t readStatus = ::read(m_fd, &timerEventCount, sizeof(timerEventCount));
			if (readStatus>0) {
				if (m_eventHandler) {
					m_eventHandler(true);
				}
			}
			return readStatus;
		}
	}
}
