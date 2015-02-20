// Copyright 2014 Hottinger Baldwin Messtechnik
// Distributed under MIT license
// See file LICENSE provided

#include <iostream>
#include <vector>


#ifndef _WIN32
#define BOOST_TEST_DYN_LINK
#endif
#define BOOST_TEST_MODULE StringReplaceTest
#include <boost/test/unit_test.hpp>


#include <iostream>

#include "../replace.h"


namespace hbm {
	namespace string {
		namespace test {


			struct Fixture1
			{
			public:
				Fixture1()
				{
					BOOST_TEST_MESSAGE("setup Fixture1");
				}

				~Fixture1()
				{
					BOOST_TEST_MESSAGE("teardown Fixture1");
				}
			};

			BOOST_FIXTURE_TEST_SUITE( Fixture1_Test, Fixture1 )

			BOOST_AUTO_TEST_CASE( test_case_replace_char )
			{
				std::string result = hbm::string::replace("abc:defgh:ijkl", ':', ' ');
				BOOST_CHECK_EQUAL(result, "abc defgh ijkl");
				result = hbm::string::replace("1.2.3", '.', ' ');
				BOOST_CHECK_EQUAL(result, "1 2 3");

				// replace with same should not end in infinite loop!
				result = hbm::string::replace("1.2.3", '.', '.');
			}

			BOOST_AUTO_TEST_CASE( test_case_replace_string )
			{
				std::string result = hbm::string::replace("abc::defgh::ijkl", "::", "  ");
				BOOST_CHECK_EQUAL(result, "abc  defgh  ijkl");
				result = hbm::string::replace("1..2..3", "..", "  ");
				BOOST_CHECK_EQUAL(result, "1  2  3");

				// replace with same should not end in infinite loop!
				result = hbm::string::replace("1..2..3", "..", "..");
				BOOST_CHECK_EQUAL(result, "1..2..3");
			}
			BOOST_AUTO_TEST_SUITE_END()
		}
	}
}
