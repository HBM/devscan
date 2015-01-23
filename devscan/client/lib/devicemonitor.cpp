// Copyright 2014 Hottinger Baldwin Messtechnik
// Distributed under MIT license
// See file LICENSE provided

#include <cstddef>
#include <iostream>
#include <sstream>
#include <exception>
#include <vector>
#include <chrono>

#include <json/value.h>
#include <json/reader.h>

#include "hbm/string/split.h"

#include "hbm/exception/exception.hpp"
#include "hbm/jsonrpc/jsonrpc_defines.h"

#include "devicemonitor.h"
#include "defines.h"


namespace hbm {
	namespace devscan {
		DeviceMonitor::communicationPath::communicationPath(const std::string &communicationPath)
		{
			std::vector < std::string > tokens = hbm::string::split(communicationPath, ":");

			if (tokens.size() != 3) {
				throw std::runtime_error("invalid communication path");
			}

			receivingInterface = tokens[0];
			sendingInterface = tokens[1];
			uuid = tokens[2];
		}

		DeviceMonitor::DeviceMonitor()
			: m_announcements(),
			  m_announceCb(NULL),
			  m_expireCb(NULL),
			  m_errorCb(NULL)
		{
		}

		void DeviceMonitor::setAnnounceCb(announceCb_t cb)
		{
			m_announceCb = cb;
			if (m_announceCb) {
				for (announcements_t::iterator iter=m_announcements.begin(); iter!=m_announcements.end(); ++iter) {
					try {
						communicationPath path(iter->first);
						m_announceCb(path.uuid, path.receivingInterface, path.sendingInterface, iter->second.announcement);
					} catch(...)
					{
					}
				}
			}
		}

		void DeviceMonitor::setExpireCb(expireCb_t cb)
		{
			m_expireCb = cb;
		}

		void DeviceMonitor::setErrorCb(errorCb_t cb)
		{
			m_errorCb = cb;
		}

		/// Notify the client about an error
		void DeviceMonitor::callErrorCb(uint32_t errorCode, const std::string& userMessage, const std::string& announcement)
		{
			if(m_errorCb)
			{
				try	{
					m_errorCb(errorCode, userMessage, announcement);
				}
				catch(...) {
					// there is nothing left we can do here!
				}
			}
		}

		void DeviceMonitor::processReceivedAnnouncement(std::string interfaceName, const std::string &message)
		{
			Json::Value announcement;

			try {
				if ( ! Json::Reader().parse(message, announcement)) {
					callErrorCb(cb_t::DATA_DROPPED | cb_t::ERROR_PARSE, "JSON parser failed", message);
					return;
				}
				if ( ! (announcement[hbm::jsonrpc::METHOD].asString()==TAG_Announce) ) {
					callErrorCb(cb_t::DATA_DROPPED | cb_t::ERROR_METHOD, "Missing \"announcement\" in JSON-document", message);
					return;
				}
				// this is an annoucement!
				const Json::Value& ipV4Node = announcement[hbm::jsonrpc::PARAMS][TAG_NetSettings][TAG_Interface][TAG_ipV4];
				if (ipV4Node.empty()) {
					callErrorCb(cb_t::DATA_DROPPED | cb_t::ERROR_IPADDR, "Missing ip-address in JSON-document", message);
					return;
				}

				// it would be better to use the ipv6 link local address here. As long our ip over firewire driver for windows does not support ipv6 we have to use the ipv4 address.
				std::string sendingInterfaceAddress(ipV4Node[0][TAG_address].asString());
				if (sendingInterfaceAddress.size() == 0) {
					callErrorCb(cb_t::DATA_DROPPED | cb_t::ERROR_IPADDR, "Missing ip-address in JSON-document", message);
					return;
				}
				std::string sendingUuid(announcement[hbm::jsonrpc::PARAMS][TAG_Device][TAG_Uuid].asString());
				if (sendingUuid.size() == 0) {
					callErrorCb(cb_t::DATA_DROPPED | cb_t::ERROR_UUID, "Missing uuid in JSON-document", message);
					return;
				}
				std::string key(interfaceName+":"+sendingInterfaceAddress+":"+sendingUuid);

				std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
				std::chrono::seconds expire(announcement[hbm::jsonrpc::PARAMS][TAG_Expiration].asUInt());
				if (expire.count() == 0 ) {
					callErrorCb(cb_t::DATA_DROPPED | cb_t::ERROR_EXPIRE, "Missing expiration in JSON-document", message);
					return;
				}

				announcements_t::iterator iter = m_announcements.find(key);
				if (iter!=m_announcements.end()) {
					// update existing entry
					expiringEntry& currentEntry = iter->second;
					currentEntry.timeOfExpiry = now + expire;
					if (message!=currentEntry.announcement) {
						// something has changed
						currentEntry.announcement = message;
						if (m_announceCb) {
							m_announceCb(sendingUuid, interfaceName, sendingInterfaceAddress, message);
						}
					}
				} else {
					// new entry
					expiringEntry entry;
					entry.announcement = message;
					entry.timeOfExpiry = now + expire;
					m_announcements[key] = entry;
					if (m_announceCb) {
						m_announceCb(sendingUuid, interfaceName, sendingInterfaceAddress, message);
					}
				}
			}
			catch(std::exception &e) {
				callErrorCb(cb_t::DATA_DROPPED | cb_t::E_EXCEPTION1, "Receiving error 1", message);
			}
			catch(...) {
				callErrorCb(cb_t::DATA_DROPPED | cb_t::E_EXCEPTION2, "Receiving error 2", message);
			}
		}

		void DeviceMonitor::checkForExpiredAnnouncements()
		{
			std::chrono::steady_clock::time_point timeNow = std::chrono::steady_clock::now();

			announcements_t::iterator iter=m_announcements.begin();
			while (iter!=m_announcements.end()) {
				if (iter->second.timeOfExpiry< timeNow) {
					if (m_expireCb) {
						try {
							communicationPath path(iter->first);
							m_expireCb(path.uuid, path.receivingInterface, path.sendingInterface);
						} catch(...)
						{
						}
					}
					iter = m_announcements.erase(iter);
				} else {
					++iter;
				}
			}
		}
	}
}
