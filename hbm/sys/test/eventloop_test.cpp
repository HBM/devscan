// Copyright 2014 Hottinger Baldwin Messtechnik
// Distributed under MIT license
// See file LICENSE provided


#ifndef _WIN32
#define BOOST_TEST_DYN_LINK
#endif
#define BOOST_TEST_MAIN

#define BOOST_TEST_MODULE eventloop tests
#include <iostream>
#include <chrono>
#include <thread>
#include <functional>
#include <vector>

#include <boost/test/unit_test.hpp>

#include "hbm/sys/eventloop.h"
#include "hbm/sys/timer.h"
#include "hbm/sys/notifier.h"
#include "hbm/exception/exception.hpp"


static ssize_t eventHandlerPrint()
{
	std::cout << __FUNCTION__ << std::endl;
	return 0;
}


/// by returning error, the execute() method, that is doing the eventloop, exits
static ssize_t eventHandlerIncrement(unsigned int* pValue)
{
	++(*pValue);
	return 0;
}



/// start the eventloop in a separate thread wait some time and stop it.
BOOST_AUTO_TEST_CASE(stop_test)
{
	hbm::sys::EventLoop eventLoop;

	static const std::chrono::milliseconds waitDuration(300);

	std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now();
	std::thread worker(std::bind(&hbm::sys::EventLoop::execute, std::ref(eventLoop)));
	std::this_thread::sleep_for(waitDuration);
	eventLoop.stop();
	worker.join();
	std::chrono::steady_clock::time_point endTime = std::chrono::steady_clock::now();

	std::chrono::milliseconds delta = std::chrono::duration_cast < std::chrono::milliseconds > (endTime-startTime);

	BOOST_CHECK_MESSAGE(delta.count()>=300, "unexpected wait difference of " << std::to_string(delta.count()) << "ms");
}


BOOST_AUTO_TEST_CASE(waitforend_test)
{
	hbm::sys::EventLoop eventLoop;

	static const std::chrono::milliseconds duration(100);

	std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now();
	int result = eventLoop.execute_for(duration);
	std::chrono::steady_clock::time_point endTime = std::chrono::steady_clock::now();

	std::chrono::milliseconds delta = std::chrono::duration_cast < std::chrono::milliseconds > (endTime-startTime);

	BOOST_CHECK_EQUAL(result, 0);
	BOOST_CHECK_GE(delta.count(), duration.count()-3);
}

BOOST_AUTO_TEST_CASE(restart_test)
{
	hbm::sys::EventLoop eventLoop;

	static const std::chrono::milliseconds duration(100);

	for (unsigned int i=0; i<10; ++i) {
		std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now();
		int result = eventLoop.execute_for(duration);
		std::chrono::steady_clock::time_point endTime = std::chrono::steady_clock::now();
		std::chrono::milliseconds delta = std::chrono::duration_cast < std::chrono::milliseconds > (endTime-startTime);

		BOOST_CHECK_EQUAL(result, 0);
		BOOST_CHECK_GE(delta.count(), duration.count()-3);
	}
}

BOOST_AUTO_TEST_CASE(notify_test)
{
	unsigned int value = 0;
	int result;
	static const std::chrono::milliseconds duration(100);
	hbm::sys::EventLoop eventLoop;
	hbm::sys::Notifier notifier(eventLoop, std::bind(&eventHandlerIncrement, &value));
	BOOST_CHECK_EQUAL(value, 0);

	std::thread worker(std::bind(&hbm::sys::EventLoop::execute, &eventLoop));


	static const unsigned int count = 10;
	for(unsigned int i=0; i<count; ++i) {
		notifier.notify();
	}


	std::this_thread::sleep_for(std::chrono::milliseconds(10));
	eventLoop.stop();
	worker.join();

	BOOST_CHECK_EQUAL(value, count);

}

BOOST_AUTO_TEST_CASE(oneshottimer_test)
{
	static const unsigned int timerCycle = 100;
	static const unsigned int timerCount = 10;
	static const std::chrono::milliseconds duration(timerCycle * timerCount);
	hbm::sys::EventLoop eventLoop;

	unsigned int counter = 0;

	hbm::sys::Timer cyclicTimer(timerCycle, false, eventLoop, std::bind(&eventHandlerIncrement, &counter));

	int result = eventLoop.execute_for(duration);
	BOOST_CHECK_EQUAL(counter, 1);
	BOOST_CHECK_EQUAL(result, 0);
}

BOOST_AUTO_TEST_CASE(cyclictimer_test)
{
	static const unsigned int excpectedMinimum = 10;
	static const unsigned int timerCycle = 100;
	static const unsigned int timerCount = excpectedMinimum+1;
	static const std::chrono::milliseconds duration(timerCycle * timerCount);
	hbm::sys::EventLoop eventLoop;

	unsigned int counter = 0;

	hbm::sys::Timer cyclicTimer(timerCycle, true, eventLoop, std::bind(&eventHandlerIncrement, &counter));

	int result = eventLoop.execute_for(duration);
	BOOST_CHECK_GE(counter, excpectedMinimum);
	BOOST_CHECK_EQUAL(result, 0);
}

BOOST_AUTO_TEST_CASE(canceltimer_test)
{
	static const unsigned int timerCycle = 100;
	static const unsigned int timerCount = 10;
	static const std::chrono::milliseconds duration(timerCycle * timerCount);
	hbm::sys::EventLoop eventLoop;

	unsigned int counter = 0;

	hbm::sys::Timer cyclicTimer(timerCycle, false, eventLoop, std::bind(&eventHandlerIncrement, &counter));
	std::thread worker = std::thread(std::bind(&hbm::sys::EventLoop::execute_for, std::ref(eventLoop), duration));

	cyclicTimer.cancel();

	worker.join();

	BOOST_CHECK_EQUAL(counter, 0);
}

BOOST_AUTO_TEST_CASE(removenotifier_test)
{
	static const unsigned int timerCycle = 100;
	static const unsigned int timerCount = 10;
	static const std::chrono::milliseconds duration(timerCycle * timerCount);
	hbm::sys::EventLoop eventLoop;

	unsigned int counter = 0;
	std::thread worker = std::thread(std::bind(&hbm::sys::EventLoop::execute_for, std::ref(eventLoop), duration));

	{
		// leaving this scope leads to destruction of the timer and the removal from the event loop
		hbm::sys::Timer cyclicTimer(timerCycle, true, eventLoop, std::bind(&eventHandlerIncrement, &counter));
		std::this_thread::sleep_for(std::chrono::milliseconds(timerCycle * timerCount / 2));
	}

	worker.join();

	BOOST_CHECK_LT(counter, timerCount);
}
