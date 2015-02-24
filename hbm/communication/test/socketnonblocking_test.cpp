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
#include "hbm/communication/tcpacceptor.h"
#include "hbm/communication/test/socketnonblocking_test.h"
#include "hbm/sys/eventloop.h"

namespace hbm {
	namespace communication {
		namespace test {

			static const unsigned int PORT = 22222;


			serverFixture::serverFixture()
				: m_acceptor(m_eventloop, std::bind(&serverFixture::acceptCb, this, std::placeholders::_1), std::bind(&serverFixture::echo, this, std::placeholders::_1))
			{
				BOOST_TEST_MESSAGE("setup Fixture1");
				int result = m_acceptor.start(PORT, 3);
				BOOST_CHECK_NE(result, -1);
				m_server = std::thread(std::bind(&hbm::sys::EventLoop::execute, std::ref(m_eventloop)));
			}

			serverFixture::~serverFixture()
			{
				BOOST_TEST_MESSAGE("teardown Fixture1");
				m_eventloop.stop();
				m_server.join();
			}


			void serverFixture::acceptCb(TcpAcceptor::worker_t worker)
			{
				m_workers.insert(std::move(worker));
			}

			int serverFixture::echo(hbm::communication::SocketNonblocking* pSocket)
			{
				char buffer[1024];
				ssize_t result;

				do {
					result = pSocket->receive(buffer, sizeof(buffer));
					if (result>0) {
						result = pSocket->sendBlock(buffer, result, false);
					}
				} while (result>0);
				return result;
			}



			static int receiver(hbm::communication::SocketNonblocking* pSocket)
			{
				char buffer[1024];
				ssize_t result;

				do {
					result = pSocket->receive(buffer, sizeof(buffer));
				} while (result>0);
				return result;
			}


			BOOST_FIXTURE_TEST_SUITE( socket_test, serverFixture )


			BOOST_AUTO_TEST_CASE(echo_test)
			{
				int result;
				const char msg[] = "hallo!";
				char answer[1024];

				hbm::sys::EventLoop eventloop;
				std::thread worker(std::bind(&hbm::sys::EventLoop::execute, std::ref(eventloop)));

				hbm::communication::SocketNonblocking client(eventloop, std::bind(&receiver, std::placeholders::_1));
				result = client.connect("127.0.0.1", std::to_string(PORT));

				result = client.sendBlock(msg, sizeof(msg), false);
				BOOST_CHECK_NE(result, -1);
				result = client.receiveComplete(answer, sizeof(msg));
				BOOST_CHECK_NE(result, -1);

				client.disconnect();


				eventloop.stop();
				worker.join();
			}

			//BOOST_AUTO_TEST_CASE(start_send_stop_test)
			//{
			//	static const unsigned int CYCLECOUNT = 1000;

			//	static const std::string MSG = "testest";
			//	char buffer[1024];

			//	std::thread echoThread(&echo);

			//	hbm::communication::SocketNonblocking caller;

			//	for (unsigned int i=0; i<CYCLECOUNT; ++i) {
			//		memset(buffer, 0, sizeof(buffer));
			//		caller.connect("127.0.0.1", std::to_string(PORT));
			//		caller.sendBlock(MSG.c_str(), MSG.length(), false);
			//		ssize_t result = caller.receiveComplete(buffer, MSG.length());
			//		BOOST_CHECK(MSG==std::string(buffer, static_cast < size_t > (result)));
			//		caller.stop();
			//	}
			//	caller.connect("127.0.0.1", std::to_string(PORT));
			//	caller.sendBlock(STP, strlen(STP), false);
			//	caller.stop();
			//	echoThread.join();
			//}


			BOOST_AUTO_TEST_SUITE_END()

		}
	}
}






