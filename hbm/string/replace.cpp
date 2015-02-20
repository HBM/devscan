// Copyright 2015 Hottinger Baldwin Messtechnik
// Distributed under MIT license
// See file LICENSE provided


#include <algorithm>
#include <string>

#include "replace.h"


namespace hbm {
	namespace string {
		std::string replace(std::string text, char search, char substitute)
		{
			std::replace( text.begin(), text.end(), search, substitute);
			return text;
		}

		std::string replace(std::string text, std::string search, std::string substitute)
		{
			size_t position = 0;
			while((position=text.find(search, position))!=std::string::npos) {
				text.replace(position, search.length(), substitute);
				position += search.length();
			}
			return text;
		}
	}
}

