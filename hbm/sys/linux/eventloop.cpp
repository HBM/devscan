// Copyright 2014 Hottinger Baldwin Messtechnik
// Distributed under MIT license
// See file LICENSE provided

#include <chrono>
#include <cstring>
#include <unistd.h>
#include <functional>

#include <syslog.h>
#include <sys/epoll.h>


#include "hbm/sys/eventloop.h"
#include "hbm/sys/notifier.h"

namespace hbm {
	namespace sys {
		EventLoop::EventLoop()
			: m_epollfd(epoll_create(1)) // parameter is ignored but must be greater than 0
		{
			if(m_epollfd==-1) {
				throw hbm::exception::exception(std::string("epoll_create failed)") + strerror(errno));
			}

			m_stopEvent.fd = m_stopNotifier.getFd();

			struct epoll_event ev;
			ev.events = EPOLLIN;
			ev.data.ptr = nullptr;
			if (epoll_ctl(m_epollfd, EPOLL_CTL_ADD, m_stopEvent.fd, &ev) == -1) {
				syslog(LOG_ERR, "epoll_ctl failed %s", strerror(errno));
			}
		}

		EventLoop::~EventLoop()
		{
			close(m_epollfd);
		}

		void EventLoop::addEvent(event fd, eventHandler_t eventHandler)
		{
			eraseEvent(fd);

			eventInfo_t evi;
			evi.fd = fd;
			evi.eventHandler = eventHandler;

			eventInfo_t& eviRef = m_eventInfos[fd] = evi;


			struct epoll_event ev;
			ev.events = EPOLLIN;
			ev.data.ptr = &eviRef;
			if (epoll_ctl(m_epollfd, EPOLL_CTL_ADD, eviRef.fd, &ev) == -1) {
				syslog(LOG_ERR, "epoll_ctl failed %s", strerror(errno));
			}
		}

		void EventLoop::eraseEvent(event fd)
		{
			eventInfos_t::iterator iter = m_eventInfos.find(fd);
			if(iter!=m_eventInfos.end()) {
				const eventInfo_t& eviRef = iter->second;
				epoll_ctl(m_epollfd, EPOLL_CTL_DEL, eviRef.fd, NULL);
				m_eventInfos.erase(iter);
			}
		}

		void EventLoop::clear()
		{
			for (eventInfos_t::iterator iter = m_eventInfos.begin(); iter!=m_eventInfos.end(); ++iter) {
				const eventInfo_t& eviRef = iter->second;
				epoll_ctl(m_epollfd, EPOLL_CTL_DEL, eviRef.fd, NULL);
			}
			m_eventInfos.clear();
		}

		int EventLoop::execute()
		{
			ssize_t result = 0;

			int nfds;
			static const unsigned int MAXEVENTS = 16;
			struct epoll_event events[MAXEVENTS];

			do {
				do {
					nfds = epoll_wait(m_epollfd, events, MAXEVENTS, -1);
				} while ((nfds==-1) && (errno==EINTR));

				if((nfds==0) || (nfds==-1)) {
					// 0: time out!
					return nfds;
				}

				for (int n = 0; n < nfds; ++n) {
					if(events[n].events & EPOLLIN) {
						eventInfo_t* pEventInfo = reinterpret_cast < eventInfo_t* > (events[n].data.ptr);
						if(pEventInfo==nullptr) {
							return 0;
						}
						result = pEventInfo->eventHandler();
						if(result<0) {
							return result;
						}
					}
				}
			} while (result>=0);
			return result;
		}

		int EventLoop::execute_for(std::chrono::milliseconds timeToWait)
		{
			int timeout;
			ssize_t result = 0;
			std::chrono::steady_clock::time_point endTime;
			if(timeToWait!=std::chrono::milliseconds(0)) {
				endTime = std::chrono::steady_clock::now() + timeToWait;
			}

			int nfds;
			static const unsigned int MAXEVENTS = 16;
			struct epoll_event events[MAXEVENTS];

			do {
				std::chrono::milliseconds timediff = std::chrono::duration_cast < std::chrono::milliseconds > (endTime-std::chrono::steady_clock::now());

				timeout = static_cast< int > (timediff.count());

				do {
					nfds = epoll_wait(m_epollfd, events, MAXEVENTS, timeout);
				} while ((nfds==-1) && (errno==EINTR));

				if((nfds==0) || (nfds==-1)) {
					// 0: time out!
					return nfds;
				}

				for (int n = 0; n < nfds; ++n) {
					if(events[n].events & EPOLLIN) {
						eventInfo_t* pEventInfo = reinterpret_cast < eventInfo_t* > (events[n].data.ptr);
						result = pEventInfo->eventHandler();
						if(result<0) {
							break;
						}
					}
				}
			} while (result>=0);
			return result;
		}

		void EventLoop::stop()
		{
			m_stopNotifier.notify();
		}
	}
}
