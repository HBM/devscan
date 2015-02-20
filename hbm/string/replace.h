// Copyright 2015 Hottinger Baldwin Messtechnik
// Distributed under MIT license
// See file LICENSE provided

#ifndef __HBM__STRING__REPLACE_H
#define __HBM__STRING__REPLACE_H

#include <string>

namespace hbm {
	namespace string {

		/// \brief replace al occurences of 'search with 'substitute
		///
		/// \param text  the string to split
		/// \param search  the string to search for in the text
		std::string replace(std::string text, char search, char substitute);
		std::string replace(std::string text, std::string search, std::string substitute);
	}
}
#endif // __HBM__STRING__REPLACE_H
