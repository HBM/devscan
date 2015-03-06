// Copyright 2014 Hottinger Baldwin Messtechnik
// Distributed under MIT license
// See file LICENSE provided

#ifndef __HBM__COMMUNICATION_SOCKETTEST_H
#define __HBM__COMMUNICATION_SOCKETTEST_H

#include <thread>
#include <set>

#include "hbm/communication/socketnonblocking.h"
#include "hbm/communication/tcpserver.h"

#include "hbm/sys/eventloop.h"


namespace hbm {
	namespace communication {
		namespace test {

			class serverFixture {
			public:
				int clientReceive(hbm::communication::SocketNonblocking* pSocket);
			protected:
				serverFixture();
				virtual ~serverFixture();
				void acceptCb(workerSocket_t worker);
				int serverEcho(SocketNonblocking* pSocket);
				void removeWorker(workerSocket_t worker);
				void clearAnswer()
				{
					m_answer.clear();
				}

				std::string getAnswer() const
				{
					return m_answer;
				}

			private:
				std::string m_answer;
				std::set < workerSocket_t > m_workers;
				std::thread m_serverWorker;
				sys::EventLoop m_eventloop;
				TcpServer m_server;
			};
		}
	}
}

#endif
