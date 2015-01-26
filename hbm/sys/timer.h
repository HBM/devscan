// Copyright 2014 Hottinger Baldwin Messtechnik
// Distributed under MIT license
// See file LICENSE provided


#ifndef _HBM__TIMER_H
#define _HBM__TIMER_H

#ifdef _WIN32
#include <WinSock2.h>
#ifndef ssize_t
#define ssize_t int
#endif
typedef HANDLE event;
#else
#include <unistd.h>
typedef int event;
#endif


#include "hbm/exception/exception.hpp"

namespace hbm {
	namespace sys {
		/// A timer running periodically. Starts when setting the period. Event gets signaled when period is reached.
		class Timer {
		public:
			/// \throws hbm::exception
			Timer();

			/// @param period_ms timer interval in ms
			/// \throws hbm::exception
			Timer(unsigned int period_ms);
			~Timer();

			/// @param period_ms timer interval in ms
			int set(unsigned int period_ms);

			/// \return 0 if timer has not expired yet. 1 if timer has expired. -1 if timer was not started
			int read();

			/// \return 0 if timer was stopped before expiration. 1 if timer has expired. -1 if timer was not started
			int wait();

			/// to poll
			event getFd() const;

			int cancel();

		private:
			/// must not be copied
			Timer(const Timer& op);
			/// must not be assigned
			Timer operator=(const Timer& op);

			event m_fd;
#ifdef _WIN32
			/// workaround for windows to determine whether the timer is stopped or got signaled
			bool m_canceled;
#endif

		};
	}
}
#endif
