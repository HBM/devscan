// Copyright 2014 Hottinger Baldwin Messtechnik
// Distributed under MIT license
// See file LICENSE provided

#ifndef _DEFINES_H
#define _DEFINES_H



namespace hbm
{
	namespace devscan
	{
		/// \file defines.h  this file contains constants that are relevant for
		/// device software and client software concerning HBM Device Scan

		/// announcements will be send to this port
		const unsigned int ANNOUNCE_UDP_PORT = 31416;
		/// configuration requests and responses go to this port
		const unsigned int CONFIG_UDP_PORT = 31417;

		/// announcements will be send to this group
		const char ANNOUNCE_IPV4_ADDRESS[] = "239.255.77.76";
		/// configuration requests and responses go to this group
		const char CONFIG_IPV4_ADDRESS[] = "239.255.77.77";
		

		static const char TAG_Announce[] = "announce";
		static const char TAG_Configure[] = "configure";

		static const char TAG_Services[] = "services";
		static const char TAG_Expiration[] = "expiration";

		static const char TAG_NetSettings[] = "netSettings";

		static const char TAG_Interface[] = "interface";
		static const char TAG_Name[] = "name";
		static const char TAG_Description[] = "description";
		static const char TAG_HardwareId[] = "hardwareId";
		static const char TAG_Type[] = "type";
		static const char TAG_Label[] = "label";
		static const char TAG_FamilyType[] = "familyType";

		static const char CONFIGURATION_METHOD[] = "configurationMethod";

		static const char SRV[] = "services";
		static const char SRV_HBM_PROTOCOL[] = "hbmProtocol";
		static const char SRV_STREAM_DATA[] = "daqStream";
		static const char SRV_HTTP[] = "http";
		static const char SRV_SSH[] = "ssh";
		static const char SRV_JETD[] = "jetd";
		static const char SRV_JETWS[] = "jetws";

		static const char NET_PATH[] = "net";
		static const char INTERFACES[] = "interfaces";
		static const char RESULTS[] = "results";
		static const char REACHABLEDEVICES_PATH[] = "reachableDevices";
		static const char ROUTEDDEVICES_PATH[] = "routedDevices";

		static const char TAG_eth[] = "eth";
		static const char TAG_manualAddress[] = "manualAddress";
		static const char TAG_manualNetmask[] = "manualNetmask";
		static const char TAG_Port[] = "port";

		static const char TAG_ipV4[] = "ipv4";
		static const char TAG_ipV6[] = "ipv6";
		static const char TAG_address[] = "address";
		static const char TAG_netMask[] = "netmask";
		static const char TAG_ipV4Address[] = "ipv4Address";
		static const char TAG_ipV6Address[] = "ipv6Address";
		static const char TAG_prefix[] = "prefix";

		static const char TAG_DefaultGateway[] = "defaultGateway";
		static const char PrepareDefaultGatewayMethod[] = "prepareDefaultGateway";

		static const char TAG_Device[] = "device";
		static const char TAG_IsRouter[] = "isRouter";
		static const char TAG_Router[] = "router";
		static const char TAG_Uuid[] = "uuid";
		static const char TAG_Ttl[] = "ttl";
		static const char TAG_FirmwareVersion[] = "firmwareVersion";

		/// configure("interface"."configuration_method"="manual")
		static const char CONFIG_METHOD_MANUAL[] = "manual";
		/// configure("interface"."configuration_method"="dhcp")
		static const char CONFIG_METHOD_DHCP[] = "dhcp";

		static const char CONFIG[] = "config";
		static const char PREPARE[] = "prepare";
	}
}
#endif // _DEFINES_H
