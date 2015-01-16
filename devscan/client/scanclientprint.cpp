// Copyright 2014 Hottinger Baldwin Messtechnik
// Distributed under MIT license
// See file LICENSE provided

#include <iostream>
#include <string>

#include <json/value.h>
#include <json/reader.h>
#include <json/writer.h>


#include "devscan/receiver.h"


void announceCb(const std::string uuid, const std::string& receivingInterfaceName, const std::string& sendingInterfaceName, const std::string& announcement)
{

	std::cout << "announcement: " << uuid << " destination interface: " << receivingInterfaceName << ", source interface: " << sendingInterfaceName << std::endl;

	// produce output that is structured
	Json::Value val;
	Json::Reader().parse(announcement, val);
	std::cout << Json::StyledWriter().write(val) << std::endl;
}

void expireCb(const std::string uuid, const std::string& receivingInterfaceName, const std::string& sendingInterfaceName)
{
	std::cout << "expired: " << uuid << " destination interface: " << receivingInterfaceName << ", source interface: " << sendingInterfaceName << std::endl;
}

int main(int argc, char* argv[])
{
	if(argc>1) {
		if(std::string(argv[1])=="-h") {
			std::cout << "Listens for announcements and prints new/changed announcements as received" << std::endl;
			return 0;
		}
	}

	hbm::devscan::Receiver receiver;

	// we want to be notified about new or changed announcements
	receiver.setAnnounceCb(&announceCb);

	// we want to be notified about expired announcements
	receiver.setExpireCb(&expireCb);

	// start receiving for indefinite time
	receiver.start();
	return 0;
}
