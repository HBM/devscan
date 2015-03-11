// Copyright 2014 Hottinger Baldwin Messtechnik
// Distributed under MIT license
// See file LICENSE provided

#include <iostream>
#include <vector>
#include <ctime>
#include <functional>
#include <chrono>

#ifndef _WIN32
#define BOOST_TEST_DYN_LINK
#endif
#define BOOST_TEST_MODULE scanClientTest
#include <boost/test/unit_test.hpp>

#include <json/value.h>
#include <json/writer.h>
#include <json/reader.h>

#include "hbm/communication/netadapter.h"
#include "hbm/communication/netadapterlist.h"

#include "hbm/jsonrpc/jsonrpc_defines.h"

#include "devscan/receiver.h"
#include "devscan/configureclient.h"

#include "defines.h"

#include "devicemonitortest.h"

#include "devscan/devicemonitor.h"


namespace hbm {
	namespace devscan {
		namespace test {

			/// Test cases for the DeviceMonitor.
			struct FixtureDeviceMonitor
			{
				FixtureDeviceMonitor() : m_deviceMonitor()
				{
					BOOST_TEST_MESSAGE("setup FixtureDeviceMonitor");
					std::cerr << "LOG: setup FixtureDeviceMonitor" << std::endl ;
					countAnnouncement = 0;
					lastAnnouncement = "";
					countExpiration = 0;
					countErrors = 0;
					lastErrorCode = 0;
				}

				~FixtureDeviceMonitor()
				{
					BOOST_TEST_MESSAGE("teardown FixtureDeviceMonitor");
					std::cerr << "LOG: teardown FixtureDeviceMonitor" << std::endl ;
				}

				DeviceMonitor m_deviceMonitor;


                          static unsigned int  countAnnouncement;
                          static std::string   lastAnnouncedUuid;
                          static std::string   lastReceivingInterfaceName;
                          static std::string   lastRouter;
                          static std::string   lastAnnouncement;

				/// called on new or changed announcement
                          static void announceCbTest(const std::string& uuid, const std::string& receivingInterfaceName, const std::string& sendingInterfaceName, const std::string& router, const std::string& announcement)
				{
					BOOST_TEST_MESSAGE("announceCb called");
					countAnnouncement++;
					lastAnnouncedUuid = uuid;
					lastReceivingInterfaceName = receivingInterfaceName;
                                        lastRouter = router;
					lastAnnouncement = announcement;
	//				std::cerr << "LOG: announceCb called" << std::endl ;
				}

				/// \throws (int)0
				static void announceCbThrows(const std::string& uuid, const std::string& receivingInterfaceName, const std::string& sendingInterfaceName, const std::string& router, const std::string& announcement)
				{
					throw 0;
				}

				static unsigned int  countExpiration;
				static std::string   lastExpiredUuid;

				/// called on exipry of an announcement
				static void expireCbTest(const std::string& uuid, const std::string& receivingInterfaceName, const std::string& sendingInterfaceName, const std::string& router)
				{
					countExpiration++;
					lastExpiredUuid = uuid;
					std::cerr << "expired: " << uuid << " " << receivingInterfaceName << " " << sendingInterfaceName << std::endl;
				}

				/// \throws (const char*)0
				static void expireCbThrows(const std::string& uuid, const std::string& receivingInterfaceName, const std::string& sendingInterfaceName, const std::string& router)
				{
					throw (const char*)0;
				}


				static unsigned int  countErrors;
				static uint32_t      lastErrorCode;

				/// called on an error during message processing
				static void errorCbTest(uint32_t errorCode, const std::string& userMessage, const std::string& announcement)
				{
					countErrors++;
					lastErrorCode = errorCode;
					std::cerr << "Error " << errorCode << ": " << userMessage << " \"" << announcement << "\"" << std::endl;
				}

				/// \throws
				static void errorCbThrows(uint32_t errorCode, const std::string& userMessage, const std::string& announcement)
				{
					throw new std::exception;
				}


				static const std::string minimalMessage;
				static const std::string validMessage;



