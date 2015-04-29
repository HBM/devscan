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
#include "hbm/jsonrpc/jsonrpc_defines.h"

#include "devscan/receiver.h"
#include "devscan/configureclient.h"

#include "defines.h"
//#include "common/JetNames.h"

#include "scanclienttest.h"

namespace hbm {
		namespace devscan {
		namespace test {

				firstAnnouncment::firstAnnouncment()
						: m_scanClient()
						, m_announcement()
				{
						BOOST_TEST_MESSAGE( "setup first device fixture" );

						m_scanClient.setAnnounceCb(std::bind(&firstAnnouncment::collectFirstAnnouncement, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));

						std::chrono::milliseconds timeToWait(10000);

						m_scanClient.start_for(timeToWait);

						BOOST_ASSERT_MSG(m_announcement.isNull()==false, "no device found!");
				}

				firstAnnouncment::~firstAnnouncment()
				{
						BOOST_TEST_MESSAGE( "teardown first device fixture" );
				}

				void firstAnnouncment::collectFirstAnnouncement(const std::string uuid, const std::string& receivingInterfaceName, const std::string& sendingInterfaceName, const std::string& announcement)
				{
						if(Json::Reader().parse(announcement, m_announcement)) {
								m_scanClient.stop();
						}
				}

				specificAnnouncment::specificAnnouncment()
						: m_scanClient()
						, m_desiredUuid("0009E500087B")
				{
						refreshAnnouncment();
				}

				specificAnnouncment::specificAnnouncment(const std::string desiredUuid)
						: m_scanClient()
						, m_desiredUuid(desiredUuid)
				{
						refreshAnnouncment();
				}

				specificAnnouncment::~specificAnnouncment()
				{
						BOOST_TEST_MESSAGE( "teardown specific device fixture" );
				}

				void specificAnnouncment::refreshAnnouncment()
				{
						BOOST_TEST_MESSAGE( "setup specific device fixture" );

						m_announcement = Json::Value();

						m_scanClient.setAnnounceCb(std::bind(&specificAnnouncment::filterAnnouncements, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));

						static const std::chrono::milliseconds timeToWait(10000);

						m_scanClient.start_for(timeToWait);


						std::stringstream msg;
						msg << "device " << m_desiredUuid << " not found!";
						BOOST_ASSERT_MSG(m_announcement.isNull()==false, msg.str().c_str());
				}

				void specificAnnouncment::filterAnnouncements(const std::string uuid, const std::string& receivingInterfaceName, const std::string& sendingInterfaceName, const std::string& announcement)
				{
						if(uuid==m_desiredUuid) {
								if(Json::Reader().parse(announcement, m_announcement)) {
										m_scanClient.stop();
								}
								std::cout << m_announcement << std::endl;
						}
				}

				struct deviceBehindRouterFixture {
						deviceBehindRouterFixture()
								: scanClient()
								, desiredUuidBehindRouter("0009E50007FA")
								, desiredUuidRouter("0009E50013E2")
						{
								BOOST_TEST_MESSAGE( "setup device behind router fixture" );
						}

						~deviceBehindRouterFixture()
						{
								BOOST_TEST_MESSAGE( "teardown device behind router fixture" );
						}

						Receiver scanClient;
						const std::string desiredUuidBehindRouter;
						const std::string desiredUuidRouter;
				};

//				BOOST_FIXTURE_TEST_SUITE( firstDevice, firstAnnouncment )

//				BOOST_AUTO_TEST_CASE( testDeviceInformation )
//				{

//						Json::Value result;

//						std::string interfaceName = m_announcement[TAG_NetSettings][TAG_Interface][TAG_Name].asString();


//						Json::Path path("result.router");
//						Json::Value routerNode = path.resolve(result);
//						if(routerNode.isNull()) {
//								Json::Value servicesNode = m_announcement["services"];
//								unsigned int serviceCount = servicesNode.size();
//								for(unsigned int serviceIndex=0; serviceIndex<serviceCount; ++serviceIndex) {
//										Json::Value serviceNode = servicesNode[serviceIndex];
//										std::string serviceType = serviceNode[hbm::jet::TYPE].asString();

//										if(serviceType==devscan::SRV_STREAM_DATA) {
//												BOOST_ASSERT_MSG(serviceNode[hbm::jet::PORT].asUInt()==7411, "wrong port for daqStream!");
//										} else if(serviceType==devscan::SRV_HBM_PROTOCOL) {
//												BOOST_ASSERT_MSG(serviceNode[hbm::jet::PORT].asUInt()==5001, "wrong port for HBM protocol!");
//										} else if(serviceType==devscan::SRV_JETD) {
//												BOOST_ASSERT_MSG(serviceNode[hbm::jet::PORT].asUInt()==11122, "wrong port for jetd!");
//										} else if(serviceType==devscan::SRV_JETWS) {
//												BOOST_ASSERT_MSG(serviceNode[hbm::jet::PORT].asUInt()==11123, "wrong port for jetws!");
//										} else if(serviceType==devscan::SRV_SSH) {
//												BOOST_ASSERT_MSG(serviceNode[hbm::jet::PORT].asUInt()==22, "wrong port for sshd!");
//										} else if(serviceType==devscan::SRV_HTTP) {
//												BOOST_ASSERT_MSG(serviceNode[hbm::jet::PORT].asUInt()==80, "wrong port for httpd!");
//										}
//								}
//						}
//				}
//				BOOST_AUTO_TEST_SUITE_END()


