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
				: m_acceptor(m_eventloop)
			{
				BOOST_TEST_MESSAGE("setup Fixture1");
				int result = m_acceptor.start(PORT, 3, std::bind(&serverFixture::acceptCb, this, std::placeholders::_1), std::bind(&serverFixture::serverEcho, this, std::placeholders::_1));
				BOOST_CHECK_NE(result, -1);
				m_server = std::thread(std::bind(&hbm::sys::EventLoop::execute, std::ref(m_eventloop)));
			}

			serverFixture::~serverFixture()
			{
				BOOST_TEST_MESSAGE("teardown Fixture1");
				m_eventloop.stop();
				m_server.join();
			}


			void serverFixture::acceptCb(TcpAcceptor::workerSocket_t worker)
			{
				m_workers.insert(std::move(worker));
			}

			int serverFixture::serverEcho(hbm::communication::SocketNonblocking* pSocket)
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



			int serverFixture::clientReceive(hbm::communication::SocketNonblocking* pSocket)
			{
				char buffer[1024];
				ssize_t result;

				result = pSocket->receive(buffer, sizeof(buffer));
				if (result>0) {
					m_answer += buffer;
				}

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

				hbm::communication::SocketNonblocking client(eventloop);
				result = client.connect("127.0.0.1", std::to_string(PORT), std::bind(&serverFixture::clientReceive, this, std::placeholders::_1));

				cleaAnswer();
				result = client.sendBlock(msg, sizeof(msg), false);
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
				BOOST_CHECK_EQUAL(result, sizeof(msg));
				client.disconnect();

				BOOST_CHECK_EQUAL(getAnswer(), msg);


				eventloop.stop();
				worker.join();
			}


			BOOST_AUTO_TEST_SUITE_END()
		}
	}
}






