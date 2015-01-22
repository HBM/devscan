// Copyright 2014 Hottinger Baldwin Messtechnik
// Distributed under MIT license
// See file LICENSE provided


#include <sstream>
#include <cstdlib>
#include <chrono>

#include <json/value.h>
#include <json/reader.h>
#include <json/writer.h>

#include "hbm/communication/netadapter.h"
#include "hbm/jsonrpc/jsonrpc_defines.h"

#include "configureclient.h"
#include "defines.h"

namespace hbm {
	namespace devscan {
		const std::chrono::milliseconds ConfigureClient::TIMETOWAITFORANSWERS(3000);

		ConfigureClient::ConfigureClient()
			: m_MulticastServer(CONFIG_IPV4_ADDRESS, CONFIG_UDP_PORT, m_netadapterList)
		{
			m_MulticastServer.start();
			m_MulticastServer.addAllInterfaces();

			srand (static_cast < unsigned int > (time(NULL)));
		}

		ConfigureClient::~ConfigureClient()
		{
			m_MulticastServer.stop();
		}

		std::string ConfigureClient::executeRequest(const std::string& interfaceIp, unsigned char ttl, const std::string& id, const std::string& Message)
		{
			m_netadapterList.update();


			if (interfaceIp.empty()) {
				m_MulticastServer.send(Message, ttl);
			} else {
				m_MulticastServer.sendOverInterfaceByAddress(interfaceIp, Message, ttl);
			}


			// collect answer(s) here
			ssize_t nbytes;
			const size_t BUFFERSIZE = 65536;
			char readBuffer[BUFFERSIZE];
			int adapterIndex;

			std::chrono::steady_clock::time_point endTime = std::chrono::steady_clock::now() + TIMETOWAITFORANSWERS;

			do {
				std::chrono::milliseconds timediff = std::chrono::duration_cast < std::chrono::milliseconds > (endTime - std::chrono::steady_clock::now());

				// receive responses until timeout or error.
				nbytes = m_MulticastServer.receiveTelegramBlocking(readBuffer, sizeof(readBuffer), adapterIndex, timediff);
				if (nbytes > 0) {
					Json::Value telegramNode;

					if(Json::Reader().parse(readBuffer, readBuffer+nbytes, telegramNode)) {
						if(telegramNode.isMember(hbm::jsonrpc::RESULT) || telegramNode.isMember(hbm::jsonrpc::ERR)) {
							// this is a result or an error!

							// is this a response to our question ( id does match)?
							if (id == telegramNode[hbm::jsonrpc::ID].asString()) {
								return std::string(readBuffer, nbytes);
							}
						}
					}
				}
			} while (nbytes > 0);

			// no response received in time!
			return "";
		}

		std::string ConfigureClient::setInterfaceConfigurationMethod(const std::string& outgoingInterfaceIp, unsigned char ttl, const std::string& Uuid, const std::string& InterfaceName, const std::string& method)
		{
		//  {
		//    "jsonrpc": "2.0",
		//    "method": "configure",
		//    "params": {
		//			"net_settings" : {
		//	      "interface": {
		//		      "name": <string>,
		//			    "configuration_method": <string>,
		//	      },
		//			},
		//      "device": {
		//        "uuid": <string>
		//      },
		//      "ttl": <number>
		//    },
		//    "id": <string>|<number
		//  }
			Json::Value tree;
			Json::FastWriter writer;
			writer.omitEndingLineFeed();
			std::string id(createId());

			tree[hbm::jsonrpc::JSONRPC] = "2.0";
			tree[hbm::jsonrpc::METHOD] = TAG_Configure;
			tree[hbm::jsonrpc::PARAMS][TAG_NetSettings][TAG_Interface][TAG_Name] = InterfaceName;
			tree[hbm::jsonrpc::PARAMS][TAG_NetSettings][TAG_Interface][CONFIGURATION_METHOD] = method;
			tree[hbm::jsonrpc::PARAMS][TAG_Device][TAG_Uuid] = Uuid;
			tree[hbm::jsonrpc::PARAMS][TAG_Ttl] = static_cast < int >(ttl);
			tree[hbm::jsonrpc::ID] = id;


			return executeRequest(outgoingInterfaceIp, ttl, id, writer.write(tree));
		}


