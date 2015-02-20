// Copyright 2014 Hottinger Baldwin Messtechnik
// Distributed under MIT license
// See file LICENSE provided

#include <iostream>
#include <vector>


#ifndef _WIN32
#define BOOST_TEST_DYN_LINK
#endif
#define BOOST_TEST_MODULE StringTrimTest
#include <boost/test/unit_test.hpp>


#include <iostream>

#include "../trim.h"


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

			BOOST_FIXTURE_TEST_SUITE( trim_copy_Test, Fixture1 )
			BOOST_AUTO_TEST_CASE( test_case_right )
			{
				std::string result = hbm::string::trim_copy("hallo  ");
				BOOST_CHECK_EQUAL(result, "hallo");
			}

			BOOST_AUTO_TEST_CASE( test_case_left )
			{
				std::string result = hbm::string::trim_copy("  hallo");
				BOOST_CHECK_EQUAL(result, "hallo");
			}

			BOOST_AUTO_TEST_CASE( test_case_both )
			{
				std::string result = hbm::string::trim_copy("  hallo  ");
				BOOST_CHECK_EQUAL(result, "hallo");
			}

			BOOST_AUTO_TEST_CASE( test_case_nothing )
			{
				std::string result = hbm::string::trim_copy("hallo");
				BOOST_CHECK_EQUAL(result, "hallo");
			}

			BOOST_AUTO_TEST_CASE( test_case_empty )
			{
				std::string result = hbm::string::trim_copy("    ");
				BOOST_CHECK_EQUAL(result, "");
			}
			BOOST_AUTO_TEST_SUITE_END()




			BOOST_FIXTURE_TEST_SUITE( trim_Test, Fixture1 )
			BOOST_AUTO_TEST_CASE( test_case_right )
			{
				std::string text = "hallo  ";
				hbm::string::trim(text);
				BOOST_CHECK_EQUAL(text, "hallo");
			}

			BOOST_AUTO_TEST_CASE( test_case_left )
			{
				std::string text = "  hallo";
				hbm::string::trim(text);
				BOOST_CHECK_EQUAL(text, "hallo");
			}

			BOOST_AUTO_TEST_CASE( test_case_both )
			{
				std::string text = "  hallo  ";
				hbm::string::trim(text);
				BOOST_CHECK_EQUAL(text, "hallo");
			}

			BOOST_AUTO_TEST_CASE( test_case_nothing )
			{
				std::string text = "hallo";
				hbm::string::trim(text);
				BOOST_CHECK_EQUAL(text, "hallo");
			}

			BOOST_AUTO_TEST_CASE( test_case_empty )
			{
				std::string text = "    ";
				hbm::string::trim(text);
				BOOST_CHECK_EQUAL(text, "");
			}
			BOOST_AUTO_TEST_SUITE_END()
		}
	}
}
