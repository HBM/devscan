// Copyright 2014 Hottinger Baldwin Messtechnik
// Distributed under MIT license
// See file LICENSE provided


#ifndef _HBM__TIMER_H
#define _HBM__TIMER_H

#include <functional>
#include <chrono>

#include "hbm/exception/exception.hpp"
#include "hbm/sys/defines.h"
#include "hbm/sys/eventloop.h"

namespace hbm {
	namespace sys {
		/// A timer running periodically or in single-shot-mode. Starts when setting the period. Callback routine gets called when period elapsed or running timer gets canceled.
		class Timer {
		public:
			/// called when timer fires or is being cancled
			/// \param false if timer got canceled; true if timer fired
			typedef std::function < void (bool fired) > Cb_t;

			/// \throws hbm::exception
			Timer(EventLoop& eventLoop);

			Timer(Timer&& source);

			~Timer();

			/// @param period_ms timer interval in ms
			int set(unsigned int period_ms, bool repeated, Cb_t eventHandler);
			int set(std::chrono::milliseconds period, bool repeated, Cb_t eventHandler);

			/// if timer is running, callback routine will be called with fired=false
			/// \return 1 success, timer was running; 0 success
			int cancel();

		private:
			/// must not be copied
			Timer(const Timer& op);
			/// must not be assigned
			Timer operator=(const Timer& op);

			/// called by eventloop
			int process();

			event m_fd;
			EventLoop& m_eventLoop;
			Cb_t m_eventHandler;
#ifdef _WIN32
			bool m_isRunning;
#endif
		};
	}
}
#endif
