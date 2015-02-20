// Copyright 2014 Hottinger Baldwin Messtechnik
// Distributed under MIT license
// See file LICENSE provided

#include <iostream>
#include <chrono>
#include <cstring>
#include <unistd.h>
#include <functional>

#include <syslog.h>
#include <sys/epoll.h>

#include <errno.h>


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

			struct epoll_event ev;
			ev.events = EPOLLIN | EPOLLET;
			eventInfo_t stopEvent;
			stopEvent.fd = m_stopNotifier.getFd();
			stopEvent.eventHandler = eventHandler_t();
			ev.data.ptr = nullptr;
			if (epoll_ctl(m_epollfd, EPOLL_CTL_ADD, stopEvent.fd, &ev) == -1) {
				syslog(LOG_ERR, "epoll_ctl failed %s", strerror(errno));
			}

			m_changeEvent.fd = m_changeNotifier.getFd();
			m_changeEvent.eventHandler = std::bind(&EventLoop::changeHandler, this);
			ev.data.ptr = &m_changeEvent;
			if (epoll_ctl(m_epollfd, EPOLL_CTL_ADD, m_changeEvent.fd, &ev) == -1) {
				syslog(LOG_ERR, "epoll_ctl failed %s", strerror(errno));
			}
		}

		EventLoop::~EventLoop()
		{
			stop();
			close(m_epollfd);
		}

		int EventLoop::changeHandler()
		{
			std::lock_guard < std::mutex > lock(m_changeListMtx);
			m_changeNotifier.read();
			for(changelist_t::const_iterator iter = m_changeList.begin(); iter!=m_changeList.end(); ++iter) {
				const eventInfo_t& item = *iter;
				if(item.eventHandler) {
					// add
					m_eventInfos[item.fd] = item;

					struct epoll_event ev;
					ev.events = EPOLLIN | EPOLLET;
					// important: elements of maps are guaranteed to keep there position in memory if members are added/removed!
					ev.data.ptr = &m_eventInfos[item.fd];
					if (epoll_ctl(m_epollfd, EPOLL_CTL_ADD, item.fd, &ev) == -1) {
						syslog(LOG_ERR, "epoll_ctl failed %s", strerror(errno));
					}
				} else {
					// remove
					epoll_ctl(m_epollfd, EPOLL_CTL_DEL, item.fd, NULL);
					m_eventInfos.erase(item.fd);
				}
			}
			m_changeList.clear();
			return 0;
		}


		void EventLoop::addEvent(event fd, eventHandler_t eventHandler)
		{
			if(!eventHandler) {
				return;
			}
			eventInfo_t evi;
			evi.fd = fd;
			evi.eventHandler = eventHandler;
			{
				std::lock_guard < std::mutex > lock(m_changeListMtx);
				m_changeList.push_back(evi);
				m_changeNotifier.notify();
			}
		}

		void EventLoop::eraseEvent(event fd)
		{
			eventInfo_t evi;
			evi.fd = fd;
			evi.eventHandler = eventHandler_t(); // empty handler signals removal
			{
				std::lock_guard < std::mutex > lock(m_changeListMtx);
				m_changeList.push_back(evi);
				m_changeNotifier.notify();
			}
		}

		int EventLoop::execute()
		{
			int nfds;
			static const unsigned int MAXEVENTS = 16;
			struct epoll_event events[MAXEVENTS];

			while (true) {
				do {
					nfds = epoll_wait(m_epollfd, events, MAXEVENTS, -1);
				} while ((nfds==-1) && (errno==EINTR));

				if(nfds<=0) {
					// 0: time out!
					return nfds;
				}

				for (int n = 0; n < nfds; ++n) {
					if(events[n].events & EPOLLIN) {
						eventInfo_t* pEventInfo = reinterpret_cast < eventInfo_t* > (events[n].data.ptr);
						if(pEventInfo==nullptr) {
							// stop notification!
							return 0;
						}
						ssize_t result;
						do {
							result = pEventInfo->eventHandler();
						} while (result>0);
						if(result<0) {
							if ((errno!=EAGAIN) && (errno!=EWOULDBLOCK)) {
								// this event is removed from the event loop.
								eraseEvent(pEventInfo->fd);
							}
						}
					}
				}
			}
		}

		int EventLoop::execute_for(std::chrono::milliseconds timeToWait)
		{
			int timeout;
			std::chrono::steady_clock::time_point endTime;
			if(timeToWait!=std::chrono::milliseconds(0)) {
				endTime = std::chrono::steady_clock::now() + timeToWait;
			}

			int nfds;
			static const unsigned int MAXEVENTS = 16;
			struct epoll_event events[MAXEVENTS];

			while (true) {
				std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
				if(now>=endTime) {
					return 0;
				}
				std::chrono::milliseconds timediff = std::chrono::duration_cast < std::chrono::milliseconds > (endTime-now);

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
						if(pEventInfo==nullptr) {
							// stop notification!
							return 0;
						}
						ssize_t result;
						do {
							result = pEventInfo->eventHandler();
						} while (result>0);
						if(result<0) {
							if ((errno!=EAGAIN) && (errno!=EWOULDBLOCK)) {
								// this event is removed from the event loop.
								eraseEvent(pEventInfo->fd);
							}
						}
					}
				}
			}
		}

		void EventLoop::stop()
		{
			m_stopNotifier.notify();
		}
	}
}
