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

			if (tokens.size() != 4) {
				throw std::runtime_error("invalid communication path");
			}

			receivingInterface = tokens[0];
			sendingInterface = tokens[1];
			uuid = tokens[2];
			router = tokens[3];
		}

		DeviceMonitor::DeviceMonitor()
			: m_announcements()
			, m_announceCb(announceCb_t())
			, m_expireCb(expireCb_t())
			, m_errorCb(errorCb_t())
		{
		}

		void DeviceMonitor::setAnnounceCb(announceCb_t cb)
		{
			m_announceCb = cb;
			if (m_announceCb) {
				for (announcements_t::iterator iter=m_announcements.begin(); iter!=m_announcements.end(); ++iter) {
					try {
						communicationPath path(iter->first);
						m_announceCb(path.uuid, path.receivingInterface, path.sendingInterface, path.router, iter->second.announcement);
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

		void DeviceMonitor::processReceivedAnnouncement(std::string receivingInterfaceName, const std::string &message)
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

				const Json::Value& params = announcement[hbm::jsonrpc::PARAMS];

				// this is an announcement!
				std::string sendingInterfaceName = params[TAG_NetSettings][TAG_Interface][TAG_Name].asString();
				if (sendingInterfaceName.empty()) {
					callErrorCb(cb_t::DATA_DROPPED | cb_t::ERROR_IPADDR, "Missing interface name in JSON-document", message);
					return;
				}

				std::string sendingUuid(params[TAG_Device][TAG_Uuid].asString());
				if (sendingUuid.empty()) {
					callErrorCb(cb_t::DATA_DROPPED | cb_t::ERROR_UUID, "Missing uuid in JSON-document", message);
					return;
				}

				std::string router(params[TAG_Router][TAG_Uuid].asString());
				std::string key(receivingInterfaceName+":"+sendingInterfaceName+":"+sendingUuid+":"+router);

				std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
				std::chrono::seconds expire(params[TAG_Expiration].asUInt());
				if (expire.count() == 0 ) {
					callErrorCb(cb_t::DATA_DROPPED | cb_t::ERROR_EXPIRE, "Missing expiration in JSON-document", message);
					return;
				}

//				callErrorCb(0, "received", message);

				announcements_t::iterator iter = m_announcements.find(key);
				if (iter!=m_announcements.end()) {
					// update existing entry
					expiringEntry& currentEntry = iter->second;
					currentEntry.timeOfExpiry = now + expire;
					if (message!=currentEntry.announcement) {
						// something has changed
						currentEntry.announcement = message;
						if (m_announceCb) {
							m_announceCb(sendingUuid, receivingInterfaceName, sendingInterfaceName, router, message);
						}
					}
				} else {
					// new entry
					expiringEntry entry;
					entry.announcement = message;
					entry.timeOfExpiry = now + expire;
					m_announcements[key] = entry;
					if (m_announceCb) {
						m_announceCb(sendingUuid, receivingInterfaceName, sendingInterfaceName, router, message);
					}
				}
			}
			catch(std::exception &) {
				callErrorCb(cb_t::DATA_DROPPED | cb_t::E_EXCEPTION1, "Receiving error 1", message);
			}
			catch(...) {
				callErrorCb(cb_t::DATA_DROPPED | cb_t::E_EXCEPTION2, "Receiving error 2", message);
			}
		}

		void DeviceMonitor::checkForExpiredTimerCb(bool fired)
		{
			if (fired==false) {
				return;
			}

			checkForExpiredAnnouncements();
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
							m_expireCb(path.uuid, path.receivingInterface, path.sendingInterface, path.router);
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
