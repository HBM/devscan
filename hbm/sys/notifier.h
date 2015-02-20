// Copyright 2014 Hottinger Baldwin Messtechnik
// Distributed under MIT license
// See file LICENSE provided


#ifndef _HBM__NOTIFIER_H
#define _HBM__NOTIFIER_H

#include "hbm/sys/defines.h"
#include "hbm/exception/exception.hpp"

namespace hbm {
	namespace sys {
		class Notifier {
		public:
			/// \throws hbm::exception
			Notifier();
			Notifier(Notifier&& source);

			virtual ~Notifier();

			int notify();

			int read();

			int wait();

			int wait_for(int period_ms);

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
