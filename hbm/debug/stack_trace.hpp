// Copyright 2014 Hottinger Baldwin Messtechnik
// Distributed under MIT license
// See file LICENSE provided


#include <string>

#ifdef __GNUG__
#include <execinfo.h>
#endif

namespace hbm {
	namespace debug {

		static const unsigned int backtrace_size = 100;

		static std::string fill_stack_trace()
		{
			std::string output;
#ifdef __GNUG__
			void *backtrace_buffer[backtrace_size];
			unsigned int num_functions = ::backtrace(backtrace_buffer, backtrace_size);
			char **function_strings = ::backtrace_symbols(backtrace_buffer, num_functions);
			if (function_strings != NULL) {
				output.append("\n----------- stacktrace begin\n");
				output.append("\n");
				for (unsigned int i = 0; i < num_functions; ++i) {
					output.append(function_strings[i]);
					output.append("\n");
				}
				output.append("----------- stacktrace end\n");
				::free(function_strings);
			} else {
				output.append("No backtrace!\n");
			}
#else
			output.append("No backtrace!\n");
#endif
			return output;
		}
	}
}
