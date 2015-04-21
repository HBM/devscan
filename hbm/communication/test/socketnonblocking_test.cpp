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
#include "hbm/communication/tcpserver.h"
#include "hbm/communication/test/socketnonblocking_test.h"
#include "hbm/sys/eventloop.h"

namespace hbm {
	namespace communication {
		namespace test {

			static const unsigned int PORT = 22222;


			serverFixture::serverFixture()
				: m_server(m_eventloop)
			{
				BOOST_TEST_MESSAGE("setup Fixture1");
				int result = m_server.start(PORT, 3, std::bind(&serverFixture::acceptCb, this, std::placeholders::_1));
				BOOST_CHECK_NE(result, -1);
				m_serverWorker = std::thread(std::bind(&hbm::sys::EventLoop::execute, std::ref(m_eventloop)));
			}

			serverFixture::~serverFixture()
			{
				BOOST_TEST_MESSAGE("teardown Fixture1");
				m_workers.clear();
				m_eventloop.stop();
				m_serverWorker.join();
			}


			void serverFixture::acceptCb(workerSocket_t worker)
			{
				worker->setDataCb(std::bind(&serverFixture::serverEcho, this, std::placeholders::_1));

				m_workers.insert(std::move(worker));
			}

			void serverFixture::removeWorker(workerSocket_t worker)
			{

			}

			int serverFixture::serverEcho(hbm::communication::SocketNonblocking* pSocket)
			{
				char buffer[1024];
				ssize_t result;

				do {
					result = pSocket->receive(buffer, sizeof(buffer));
					if (result>0) {
						result = pSocket->sendBlock(buffer, result, false);
					} else if (result==0) {
						// socket got closed
					} else {
						if ((errno!=EAGAIN) && (errno!=EWOULDBLOCK)) {
							// a real error
						}
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

				hbm::sys::EventLoop eventloop;
				std::thread worker(std::bind(&hbm::sys::EventLoop::execute, std::ref(eventloop)));

				hbm::communication::SocketNonblocking client(eventloop);
				result = client.connect("127.0.0.1", std::to_string(PORT));
				BOOST_CHECK_NE(result, -1);
				client.setDataCb(std::bind(&serverFixture::clientReceive, this, std::placeholders::_1));

				clearAnswer();
				result = client.sendBlock(msg, sizeof(msg), false);
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
				BOOST_CHECK_EQUAL(result, sizeof(msg));
				client.disconnect();

				BOOST_CHECK_EQUAL(getAnswer(), msg);


				eventloop.stop();
				worker.join();
			}

			BOOST_AUTO_TEST_CASE(writev_test)
			{
				int result;
				static const size_t bufferSize = 100000;
				static const size_t blockCount = 10;
				static const size_t blockSize = bufferSize/blockCount;
				char buffer[bufferSize] = "a";

				hbm::communication::dataBlock_t dataBlocks[blockCount];

				for(unsigned int i=0; i<blockCount; ++i) {
					dataBlocks[i].size = blockSize;
					dataBlocks[i].pData = &buffer[i*blockSize];
				}

				hbm::sys::EventLoop eventloop;
				std::thread worker(std::bind(&hbm::sys::EventLoop::execute, std::ref(eventloop)));

				hbm::communication::SocketNonblocking client(eventloop);
				result = client.connect("127.0.0.1", std::to_string(PORT));
				BOOST_CHECK_NE(result, -1);

//				// force a small send buffer
//				int val;
//				socklen_t len;
//				result = getsockopt(client.getFd(), SOL_SOCKET, SO_SNDBUF, &val, &len);
//				val = 1;
//				result = setsockopt(client.getFd(), SOL_SOCKET, SO_SNDBUF, &val, sizeof(val));
//				BOOST_CHECK_NE(result, -1);
//				result = getsockopt(client.getFd(), SOL_SOCKET, SO_SNDBUF, &val, &len);
//				BOOST_CHECK_NE(result, -1);

				client.setDataCb(std::bind(&serverFixture::clientReceive, this, std::placeholders::_1));

				clearAnswer();
				result = client.sendBlocks(dataBlocks, blockCount);
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
				//BOOST_CHECK_EQUAL(result, sizeof(msg));
				client.disconnect();

				//BOOST_CHECK_EQUAL(getAnswer(), msg);


				eventloop.stop();
				worker.join();
			}


			BOOST_AUTO_TEST_SUITE_END()
		}
	}
}






