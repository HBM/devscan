// Copyright 2014 Hottinger Baldwin Messtechnik
// Distributed under MIT license
// See file LICENSE provided


#include <iomanip>
#include <sstream>
#include <string>
#include <stdint.h>
#include <iterator>
#include <mutex>
#include <cstring>

#ifdef _WIN32
#include <windows.h>
#include <iphlpapi.h>
#define syslog fprintf
#define LOG_ERR stderr
#else
#include <unistd.h>
#include <syslog.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ifaddrs.h>

#include <sys/ioctl.h>
#endif

#include "hbm/exception/exception.hpp"

#include "netadapterlist.h"
#include "netadapter.h"

namespace hbm {
	namespace communication {
		NetadapterList::NetadapterList()
		{
			enumAdapters();
		}

	#ifdef _WIN32
		void NetadapterList::enumAdapters()
		{
			IP_ADAPTER_INFO* pAdptInfo = NULL;
			IP_ADAPTER_INFO* pNextAd = NULL;
			ULONG ulLen = 0;
			DWORD erradapt;

			//This call returns the number of network adapter in ulLen
			erradapt = ::GetAdaptersInfo(pAdptInfo, &ulLen);

			if (erradapt == ERROR_BUFFER_OVERFLOW) {
				pAdptInfo = reinterpret_cast < IP_ADAPTER_INFO* >(new UINT8[ulLen]);
				erradapt = ::GetAdaptersInfo(pAdptInfo, &ulLen);
			}

			if (erradapt == ERROR_SUCCESS) {
				std::lock_guard < std::mutex > lock(m_adaptersMtx);
				m_adapters.clear();
				// initialize the pointer we use the move through
				// the list.
				pNextAd = pAdptInfo;

				// loop through for all available interfaces and setup an associated
				// CNetworkAdapter class.
				while (pNextAd) {
					std::stringstream macStream;
					Netadapter Adapt;
					std::vector < std::string > GatewayList;
					IP_ADDR_STRING* pNext	= NULL;

					unsigned int adapterIndex = pNextAd->Index;
					Adapt.m_index = adapterIndex;
					Adapt.m_macAddress.clear();

					for (unsigned int i = 0; i < pNextAd->AddressLength; i++) {
						if (i > 0) {
							macStream << ":";
						}

						macStream << std::hex << std::setw(2) << std::setfill('0') << static_cast < unsigned int >(pNextAd->Address[i]) << std::dec;
					}

					Adapt.m_macAddress = macStream.str();

					ipv4Address_t addressWithNetmask;

					if (pNextAd->CurrentIpAddress) {
						addressWithNetmask.address = pNextAd->CurrentIpAddress->IpAddress.String;
						addressWithNetmask.netmask = pNextAd->CurrentIpAddress->IpMask.String;
					} else {
						addressWithNetmask.address = pNextAd->IpAddressList.IpAddress.String;
						addressWithNetmask.netmask = pNextAd->IpAddressList.IpMask.String;
					}

					// there might be several addresses per interface
					Adapt.m_ipv4Addresses.push_back(addressWithNetmask);

					// an adapter usually has just one gateway however the provision exists
					// for more than one so to "play" as nice as possible we allow for it here
					// as well.
					pNext = &(pNextAd->GatewayList);

					while (pNext) {
						GatewayList.push_back(pNext->IpAddress.String);
						pNext = pNext->Next;
					}


					if (addressWithNetmask.address != "0.0.0.0") { // HBM only wants connected Interfaces to be enumerated

						Adapt.m_name = pNextAd->Description;
						m_adapters[adapterIndex] = Adapt;
					}

					// move forward to the next adapter in the list so
					// that we can collect its information.
					pNextAd = pNextAd->Next;
				}
			}

			// free any memory we allocated from the heap before exit.
			delete[] pAdptInfo;
		}
	#else
		static void getHardwareAddress(const std::string& interfaceName, std::string& hwAddrString, uint64_t& fwGuid)
		{
			struct ifreq ifr;
			memset(&ifr, 0, sizeof(ifr));

			fwGuid = 0;
			hwAddrString.clear();


			int sd =::socket(AF_INET, SOCK_DGRAM, 0);
			if(sd==-1) {
				::syslog(LOG_ERR, "%s: could not create socket!", __FUNCTION__);
			} else {
				::strncpy(ifr.ifr_name, interfaceName.c_str(), sizeof(ifr.ifr_name));
				ifr.ifr_name[sizeof(ifr.ifr_name) - 1] = '\0';

				if ((::ioctl(sd, SIOCGIFHWADDR, reinterpret_cast < caddr_t > (&ifr), sizeof(ifr))) == 1) {
					::syslog(LOG_ERR, "%s: error calling ioctl SIOCGIFHWADDR!", __FUNCTION__);
				} else {
					int macLen;

					switch (ifr.ifr_hwaddr.sa_family) {
					case ARPHRD_ETHER:
						macLen = 6;
						break;

					case ARPHRD_IEEE1394:
						macLen = 16;

						for (unsigned int i = 0; i < sizeof(fwGuid); ++i) {
							fwGuid = fwGuid << 8;
							fwGuid |= ifr.ifr_hwaddr.sa_data[i];
						}

						break;

					default:
						macLen = 0;
						break;
					}

					std::stringstream hwAddressStream;

					for (int i = 0; i < macLen; ++i) {
						if (i > 0) {
							hwAddressStream << ":";
						}

						hwAddressStream <<  std::uppercase << std::setfill('0') << std::setw(2) << std::hex <<  static_cast <unsigned int> ( static_cast < unsigned char >(ifr.ifr_hwaddr.sa_data[i]));
					}

					hwAddrString = hwAddressStream.str();

				}

				::close(sd);
			}
		}

