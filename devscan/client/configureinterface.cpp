// Copyright 2014 Hottinger Baldwin Messtechnik
// Distributed under MIT license
// See file LICENSE provided

#include <iostream>
#include "devscan/configureclient.h"

int main(int argc, char* argv[])
{
	if( argc!=6 ) {
		std::cout << "syntax: " << argv[0] << " <uuid of the device> <interface name> dhcp|manual <manual address> <manual netmask>" << std::endl;
		return 0;
	}

	std::string uuid = argv[1];
	std::string interfaceName = argv[2];
	std::string method = argv[3];
	std::string manualAddress = argv[4];
	std::string manualNetmask = argv[5];
	hbm::devscan::ConfigureClient configureClient;

	std::string result = configureClient.setInterfaceConfiguration("", 1, uuid, interfaceName, method, manualAddress, manualNetmask);

	std::cout << result << std::endl;
	return 0;
}
