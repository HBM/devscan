// Copyright 2014 Hottinger Baldwin Messtechnik
// Distributed under MIT license
// See file LICENSE provided


#ifndef __HBM__JET__JSONEXCEPTION
#define __HBM__JET__JSONEXCEPTION

#include <string>

#include <json/value.h>
#include "hbm/exception/exception.hpp"

namespace hbm {
	namespace exception {
		class jsonrpcException : public hbm::exception::exception	{
		public:
			jsonrpcException(int code, const std::string& message="");
			jsonrpcException(const Json::Value& error);

			virtual ~jsonrpcException() throw();

			const char* what() const throw();

			const Json::Value& json() const;

		private:
			Json::Value m_error_obj;
			std::string m_localWhat;
		};
	}
}
#endif
