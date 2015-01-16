// Copyright 2014 Hottinger Baldwin Messtechnik
// Distributed under MIT license
// See file LICENSE provided



#ifndef HBM__EXCEPTION_H
#define HBM__EXCEPTION_H

#include <stdexcept>
#include <string>

#ifdef __GNUG__
#include <cstdlib>

#include <execinfo.h>
#endif

#include "hbm/debug/stack_trace.hpp"

namespace hbm {
	namespace exception {
		/// base class for all exceptions raised/thrown by hbm-code.
		/// this base class is required for catching all hbm-specific exceptions with
		/// one catch clause like
		/// try
		/// {
		/// }
		/// catch( const hbm::exception& e )
		/// {
		/// }
		class exception : public std::runtime_error {
			public:
				exception(const std::string& description)
					: std::runtime_error(description)
					, output(std::string(std::runtime_error::what()))
				{
					output.append("\n");
					output.append(hbm::debug::fill_stack_trace());
				}
				virtual ~exception() throw() {}
				virtual const char* what() const throw()
				{
					return output.c_str();
				}
			protected:
				std::string output;

			private:
				static const unsigned int backtrace_size = 100;
		};
	}
}

#endif

