// Copyright 2014 Hottinger Baldwin Messtechnik
// Distributed under MIT license
// See file LICENSE provided


#ifndef _EXECUTECOMMAND_H
#define _EXECUTECOMMAND_H

#include <string>
#include <vector>

namespace hbm {
	namespace sys {
		typedef std::vector < std::string > params_t;
		/// \throws hbm::exception
		std::string executeCommand(const std::string& command);

		int executeCommand(const std::string& command, const params_t& params, const std::string& stdinString);
	}
}
#endif