				/// Compose a minimal Json announcement message.
				/// \see static std::string getJsonAnnouncementString(unsigned int expiration, const char * const uuid, const char * const ipaddress)
				///
				/// \param expiration  Set to -999 to omit the corresponding json-node
				/// \param uuid  Set to NULL to omit the corresponding json-node
				/// \param ipaddress  Set to NULL to omit the corresponding json-node
				/// \return The composed json-message
				static Json::Value getJsonAnnouncement(unsigned int expiration, const char * const uuid, const char * const interfaceName)
				{
					Json::Value jsonAnnouncement;
					jsonAnnouncement[hbm::jsonrpc::METHOD]  = TAG_Announce;
					if(expiration != -999)
					{
						jsonAnnouncement[hbm::jsonrpc::PARAMS][TAG_Expiration] = expiration;
					}
					if(uuid)
					{
						jsonAnnouncement[hbm::jsonrpc::PARAMS][TAG_Device][TAG_Uuid] = uuid;
					}
					if(interfaceName)
					{
						jsonAnnouncement[hbm::jsonrpc::PARAMS][TAG_NetSettings][TAG_Interface][TAG_Name] = interfaceName;
					}

					return jsonAnnouncement;
				}

				/// Compose a minimal Json announcement message.
				/// \see static std::string getJsonAnnouncementString(unsigned int expiration, const char * const uuid, const char * const ipaddress)
				///
				/// \param expiration  Set to -999 to omit the corresponding json-node
				/// \param uuid  Set to NULL to omit the corresponding json-node
				/// \param ipaddress  Set to NULL to omit the corresponding json-node
				/// \return The string representation of the composed json-message
				static std::string getJsonAnnouncementString(unsigned int expiration, const char * const uuid, const char * const ipaddress)
				{
					Json::Value jsonAnnouncement = getJsonAnnouncement(expiration, uuid, ipaddress);

					Json::StyledWriter writer;
					std::string message = writer.write(jsonAnnouncement);
					return message;
				}
			};

			unsigned int  FixtureDeviceMonitor::countAnnouncement;
			std::string   FixtureDeviceMonitor::lastAnnouncedUuid;
			std::string   FixtureDeviceMonitor::lastReceivingInterfaceName;
			std::string   FixtureDeviceMonitor::lastRouter;
			std::string   FixtureDeviceMonitor::lastAnnouncement;

			unsigned int  FixtureDeviceMonitor::countExpiration;
			std::string   FixtureDeviceMonitor::lastExpiredUuid;

			unsigned int  FixtureDeviceMonitor::countErrors;
			uint32_t      FixtureDeviceMonitor::lastErrorCode;

			const std::string FixtureDeviceMonitor::minimalMessage =
				"{"
					"\"method\":\"announce\","  // mandatory
					"\"params\":"
					"{"
						"\"device\":"
						"{"
							"\"uuid\":\"0009E5111111\""  // mandatory (for key)
						"},"
						"\"expiration\":15,"  // mandatory
						"\"netSettings\":"
						"{"
							"\"interface\":"
							"{"
								"\"name\":\"eth0\""  // mandatory (for key), sendingInterfaceName
							"}"
						"}"
					"}"
				"}"
				"\n";


