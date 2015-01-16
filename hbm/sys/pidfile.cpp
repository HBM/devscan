// Copyright 2014 Hottinger Baldwin Messtechnik
// Distributed under MIT license
// See file LICENSE provided


#include <cstdio>

#ifdef _WIN32
#define LOG_DEBUG stdout
#define LOG_INFO stdout
#define LOG_ERR stderr
#include <process.h>
#define getpid _getpid
#define basename(x) x
#else
#include <sys/types.h>
#include <unistd.h>
#include <libgen.h>
#endif

#include "hbm/exception/exception.hpp"

#include "hbm/sys/pidfile.h"

namespace hbm {
	namespace sys {
		PidFile::PidFile(char* name)
	#ifdef _HBM_HARDWARE
			: m_pidFileName("/var/run/")
	#else
			: m_pidFileName()
	#endif
		{
			m_pidFileName += basename(name);
			m_pidFileName += ".pid";
			FILE* pidFile = ::fopen(m_pidFileName.c_str(), "w");

			if (pidFile == NULL) {
				std::string msg;
				msg = "could not create pid file ";
				msg += m_pidFileName;
				throw hbm::exception::exception(msg);
			} else {
				::fprintf(pidFile, "%d\n", getpid());
				::fclose(pidFile);
			}
		}

		PidFile::~PidFile()
		{
			::remove(m_pidFileName.c_str());
		}
	}
}
