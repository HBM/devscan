// Copyright 2014 Hottinger Baldwin Messtechnik
// Distributed under MIT license
// See file LICENSE provided


#ifndef _WIN32
#define BOOST_TEST_DYN_LINK
#endif
#define BOOST_TEST_MAIN

#define BOOST_TEST_MODULE communication tests

#include <thread>
#include <chrono>
#include <functional>

#include <boost/test/unit_test.hpp>

#include "hbm/communication/multicastserver.h"
#include "hbm/communication/netadapter.h"
#include "hbm/communication/netadapterlist.h"

#include "hbm/sys/eventloop.h"

static std::string received;


static int receiveAndKeep(hbm::communication::MulticastServer* pMcs)
{
	ssize_t result;
	do {
		char buf[1042];
		int adapterIndex;
		int ttl;
		result = pMcs->receiveTelegram(buf, sizeof(buf), adapterIndex, ttl);
		if(result>0) {
			received = std::string(buf, result);
			std::cout << __FUNCTION__ << " '" << received << "'" <<std::endl;
		} else if (result==-1) {
			if(errno==EAGAIN || errno==EWOULDBLOCK) {
				return 0;
			}
			std::cout << __FUNCTION__ <<  " " << std::to_string(errno) << " " << strerror(errno) << std::endl;
		} else {
			std::cout << __FUNCTION__ << " empty" << std::endl;
		}
	} while(result>=0);
	return 0;
}

static int receiveAndDiscard(hbm::communication::MulticastServer* pMcs)
{
	ssize_t result;
	do {
		char buf[1042];
		int adapterIndex;
		int ttl;
		result = pMcs->receiveTelegram(buf, sizeof(buf), adapterIndex, ttl);
	} while(result>=0);
	return 0;
}


BOOST_AUTO_TEST_CASE(start_send_stop_test)
{
	static const char MULTICASTGROUP[] = "239.255.77.177";
	static const unsigned int UDP_PORT = 22222;
	static const unsigned int CYCLECOUNT = 1000;

	static const std::string MSG = "testest";

	hbm::sys::EventLoop eventloop;
	std::thread worker(std::thread(std::bind(&hbm::sys::EventLoop::execute, std::ref(eventloop))));
	hbm::communication::NetadapterList adapters;
	hbm::communication::MulticastServer mcsReceiver(adapters, eventloop);
	hbm::communication::MulticastServer mcsSender(adapters, eventloop);

	mcsSender.start(MULTICASTGROUP, UDP_PORT, std::bind(&receiveAndDiscard, std::placeholders::_1));
	mcsSender.addAllInterfaces();

	for (unsigned int i=0; i<CYCLECOUNT; ++i)
	{
		received.clear();
		mcsReceiver.start(MULTICASTGROUP, UDP_PORT, std::bind(&receiveAndKeep, std::placeholders::_1));
		mcsReceiver.addAllInterfaces();
		mcsSender.send(MSG.c_str(), MSG.length());
		mcsReceiver.stop();
		BOOST_CHECK(MSG==received);

		std::cout << __FUNCTION__ << " " << i << std::endl;
	}

	mcsSender.stop();

	eventloop.stop();
	worker.join();

	std::cout << __FUNCTION__ << " done" << std::endl;
}
