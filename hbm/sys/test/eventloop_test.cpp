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
static ssize_t eventHandlerIncrement(unsigned int* pValue, hbm::sys::Timer* pTimer)
{
	// under Linux read sets the timer to not-signaled!
	pTimer->read();
	++(*pValue);

	return 0;
}



/// start the eventloop in a separate thread wait some time and stop it.
BOOST_AUTO_TEST_CASE(stop_test)
{
	hbm::sys::EventLoop eventLoop;

	static const std::chrono::milliseconds waitDuration(300);

	std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now();
	std::thread worker(std::bind(&hbm::sys::EventLoop::execute, &eventLoop));
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

BOOST_AUTO_TEST_CASE(notify_test)
{
	int result;
	static const std::chrono::milliseconds duration(100);
	hbm::sys::EventLoop eventLoop;
	hbm::sys::Notifier notifier;
	result = notifier.wait_for(0);
	BOOST_CHECK_EQUAL(result, -1);
	result = notifier.read();
	BOOST_CHECK_EQUAL(result, 0);


	notifier.notify();
	result = notifier.wait_for(0);
	BOOST_CHECK_EQUAL(result, 1);
	result = notifier.wait_for(0);
	BOOST_CHECK_EQUAL(result, -1);

	notifier.notify();
	result = notifier.read();
	BOOST_CHECK_EQUAL(result, 1);
	result = notifier.read();
	BOOST_CHECK_EQUAL(result, 0);
}

BOOST_AUTO_TEST_CASE(oneshottimer_test)
{
	static const unsigned int timerCycle = 100;
	static const unsigned int timerCount = 10;
	static const std::chrono::milliseconds duration(timerCycle * timerCount);
	hbm::sys::EventLoop eventLoop;

	unsigned int counter = 0;

	hbm::sys::Timer singleshotTimer(timerCycle, false);
	eventLoop.addEvent(singleshotTimer.getFd(), std::bind(&eventHandlerIncrement, &counter, &singleshotTimer));

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

	hbm::sys::Timer singleshotTimer(timerCycle, true);
	eventLoop.addEvent(singleshotTimer.getFd(), std::bind(&eventHandlerIncrement, &counter, &singleshotTimer));

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

	hbm::sys::Timer singleshotTimer(timerCycle, false);
	eventLoop.addEvent(singleshotTimer.getFd(), std::bind(&eventHandlerIncrement, &counter, &singleshotTimer));
	std::thread worker = std::thread(std::bind(&hbm::sys::EventLoop::execute_for, std::ref(eventLoop), duration));

	singleshotTimer.cancel();

	worker.join();

	BOOST_CHECK_EQUAL(counter, 0);
}

BOOST_AUTO_TEST_CASE(removefromeventloop_test)
{
	static const unsigned int timerCycle = 100;
	static const unsigned int timerCount = 10;
	static const std::chrono::milliseconds duration(timerCycle * timerCount);
	hbm::sys::EventLoop eventLoop;

	unsigned int counter = 0;

	hbm::sys::Timer singleshotTimer(timerCycle, false);
	eventLoop.addEvent(singleshotTimer.getFd(), std::bind(&eventHandlerIncrement, &counter, &singleshotTimer));
	std::thread worker = std::thread(std::bind(&hbm::sys::EventLoop::execute_for, std::ref(eventLoop), duration));
	eventLoop.eraseEvent(singleshotTimer.getFd());
	int result = singleshotTimer.wait_for(duration.count());
	BOOST_CHECK_EQUAL(result, 1);
	worker.join();

	BOOST_CHECK_EQUAL(counter, 0);
}


BOOST_AUTO_TEST_CASE(severaltimers_test)
{
	static const unsigned int timerCycle = 10;
	static const unsigned int timerCount = 10;
	static const std::chrono::milliseconds duration(timerCycle * timerCount);
	hbm::sys::EventLoop eventLoop;

	unsigned int counter = 0;

	typedef std::vector < hbm::sys::Timer > timers_t;
	timers_t timers(10);

	for (timers_t::iterator iter = timers.begin(); iter != timers.end(); ++iter) {
		hbm::sys::Timer& timer = *iter;
		timer.set(timerCycle, false);
		eventLoop.addEvent(timer.getFd(), std::bind(&eventHandlerIncrement, &counter ,&timer));
	}

	for (timers_t::iterator iter = timers.begin(); iter != timers.end(); ++iter) {
		hbm::sys::Timer& timer = *iter;
		timer.set(timerCycle, false);
		eventLoop.eraseEvent(timer.getFd());
	}

	for (timers_t::iterator iter = timers.begin(); iter != timers.end(); ++iter) {
		hbm::sys::Timer& timer = *iter;
		timer.set(timerCycle, false);
		eventLoop.addEvent(timer.getFd(), std::bind(&eventHandlerIncrement, &counter ,&timer));
	}

	int result = eventLoop.execute_for(duration);
	BOOST_CHECK_EQUAL(counter, timers.size());
	BOOST_CHECK(result == 0);
}
