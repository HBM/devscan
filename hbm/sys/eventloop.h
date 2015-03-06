// Copyright 2014 Hottinger Baldwin Messtechnik
// Distributed under MIT license
// See file LICENSE provided

#ifndef _EventLoop_H
#define _EventLoop_H


#include <list>
#include <unordered_map>
#ifdef _WIN32
	#include <WinSock2.h>
	#include <Windows.h>
	typedef HANDLE event;
#else
	typedef int event;
#endif
#include <functional>
#include <chrono>
#include <mutex>

#include "hbm/exception/exception.hpp"
#include "hbm/sys/defines.h"

namespace hbm {
	namespace sys {
		class EventLoop {
		public:
			/// \throws hbm::exception
			EventLoop();
			virtual ~EventLoop();

			/// existing event handler of an fd will be replaced
			void addEvent(event fd, EventHandler_t eventHandler);

			void eraseEvent(event fd);

			/// \return 0 stopped; -1 error
			int execute();
			/// \return 0 stopped or if given time to wait was reached; -1 error
			int execute_for(std::chrono::milliseconds timeToWait);

			void stop();
		private:
			struct eventInfo_t {
				event fd;
				EventHandler_t eventHandler;
			};

			/// fd is the key
			typedef std::unordered_map <event, eventInfo_t > eventInfos_t;
			typedef std::list < eventInfo_t > changelist_t;

			/// called from within the event loop for thread-safe add and remove of events
			int changeHandler();
#ifdef _WIN32
			std::vector < HANDLE > m_handles;
#else
			int m_epollfd;
#endif
			event m_changeFd;
			event m_stopFd;

			eventInfo_t m_changeEvent;

			/// events to be added/removed go in here
			changelist_t m_changeList;
			std::mutex m_changeListMtx;

			/// events handled by event loop
			eventInfos_t m_eventInfos;
		};
	}
}
#endif
