// Copyright 2014 Hottinger Baldwin Messtechnik
// Distributed under MIT license
// See file LICENSE provided



#ifndef HBM__ERRNO_EXCEPTION_H
#define HBM__ERRNO_EXCEPTION_H


#include <cstring>
#include <string>

#include <errno.h>

#include "exception.hpp"

namespace hbm {
	namespace exception {

	/// exception which reads back errno stores its value and retrieves the
	/// corresponding error string which will be returned by
	/// virtual base method std::exception::what.
	/// suitable if dealing with naked unix methods which returns an error
	/// and as base-class for more specific exceptions.
	/// usage:
	/// if( write( fd, "abc", sizeof("abc") ) == -1 )
		///    throw errno_exception();
		class errno_exception : public hbm::exception::exception {

		public:
			errno_exception()
				: hbm::exception::exception(::strerror(errno)),
				  _errorno(errno)
			{
			}

			virtual ~errno_exception() throw() {}

			int errorno() const {
				return _errorno;
			}
		private:
			const int _errorno;
		};
	}
}

#endif

