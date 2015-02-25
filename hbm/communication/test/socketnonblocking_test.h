// Copyright 2014 Hottinger Baldwin Messtechnik
// Distributed under MIT license
// See file LICENSE provided


#ifndef __HBM__COMMUNICATION__SOCKETNONBLOCKING_TEST_H
#define __HBM__COMMUNICATION__SOCKETNONBLOCKING_TEST_H

//#ifndef _WIN32
//#define BOOST_TEST_DYN_LINK
//#endif
//#define BOOST_TEST_MAIN

//#define BOOST_TEST_MODULE socketnonblocking tests

//#include <boost/test/unit_test.hpp>

//#include <string>
#include <thread>
//#include <functional>
#include <memory>


#include "hbm/communication/socketnonblocking.h"
#include "hbm/communication/tcpacceptor.h"
#include "hbm/sys/eventloop.h"

#include <set>

namespace hbm {
	namespace communication {
		namespace test {

			struct serverFixture
			{
			public:
				serverFixture();
				virtual ~serverFixture();

				int clientReceive(hbm::communication::SocketNonblocking* pSocket);

				void cleaAnswer()
				{
					m_answer.clear();
				}

				std::string getAnswer() const
				{
					return m_answer;
				}

			private:
				typedef std::set < TcpAcceptor::workerSocket_t > workers_t;
				void acceptCb(TcpAcceptor::workerSocket_t worker);
				int serverEcho(hbm::communication::SocketNonblocking* pSocket);

				hbm::sys::EventLoop m_eventloop;
				hbm::communication::TcpAcceptor m_acceptor;
				workers_t m_workers;
				std::thread m_server;

				std::string m_answer;
			};
		}
	}
}
#endif
