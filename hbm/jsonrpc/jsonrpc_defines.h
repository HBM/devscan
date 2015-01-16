// Copyright 2014 Hottinger Baldwin Messtechnik
// Distributed under MIT license
// See file LICENSE provided

#ifndef _HBM__JSON__JSONNAME_H
#define _HBM__JSON__JSONNAME_H
namespace hbm {
	namespace jsonrpc {
		/// some error codes from the JSON RPC spec
		static const int parseError = -32700;
		static const int invalidRequest = -32600;
		static const int methodNotFound = -32601;
		static const int invalidParams = -32602;
		static const int internalError = -32603;

		/// some string constants from the JSON RPC spec
		static const char JSONRPC[] = "jsonrpc";
		static const char METHOD[] = "method";
		static const char RESULT[] = "result";
		static const char ERR[] = "error";
		static const char CODE[] = "code";
		static const char MESSAGE[] = "message";
		static const char DATA[] = "data";
		static const char PARAMS[] = "params";
		static const char ID[] = "id";
	}
}
#endif
