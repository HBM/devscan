// Copyright 2014 Hottinger Baldwin Messtechnik
// Distributed under MIT license
// See file LICENSE provided


#ifndef SCANCLIENTTEST_H
#define SCANCLIENTTEST_H

#include <string>

#include <json/value.h>

#include "devscan/receiver.h"
#include "devscan/configureclient.h"


namespace hbm {
	namespace devscan {
		namespace test {

		class firstAnnouncment {
		public:
			firstAnnouncment();

			virtual ~firstAnnouncment();
			Json::Value get() const
			{
				return m_announcement;
			}
		protected:
			Receiver m_scanClient;
			Json::Value m_announcement;
		private:
			void collectFirstAnnouncement(const std::string uuid, const std::string& receivingInterfaceName, const std::string& sendingInterfaceName, const std::string& announcement);

		};


		/// waits for the desired uuid. the complete communication path is not being checked
		class specificAnnouncment {
		public:
			specificAnnouncment();
			specificAnnouncment(const std::string desiredUuid);

			virtual ~specificAnnouncment();

			/// the announcement parameters
			const Json::Value& announcementParameters() const
			{
				return m_announcement[hbm::jsonrpc::PARAMS];
			}
		protected:
			void refreshAnnouncment();

			Receiver m_scanClient;
			ConfigureClient configureClient;
			const std::string m_desiredUuid;
		private:
			void filterAnnouncements(const std::string uuid, const std::string& receivingInterfaceName, const std::string& sendingInterfaceName, const std::string& announcement);

			/// the complete JSON notifications
			Json::Value m_announcement;
		};

		}
	}
}
#endif // SCANCLIENTTEST_H
