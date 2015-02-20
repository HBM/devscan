// Copyright 2014 Hottinger Baldwin Messtechnik
// Distributed under MIT license
// See file LICENSE provided


#include <sys/types.h>
#include <sys/socket.h>

#include <linux/netlink.h>
#include <linux/rtnetlink.h>

#include <syslog.h>

#include <unistd.h>

#include <cstring>

#include "hbm/communication/netlink.h"
#include "hbm/exception/exception.hpp"

namespace hbm {
	Netlink::Netlink()
		: m_fd(socket(AF_NETLINK, SOCK_RAW | SOCK_NONBLOCK, NETLINK_ROUTE))
	{
		if (m_fd<0) {
			throw hbm::exception::exception("could not open netlink socket");
		}
		struct sockaddr_nl netLinkAddr;

		memset(&netLinkAddr, 0, sizeof(netLinkAddr));

		netLinkAddr.nl_family = AF_NETLINK;
		netLinkAddr.nl_pid = getpid();
		netLinkAddr.nl_groups = RTMGRP_LINK | RTMGRP_NOTIFY | RTMGRP_IPV4_IFADDR | RTMGRP_IPV6_IFADDR | RTMGRP_IPV4_ROUTE;

		if (::bind(m_fd, reinterpret_cast < struct sockaddr *> (&netLinkAddr), sizeof(netLinkAddr))<0) {
			throw hbm::exception::exception("could not bind netlink socket");
		}
	}

	Netlink::~Netlink()
	{
		stop();
	}

	ssize_t Netlink::receive(char *pReadBuffer, size_t bufferSize) const
	{
		struct sockaddr_nl nladdr;
		struct iovec iov = {
			pReadBuffer,
			bufferSize
		};
		struct msghdr msg;
		memset(&msg, 0, sizeof(msg));
		msg.msg_name = &nladdr;
		msg.msg_namelen = sizeof(nladdr);
		msg.msg_iov = &iov;
		msg.msg_iovlen = 1;
		return ::recvmsg(m_fd, &msg, 0);
	}

	void Netlink::process(char *pReadBuffer, size_t bufferSize, communication::NetadapterList &netadapterlist, communication::MulticastServer &mcs) const
	{
		for (struct nlmsghdr *nh = reinterpret_cast <struct nlmsghdr *> (pReadBuffer); NLMSG_OK (nh, bufferSize); nh = NLMSG_NEXT (nh, bufferSize)) {
			if (nh->nlmsg_type == NLMSG_DONE) {
				// The end of multipart message.
				break;
			} else if (nh->nlmsg_type == NLMSG_ERROR) {
				::syslog(LOG_ERR, "error processing netlink events");
				break;
			} else {
				netadapterlist.update();
				switch(nh->nlmsg_type) {
				case RTM_NEWADDR:
					{
						struct ifaddrmsg* pIfaddrmsg = reinterpret_cast <struct ifaddrmsg*> (NLMSG_DATA(nh));
						if(pIfaddrmsg->ifa_family==AF_INET) {
							struct rtattr *rth = IFA_RTA(pIfaddrmsg);
							int rtl = IFA_PAYLOAD(nh);
							while (rtl && RTA_OK(rth, rtl)) {
								if (rth->rta_type == IFA_LOCAL) {
									// this is to be ignored if there are more than one ipv4 addresses assigned to the interface!
									try {
										communication::Netadapter adapter = netadapterlist.getAdapterByInterfaceIndex(pIfaddrmsg->ifa_index);
										if(adapter.getIpv4Addresses().size()==1) {
											struct in_addr* pIn = reinterpret_cast < struct in_addr* > (RTA_DATA(rth));
											std::string interfaceAddress = inet_ntoa(*pIn);
											mcs.addInterface(interfaceAddress);
										}
									} catch(const hbm::exception::exception&) {
									}
								}
								rth = RTA_NEXT(rth, rtl);
							}
						}
					}
					break;
				case RTM_DELADDR:
					{
						struct ifaddrmsg* pIfaddrmsg = reinterpret_cast <struct ifaddrmsg*> (NLMSG_DATA(nh));
						if(pIfaddrmsg->ifa_family==AF_INET) {
							struct rtattr *rth = IFA_RTA(pIfaddrmsg);
							int rtl = IFA_PAYLOAD(nh);
							while (rtl && RTA_OK(rth, rtl)) {
								if (rth->rta_type == IFA_LOCAL) {
									// this is to be ignored if there is another ipv4 address left for the interface!
									try {
										communication::Netadapter adapter = netadapterlist.getAdapterByInterfaceIndex(pIfaddrmsg->ifa_index);
										if(adapter.getIpv4Addresses().empty()==true) {
											const struct in_addr* pIn = reinterpret_cast < const struct in_addr* > (RTA_DATA(rth));
											std::string interfaceAddress = inet_ntoa(*pIn);
											mcs.dropInterface(interfaceAddress);
										}
									} catch(const hbm::exception::exception&) {
									}
								}
								rth = RTA_NEXT(rth, rtl);
							}
						}
					}
					break;
				default:
					break;
				}
			}
		}
	}

	ssize_t Netlink::receiveAndProcess(communication::NetadapterList &netadapterlist, communication::MulticastServer &mcs) const
	{
		char readBuffer[communication::MAX_DATAGRAM_SIZE];
		ssize_t nBytes = receive(readBuffer, sizeof(readBuffer));
		if (nBytes>0) {
			process(readBuffer, nBytes, netadapterlist, mcs);
		}
		return nBytes;
	}


	int Netlink::stop()
	{
		return ::close(m_fd);
	}
}