		std::string ConfigureClient::setInterfaceConfiguration(const std::string& outgoingInterfaceIp, unsigned char ttl, const std::string& uuid, const std::string& interfaceName, const std::string& method, const std::string& manualAddress, const std::string& manualNetmask)
		{
			Json::Value tree;
			Json::FastWriter writer;
			writer.omitEndingLineFeed();
			std::string id(createId());

			tree[(const char*)NULL] = "2.0";

			tree[hbm::jsonrpc::JSONRPC] = "2.0";
			tree[hbm::jsonrpc::METHOD] = TAG_Configure;
			tree[hbm::jsonrpc::PARAMS][TAG_NetSettings][TAG_Interface][TAG_Name] = interfaceName;
			tree[hbm::jsonrpc::PARAMS][TAG_NetSettings][TAG_Interface][CONFIGURATION_METHOD] = method;
			tree[hbm::jsonrpc::PARAMS][TAG_NetSettings][TAG_Interface][TAG_ipV4][TAG_manualAddress] = manualAddress;
			tree[hbm::jsonrpc::PARAMS][TAG_NetSettings][TAG_Interface][TAG_ipV4][TAG_manualNetmask] = manualNetmask;
			tree[hbm::jsonrpc::PARAMS][TAG_Device][TAG_Uuid] = uuid;
			tree[hbm::jsonrpc::PARAMS][TAG_Ttl] = static_cast < int >(ttl);
			tree[hbm::jsonrpc::ID] = id;

			return executeRequest(outgoingInterfaceIp, ttl, id, writer.write(tree));
		}

		std::string ConfigureClient::setDefaultGateway(const std::string& outgoingInterfaceIp, unsigned char ttl, const std::string& uuid, const std::string& ipv4DefaultGateWay)
		{
		//  {
		//    "jsonrpc": "2.0",
		//    "method": "configure",
		//    "params": {
		//			"net_settings" : {
		//	      "default_gateway" : {
		//					"ipv4" : <string>,
		//					"ipv6" : <string>
		//				}
		//			},
		//      "device": {
		//        "uuid": <string>
		//      },
		//      "ttl": <number>
		//    },
		//    "id": <string>|<number
		//  }

			Json::Value tree;
			Json::FastWriter writer;
			writer.omitEndingLineFeed();
			std::string id(createId());

			tree[hbm::jsonrpc::JSONRPC] = "2.0";
			tree[hbm::jsonrpc::METHOD] = TAG_Configure;
			tree[hbm::jsonrpc::PARAMS][TAG_NetSettings][TAG_DefaultGateway][TAG_ipV4Address] = ipv4DefaultGateWay;
			tree[hbm::jsonrpc::PARAMS][TAG_Device][TAG_Uuid] = uuid;
			tree[hbm::jsonrpc::PARAMS][TAG_Ttl] = static_cast < int >(ttl);
			tree[hbm::jsonrpc::ID] = id;

			return executeRequest(outgoingInterfaceIp, ttl, id, writer.write(tree));
		}

		std::string ConfigureClient::setInterfaceManualConfiguration(const std::string& outgoingInterfaceIp, unsigned char ttl, const std::string& Uuid, const std::string& InterfaceName, const std::string& address, const std::string& netmask)
		{

		//  {
		//    "jsonrpc": "2.0",
		//    "method": "configure",
		//    "params": {
		//			"net_settings" : {
		//				"interface": {
		//					"name": <string>,
		//	        "ipv4": {
		//		        "manual_address": <string>,
		//			      "manual_netmask": <string>
		//			    },
		//				},
		//      },
		//      "device": {
		//        "uuid": <string>
		//      },
		//      "ttl": <number>
		//    },
		//    "id": <string>|<number>
		//  }


			Json::Value tree;
			Json::FastWriter writer;
			writer.omitEndingLineFeed();
			std::string id(createId());

			tree[hbm::jsonrpc::JSONRPC] = "2.0";
			tree[hbm::jsonrpc::METHOD] = TAG_Configure;
			tree[hbm::jsonrpc::PARAMS][TAG_NetSettings][TAG_Interface][TAG_Name] = InterfaceName;
			tree[hbm::jsonrpc::PARAMS][TAG_NetSettings][TAG_Interface][TAG_ipV4][TAG_manualAddress] = address;
			tree[hbm::jsonrpc::PARAMS][TAG_NetSettings][TAG_Interface][TAG_ipV4][TAG_manualNetmask] = netmask;
			tree[hbm::jsonrpc::PARAMS][TAG_Device][TAG_Uuid] = Uuid;
			tree[hbm::jsonrpc::PARAMS][TAG_Ttl] = static_cast < int >(ttl);
			tree[hbm::jsonrpc::ID] = id;

			return executeRequest(outgoingInterfaceIp, ttl, id, writer.write(tree));
		}

		std::string ConfigureClient::createId() const
		{
			std::stringstream idStream;
			idStream << time(NULL);
			idStream << ":";
			idStream << rand() % 1000;

			return idStream.str();
		}
	}
}
