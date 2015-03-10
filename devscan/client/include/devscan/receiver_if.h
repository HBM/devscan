// Copyright 2014 Hottinger Baldwin Messtechnik
// Distributed under MIT license
// See file LICENSE provided

/// \mainpage DeviceScan Interface
/// \tableofcontents
///
/// \section section_introduction Introduction
///
/// The DeviceScan interface allows automatic discovery and monitoring
/// reachability of devices connected to the network.
///
/// Protocols providing similar functionality are for example:
/// <ul>
/// <li>Apple Bonjour (former: Rendezvous):
///     http://en.wikipedia.org/wiki/Bonjour_%28software%29
/// <li>UPnP's Simple Service Discovery Protocol (SSDP):
///     http://en.wikipedia.org/wiki/Simple_Service_Discovery_Protocol
/// <li>Avahi: http://avahi.org
/// </ul>
///
/// \section section_usage Usage
///
/// The abstract interface of the DeviceScan library is defined in
/// hbm.devscan.ReceiverIf
/// The library provides an executable implementation of
/// this interface in hbm.devscan.Receiver.
/// <br>
/// The user application should create an instance of the hbm.devscan.Receiver
/// class to make use of the provided DeviceScan code.
///
/// \image HTML devscan_receiver.class.png "Documented interface and provided implementation"
///
/// The hbm.devscan.Receiver class provides methods to register application
/// provided callback functions:
/// <ul>
/// <li> hbm.devscan.ReceiverIf::setAnnounceCb()
/// <li> hbm.devscan.ReceiverIf::setExpireCb()
/// <li> hbm.devscan.ReceiverIf::setErrorCb()
/// </ul>
/// It is not mandatory to register any callback function. Not making use of
/// announcement and expire callback will limit the functionality of the
/// application dramatically. Registering an error
/// callback is explicitly optional, as it serves for debugging only.
/// <p>
///
/// The application provided callback functions need to match the following
/// signatures:
/// <ul>
/// <li> hbm.devscan.announceCb_t
/// <li> hbm.devscan.expireCb_t
/// <li> hbm.devscan.errorCb_t
/// </ul>
/// See the documentation of theses typedefs for information on the data passed
/// to the user's application.
///
/// <p>
/// Execution time of the hbm.devscan.Receiver can be controlled with the methods:
/// <ul>
/// <li> hbm.devscan.ReceiverIf.start()
/// <li> hbm.devscan.ReceiverIf.stop()
/// </ul>
/// Registration of callbacks can be done at any time, regardless of the running
/// status of the hbm.devscan.Receiver object.
///
/// \subsection section_sequence Sequence
///
/// \image HTML devscan_client.sequence.png "Typical usage of the ReceiverIf"
///
/// \subsection section_examples Examples
///
/// scanclientnotifier main:
/// \snippet scanclientnotifier.cpp main


#ifndef _RECEIVER_IF_H
#define _RECEIVER_IF_H

#include <functional>
#include <chrono>


/// \namespace hbm
/// \brief Namespace for software by Hottinger Baldwin Messtechnik.
///
/// Any source code from HBM should use this namespace.
/// \see http://www.hbm.com
namespace hbm {

	/// \namespace hbm::devscan
	/// \brief Namespace for the device scanning interface.
	///
	/// All identifiers in this project are using this namespace.
	namespace devscan {

		/// \typedef announceCb_t
		/// \brief Typedef for the user provided callback function which is called once a new device is discovered.
		///
		/// This user-provided callback is called whenever a new device is
		/// detected on the network. When the callback is called, it is ensured
		/// that the announcement message is valid and contains all mandatory
		/// information (in other cases, the error callback is called).
		///
		/// \param uuid  the device's unique id, as transmitted in the network message
		/// \param receivingInterfaceName  the name of the host's network adapter
		///                                receiving the announcement network message,
		///                                on Linux for example: eth0
		/// \param sendingInterfaceName
		/// \param announcement  the received network message, as a string
		/// \see ReceiverIf::setAnnounceCb()
		/// \see expireCb_t
		typedef std::function < void (const std::string uuid, const std::string& receivingInterfaceName, const std::string& sendingInterfaceName, const std::string& router, const std::string& announcement) > announceCb_t;

