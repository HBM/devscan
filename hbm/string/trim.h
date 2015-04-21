// Copyright 2015 Hottinger Baldwin Messtechnik
// Distributed under MIT license
// See file LICENSE provided

#ifndef __HBM__STRING__TRIM_H
#define __HBM__STRING__TRIM_H

#include <string>

namespace hbm {
	namespace string {

		std::string trim_copy(std::string text);

		void trim_left(std::string& text);

		void trim_right(std::string& text);

		void trim(std::string& text);

		void trim_left_if(std::string& text, char ch);

		void trim_right_if(std::string& text, char ch);
	}
}
#endif // __HBM__STRING__TRIM_H
