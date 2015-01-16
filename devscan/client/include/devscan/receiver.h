// Copyright 2014 Hottinger Baldwin Messtechnik
// Distributed under MIT license
// See file LICENSE provided

#ifndef _RECEIVER_H
#define _RECEIVER_H

#include <string>
#include <chrono>


#include "hbm/communication/multicastserver.h"
#include "hbm/communication/netadapterlist.h"
#ifndef _WIN32
#include "hbm/communication/netlink.h"
#endif
#include "hbm/sys/timer.h"


#include "receiver_if.h"
#include "devicemonitor.h"


namespace hbm {
	namespace devscan {


		/// \class Receiver
		/// \brief The main class for HBM Scan Clients.
		/// It receives announcements interprets them and notifies about new,
		/// changed and expired announcements.
		class Receiver: public ReceiverIf
		{
		public:
			Receiver();

			/// set the callback method to be called on arrival of new or change of an already known announcement
			void setAnnounceCb(announceCb_t cb);
			/// set the callback method to be called on expiration of an announcement
			void setExpireCb(expireCb_t cb);
			/// set the callback method to be called on errors in the received network telegram
			void setErrorCb(errorCb_t cb);

			/// Start event loop that collects announcements, retires expired announcements.
			/// Under Linux network events (like new network interfaces) are handled too.
			/// Returns after the specified time, on execution of stop() or if an error occurs.
			/// \param timeOfExecution amount of time in ms to execute. 0 for indefinite
			void start(std::chrono::milliseconds timeOfExecution=std::chrono::milliseconds(0));

			void stop();

		private:
			communication::NetadapterList m_netadapterList;
			communication::MulticastServer m_scanner;
			sys::Timer m_timer;
			DeviceMonitor m_deviceMonitor;

#ifndef _WIN32
			Netlink m_netlink;

			ssize_t netLinkEventHandler();
#endif
			ssize_t receiveEventHandler();
			ssize_t retireEventHandler();
		};
	}
}
#endif
