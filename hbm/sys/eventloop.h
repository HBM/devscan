// Copyright 2014 Hottinger Baldwin Messtechnik
// Distributed under MIT license
// See file LICENSE provided

#ifndef _EventLoop_H
#define _EventLoop_H


#ifdef _WIN32
	#include <vector>
	#include <WinSock2.h>
	#include <Windows.h>
	typedef HANDLE event;
#else
	#include <unordered_map>
	typedef int event;
#endif
#include <functional>

#include <chrono>
#include "hbm/exception/exception.hpp"
#include "hbm/sys/notifier.h"

namespace hbm {
	namespace sys {
		typedef std::function < int () > eventHandler_t;


		/// \warning not thread-safe
		class EventLoop {
		public:
			/// \throws hbm::exception
			EventLoop();
			virtual ~EventLoop();
			void addEvent(event fd, eventHandler_t eventHandler);

			void eraseEvent(event fd);

			void clear();

			/// \return -1 eventloop stopped because one callback function returned error (-1).
			int execute();
			/// \return 0 if given time to wait was reached. -1 eventloop stopped because one callback function returned error (-1).
			int execute_for(std::chrono::milliseconds timeToWait);

			void stop();
		private:
			struct eventInfo_t {
				event fd;
				eventHandler_t eventHandler;
			};

			/// fd is the key
	#ifdef _WIN32
			typedef std::vector < eventInfo_t > eventInfos_t;
	#else
			typedef std::unordered_map <event, eventInfo_t > eventInfos_t;
			int m_epollfd;
	#endif

			Notifier m_stopNotifier;
			eventInfos_t m_eventInfos;
			eventInfo_t m_stopEvent;
		};
	}
}
#endif
