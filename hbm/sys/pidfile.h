// Copyright 2014 Hottinger Baldwin Messtechnik
// Distributed under MIT license
// See file LICENSE provided


#ifndef _PIDFILE_H
#define _PIDFILE_H

#include <string>

namespace hbm {
	namespace sys {
		/// creates pid file on construction deletes pid file on destruction
		class PidFile
		{
		public:
			/// @param[in, out] name may be manipulated!
			/// \throws hbm::exception
			PidFile(char *name);
			virtual ~PidFile();

		private:
			std::string m_pidFileName;
		};
	}
}
#endif
