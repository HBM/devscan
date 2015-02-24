// Copyright 2014 Hottinger Baldwin Messtechnik
// Distributed under MIT license
// See file LICENSE provided


#include <sstream>
#include <cstdlib>
#include <chrono>

#include <json/value.h>
#include <json/reader.h>
#include <json/writer.h>

#include "hbm/communication/multicastserver.h"
#include "hbm/communication/netadapter.h"
#include "hbm/jsonrpc/jsonrpc_defines.h"
#include "hbm/sys/eventloop.h"

#include "configureclient.h"
#include "defines.h"

namespace hbm {
	namespace devscan {
		const std::chrono::milliseconds ConfigureClient::TIMETOWAITFORANSWERS(3000);

		ConfigureClient::ConfigureClient()
			: m_MulticastServer(CONFIG_IPV4_ADDRESS, CONFIG_UDP_PORT, m_netadapterList, m_eventloop, std::bind(&ConfigureClient::recvCb, this, std::placeholders::_1))
		{
			m_MulticastServer.start();
			m_MulticastServer.addAllInterfaces();

			srand (static_cast < unsigned int > (time(NULL)));
		}

		ConfigureClient::~ConfigureClient()
		{
			m_MulticastServer.stop();
		}

		std::string ConfigureClient::executeRequest(const std::string& interfaceIp, unsigned char ttl, const std::string& Message)
		{
			m_netadapterList.update();


			if (interfaceIp.empty()) {
				m_MulticastServer.send(Message, ttl);
			} else {
				m_MulticastServer.sendOverInterfaceByAddress(interfaceIp, Message, ttl);
			}

			m_eventloop.execute_for(TIMETOWAITFORANSWERS);
			return m_response;
		}

		int ConfigureClient::recvCb(communication::MulticastServer* mcs)
		{
			ssize_t result;
			char buf[65536];
			int adapterIndex;
			int ttl;
			result = mcs->receiveTelegram(buf, sizeof(buf), adapterIndex, ttl);
			if (result>0) {
				Json::Value telegramNode;

				/// \throw std::runtime_error in case of a parse error
				if(Json::Reader().parse(buf, buf+result, telegramNode)) {
					if(telegramNode.isMember(hbm::jsonrpc::RESULT) || telegramNode.isMember(hbm::jsonrpc::ERR)) {
						// this is a result or an error!

						// is this a response to our question ( id does match)?
						if (m_id == telegramNode[hbm::jsonrpc::ID].asString()) {
							m_response = std::string(buf, result);
							m_eventloop.stop();
						}
					}
				}
			}
			return result;
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
			m_id = createId();

			/// \throw std::runtime_error if tree.type != NULL
			tree[hbm::jsonrpc::JSONRPC] = "2.0";
			tree[hbm::jsonrpc::METHOD] = TAG_Configure;
			tree[hbm::jsonrpc::PARAMS][TAG_NetSettings][TAG_Interface][TAG_Name] = InterfaceName;
			tree[hbm::jsonrpc::PARAMS][TAG_NetSettings][TAG_Interface][CONFIGURATION_METHOD] = method;
			tree[hbm::jsonrpc::PARAMS][TAG_Device][TAG_Uuid] = Uuid;
			tree[hbm::jsonrpc::PARAMS][TAG_Ttl] = static_cast < int >(ttl);
			tree[hbm::jsonrpc::ID] = m_id;

			return executeRequest(outgoingInterfaceIp, ttl, writer.write(tree));
		}


		std::string ConfigureClient::setInterfaceConfiguration(const std::string& outgoingInterfaceIp, unsigned char ttl, const std::string& uuid, const std::string& interfaceName, const std::string& method, const std::string& manualAddress, const std::string& manualNetmask)
		{
			Json::Value tree;
			Json::FastWriter writer;
			writer.omitEndingLineFeed();
			m_id = createId();

			/// \throw std::runtime_error if tree.type != NULL
			tree[hbm::jsonrpc::JSONRPC] = "2.0";
			tree[hbm::jsonrpc::METHOD] = TAG_Configure;
			tree[hbm::jsonrpc::PARAMS][TAG_NetSettings][TAG_Interface][TAG_Name] = interfaceName;
			tree[hbm::jsonrpc::PARAMS][TAG_NetSettings][TAG_Interface][CONFIGURATION_METHOD] = method;
			tree[hbm::jsonrpc::PARAMS][TAG_NetSettings][TAG_Interface][TAG_ipV4][TAG_manualAddress] = manualAddress;
			tree[hbm::jsonrpc::PARAMS][TAG_NetSettings][TAG_Interface][TAG_ipV4][TAG_manualNetmask] = manualNetmask;
			tree[hbm::jsonrpc::PARAMS][TAG_Device][TAG_Uuid] = uuid;
			tree[hbm::jsonrpc::PARAMS][TAG_Ttl] = static_cast < int >(ttl);
			tree[hbm::jsonrpc::ID] = m_id;

			return executeRequest(outgoingInterfaceIp, ttl, writer.write(tree));
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
			m_id = createId();

			/// \throw std::runtime_error if tree.type != NULL
			tree[hbm::jsonrpc::JSONRPC] = "2.0";
			tree[hbm::jsonrpc::METHOD] = TAG_Configure;
			tree[hbm::jsonrpc::PARAMS][TAG_NetSettings][TAG_DefaultGateway][TAG_ipV4Address] = ipv4DefaultGateWay;
			tree[hbm::jsonrpc::PARAMS][TAG_Device][TAG_Uuid] = uuid;
			tree[hbm::jsonrpc::PARAMS][TAG_Ttl] = static_cast < int >(ttl);
			tree[hbm::jsonrpc::ID] = m_id;

			return executeRequest(outgoingInterfaceIp, ttl, writer.write(tree));
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
			m_id = createId();

			/// \throw std::runtime_error if tree.type != NULL
			tree[hbm::jsonrpc::JSONRPC] = "2.0";
			tree[hbm::jsonrpc::METHOD] = TAG_Configure;
			tree[hbm::jsonrpc::PARAMS][TAG_NetSettings][TAG_Interface][TAG_Name] = InterfaceName;
			tree[hbm::jsonrpc::PARAMS][TAG_NetSettings][TAG_Interface][TAG_ipV4][TAG_manualAddress] = address;
			tree[hbm::jsonrpc::PARAMS][TAG_NetSettings][TAG_Interface][TAG_ipV4][TAG_manualNetmask] = netmask;
			tree[hbm::jsonrpc::PARAMS][TAG_Device][TAG_Uuid] = Uuid;
			tree[hbm::jsonrpc::PARAMS][TAG_Ttl] = static_cast < int >(ttl);
			tree[hbm::jsonrpc::ID] = m_id;

			return executeRequest(outgoingInterfaceIp, ttl, writer.write(tree));
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