		void NetadapterList::enumAdapters()
		{
			struct ifaddrs* interfaces;
			struct ifaddrs* interface;
			sa_family_t family;
			char buf[INET6_ADDRSTRLEN];

			tAdapters adapterMap;

			if (::getifaddrs(&interfaces) < 0) {
				::syslog(LOG_ERR, "Error calling getifaddrs!");
				return;
			}

			interface = interfaces;

			while (interface != NULL) {
				if (interface->ifa_addr != NULL) {
					if ((interface->ifa_flags & IFF_UP) &&
						(interface->ifa_flags & IFF_BROADCAST) &&
						!(interface->ifa_flags & IFF_LOOPBACK)
						) {
							// If the adapter is not known yet, it will be created.
							unsigned int interfaceIndex = if_nametoindex(interface->ifa_name);
							Netadapter& Adapt = adapterMap[interfaceIndex];

							// An interface may have several ip addresses. Do this only once per interface.
							if(Adapt.m_macAddress.empty()) {
								Adapt.m_index = interfaceIndex;
								Adapt.m_name = interface->ifa_name;
								getHardwareAddress(interface->ifa_name, Adapt.m_macAddress, Adapt.m_fwGuid);
							}

							family = interface->ifa_addr->sa_family;

							if (family == AF_INET) {
								ipv4Address_t addressWithNetmask;
								struct sockaddr_in* pSin = reinterpret_cast <struct sockaddr_in*> (interface->ifa_addr);
								addressWithNetmask.address = inet_ntop(family, &pSin->sin_addr, buf, sizeof(buf));

								pSin = reinterpret_cast <struct sockaddr_in*> (interface->ifa_netmask);
								addressWithNetmask.netmask = inet_ntop(family, &pSin->sin_addr, buf, sizeof(buf));

								Adapt.m_ipv4Addresses.push_back(addressWithNetmask);
							}	else if (family == AF_INET6) {
								ipv6Address_t address;

								struct sockaddr_in6* pSin = reinterpret_cast <struct sockaddr_in6*> (interface->ifa_addr);
								address.address = inet_ntop(family, &pSin->sin6_addr, buf, sizeof(buf));

								pSin = reinterpret_cast <struct sockaddr_in6*> (interface->ifa_netmask);

								// calculate prefix: count bits in netmask. ipv6 forces the following form for the prefix 111..11110..00 gaps filled with zero like 101 are not allowed!
								unsigned int bitCount = 0;
								for(unsigned int bytePos=0; bytePos<16; ++bytePos) {
									if(pSin->sin6_addr.s6_addr[bytePos]==0) {
										break;
									}

									unsigned char byte = pSin->sin6_addr.s6_addr[bytePos];
									unsigned char mask = 1;
									for(unsigned int bitPos=0; bitPos<8; ++bitPos) {
										if(byte & mask) {
											++bitCount;
										}
										mask <<= 1;
									}
								}

								address.prefix = bitCount;
								Adapt.m_ipv6Addresses.push_back(address);
							}
					}
				}

				interface = interface->ifa_next;
			}

			{
				std::lock_guard < std::mutex > lock(m_adaptersMtx);
				m_adapters = adapterMap;
			}

			::freeifaddrs(interfaces);
		}
	#endif

		NetadapterList::tAdapters NetadapterList::get() const
		{
			std::lock_guard < std::mutex > lock(m_adaptersMtx);
			return m_adapters;
		}

		NetadapterList::tAdapterArray NetadapterList::getArray() const
		{
			std::lock_guard < std::mutex > lock(m_adaptersMtx);
			tAdapterArray result;
			result.reserve(m_adapters.size());

			for(tAdapters::const_iterator iter = m_adapters.begin(); iter!=m_adapters.end(); ++iter) {
				result.push_back(iter->second);
			}

			return result;
		}

		Netadapter NetadapterList::getAdapterByName(const std::string& adapterName) const
		{
			std::lock_guard < std::mutex > lock(m_adaptersMtx);

			for (tAdapters::const_iterator iter = m_adapters.begin(); iter != m_adapters.end(); ++iter) {
				if (iter->second.getName().compare(adapterName) == 0) {
					return iter->second;
				}
			}

			throw hbm::exception::exception("invalid interface");
			return Netadapter();
		}

		Netadapter NetadapterList::getAdapterByInterfaceIndex(unsigned int interfaceIndex) const
		{
			std::lock_guard < std::mutex > lock(m_adaptersMtx);

			tAdapters::const_iterator iter = m_adapters.find(interfaceIndex);
			if(iter==m_adapters.end()) {
				throw hbm::exception::exception("invalid interface");
			}

			return iter->second;
		}

		void NetadapterList::update()
		{
			enumAdapters();
		}
	}
}
