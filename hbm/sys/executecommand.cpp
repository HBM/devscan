// Copyright 2014 Hottinger Baldwin Messtechnik
// Distributed under MIT license
// See file LICENSE provided



#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#ifdef _STANDARD_HARDWARE
	#include <iostream>
#endif
#include <stdexcept>

#include <sys/types.h>


#ifdef _WIN32
#define syslog fprintf
#define LOG_DEBUG stdout
#define LOG_INFO stdout
#define LOG_ERR stderr
#define popen(a, b) _popen(a, b)
#define pclose(a) _pclose(a)
#else
#include <wait.h>
#include <syslog.h>
#include <unistd.h>
#endif

#include <errno.h>

#include "hbm/exception/exception.hpp"
#include "hbm/sys/executecommand.h"

namespace hbm {
	namespace sys {
		std::string executeCommand(const std::string& command)
		{
			std::string retVal;
#ifdef _STANDARD_HARDWARE
			std::cout << command << std::endl;
#else
			FILE* f = popen(command.c_str(),"r");
			if (f == NULL) {
				std::string msg = std::string(__FUNCTION__) + "popen failed (cmd=" + command + ")!";
				throw hbm::exception::exception(msg);
			} else {
				char buffer[1024];
				do {
					size_t count = fread(buffer, 1, sizeof(buffer), f);
					if (count==0) {
						break;
					}
					retVal += std::string(buffer, count);
				} while(true);
				pclose(f);
			}
#endif
			return retVal;
		}

		int executeCommand(const std::string& command, const params_t &params, const std::string& stdinString)
		{
#ifdef _STANDARD_HARDWARE
			std::cout << command << " ";

			for(params_t::const_iterator iter = params.begin(); iter!=params.end(); ++iter) {
				std::cout << *iter << " ";
			}

			std::cout << " < " << stdinString << std::endl;
#else
			static const unsigned int PIPE_READ = 0;
			static const unsigned int PIPE_WRITE = 1;
			int pfd[2];
			pid_t cpid;

			if (pipe(pfd) == -1){
				syslog(LOG_ERR, "error creating pipe");
				return -1;
			}
			cpid = fork();
			if (cpid == -1) {
				syslog(LOG_ERR, "error forking process");
				return -1;
			}
			if (cpid == 0) {
				// Child
				close(pfd[PIPE_WRITE]); // close unused write end

				// redirect stdin
				if(pfd[PIPE_READ] != STDIN_FILENO) {
					if (dup2(pfd[PIPE_READ], STDIN_FILENO) == -1) {
						syslog(LOG_ERR, "error redirecting stdin");
						return -1;
					}
				}

				std::vector < char* > argv;
				argv.push_back(const_cast < char* > (command.c_str()));
				for(params_t::const_iterator iter = params.begin(); iter!=params.end(); ++iter) {
					argv.push_back( const_cast < char* > ((*iter).c_str()));
				}
				argv.push_back(NULL);

				execve(command.c_str(), &argv[0], NULL);
				// if we get here at all, an error occurred, but we are in the child
				// process, so just exit
				syslog(LOG_ERR, "error executing '%s' '%s'", command.c_str(), strerror(errno));
				exit(EXIT_FAILURE);
			} else if ( cpid > 0 ) {
				int waitStatus;
				// Parent
				close(pfd[PIPE_READ]); // close unused read end

				// send data to stdin of child
				const char* pStdinData = stdinString.c_str();
				size_t stdinDataSize = stdinString.size();
				while(stdinDataSize) {
					ssize_t ret = write(pfd[PIPE_WRITE], pStdinData, stdinDataSize);
					if (ret == -1) {
						syslog(LOG_ERR, "error writing to stdin of child '%s'", command.c_str());
						break;
					}
					stdinDataSize -= ret;
					pStdinData += ret;
				}
				close(pfd[PIPE_WRITE]);

				// wait for child to finish
				waitpid(cpid, &waitStatus, 0);

				if(WIFEXITED(waitStatus)==false) {
					// child process did not exit normally
					return -1;
				}

				if(WEXITSTATUS(waitStatus)!=EXIT_SUCCESS) {
					// child did exit with failure. Maybe the desired program could not be executed. Otherwise the executed program itself returned the error.
					return -1;
				}
			} else {
				syslog(LOG_ERR, "failed to create child!");
				close(pfd[PIPE_READ]);
				close(pfd[PIPE_WRITE]);
				return -1;
			}
#endif
			return 0;
		}
	}
}

