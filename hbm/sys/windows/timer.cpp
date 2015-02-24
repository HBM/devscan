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

		//VOID
		//	(APIENTRY *PTIMERAPCROUTINE)(
		//	_In_opt_ LPVOID lpArgToCompletionRoutine,
		//	_In_     DWORD dwTimerLowValue,
		//	_In_     DWORD dwTimerHighValue
		//	);

		static VOID CALLBACK resetRunningFlag(LPVOID lpArgToCompletionRoutine, DWORD dwTimerLowValue, DWORD dwTimerHighValue)
		{
			Timer* pTimer = reinterpret_cast < Timer* > (lpArgToCompletionRoutine);
			pTimer->cancel();
		}

		Timer::Timer(EventLoop& eventLoop, EventHandler_t eventHandler)
			: m_fd(NULL)
			, m_eventLoop(eventLoop)
			, m_eventHandler(eventHandler)
			, m_isRunning(false)
		{
			m_fd = CreateWaitableTimer(NULL, FALSE, NULL);
			cancel();
			m_eventLoop.addEvent(m_fd, std::bind(&Timer::process, this));
		}

		Timer::Timer(unsigned int period_ms, bool repeated, EventLoop& eventLoop, EventHandler_t eventHandler)
			: m_fd(NULL)
			, m_eventLoop(eventLoop)
			, m_eventHandler(eventHandler)
			, m_isRunning(false)
		{
			m_fd = CreateWaitableTimer(NULL, FALSE, NULL);
			m_eventLoop.addEvent(m_fd, std::bind(&Timer::process, this));
			set(period_ms, repeated);
		}

		Timer::~Timer()
		{
			m_eventLoop.eraseEvent(m_fd);
			CloseHandle(m_fd);
		}

		int Timer::set(unsigned int period_ms, bool repeated)
		{
			LARGE_INTEGER dueTime;
			static const int64_t multilpier = -10000; // negative because we want a relative time
			LONG period = 0; // in ms

			if (repeated) {
				period = period_ms;
			}
			dueTime.QuadPart = period_ms*multilpier; // in 100ns
			BOOL Result = SetWaitableTimer(
				m_fd,
				&dueTime,
				period,
				&resetRunningFlag,
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
			int result = read();
			if (result > 0) {
				if (m_eventHandler) {
					m_eventHandler();
				}
			}
			return result;
		}

		int Timer::read()
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

		int Timer::cancel()
		{
			int result = 0;
			if (m_isRunning) {
				m_isRunning = false;
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
