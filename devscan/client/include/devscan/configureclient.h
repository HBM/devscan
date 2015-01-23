// Copyright 2014 Hottinger Baldwin Messtechnik
// Distributed under MIT license
// See file LICENSE provided

#ifndef _ConfigureClient_H
#define _ConfigureClient_H

#include <string>
#include <chrono>

#include "hbm/communication/multicastserver.h"
#include "hbm/communication/netadapterlist.h"

namespace hbm {
	namespace devscan {
		/// this class sends HBM scan network configuration requests, and waits for the response.
		class ConfigureClient
		{
		public:
			/// starts the multicast server listening for incoming messages
			ConfigureClient();

			/// stops the multicast server
			~ConfigureClient();

			/// \brief sets the interface configuration-method of a device
			///
			/// Sends a request and waits some time (TIMETOWAITFORANSWERS) for the answer.
			/// \warning the device is going to reboot automatically if necessary
			/// \param outgoingInterfaceIp IP address of the interface to use leave empty ("") in order to send over all interfaces
			/// \param ttl number of maximum hops to the receiver (1 do not scan behind network routers)
			/// \param uuid Device to be configured
			/// \param interfaceName Name of the interface to be configured
			/// \param method The interface configuration-method: "manual" or "dhcp"
			/// \return empty string if no answer was received. Otherwise JSON rpc response from device.
			/// \see setInterfaceConfiguration
			/// \see hbm::devscan::CONFIG_METHOD_MANUAL
			/// \see hbm::devscan::CONFIG_METHOD_DHCP
			/// \throws std::runtime_error
			std::string setInterfaceConfigurationMethod(const std::string& outgoingInterfaceIp, unsigned char ttl, const std::string& uuid, const std::string& interfaceName, const std::string& method);

			/// \warning the device is going to reboot automatically if necessary
			/// \param outgoingInterfaceIp IP address of the interface to use leave empty ("") in order to send over all interfaces
			/// \param ttl number of maximum hops to the receiver (1 do not scan behind network routers)
			/// \param uuid Device to be configured
			/// \param ipv4DefaultGateWay ip-address of the default gateway, as a string
			/// \return empty string if no answer was received. Otherwise JSON rpc response from device.
			/// \throws std::runtime_error
			std::string setDefaultGateway(const std::string& outgoingInterfaceIp, unsigned char ttl, const std::string& uuid, const std::string& ipv4DefaultGateWay);

			/// sends a request and waits some time (TIMETOWAITFORANSWERS) for the answer
			/// \warning the device is going to reboot automatically if necessary
			/// \param outgoingInterfaceIp IP address of the interface to use leave empty ("") in order to send over all interfaces
			/// \param ttl number of maximum hops to the receiver (1 do not scan behind network routers)
			/// \param uuid Device to be configured
			/// \param interfaceName Name of the interface to be configured
			/// \param address New ip-address for the interface, as a string
			/// \param netmask New netmask for the interface, as a string
			/// \return empty string if no answer was received. Otherwise JSON rpc response from device.
			/// \throws std::runtime_error
			std::string setInterfaceManualConfiguration(const std::string& outgoingInterfaceIp, unsigned char ttl, const std::string& uuid, const std::string& interfaceName, const std::string& address, const std::string& netmask);

			/// \warning the device is going to reboot automatically if necessary
			/// \param outgoingInterfaceIp IP address of the interface to use leave empty ("") in order to send over all interfaces
			/// \param ttl number of maximum hops to the receiver (1 do not scan behind network routers)
			/// \param uuid Device to be configured
			/// \param interfaceName Name of the interface to be configured
			/// \param method The interface configuration-method: "manual" or "dhcp"
			/// \param manualAddress
			/// \param manualNetmask
			/// \return empty string if no answer was received. Otherwise JSON rpc response from device.
			/// \throws std::runtime_error
			/// \see setInterfaceConfigurationMethod
			/// \see hbm::devscan::CONFIG_METHOD_MANUAL
			/// \see hbm::devscan::CONFIG_METHOD_DHCP
			std::string setInterfaceConfiguration(const std::string& outgoingInterfaceIp, unsigned char ttl, const std::string& uuid, const std::string& interfaceName, const std::string& method, const std::string& manualAddress, const std::string& manualNetmask);

			/// sends a request to multicast group CONFIG_IPV4_ADDRESS and waits some time (TIMETOWAITFORANSWERS) for the answer
			/// \param outgoingInterfaceIp  IP address of the interface to use leave empty ("") in order to send over all interfaces
			/// \param ttl  number of maximum hops to the receiver (1 do not scan behind network routers)
			/// \param id  a (unique) string identifying the request to send and match the corresponding response
			/// \param message  complete JSOM-message to send, as a plain string
			/// \return empty string if no answer was received. Otherwise JSON rpc response from device.
			/// \throws std::runtime_error
			std::string executeRequest(const std::string& outgoingInterfaceIp, unsigned char ttl, const std::string& id, const std::string& message);

		private:
			/// Each json rpc request carries an id. The response to each request will carry the id of the request.
			/// We are sending/receiving multicast messages. Hence we see responses for requests from other other clients that are not meant for us.
			/// The id created herein is a bit more noisy in order recognize the correct response.
			std::string createId() const;

			communication::NetadapterList m_netadapterList;
			communication::MulticastServer m_MulticastServer;

			static const std::chrono::milliseconds TIMETOWAITFORANSWERS;
		};
	}
}
#endif
