// Copyright 2014 Hottinger Baldwin Messtechnik
// Distributed under MIT license
// See file LICENSE provided


#ifndef _WIN32
#define BOOST_TEST_DYN_LINK
#endif
#define BOOST_TEST_MAIN

#define BOOST_TEST_MODULE executecommand tests

#include <cstdio>
#include <iostream>
#include <boost/test/unit_test.hpp>

#include "hbm/sys/executecommand.h"
#include "hbm/exception/exception.hpp"


BOOST_AUTO_TEST_CASE(command_withparameters_test)
{
	static const std::string fileName = "bla";
	::remove(fileName.c_str());
	hbm::sys::executeCommand("/usr/bin/touch " + fileName);
	int result = ::remove(fileName.c_str());
	BOOST_CHECK(result==0);
}


BOOST_AUTO_TEST_CASE(invalid_command_test)
{
	static const std::string fileName = "bla";
	hbm::sys::params_t params;
	params.push_back(fileName);
	int result = hbm::sys::executeCommand("/usr/bin/touc", params, "");
	BOOST_CHECK(result==-1);
}

BOOST_AUTO_TEST_CASE(valid_command_test)
{
	static const std::string fileName = "bla";
	int result = ::remove(fileName.c_str());
	hbm::sys::params_t params;
	params.push_back(fileName);

	result = hbm::sys::executeCommand("/usr/bin/touch", params, "");
	BOOST_CHECK(result==0);
	result = ::remove(fileName.c_str());
	BOOST_CHECK(result==0);
}

BOOST_AUTO_TEST_CASE(stdin_test)
{
	hbm::sys::params_t params;

	params.push_back("-w");
	int result = hbm::sys::executeCommand("/usr/bin/wc", params, "bla blub");
	BOOST_CHECK(result==0);
}

BOOST_AUTO_TEST_CASE(answer_test)
{
	std::string cmd = "/bin/echo";
	std::string arg = "hallo";
	std::string result = hbm::sys::executeCommand(cmd + " " + arg);
	BOOST_CHECK(result.substr(0, result.size()-1)==arg);
}