		BOOST_FIXTURE_TEST_SUITE( networkConfig, specificAnnouncment )

				/// this test does not work in simulation!
				BOOST_AUTO_TEST_CASE( testSetDefaultGateWay )
				{
						unsigned int ttl = 1;
						Json::Value result;

						std::string uuid = announcementParameters()[TAG_Device][TAG_Uuid].asString();

						std::string originalDefaultGateway = announcementParameters()[TAG_NetSettings][TAG_DefaultGateway][TAG_ipV4Address].asString();
						std::string interfaceName = announcementParameters()[TAG_NetSettings][TAG_Interface][TAG_Name].asString();

						std::string requestedDefaultGateway = "172.19.5.8";

						if(originalDefaultGateway==requestedDefaultGateway) {
								requestedDefaultGateway = "172.19.5.9";
						}

						std::string resultString = configureClient.setDefaultGateway("", ttl, uuid, requestedDefaultGateway);
						Json::Reader().parse(resultString, result);

						BOOST_CHECK_EQUAL(result.isMember(hbm::jsonrpc::ERR),false);

						/// wait for the updated announcment.
						refreshAnnouncment();
						std::string resultingDefaultGateway = announcementParameters()[TAG_NetSettings][TAG_DefaultGateway][TAG_ipV4Address].asString();
						BOOST_CHECK_EQUAL(requestedDefaultGateway, resultingDefaultGateway);

						resultString = configureClient.setDefaultGateway("", ttl, uuid, originalDefaultGateway);
				}


				BOOST_AUTO_TEST_CASE( testManualConfiguration )
				{
						unsigned int ttl = 1;
						Json::Value result;

						std::string uuid = announcementParameters()[TAG_Device][TAG_Uuid].asString();
						std::string interfaceName = announcementParameters()[TAG_NetSettings][TAG_Interface][TAG_Name].asString();


						std::string requestedAddress("172.19.170.51");
						std::string requestedNetmask("255.255.0.0");

						std::string resultString = configureClient.setInterfaceManualConfiguration("", ttl, uuid, interfaceName, requestedAddress, requestedNetmask);
						Json::Reader().parse(resultString, result);

						BOOST_CHECK_EQUAL(result.isMember(hbm::jsonrpc::ERR),false);

						requestedAddress = "172.19.170.52";
						requestedNetmask = "255.255.0.0";

						resultString = configureClient.setInterfaceManualConfiguration("", ttl, uuid, interfaceName, requestedAddress, requestedNetmask);
						Json::Reader().parse(resultString, result);

						BOOST_CHECK_EQUAL(result.isMember(hbm::jsonrpc::ERR),false);
				}


				/// \warning when booting from NFS, configuration method may not be changed!
				BOOST_AUTO_TEST_CASE( testConfigurationMethod )
				{
						Json::Value result;

						std::string uuid = announcementParameters()[TAG_Device][TAG_Uuid].asString();
						std::string interfaceName = announcementParameters()[TAG_NetSettings][TAG_Interface][TAG_Name].asString();
						std::string originalConfigurationMethod = announcementParameters()[TAG_NetSettings][TAG_Interface][CONFIGURATION_METHOD].asString();

						unsigned int ttl = 1;
						std::string resultString;
						std::string requestedConfigurationMethod;

						if(originalConfigurationMethod == "manual") {
								requestedConfigurationMethod = "dhcp";
						} else {
								requestedConfigurationMethod = "manual";
						}

						resultString = configureClient.setInterfaceConfigurationMethod("", ttl, uuid, interfaceName, requestedConfigurationMethod);

						/// wait for the updated announcment.
						refreshAnnouncment();
						std::string resultingConfigurationMethod = announcementParameters()[TAG_NetSettings][TAG_Interface][CONFIGURATION_METHOD].asString();
						BOOST_CHECK_EQUAL(resultingConfigurationMethod, requestedConfigurationMethod);

						Json::Reader().parse(resultString, result);

						BOOST_CHECK_EQUAL(result.isMember(hbm::jsonrpc::ERR),false);

						if(originalConfigurationMethod.empty()==false) {
								resultString = configureClient.setInterfaceConfigurationMethod("", ttl, uuid, interfaceName, originalConfigurationMethod);
								Json::Reader().parse(resultString, result);

								BOOST_CHECK_EQUAL(result.isMember(hbm::jsonrpc::ERR),false);
						}
				}

