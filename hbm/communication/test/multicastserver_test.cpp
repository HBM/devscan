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

static std::string received;


static void receiveAndKeep(hbm::communication::MulticastServer* pMcs)
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
			std::cout << __FUNCTION__ <<  " " << std::to_string(errno) << " " << strerror(errno) << std::endl;
		} else {
			std::cout << __FUNCTION__ << " empty" << std::endl;
		}
	} while(result>=0);
}

static void receiveAndDiscard(hbm::communication::MulticastServer* pMcs)
{
	ssize_t result;
	do {
		char buf[1042];
		int adapterIndex;
		int ttl;
		result = pMcs->receiveTelegram(buf, sizeof(buf), adapterIndex, ttl);
	} while(result>=0);
}


BOOST_AUTO_TEST_CASE(start_send_stop_test)
{
	static const char MULTICASTGROUP[] = "239.255.77.177";
	static const unsigned int UDP_PORT = 22222;
	static const unsigned int CYCLECOUNT = 1000;

	static const std::string MSG = "testest";

	hbm::communication::NetadapterList adapters;
	hbm::communication::MulticastServer mcsReceiver(MULTICASTGROUP, UDP_PORT, adapters);
	hbm::communication::MulticastServer mcsSender(MULTICASTGROUP, UDP_PORT, adapters);

	mcsSender.start();
	mcsSender.addAllInterfaces();
	std::thread threadSender(std::bind(&receiveAndDiscard, &mcsSender));

	for (unsigned int i=0; i<CYCLECOUNT; ++i)
	{
		received.clear();
		mcsReceiver.start();
		mcsReceiver.addAllInterfaces();
		std::thread threadReceiver(std::bind(&receiveAndKeep, &mcsReceiver));
		mcsSender.send(MSG.c_str(), MSG.length());
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		mcsReceiver.stop();
		threadReceiver.join();
		BOOST_CHECK(MSG==received);

		std::cout << __FUNCTION__ << " " << i << std::endl;
	}

	mcsSender.stop();
	threadSender.join();

	std::cout << __FUNCTION__ << " done" << std::endl;
}
