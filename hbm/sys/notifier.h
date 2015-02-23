// Copyright 2014 Hottinger Baldwin Messtechnik
// Distributed under MIT license
// See file LICENSE provided


#ifndef _HBM__NOTIFIER_H
#define _HBM__NOTIFIER_H

#include "hbm/sys/defines.h"

#include "hbm/exception/exception.hpp"

namespace hbm {
	namespace sys {
		class EventLoop;

		class Notifier {
		public:
			/// \throws hbm::exception
			Notifier(EventLoop& eventLoop, EventHandler_t eventHandler);
			Notifier(Notifier&& source);

			virtual ~Notifier();

			int notify();
		private:
			/// must not be copied
			Notifier(const Notifier& op);
			/// must not be assigned
			Notifier operator=(const Notifier& op);

			/// called by eventloop
			int process();

			int read();

			event m_fd;
			EventLoop& m_eventLoop;
			EventHandler_t m_eventHandler;
		};
	}
}
#endif
