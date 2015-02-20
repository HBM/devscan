// Copyright 2014 Hottinger Baldwin Messtechnik
// Distributed under MIT license
// See file LICENSE provided


#ifndef _WIN32
#define BOOST_TEST_DYN_LINK
#endif
#define BOOST_TEST_MAIN

#define BOOST_TEST_MODULE socketnonblocking tests

#include <boost/test/unit_test.hpp>

#include <string>
#include <thread>
#include <functional>
#include <memory>


#include "hbm/communication/socketnonblocking.h"


static const unsigned int PORT = 22222;
static const char STP[] = "STP";


static void echo()
{
	char buffer[1024];
	ssize_t result;
	size_t len;
	hbm::communication::SocketNonblocking listeningFd;
	listeningFd.bind(PORT);
	do {
		listeningFd.listenToClient();
		std::unique_ptr < hbm::communication::SocketNonblocking > receiver = listeningFd.acceptClient();

		do {
			memset(buffer, 0, sizeof(buffer));
			result = receiver->receive(buffer, sizeof(buffer));
			if (result<=0) {
				break;
			}

			if (strcmp(buffer,STP)==0) {
				// finished!
				return;
			}

			len = result;
			result = receiver->sendBlock(buffer, len, false);
			if (result!=static_cast < ssize_t > (len)) {
				break;
			}

		} while (true);
	} while (true);
}

static void receiver(hbm::communication::SocketNonblocking* pSocket)
{
	char buffer[1024];
	ssize_t result;

	do {
		result = pSocket->receive(buffer, sizeof(buffer));
	} while (result>0);
}

BOOST_AUTO_TEST_CASE(stop_receiver_test)
{
	static const unsigned int CYCLECOUNT = 1000;

	std::thread echoThread(&echo);
	hbm::communication::SocketNonblocking receiverSocket;
	hbm::communication::SocketNonblocking caller;

	for (unsigned int i=0; i<CYCLECOUNT; ++i) {
		receiverSocket.connect("127.0.0.1", std::to_string(PORT));
		std::thread receiverThread(std::bind(&receiver, &receiverSocket));
		receiverSocket.stop();

		receiverThread.join();
	}

	caller.connect("127.0.0.1", std::to_string(PORT));
	caller.sendBlock(STP, strlen(STP), false);
	caller.stop();
	echoThread.join();
}

BOOST_AUTO_TEST_CASE(start_send_stop_test)
{
	static const unsigned int CYCLECOUNT = 1000;

	static const std::string MSG = "testest";
	char buffer[1024];

	std::thread echoThread(&echo);

	hbm::communication::SocketNonblocking caller;

	for (unsigned int i=0; i<CYCLECOUNT; ++i) {
		memset(buffer, 0, sizeof(buffer));
		caller.connect("127.0.0.1", std::to_string(PORT));
		caller.sendBlock(MSG.c_str(), MSG.length(), false);
		ssize_t result = caller.receiveComplete(buffer, MSG.length());
		BOOST_CHECK(MSG==std::string(buffer, static_cast < size_t > (result)));
		caller.stop();
	}
	caller.connect("127.0.0.1", std::to_string(PORT));
	caller.sendBlock(STP, strlen(STP), false);
	caller.stop();
	echoThread.join();
}