		/// \typedef expireCb_t
		/// \brief Typedef for the user provided callback function which is called once a previously discovered device disappears.
		///
		/// This user-provided callback is called whenever no announcement
		/// messages are received from a certain device on the network for period
		/// defined in the last device's announcement message.
		///
		/// \param uuid  the device's unique id, transmitted in the network message
		/// \param receivingInterfaceName  The name of the host's network adapter
		///                                receiving the announcement network message,
		///                                on Linux for example: eth0
		/// \param sendingInterfaceName
		/// \see ReceiverIf::setExpireCb()
		/// \see announceCb_t
		typedef std::function < void (const std::string uuid, const std::string& receivingInterfaceName, const std::string& sendingInterfaceName, const std::string& router) > expireCb_t;


		/// \struct cb_t
		/// \brief A container/namespace for error codes passed with the error-callback.
		///
		/// Values used as the error code in the error-callback function.
		/// \see errorCb_t
		struct cb_t
		{
			/// \brief Bitmask: The received data was discarded during processing
			///
			/// Nothing was forwarded to the announceCb or expireCb
			static const uint32_t DATA_DROPPED = 0x80000000;

			/// \brief Bitmask: Error number
			///
			/// To extract the unique error number, which is one of the
			/// following values.
			static const uint32_t ERROR_MASK   = 0x0000FFFF;

			/// \brief Error: The message could not be parsed
			static const uint32_t ERROR_PARSE  = 0x00000001;
			/// \brief Error: The JSON-message does not contain an announcement node
			static const uint32_t ERROR_METHOD = 0x00000002;
			/// \brief Error: The JSON-message does not contain an ip-address
			static const uint32_t ERROR_IPADDR = 0x00000003;
			/// \brief Error: The JSON-message does not contain a uuid
			static const uint32_t ERROR_UUID   = 0x00000004;
			/// \brief Error: The JSON-message does not contain an expiration node
			static const uint32_t ERROR_EXPIRE = 0x00000005;
			/// \brief Error: An internal exception occurred
			static const uint32_t E_EXCEPTION1 = 0x00000006;
			/// \brief Error: An internal exception occurred
			static const uint32_t E_EXCEPTION2 = 0x00000007;
		};

		/// \typedef errorCb_t
		/// \brief Typedef for the user provided callback function which is called on any incorrect data received via the network.
		///
		/// Any packet receiving on the network passes multiple parsing and interpretation steps.
		/// This callback function serves as the reporting interface for all the "internal" processing.
		/// <br>
		/// Making use if this callback is basically for debugging purpose. The registration is optional.
		/// <br>
		/// When the application receives the error callback it does not
		/// necessarily mean, that the same message is not sent via the announce
		/// callback as well. See hbm::devscan::cb_t::DATA_DROPPED.
		///
		/// \param errorcode  a bit-coded error code, synthesized from constants
		///                   in hbm::devscan::cb_t
		/// \param userMessage  a (english) human readable message describing the
		///                     cause of the error
		/// \param receivedData  the message as it was received from the network
		///
		/// \see ReceiverIf::setErrorCb()
		/// \see cb_t
		typedef std::function < void (uint32_t errorcode, const std::string& userMessage, const std::string& receivedData) > errorCb_t;

		/// \interface ReceiverIf
		/// \brief The interface to use by HBM Scan Clients.
		///
		/// An instance of a class implementing this interface is needed by the
		/// client application. The class Receiver is an existing implementation
		/// that can be used directly by a client application.
		///
		/// \see Receiver
		class ReceiverIf
		{
		public:

			/// Sets the callback method to be called on arrival of new or change of an already known announcement.
			virtual void setAnnounceCb(announceCb_t cb) = 0;

			/// Sets the callback method to be called on expiration of an announcement.
			virtual void setExpireCb(expireCb_t cb) = 0;

			/// Sets the callback method to be called on errors in the received network telegram.
			virtual void setErrorCb(errorCb_t cb) = 0;

			/// \brief Starts event loop that collects announcements messages
			///        from the network and calls callback functions.
			///
			/// The event loop collects announcements and retires expired announcements.
			/// Under Linux network events (like new network interfaces) are handled too.
			/// Returns on execution of stop() or if an error occurs.
			virtual void start() = 0;

			/// \brief Starts event loop that collects announcements messages
			///        from the network and calls callback functions.
			///
			/// The event loop collects announcements and retires expired announcements.
			/// Under Linux network events (like new network interfaces) are handled too.
			/// Returns after the specified time, on execution of stop() or if an error occurs.
			/// \param timeOfExecution  amount of time in ms to execute.
			virtual void start_for(std::chrono::milliseconds timeOfExecution) = 0;



			/// \brief Stops the event loop that was started with ReceiverIf.start.
			/// The start-call will return.
			/// \see start
			virtual void stop() = 0;

		protected:
			virtual ~ReceiverIf()
			{
			}
		};
	}
}

#endif // _RECEIVER_IF_H
