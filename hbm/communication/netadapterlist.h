// Copyright 2014 Hottinger Baldwin Messtechnik
// Distributed under MIT license
// See file LICENSE provided

#ifndef _HBM__NETADAPTERLIST_H
#define _HBM__NETADAPTERLIST_H

#include <vector>
#include <map>
#include <string>
#include <mutex>

#include "netadapter.h"

namespace hbm {
	namespace communication {
		/// Informationen ueber alle verfuegbaren IP-Schnittstellen.
		class NetadapterList
		{
		public:
			/// interface index is the key
			typedef std::map < unsigned int, Netadapter > tAdapters;
			typedef std::vector < Netadapter > tAdapterArray;

			NetadapterList();

			tAdapters get() const;

			/// the same order as returned by get()
			tAdapterArray getArray() const;

			/// \throws hbm::exception
			Netadapter getAdapterByName(const std::string& adapterName) const;

			/// get adapter by interface index
			/// \throws hbm::exception
			Netadapter getAdapterByInterfaceIndex(unsigned int interfaceIndex) const;

			void update();

		private:

			void enumAdapters();

			tAdapters m_adapters;
			mutable std::mutex m_adaptersMtx;
		};
	}
}
#endif
