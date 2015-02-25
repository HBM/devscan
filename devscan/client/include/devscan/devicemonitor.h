// Copyright 2014 Hottinger Baldwin Messtechnik
// Distributed under MIT license
// See file LICENSE provided

#ifndef _DEVICEMONITOR_H
#define _DEVICEMONITOR_H

#include <string>
#include <unordered_map>
#include <functional>
#include <chrono>


#include "receiver_if.h"


namespace hbm {
	namespace devscan {

		/// \brief this class is being used for processing received announcements
		/// It notifies new, changed and expired announcements.
		/// \warning not reentrant!
		class DeviceMonitor
		{
		public:
			DeviceMonitor();

			/// \brief sets the callback, that is to be executed on each new or changed announcement
			/// After setting the callback, the callback is called immeately for each known (previous) announcement.
			/// \param cb  user-provided callback function
			void setAnnounceCb(announceCb_t cb);

			/// set the callback, that is to be executed if an announcement expires
			/// \param cb  user-provided callback function
			void setExpireCb(expireCb_t cb);

			/// set the callback, that is to be executed if a problem with the message reveived via the network occured.
			/// \param cb  user-provided callback function
			void setErrorCb(errorCb_t cb);

			/// \brief handles every arriving network packet
			/// The same uuid might be received directly and via router.
			/// Hence we collect the announcements per receiving interface and
			/// use the uuid and the sending ip address as key.
			/// \param interfaceName  name of the networ interface which received the data
			/// \param message  the data, as it was received from the network
			/// \see setAnnounceCb
			/// \see setErrorCb
			void processReceivedAnnouncement(std::string interfaceName, const std::string &message);

			/// \brief checks all announcements for missing refresh, considering expiration time.
			/// walks through all known announcements. Each expired entry will
			/// be removed and expire callback will be fired.
			/// \see setExpireCb
			ssize_t checkForExpiredAnnouncements();

		private:
			struct expiringEntry {
				std::string announcement;
				std::chrono::steady_clock::time_point timeOfExpiry;
			};

			/// each announcement can be uniquely identified by the so called communication path.
			/// The communication path is made up of the uuid of the sending device, the address of the sending interface and the name of the receiving interface
			/// We choose the interface name instead of interface address because an interface might have several or no IP addresses, which is no problem for multicast communication.
			struct communicationPath {
				/// \throws on invalid communication path
				communicationPath(const std::string &communicationPath);

				std::string receivingInterface;
				std::string uuid;
				std::string sendingInterface;
			};

			/// objects must not be copied
			DeviceMonitor(const DeviceMonitor& op);

			/// objects must not be assigned
			DeviceMonitor& operator=(const DeviceMonitor& op);

			/// communication path (<receiving interface name>:<sending interface address>:<uuid of sender>) is key
			typedef std::unordered_map < std::string, expiringEntry > announcements_t;

			/// we keep all current announcements in order to detect changed announcements.
			/// \warning access is not synchronized!
			announcements_t m_announcements;

			announceCb_t m_announceCb;
			expireCb_t   m_expireCb;
			errorCb_t    m_errorCb;

			/// for convenience: report an 'internal' error via the error callback.
			/// Has no effect in case no error callback was set.
			void callErrorCb(uint32_t errorCode, const std::string& userMessage, const std::string& announcement);
		};
	}
}
#endif
