// Copyright 2014 Hottinger Baldwin Messtechnik
// Distributed under MIT license
// See file LICENSE provided

#include <fstream>
#include <sstream>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <cstring>
#include <map>

#ifdef _WIN32
#include <windows.h>
#define syslog fprintf
#define LOG_ERR stderr
#else
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <syslog.h>
#endif

#include "netadapter.h"

namespace hbm {
	namespace communication {
		Netadapter::Netadapter()
			: m_name()
			, m_ipv4Addresses()
			, m_ipv6Addresses()
			, m_macAddress()
			, m_fwGuid(0)
			, m_index()
		{
		}

		std::string Netadapter::getIpv4DefaultGateway()
		{
#ifdef _WIN32
			return "";
#else
			std::string gatewayString("0.0.0.0");

			FILE* fp = ::fopen("/proc/net/route", "r");

			if (fp != NULL) {
				if (fscanf(fp, "%*[^\n]\n") < 0) { // Skip the first line
					::syslog(LOG_ERR, "error reading first line of /proc/net/route!");
				} else {
					while (1) {
						int result;
						char deviceName[64];
						unsigned long destination, gateway, mask;
						int flags, refCnt, use, metric, mtu, window, irtt;
						static const int ROUTE_GW = 0x0002;
						static const int ROUTE_UP = 0x0001;

						result = ::fscanf(fp, "%63s%8lx%8lx%8X%8d%8d%8d%8lx%8d%8d%8d\n",
							deviceName, &destination, &gateway, &flags, &refCnt, &use, &metric, &mask,
							&mtu, &window, &irtt);
						if (result != 11) {
							if ((result < 0) && feof(fp)) { /* EOF with no (nonspace) chars read. */
								break;
							}
						}

						if (((flags & ROUTE_GW) != 0) &&  // route is gateway
							((flags & ROUTE_UP) != 0)) { // route is up
								if (destination == INADDR_ANY) { // default gateway
									struct sockaddr_in s_in;
									s_in.sin_addr.s_addr = gateway;
									gatewayString = ::inet_ntoa(s_in.sin_addr);
									break;
								}
						}
					}
				}
				::fclose(fp);
			}
			return gatewayString;
#endif
		}

		bool Netadapter::isApipaAddress(const std::string& address)
		{
			static const std::string apipaNet("169.254");
			if(address.find(apipaNet)==0) {
				return true;
			} else {
				return false;
			}
		}

		bool Netadapter::isValidManualIpV4Address(const std::string& ip)
		{
	#ifdef _WIN32
			unsigned long address = inet_addr(ip.c_str());

			if (address == INADDR_NONE) {
				return false;
			}
			uint32_t bigAddress = htonl(address);
	#else
			in_addr address;
			if (inet_aton(ip.c_str(), &address) == 0) {
				return false;
			}
			uint32_t bigAddress = htonl(address.s_addr);
#endif

			// check for some reserved ranges
			uint8_t upperMost = bigAddress >> 24;
			if (upperMost==0){
				// includes 0.0.0.0
				return false;
			} else if ((upperMost==24)||(upperMost==25)||(upperMost==26)) {
				// reserved for quantumx internal firewire communcation
				return false;
			} else if (upperMost==127) {
				// Loopback and diagnostics
				return false;
			} else if (upperMost>=224) {
				// 224 - 239: Reserved for Multicasting
				// 240 - 254: Experimental; used for research
				return false;
			}

			if(isApipaAddress(ip)) {
				return false;
			}

			return true;
		}

		bool Netadapter::isValidIpV4Netmask(const std::string& ip)
		{
#ifdef _WIN32
			unsigned long address = inet_addr(ip.c_str());

			if (address == INADDR_NONE) {
	#else
			in_addr address;
			if (inet_aton(ip.c_str(), &address) == 0) {
	#endif
				return false;
			}

#ifdef _WIN32
			uint32_t bigAddress = htonl(address);
#else
			uint32_t bigAddress = htonl(address.s_addr);
#endif

			if (bigAddress==0){
				// 0.0.0.0
				return false;
			} else if (bigAddress==0xffffffff) {
				// 255.255.255.255
				return false;
			}

			return true;
		}
	}
}



