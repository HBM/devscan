// Copyright 2015 Hottinger Baldwin Messtechnik
// Distributed under MIT license
// See file LICENSE provided

#ifndef __HBM__STRING__TRIM_H
#define __HBM__STRING__TRIM_H

#include <string>

namespace hbm {
	namespace string {

		std::string trim_copy(std::string text);
		void trim(std::string& text);
	}
}
#endif // __HBM__STRING__TRIM_H
