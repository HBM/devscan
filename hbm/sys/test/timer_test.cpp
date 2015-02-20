// Copyright 2014 Hottinger Baldwin Messtechnik
// Distributed under MIT license
// See file LICENSE provided


#ifndef _WIN32
#define BOOST_TEST_DYN_LINK
#endif
#define BOOST_TEST_MAIN

#define BOOST_TEST_MODULE timer tests

#include <stdint.h>
#include <iostream>
#include <chrono>
#include <thread>
#include <boost/test/unit_test.hpp>

#include "hbm/sys/timer.h"
#include "hbm/exception/exception.hpp"


BOOST_AUTO_TEST_CASE(notstarted_test)
{
	hbm::sys::Timer timer;
	ssize_t result;
	result = timer.wait_for(10);
	BOOST_CHECK_EQUAL(result, -1);
}


BOOST_AUTO_TEST_CASE(read_test)
{
	static const unsigned int timeToWait = 100;
	hbm::sys::Timer timer;
	std::chrono::steady_clock::time_point start(std::chrono::steady_clock::now());
	std::chrono::steady_clock::time_point end;
	timer.set(timeToWait, true);
	ssize_t result = timer.read();
	BOOST_CHECK(result==0);
	// wait one timer cycle + epsilon
	std::this_thread::sleep_for(std::chrono::milliseconds(timeToWait+10));
	result = timer.read();
	BOOST_CHECK(result==1);
}

BOOST_AUTO_TEST_CASE(wait_test)
{
	static const unsigned int timeToWait = 3500;
	hbm::sys::Timer timer;
	std::chrono::steady_clock::time_point start(std::chrono::steady_clock::now());
	std::chrono::steady_clock::time_point end;
	timer.set(timeToWait, true);
	ssize_t result = timer.wait();
	BOOST_CHECK(result==1);
	end = std::chrono::steady_clock::now();
	std::chrono::milliseconds delta = std::chrono::duration_cast < std::chrono::milliseconds > (end - start);
	uint64_t diff = abs((timeToWait)-delta.count());

	BOOST_CHECK_LT(diff, 20);
}

BOOST_AUTO_TEST_CASE(wait_repeated_test)
{
	static const unsigned int timeToWait = 50;
	hbm::sys::Timer timer;
	std::chrono::steady_clock::time_point start;
	std::chrono::steady_clock::time_point end;


	timer.set(timeToWait, true);
	for(unsigned int i=0; i<10; ++i) {
		start = std::chrono::steady_clock::now();
		ssize_t result = timer.wait();
		end = std::chrono::steady_clock::now();
		BOOST_CHECK(result==1);
		std::chrono::milliseconds delta = std::chrono::duration_cast < std::chrono::milliseconds > (end - start);
		uint64_t diff = abs((timeToWait)-delta.count());
		BOOST_CHECK_LT(diff, 20);
	}
}

BOOST_AUTO_TEST_CASE(wait_oneshot_test)
{
	static const unsigned int timeToWait = 10;
	hbm::sys::Timer timer;
	timer.set(timeToWait, false);
	ssize_t result = timer.wait_for(timeToWait*3);
	BOOST_CHECK_EQUAL(result, 1);
	result = timer.wait_for(timeToWait*3);
	BOOST_CHECK_EQUAL(result, -1);
}

BOOST_AUTO_TEST_CASE(cancel_wait_oneshot_test)
{
	static const unsigned int timeToWait = 100;
	hbm::sys::Timer timer;
	ssize_t result;

	result = timer.cancel();
	BOOST_CHECK_EQUAL(result, 0);

	timer.set(timeToWait, false);
	result = timer.cancel();
	BOOST_CHECK_EQUAL(result, 1);
	result = timer.wait_for(timeToWait * 3);
	BOOST_CHECK_EQUAL(result, -1);
	timer.set(timeToWait, false);
	result = timer.wait_for(timeToWait*3);
	BOOST_CHECK_EQUAL(result, 1);
	result = timer.wait_for(timeToWait*3);
	BOOST_CHECK_EQUAL(result, -1);
}


BOOST_AUTO_TEST_CASE(stop_test)
{
	static const unsigned int timeToWait = 300;
	hbm::sys::Timer timer;
	std::chrono::steady_clock::time_point start;
	std::chrono::steady_clock::time_point end;

	start = std::chrono::steady_clock::now();
	timer.set(timeToWait, true);
	timer.cancel();
	// timer is stopped, should return with timeout
	ssize_t result = timer.wait_for(timeToWait);
	BOOST_CHECK(result==-1);
}

BOOST_AUTO_TEST_CASE(move_test)
{
	static const unsigned int timeToWait = 3500;
	hbm::sys::Timer timer = hbm::sys::Timer(); // move temporary object
	std::chrono::steady_clock::time_point start(std::chrono::steady_clock::now());
	std::chrono::steady_clock::time_point end;
	timer.set(timeToWait, true);
	ssize_t result = timer.wait();
	BOOST_CHECK(result==1);
	end = std::chrono::steady_clock::now();
	std::chrono::milliseconds delta = std::chrono::duration_cast < std::chrono::milliseconds > (end - start);
	uint64_t diff = abs((timeToWait)-delta.count());

	BOOST_CHECK_LT(diff, 20);
}