			const std::string FixtureDeviceMonitor::validMessage =
				"{"
					"\"jsonrpc\":\"2.0\","
					"\"method\":\"announce\","  // mandatory
					"\"params\":"
					"{"
					  "\"apiVersion\":\"1.0\","
					  "\"device\":"
					  "{"
						"\"familyType\":\"QuantumX\","
						"\"firmwareVersion\":\"4.1.3.21715\","
						"\"hardwareId\":\"MX440A_R0\","
						"\"name\":\"TA02_MX440A\","
						"\"type\":\"MX440A\","
						"\"uuid\":\"0009E5001C49\""  // mandatory (for key)
					  "},"
					  "\"expiration\":15,"  // mandatory
					  "\"netSettings\":"
					  "{"
						"\"defaultGateway\":"
						"{"
						  "\"ipv4Address\":\"172.19.169.254\""
						"},"
						"\"interface\":"
						"{"
						  "\"configurationMethod\":\"dhcp\",\"description\":\"ethernet front side\","
						  "\"ipv4\":"  // mandatory
						  "["
							"{"
								"\"address\":\"172.19.191.121\","
							  "\"netmask\":\"255.255.0.0\""
							"},"
							"{\"address\":\"169.254.80.58\",\"netmask\":\"255.255.0.0\"}"
						  "],"
						  "\"ipv6\":"
						  "["
							"{\"address\":\"fe80::209:e5ff:fe00:13c4\",\"prefix\":64}"
						  "],"
							"\"name\":\"eth1\","   // mandatory (for key), sendingInterfaceName
							"\"type\":\"ethernet\""
						"}"
					  "},"
					  "\"router\":"
					  "{"
						"\"uuid\":\"0009E50013C3\""
					  "},"
					  "\"services\":"
					  "["
						"{\"port\":50320,\"type\":\"daqStream\"},{\"port\":50321,\"type\":\"daqStreamWS\"},{\"port\":50322,\"type\":\"hbmProtocol\"},{\"port\":50323,\"type\":\"http\"},{\"port\":50324,\"type\":\"jetd\"},"
						"{\"port\":50325,\"type\":\"jetws\"},{\"port\":50326,\"type\":\"ssh\"}"
					  "]"
					"}"
				"}"
				"\n";


			BOOST_FIXTURE_TEST_SUITE( DeviceMonitor_1, FixtureDeviceMonitor )

			/*
			 * This tests the most basic functionality of the DeviceMonitor.
			 * No nasty things. Everything should work.
			 */

			/// 1. register an announcement callback
			/// 2. inject a valid message (with a valid interface index)
			/// The message should be forwarded to the callback
			BOOST_AUTO_TEST_CASE( test_case_ok_1 )
			{
				m_deviceMonitor.setAnnounceCb(announceCbTest);
				BOOST_CHECK_EQUAL(countAnnouncement, 0);
				BOOST_CHECK_EQUAL(lastAnnouncement, "");

				m_deviceMonitor.processReceivedAnnouncement("eth9", minimalMessage);
				BOOST_CHECK_EQUAL(countAnnouncement, 1);
				BOOST_CHECK_EQUAL(lastAnnouncement, minimalMessage);

				m_deviceMonitor.processReceivedAnnouncement("eth9", validMessage);
				BOOST_CHECK_EQUAL(countAnnouncement, 2);
				BOOST_CHECK_EQUAL(lastAnnouncement, validMessage);
			}

			/// like test_case_ok_1, but the 'other way round'
			/// 1. inject a valid message (with a valid interface index)
			/// 2. register an announcement callback
			/// The message should be forwarded to the callback
			BOOST_AUTO_TEST_CASE( test_case_ok_2 )
			{
				m_deviceMonitor.processReceivedAnnouncement("eth9", validMessage);
				BOOST_CHECK_EQUAL(countAnnouncement, 0);
				BOOST_CHECK_EQUAL(lastAnnouncement, "");
				m_deviceMonitor.setAnnounceCb(announceCbTest);
				BOOST_CHECK_EQUAL(countAnnouncement, 1);
				BOOST_CHECK_EQUAL(lastAnnouncement, validMessage);
			}

			/// Test: send multiple identical announcements.
			/// Expected: notification call-back is called exactly once
			BOOST_AUTO_TEST_CASE( test_case_repeated_1 )
			{
				m_deviceMonitor.setAnnounceCb(announceCbTest);
				BOOST_CHECK_EQUAL(countAnnouncement, 0);
				BOOST_CHECK_EQUAL(lastAnnouncement, "");
				int count = 1000;
				for(int i=0 ; i<count ; i++)
				{
					m_deviceMonitor.processReceivedAnnouncement("eth9", validMessage);
					/* as long as the messages are equal they are not forwarded to the listeners */
					BOOST_CHECK_EQUAL(countAnnouncement, 1);
					BOOST_CHECK_EQUAL(lastAnnouncement, validMessage);
				}
			}

