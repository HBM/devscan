// Copyright 2014 Hottinger Baldwin Messtechnik
// Distributed under MIT license
// See file LICENSE provided


#include <iostream>
#include <WinSock2.h>

#include <cstring>
#include <stdint.h>


#include "hbm/sys/timer.h"

namespace hbm {
	namespace sys {
		Timer::Timer(EventLoop& eventLoop)
			: m_fd(CreateWaitableTimer(NULL, FALSE, NULL))
			, m_eventLoop(eventLoop)
			, m_eventHandler()
			, m_isRunning(false)
		{
			cancel();
		}

		Timer::~Timer()
		{
			m_eventLoop.eraseEvent(m_fd);
			CloseHandle(m_fd);
			m_fd = INVALID_HANDLE_VALUE;
		}

		int Timer::set(std::chrono::milliseconds period, bool repeated, Cb_t eventHandler)
		{
			return set(static_cast < unsigned int >(period.count()), repeated, eventHandler);
		}

		int Timer::set(unsigned int period_ms, bool repeated, Cb_t eventHandler)
		{
			LARGE_INTEGER dueTime;
			static const int64_t multiplier = -10000; // negative because we want a relative time
			LONG period = 0; // in ms

			m_eventHandler = eventHandler;

			m_eventLoop.addEvent(m_fd, std::bind(&Timer::process, this));

			if (repeated) {
				period = period_ms;
			}
			dueTime.QuadPart = period_ms*multiplier; // in 100ns
			BOOL Result = SetWaitableTimer(
				m_fd,
				&dueTime,
				period,
				NULL,
				this,
				FALSE
				);
			if(Result==0) {
				return -1;
			}
			m_isRunning = true;
			return 0;
		}

		int Timer::process()
		{
			if (m_eventHandler) {
				m_eventHandler(true);
			}
			return 0;
		}


		int Timer::cancel()
		{
			int result = 0;

			// Before calling callback function with fired=false, we need to clear the callback routine. 
			// Otherwise a recursive call might happen
			Cb_t originalEventHandler = m_eventHandler;
			m_eventHandler = Cb_t();

			if (m_isRunning) {
				m_isRunning = false;
				if (originalEventHandler) {
					originalEventHandler(false);
				}

				result = 1;
			}

			LARGE_INTEGER dueTime;
			dueTime.QuadPart = LLONG_MIN;
			BOOL Result = SetWaitableTimer(
				m_fd,
				&dueTime,
				0,
				NULL,
				NULL,
				FALSE
				);
			return result;
		}
	}
}
