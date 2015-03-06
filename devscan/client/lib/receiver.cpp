// Copyright 2014 Hottinger Baldwin Messtechnik
// Distributed under MIT license
// See file LICENSE provided

#include <iostream>
#include <sstream>
#include <string>
#include <functional>



#include "hbm/sys/eventloop.h"

#include "receiver.h"
#include "devicemonitor.h"

#include "defines.h"

namespace hbm {
	namespace devscan {

		Receiver::Receiver()
			: m_netadapterList()
			, m_scanner(m_netadapterList, m_eventloop)
			, m_timer(m_eventloop)
#ifndef _WIN32
			, m_netlink(m_netadapterList, m_scanner, m_eventloop)
#endif
		{
		}

		ssize_t Receiver::receiveEventHandler(communication::MulticastServer* pMcs)
		{
			// receive announcement
			char readBuffer[communication::MAX_DATAGRAM_SIZE];
			int ttl;
			std::string adapterName;
			ssize_t nBytes = pMcs->receiveTelegram(readBuffer, sizeof(readBuffer), adapterName, ttl);
//			std::cerr << "Receiver::receiveEventHandler len=" << nBytes << ": \"" << readBuffer << "\"" << std::endl;
			if(nBytes > 0) {
				m_deviceMonitor.processReceivedAnnouncement(adapterName, std::string(readBuffer, nBytes));
			}
			return nBytes;
		}

		void Receiver::setAnnounceCb(announceCb_t cb)
		{
			m_deviceMonitor.setAnnounceCb(cb);
		}


		void Receiver::setExpireCb(expireCb_t cb)
		{
			m_deviceMonitor.setExpireCb(cb);
		}


		void Receiver::setErrorCb(errorCb_t cb)
		{
			m_deviceMonitor.setErrorCb(cb);
		}


		void Receiver::start()
		{
			m_scanner.start(ANNOUNCE_IPV4_ADDRESS, ANNOUNCE_UDP_PORT, std::bind(&Receiver::receiveEventHandler, this, std::placeholders::_1));
			m_timer.set(1000, true, std::bind(&DeviceMonitor::checkForExpiredTimerCb, std::ref(m_deviceMonitor), std::placeholders::_1));
			m_scanner.addAllInterfaces();
			m_eventloop.execute();
		}


		void Receiver::start_for(std::chrono::milliseconds timeOfExecution)
		{
			m_scanner.start(ANNOUNCE_IPV4_ADDRESS, ANNOUNCE_UDP_PORT, std::bind(&Receiver::receiveEventHandler, this, std::placeholders::_1));
			m_timer.set(1000, true, std::bind(&DeviceMonitor::checkForExpiredTimerCb, std::ref(m_deviceMonitor), std::placeholders::_1));
			m_scanner.addAllInterfaces();
			m_eventloop.execute_for(timeOfExecution);
		}

		void Receiver::stop()
		{
			m_eventloop.stop();
#ifndef _WIN32
			m_netlink.stop();
#endif
			m_scanner.stop();
			m_timer.cancel();

		}
	}
}
