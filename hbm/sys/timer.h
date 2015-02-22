// Copyright 2014 Hottinger Baldwin Messtechnik
// Distributed under MIT license
// See file LICENSE provided


#ifndef _HBM__TIMER_H
#define _HBM__TIMER_H


#include "hbm/exception/exception.hpp"
#include "hbm/sys/defines.h"
#include "hbm/sys/eventloop.h"

namespace hbm {
	namespace sys {
		/// A timer running periodically. Starts when setting the period. Event gets signaled when period is reached.
		class Timer {
		public:
			/// \throws hbm::exception
			Timer(EventLoop& eventLoop, EventHandler_t eventHandler);

			Timer(Timer&& source);

			/// @param period_ms timer interval in ms
			/// \throws hbm::exception
			Timer(unsigned int period_ms, bool repeated, EventLoop& eventLoop, EventHandler_t eventHandler);
			~Timer();

			/// @param period_ms timer interval in ms
			int set(unsigned int period_ms, bool repeated);


			/// called by eventloop
			int process();

			/// to poll
			//event getFd() const;

			/// timer will not signal, wait will block.
			/// \return 1 success, timer was running; 0 success
			int cancel();

		private:
			/// must not be copied
			Timer(const Timer& op);
			/// must not be assigned
			Timer operator=(const Timer& op);

			/// \return 0 if timer has not expired yet. 1 if timer has expired
			int read();

//			/// \return 0 if timer was stopped before expiration. 1 if timer has expired
//			int wait();

//			/// \return 0 if timer was stopped before expiration. 1 if timer has expired. -1 on time out
//			int wait_for(int period_ms);


			event m_fd;
			EventLoop& m_eventLoop;
			EventHandler_t m_eventHandler;
#ifdef _WIN32
			bool m_isRunning;
#endif
		};
	}
}
#endif
