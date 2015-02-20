// Copyright 2014 Hottinger Baldwin Messtechnik
// Distributed under MIT license
// See file LICENSE provided


#include <iostream>
#include <cstring>

#include <WinSock2.h>

#include "hbm/sys/notifier.h"

namespace hbm {
	namespace sys {
		Notifier::Notifier()
			: m_fd(NULL)
		{
			m_fd = CreateEvent(NULL, false, false, NULL);
		}

		Notifier::~Notifier()
		{
			CloseHandle(m_fd);
		}

		int Notifier::notify()
		{
			if (SetEvent(m_fd)==0) {
				return -1;
			}
			return 0;
		}


		int Notifier::read()
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

		int Notifier::wait()
		{
			return wait_for(INFINITE);
		}


		int Notifier::wait_for(int period_ms)
		{
			DWORD result = WaitForSingleObject(m_fd, period_ms);
			switch (result) {
			case WAIT_OBJECT_0:
				return 1;
				break;
			default:
				return -1;
				break;
			}
			return 0;
		}

		event Notifier::getFd() const
		{
			return m_fd;
		}
	}
}