				BOOST_AUTO_TEST_CASE( testErrorHandling )
				{
						Json::Value result;

						std::string uuid = announcementParameters()[TAG_Device][TAG_Uuid].asString();
						std::string interfaceName = announcementParameters()[TAG_NetSettings][TAG_Interface][TAG_Name].asString();
						unsigned int ttl = 1;
						std::string resultString;


//						// try illegal configuration method
//						{
//								std::stringstream idStream;
//								idStream << ::time(NULL);

//								Json::Value request;
//								request["jsonrpc"] = "2.0";
//								request[hbm::jsonrpc::METHOD] = TAG_Configure;
//								request[hbm::jsonrpc::PARAMS][TAG_Device][TAG_Uuid] = uuid;
//								request[hbm::jsonrpc::PARAMS][TAG_NetSettings][TAG_Interface][TAG_Name] = interfaceName;


//								request[hbm::jsonrpc::PARAMS][TAG_NetSettings][TAG_Interface][CONFIGURATION_METHOD] = "Manuela";
//								request[hbm::jsonrpc::PARAMS][TAG_Ttl] = ttl;
//								request[hbm::jsonrpc::ID] = idStream.str();

//								std::string requestString = Json::StyledWriter().write(request);
//								resultString = configureClient.executeRequest("", ttl, idStream.str(), requestString );

//								Json::Reader().parse(resultString, result);

//								BOOST_CHECK_EQUAL(result.isMember(hbm::jsonrpc::ERR),true);

//								int errorCode = result[hbm::jsonrpc::ERR][hbm::jsonrpc::CODE].asInt();
//								BOOST_CHECK_EQUAL(errorCode, communication::ERR_INVALIDCONFIGMETHOD);
//						}

//						// try illegal ip address!
//						{
//								std::stringstream idStream;
//								idStream << ::time(NULL);

//								Json::Value request;
//								request["jsonrpc"] = "2.0";
//								request[hbm::jsonrpc::METHOD] = TAG_Configure;
//								request[hbm::jsonrpc::PARAMS][TAG_Device][TAG_Uuid] = uuid;
//								request[hbm::jsonrpc::PARAMS][TAG_NetSettings][TAG_Interface][TAG_Name] = interfaceName;

//								request[hbm::jsonrpc::PARAMS][TAG_NetSettings][TAG_Interface][TAG_ipV4][TAG_manualAddress] = "300.100.0.0";
//								request[hbm::jsonrpc::PARAMS][TAG_NetSettings][TAG_Interface][TAG_ipV4][TAG_manualNetmask] = "255.255.0.0";

//								request[hbm::jsonrpc::PARAMS][TAG_Ttl] = ttl;
//								request[hbm::jsonrpc::ID] = idStream.str();

//								std::string requestString = Json::StyledWriter().write(request);

//								resultString = configureClient.executeRequest("", ttl, idStream.str(), requestString );
//								Json::Reader().parse(resultString, result);


//								BOOST_CHECK_EQUAL(result.isMember(hbm::jsonrpc::ERR),true);

//								int errorCode = result[hbm::jsonrpc::ERR][hbm::jsonrpc::CODE].asInt();
//								BOOST_CHECK_EQUAL(errorCode, communication::ERR_INVALIDIPADDRESS);
//						}

						// try non exiting adapter
						{
								resultString = configureClient.setInterfaceManualConfiguration("", ttl, uuid, "eth6", "172.19.170.50", "255.255.0.0");
								Json::Reader().parse(resultString, result);

								BOOST_CHECK_EQUAL(result.isMember(hbm::jsonrpc::ERR),true);

								int errorCode = result[hbm::jsonrpc::ERR][hbm::jsonrpc::CODE].asInt();
								BOOST_CHECK_EQUAL(errorCode, communication::ERR_INVALIDADAPTER);
						}

						// try illegal default gateway
						{
								resultString = configureClient.setDefaultGateway("", ttl, uuid, "bla");
								Json::Reader().parse(resultString, result);

								BOOST_CHECK_EQUAL(result.isMember(hbm::jsonrpc::ERR),true);

								int errorCode = result[hbm::jsonrpc::ERR][hbm::jsonrpc::CODE].asInt();
								BOOST_CHECK_EQUAL(errorCode, communication::ERR_INVALIDIPADDRESS);
						}
				}


				BOOST_AUTO_TEST_SUITE_END()




						BOOST_FIXTURE_TEST_SUITE( behindRouter, deviceBehindRouterFixture )
						BOOST_AUTO_TEST_SUITE_END()

		}
		}
}
