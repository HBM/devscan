// Copyright 2014 Hottinger Baldwin Messtechnik
// Distributed under MIT license
// See file LICENSE provided

#include <iostream>
#include <string>
#include <chrono>

#include <json/value.h>
#include <json/reader.h>


#include "hbm/jsonrpc/jsonrpc_defines.h"

#include "devscan/receiver.h"
#include "defines.h"


using namespace hbm::devscan;



void announceCb(const std::string uuid, const std::string& receivingInterfaceName, const std::string& sendingInterfaceName, const std::string& announcement, const std::string& receivingInterfaceFilter)
{
	if((receivingInterfaceFilter.empty()==true) || (receivingInterfaceFilter==receivingInterfaceName)) {
		Json::Value announcementDoc;
		if(Json::Reader().parse(announcement, announcementDoc)) {
			const Json::Value& deviceNode = announcementDoc[hbm::jsonrpc::PARAMS][TAG_Device];
			const Json::Value& SevicesNode = announcementDoc[hbm::jsonrpc::PARAMS][TAG_Services];
			const Json::Value& interfaceNode = announcementDoc[hbm::jsonrpc::PARAMS][TAG_NetSettings][TAG_Interface];
			std::cout << deviceNode[TAG_FamilyType].asString() << "\t";
			std::cout << deviceNode[TAG_Uuid].asString() << "\t";
			std::cout << deviceNode[TAG_Type].asString() << "\t";
			std::cout << deviceNode[TAG_Name].asString() << "\t";
			if(interfaceNode[TAG_ipV4].empty()==false) {
				std::cout << interfaceNode[TAG_ipV4][0][TAG_address].asString() << "\t";
				std::cout << interfaceNode[TAG_ipV4][0][TAG_netMask].asString() << "\t";
			} else {
				std::cout << "\t\t";
			}

			// find http port:
			if(SevicesNode.isArray()) {
				// batch
				for (Json::ValueIterator iter = SevicesNode.begin(); iter!= SevicesNode.end(); ++iter) {
					const Json::Value& element = *iter;
					if(element.isObject()) {
						if(element[TAG_Type]=="http") {
							std::cout << element[TAG_Port].asUInt();
							break;
						}
					}
				}
			}
			std::cout << "\t";

			std::cout << deviceNode[TAG_FirmwareVersion].asString();
			std::cout << std::endl;
		}
	}
}


int main(int argc, char* argv[])
{
	std::string receivingInterfaceFilter;

	if(argc>1) {
		if(std::string(argv[1])=="-h") {
			std::cout << "Listens on incoming announces for a specific time. Returns a compact format about all received announcements." << std::endl;
			std::cout << "syntax: " << argv[0] << " <interface name>" << std::endl;
			std::cout << "return format: <family type> <uuid> <type> <name> <address> <netmask> <http port> <interface address> <interface netmask>" << std::endl;
			return 0;
		} else {
			receivingInterfaceFilter = argv[1];
		}
	}

	static const std::chrono::milliseconds timeToWait(10000);
	hbm::devscan::Receiver receiver;

	std::cerr << "Collecting announcements for " << timeToWait.count() << "msec" << std::endl;
	// after collecting announcements for a defined time, we set the announcement callback. As a result, the announce callback is being called for all current announcements.
	try {
		receiver.start_for(timeToWait);
	} catch(hbm::exception::exception& e) {
		std::cerr << "Error starting the receiver: " << e.what() << std::endl;
		return 1;
	}

	receiver.setAnnounceCb(std::bind(&announceCb, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, receivingInterfaceFilter));
	return 0;
}
