// Copyright 2014 Hottinger Baldwin Messtechnik
// Distributed under MIT license
// See file LICENSE provided

#include <iostream>
#include <string>
#include <iomanip>

#include <json/value.h>
#include <json/reader.h>


#include "hbm/jsonrpc/jsonrpc_defines.h"

#include "devscan/receiver.h"
#include "devscan/defines.h"


using namespace hbm::devscan;


/// called on new or changed announcment
void announceCb(const std::string uuid, const std::string& receivingInterfaceName, const std::string& sendingInterfaceName, const std::string& announcement)
{
	std::cout << "announcement: " << std::setw(16) << std::left << uuid
			  << " " << std::setw(5) << std::left << receivingInterfaceName
			  << " " << std::setw(15) << std::left << sendingInterfaceName;

	Json::Value announcementDoc;
	if(Json::Reader().parse(announcement, announcementDoc)) {
		std::cout << ": ";
		const Json::Value& deviceNode = announcementDoc[hbm::jsonrpc::PARAMS][TAG_Device];
		const Json::Value& SevicesNode = announcementDoc[hbm::jsonrpc::PARAMS][TAG_Services];
		const Json::Value& interfaceNode = announcementDoc[hbm::jsonrpc::PARAMS][TAG_NetSettings][TAG_Interface];
		std::cout << deviceNode[TAG_FamilyType].asString() << "\t";
		std::cout << std::setw(15) << std::left << deviceNode[TAG_Uuid].asString() << "\t";
		std::cout << std::setw(11) << std::left << deviceNode[TAG_Type].asString() << "\t";
		std::cout << std::setw(18) << std::left << deviceNode[TAG_Name].asString() << "\t";
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

/// called on exipry of an announcement
void expireCb(const std::string uuid, const std::string& receivingInterfaceName, const std::string& sendingIpAddress)
{
	std::cout << "expired:      " << std::setw(16) << std::left << uuid
			  << " " << std::setw(5) << std::left << receivingInterfaceName
			  << " " << std::setw(15) << std::left << sendingIpAddress
			  << std::endl;

}

/// called on errors in the received network telegram
void errorCb(uint32_t errorCode, const std::string& userMessage, const std::string& announcement)
{
	std::cerr << "Network Error " << errorCode << ": " << userMessage << " \"" << announcement << "\"" << std::endl;
}


//! [main]
int main(int argc, char* argv[])
{
	if(argc>1) {
		if(std::string(argv[1])=="-h") {
			std::cout << "Listens on incoming announces. Notifies about added, changed and removed announcements in a compact format." << std::endl;
			std::cout << "return format: <family type> <uuid> <type> <name> <address> <netmask> <http port> <interface address> <interface netmask>" << std::endl;
			return 0;
		}
	}

	hbm::devscan::Receiver receiver;
	receiver.setAnnounceCb(&announceCb);
	receiver.setExpireCb(&expireCb);
	receiver.setErrorCb(&errorCb);
	try {
		receiver.start();
	} catch(hbm::exception::exception e) {
		std::cerr << "Error starting the receiver: " << e.what() << std::endl;
		return 1;
	}

	return 0;
}
//! [main]
