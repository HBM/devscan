// Copyright 2014 Hottinger Baldwin Messtechnik
// Distributed under MIT license
// See file LICENSE provided

#include <iostream>
#include <sstream>
#include <string>
#include <functional>



#include "hbm/communication/netadapter.h"
#include "hbm/sys/eventloop.h"

#include "receiver.h"
#include "devicemonitor.h"

#include "defines.h"

namespace hbm {
	namespace devscan {

		Receiver::Receiver()
			: m_netadapterList()
			, m_scanner(ANNOUNCE_IPV4_ADDRESS, ANNOUNCE_UDP_PORT, m_netadapterList, m_eventloop, std::bind(&Receiver::receiveEventHandler, this))
			, m_timer(1000, true, m_eventloop, std::bind(&Receiver::retireEventHandler, this))
#ifndef _WIN32
			, m_netlink(m_netadapterList, m_scanner, m_eventloop)
#endif
		{
		}

//#ifndef _WIN32
//		ssize_t Receiver::netLinkEventHandler()
//		{
//			return m_netlink.process();
//		}
//#endif

		ssize_t Receiver::receiveEventHandler()
		{
			// receive announcement
			char readBuffer[communication::MAX_DATAGRAM_SIZE];
			int ttl;
			int adapterIndex;
			ssize_t nBytes = m_scanner.receiveTelegram(readBuffer, sizeof(readBuffer), adapterIndex, ttl);
//			std::cerr << "Receiver::receiveEventHandler len=" << nBytes << ": \"" << readBuffer << "\"" << std::endl;
			if(nBytes > 0) {
				std::string interfaceName = "Err";
				try {
					// getAdapterByInterfaceIndex may throw an exception, i.e. it does on 0
					communication::Netadapter adapter = communication::NetadapterList().getAdapterByInterfaceIndex(adapterIndex);
					interfaceName = adapter.getName();
				}
				catch(...) {
					interfaceName = "Undef";
					interfaceName.append(std::to_string(adapterIndex));
				}
				m_deviceMonitor.processReceivedAnnouncement(interfaceName, std::string(readBuffer, nBytes));
			}
			return nBytes;
		}

		ssize_t Receiver::retireEventHandler()
		{
			m_deviceMonitor.checkForExpiredAnnouncements();
			return 0;
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
			sys::EventLoop evl;

			m_scanner.start();
//			evl.addEvent(m_timer.getFd(), std::bind(&Receiver::retireEventHandler, this));
//#ifndef _WIN32
//			evl.addEvent(m_netlink.getFd(), std::bind(&Receiver::netLinkEventHandler, this));
//#endif
//			evl.addEvent(m_scanner.getFd(), std::bind(&Receiver::receiveEventHandler, this));

			m_scanner.addAllInterfaces();
			evl.execute();

#ifndef _WIN32
			m_netlink.stop();
#endif
			m_scanner.stop();
			m_timer.cancel();
		}


		void Receiver::start_for(std::chrono::milliseconds timeOfExecution)
		{
			sys::EventLoop evl;

			m_scanner.start();
//			evl.addEvent(m_timer.getFd(), std::bind(&Receiver::retireEventHandler, this));
//#ifndef _WIN32
//			evl.addEvent(m_netlink.getFd(), std::bind(&Receiver::netLinkEventHandler, this));
//#endif
//			evl.addEvent(m_scanner.getFd(), std::bind(&Receiver::receiveEventHandler, this));

			m_scanner.addAllInterfaces();
			evl.execute_for(timeOfExecution);

#ifndef _WIN32
			m_netlink.stop();
#endif
			m_scanner.stop();
			m_timer.cancel();
		}

		void Receiver::stop()
		{
			m_scanner.stop();
		}
	}
}
