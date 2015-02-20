// Copyright 2015 Hottinger Baldwin Messtechnik
// Distributed under MIT license
// See file LICENSE provided

#ifndef __HBM__STRING__SPLIT_H
#define __HBM__STRING__SPLIT_H

#include <string>
#include <vector>

namespace hbm {
	namespace string {

		/// \brief split a string by a separator-string
		///
		/// The string text is searched for occurenced of the substring separator
		/// from left ro right.
		/// <br>
		/// If text is an empty string, a vector with one member is returned,
		/// which is the empty string.
		/// <br>
		/// If text does not contain the separator, a vector with one member is
		/// returned, which is the original string text.
		///
		/// \param text  the string to split
		/// \param separator  the string to search for in the text
		/// \return vector of all pieces of text resulted by chopping it at each separator.
		std::vector<std::string> split(std::string text, std::string separator);
	}
}

#endif // __HBM__STRING__SPLIT_H
