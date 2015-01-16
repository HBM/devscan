// Copyright 2014 Hottinger Baldwin Messtechnik
// Distributed under MIT license
// See file LICENSE provided


#ifndef _HBM__NOTIFIER_H
#define _HBM__NOTIFIER_H

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
		class Notifier {
		public:
			/// \throws hbm::exception
			Notifier();
			~Notifier();

			int notify();

			int wait();

			int cancel();

			/// to poll
			event getFd() const;

		private:
			/// must not be copied
			Notifier(const Notifier& op);
			/// must not be assigned
			Notifier operator=(const Notifier& op);

			event m_fd;
		};
	}
}
#endif
