// Copyright 2014 Hottinger Baldwin Messtechnik
// Distributed under MIT license
// See file LICENSE provided


#include <json/writer.h>

#include "jsonrpc_exception.h"
#include "hbm/jsonrpc/jsonrpc_defines.h"

namespace hbm {
	namespace exception {
		jsonrpcException::jsonrpcException(const Json::Value& error)
			: exception("")
			, m_error_obj(error)
			, m_localWhat()
		{
			Json::FastWriter writer;
			writer.omitEndingLineFeed();
			m_localWhat = writer.write(m_error_obj);
			m_localWhat += exception::what();
		}

		jsonrpcException::jsonrpcException( int code, const std::string& message)
			: exception("")
			, m_error_obj()
			, m_localWhat()
		{
			if(message.empty()==false) {
				m_error_obj[jsonrpc::ERR][jsonrpc::MESSAGE] = message;
			}

			m_error_obj[jsonrpc::ERR][jsonrpc::CODE] = code;
			Json::FastWriter writer;
			writer.omitEndingLineFeed();
			m_localWhat = writer.write(m_error_obj);
			m_localWhat += exception::what();
		}

		jsonrpcException::~jsonrpcException() throw()
		{
		}

		const Json::Value& jsonrpcException::json() const
		{
			return m_error_obj;
		}

		const char* jsonrpcException::what() const throw()
		{
			return m_localWhat.c_str();
		}
	}
}
