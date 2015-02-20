// Copyright 2015 Hottinger Baldwin Messtechnik
// Distributed under MIT license
// See file LICENSE provided


#include <string>

#include "trim.h"


namespace hbm {
	namespace string {
		std::string trim_copy(std::string text)
		{
			std::string::size_type start = text.find_first_not_of(' ');
			if (start==std::string::npos) {
				return "";
			}
			std::string::size_type end = text.find_last_not_of(' ');
			std::string::size_type length = end-start+1;

			return text.substr(start, length);
		}

		void trim(std::string& text)
		{
			std::string::size_type start = text.find_first_not_of(' ');
			text.erase(0, start);

			std::string::size_type end = text.find_last_not_of(' ');
			if (end==std::string::npos) {
				return;
			}
			text.erase(end+1);
		}
	}
}