			/// Test: some uncommon or broken interface names
			BOOST_AUTO_TEST_CASE( test_case_invalid_interface )
			{
				std::string testname="";

				int callcount = 0;

				m_deviceMonitor.setAnnounceCb(announceCbTest);
				BOOST_CHECK_EQUAL(countAnnouncement, callcount++);
				BOOST_CHECK_EQUAL(lastAnnouncement, "");

				m_deviceMonitor.setErrorCb(errorCbTest);
				BOOST_CHECK_EQUAL(countErrors, 0);

				testname = ""; /*empty string*/
				m_deviceMonitor.processReceivedAnnouncement(testname, validMessage);
				BOOST_CHECK_EQUAL(countAnnouncement, callcount++);
				BOOST_CHECK_EQUAL(lastReceivingInterfaceName, testname);
				BOOST_CHECK_EQUAL(lastAnnouncement, validMessage);
				BOOST_CHECK_EQUAL(countErrors, 0);

				testname = " "; /*a single space*/
				m_deviceMonitor.processReceivedAnnouncement(testname, validMessage);
				BOOST_CHECK_EQUAL(countAnnouncement, callcount++);
				BOOST_CHECK_EQUAL(lastReceivingInterfaceName, testname);
				BOOST_CHECK_EQUAL(lastAnnouncement, validMessage);
				BOOST_CHECK_EQUAL(countErrors, 0);

				testname = "\a\f\n\t\"\'\\\001"; /*some ugly chars*/
				m_deviceMonitor.processReceivedAnnouncement(testname , validMessage);
				BOOST_CHECK_EQUAL(countAnnouncement, callcount++);
				BOOST_CHECK_EQUAL(lastReceivingInterfaceName, testname);
				BOOST_CHECK_EQUAL(lastAnnouncement, validMessage);
				BOOST_CHECK_EQUAL(countErrors, 0);

				testname = "some rather long interface name";
				m_deviceMonitor.processReceivedAnnouncement(testname, validMessage);
				BOOST_CHECK_EQUAL(countAnnouncement, callcount++);
				BOOST_CHECK_EQUAL(lastReceivingInterfaceName, testname);
				BOOST_CHECK_EQUAL(lastAnnouncement, validMessage);
				BOOST_CHECK_EQUAL(countErrors, 0);
			}

			/// Test: some broken JSON messages: No JSON at all
			BOOST_AUTO_TEST_CASE( test_case_no_json )
			{
				int errorcount = 0;

				m_deviceMonitor.setAnnounceCb(announceCbTest);
				BOOST_CHECK_EQUAL(countAnnouncement, 0);
				BOOST_CHECK_EQUAL(lastAnnouncement, "");

				m_deviceMonitor.setErrorCb(errorCbTest);
				BOOST_CHECK_EQUAL(countErrors, 0);

				m_deviceMonitor.processReceivedAnnouncement("eth_some_long_name", /* empty string */ "");
				BOOST_CHECK_EQUAL(countAnnouncement, 0);
				BOOST_CHECK_EQUAL(lastAnnouncement, "");
				BOOST_CHECK_EQUAL(countErrors, ++errorcount);
				BOOST_CHECK_EQUAL(lastErrorCode, cb_t::DATA_DROPPED | cb_t::ERROR_PARSE);

				m_deviceMonitor.processReceivedAnnouncement("eth_some_long_name", /* a single space */ " ");
				BOOST_CHECK_EQUAL(countAnnouncement, 0);
				BOOST_CHECK_EQUAL(lastAnnouncement, "");
				BOOST_CHECK_EQUAL(countErrors, ++errorcount);
				BOOST_CHECK_EQUAL(lastErrorCode, cb_t::DATA_DROPPED | cb_t::ERROR_PARSE);

				m_deviceMonitor.processReceivedAnnouncement("eth_some_long_name", "This is not valid JSON");
				BOOST_CHECK_EQUAL(countAnnouncement, 0);
				BOOST_CHECK_EQUAL(lastAnnouncement, "");
				BOOST_CHECK_EQUAL(countErrors, ++errorcount);
				BOOST_CHECK_EQUAL(lastErrorCode, cb_t::DATA_DROPPED | cb_t::ERROR_PARSE);
			}

			/// Test: some broken JSON messages: JSON, but missing something
			BOOST_AUTO_TEST_CASE( test_case_json_invalid )
			{
				int errorcount = 0;

				m_deviceMonitor.setAnnounceCb(announceCbTest);
				BOOST_CHECK_EQUAL(countAnnouncement, 0);
				BOOST_CHECK_EQUAL(lastAnnouncement, "");

				m_deviceMonitor.setErrorCb(errorCbTest);
				BOOST_CHECK_EQUAL(countErrors, 0);

				std::string message;

				// Test with a valid message first
				message = getJsonAnnouncementString(15, "112233445566", "eth0");
				m_deviceMonitor.processReceivedAnnouncement("eth_test", message);
				BOOST_CHECK_EQUAL(countAnnouncement, 1);
				BOOST_CHECK_EQUAL(lastAnnouncement, message);
				BOOST_CHECK_EQUAL(countErrors, 0);

				// omit expiration:  announcement callback will not be called
				message = getJsonAnnouncementString(-999, "112233445566", "eth0");
				m_deviceMonitor.processReceivedAnnouncement("eth_test", message);
				BOOST_CHECK_EQUAL(countAnnouncement, 1);
				BOOST_CHECK_EQUAL(countErrors, ++errorcount);
				BOOST_CHECK_EQUAL(lastErrorCode, cb_t::DATA_DROPPED | cb_t::ERROR_EXPIRE);

				// omit uuid:  announcement callback will not be called
				message = getJsonAnnouncementString(20, NULL, "eth0");
				m_deviceMonitor.processReceivedAnnouncement("eth_test", message);
				BOOST_CHECK_EQUAL(countAnnouncement, 1);
				BOOST_CHECK_EQUAL(countErrors, ++errorcount);
				BOOST_CHECK_EQUAL(lastErrorCode, cb_t::DATA_DROPPED | cb_t::ERROR_UUID);

				// omit interface name:  announcement callback will not be called
				message = getJsonAnnouncementString(20, "112233445566", NULL);
				m_deviceMonitor.processReceivedAnnouncement("eth_test", message);
				BOOST_CHECK_EQUAL(countAnnouncement, 1);
				BOOST_CHECK_EQUAL(countErrors, ++errorcount);
				BOOST_CHECK_EQUAL(lastErrorCode, cb_t::DATA_DROPPED | cb_t::ERROR_IPADDR);
			}

			/// Test: large number of valid messages (devices)
			BOOST_AUTO_TEST_CASE( test_case_many_devices )
			{
				m_deviceMonitor.setAnnounceCb(announceCbTest);
				BOOST_CHECK_EQUAL(countAnnouncement, 0);
				BOOST_CHECK_EQUAL(lastAnnouncement, "");

				unsigned long largenumber = 10000;

				std::string message;

				for(unsigned long dev=1 ; dev <= largenumber ; dev++)
				{
					std::string uid=std::to_string(dev);

					int ip=dev;
					int e0 = ip%256; ip = ip/256;
					int e1 = ip%256; ip = ip/256;
					int e2 = ip%256; ip = ip/256;
					int e3 = ip%256;

					std::string ipaddress = std::to_string(e3).append(".").
					  append(std::to_string(e2)).append(".").
					  append(std::to_string(e1)).append(".").
					  append(std::to_string(e0));

					message = getJsonAnnouncementString(15, uid.c_str(), ipaddress.c_str());
	//				std::cout << message << std::endl;

					m_deviceMonitor.processReceivedAnnouncement("eth_test", message);
					BOOST_CHECK_EQUAL(countAnnouncement, dev);
					BOOST_CHECK_EQUAL(lastAnnouncement, message);
				}

				for(unsigned long dev=1 ; dev <= largenumber ; dev++)
				{
					std::string uid=std::to_string(dev);

					int ip=dev;
					int e0 = ip%256; ip = ip/256;
					int e1 = ip%256; ip = ip/256;
					int e2 = ip%256; ip = ip/256;
					int e3 = ip%256;

					std::string ipaddress = std::to_string(e3).append(".").
					  append(std::to_string(e2)).append(".").
					  append(std::to_string(e1)).append(".").
					  append(std::to_string(e0));

					message = getJsonAnnouncementString(15, uid.c_str(), ipaddress.c_str());

					m_deviceMonitor.processReceivedAnnouncement("eth_test", message);
					BOOST_CHECK_EQUAL(countAnnouncement, largenumber);
	//				BOOST_CHECK_EQUAL(lastAnnouncement, message);
				}
			}

			/// Test: expiration
			BOOST_AUTO_TEST_CASE( test_case_expire )
			{
				m_deviceMonitor.setAnnounceCb(announceCbTest);
				BOOST_CHECK_EQUAL(countAnnouncement, 0);
				BOOST_CHECK_EQUAL(lastAnnouncement, "");

				m_deviceMonitor.setExpireCb(expireCbTest);
				BOOST_CHECK_EQUAL(countExpiration, 0);

				std::string message;

				const char* const uuid1 = "to_expire_in_1_sec";
				const char* const uuid3 = "to_expire_in_3_sec";
				const char* const uuid5 = "to_expire_in_5_sec";

				// insert announcements "out of any order"

				// select an expiration time of 3 seconds
				message = getJsonAnnouncementString(3, uuid3, "eth0");
				m_deviceMonitor.processReceivedAnnouncement("eth_test", message);
				BOOST_CHECK_EQUAL(countAnnouncement, 1);
				BOOST_CHECK_EQUAL(lastAnnouncedUuid, uuid3);
				BOOST_CHECK_EQUAL(lastAnnouncement, message);

				// select an expiration time of 1 seconds
				message = getJsonAnnouncementString(1, uuid1, "eth0");
				m_deviceMonitor.processReceivedAnnouncement("eth_test", message);
				BOOST_CHECK_EQUAL(countAnnouncement, 2);
				BOOST_CHECK_EQUAL(lastAnnouncedUuid, uuid1);
				BOOST_CHECK_EQUAL(lastAnnouncement, message);

				// select an expiration time of 5 seconds
				message = getJsonAnnouncementString(5, uuid5, "eth0");
				m_deviceMonitor.processReceivedAnnouncement("eth_test", message);
				BOOST_CHECK_EQUAL(countAnnouncement, 3);
				BOOST_CHECK_EQUAL(lastAnnouncedUuid, uuid5);
				BOOST_CHECK_EQUAL(lastAnnouncement, message);

				BOOST_CHECK_EQUAL(countExpiration, 0);

				sleep(2); // wait for 2 seconds
				m_deviceMonitor.checkForExpiredAnnouncements();
				// expiration should have occurred
				BOOST_CHECK_EQUAL(countExpiration, 1);
				BOOST_CHECK_EQUAL(lastExpiredUuid, uuid1);

				sleep(2); // wait for 2 seconds
				m_deviceMonitor.checkForExpiredAnnouncements();
				// expiration should have occurred
				BOOST_CHECK_EQUAL(countExpiration, 2);
				BOOST_CHECK_EQUAL(lastExpiredUuid, uuid3);

				sleep(2); // wait for 2 seconds
				m_deviceMonitor.checkForExpiredAnnouncements();
				// expiration should have occurred
				BOOST_CHECK_EQUAL(countExpiration, 3);
				BOOST_CHECK_EQUAL(lastExpiredUuid, uuid5);
			}


			/// Test: exceptions in user-provided callback-functions
			BOOST_AUTO_TEST_CASE( test_callback_throws )
			{
				m_deviceMonitor.setAnnounceCb(announceCbThrows);
				m_deviceMonitor.setExpireCb(expireCbThrows);
				m_deviceMonitor.setErrorCb(errorCbThrows);

				// select an expiration time of 1 seconds
				std::string message = getJsonAnnouncementString(1, "to_expire_in_1_sec", "1.1.2.1");

				// trigger announce call-back
				m_deviceMonitor.processReceivedAnnouncement("eth_test", message);
				// re-trigger announce call-back
				m_deviceMonitor.setAnnounceCb(announceCbThrows);

				sleep(2); // wait for 2 seconds
				// trigger expire call-back
				m_deviceMonitor.checkForExpiredAnnouncements();
				// trigger error call-back
				m_deviceMonitor.processReceivedAnnouncement("eth_some_long_name", "This is not a valid message");

			}

			BOOST_AUTO_TEST_SUITE_END()
		}
	}
}
