// Copyright 2014 Hottinger Baldwin Messtechnik
// Distributed under MIT license
// See file LICENSE provided

#include <iostream>
#include <vector>


#ifndef _WIN32
#define BOOST_TEST_DYN_LINK
#endif
#define BOOST_TEST_MODULE StringSplitTest
#include <boost/test/unit_test.hpp>

#include "../split.h"


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


				static bool equals(std::vector<std::string> a, std::vector<std::string> b)
				{
					size_t asize = a.size();
					size_t bsize = b.size();

					if(asize != bsize)
					{
						std::string message = "Vectors differ in size: "+std::to_string(asize)+" != "+std::to_string(bsize);
						BOOST_ERROR(message);
					}

					size_t minsize = std::min(asize, bsize);

					std::vector<std::string>::iterator ait = a.begin();
					std::vector<std::string>::iterator bit = b.begin();

					for(size_t index=0 ; index < minsize ; index++)
					{
						if( (*ait).compare(*bit) != 0 )
						{
							std::string message = "Vectors differ at ["+std::to_string(index)+"] ";
							message += "\"" + *ait + "\" != \"" + *bit + "\"";
							BOOST_ERROR(message);
							return false;
						}
						ait++;
						bit++;
					}
					return true;
				}
			};

			BOOST_FIXTURE_TEST_SUITE( Fixture1_Test, Fixture1 )

/*
			BOOST_AUTO_TEST_CASE( test_selfcheck )
			{
				Fixture1::equals({"ABC","xyz"}, {"ABC","123"});
				Fixture1::equals({"ABC","xyz"}, {"ABC"});
			}
*/

			BOOST_AUTO_TEST_CASE( test_case_common )
			{
				Fixture1::equals(hbm::string::split("abc:defgh:ijkl", ":"), {"abc","defgh","ijkl"});
				Fixture1::equals(hbm::string::split("1.2.3", "."), {"1","2","3"});
			}

			BOOST_AUTO_TEST_CASE( test_case_empty )
			{
				Fixture1::equals(hbm::string::split("", ":"), {""});
				Fixture1::equals(hbm::string::split("abd:defgh:ijkl", ""), {"abd:defgh:ijkl"});
			}

			BOOST_AUTO_TEST_CASE( test_case_null )
			{
				BOOST_CHECK_THROW(hbm::string::split(NULL, ":"), std::logic_error);
				BOOST_CHECK_THROW(hbm::string::split("abd:def", NULL), std::logic_error);
			}

			BOOST_AUTO_TEST_CASE( test_case_pattern )
			{
				Fixture1::equals(hbm::string::split("abd:de::fgh:ijk::l", "::"), {"abd:de","fgh:ijk","l"});

				Fixture1::equals(hbm::string::split("::a::bc::def::g::", "::"), {"","a","bc","def","g",""});
				Fixture1::equals(hbm::string::split("........", ".."), {"","","","",""});
				Fixture1::equals(hbm::string::split("........", "..."), {"","",".."});
				Fixture1::equals(hbm::string::split("........", "...."), {"","",""});
				Fixture1::equals(hbm::string::split("........", "....."), {"","..."});
			}

			BOOST_AUTO_TEST_CASE( test_case_split_at_ends )
			{
				Fixture1::equals(hbm::string::split(".1234.5678", "."), {"","1234","5678"});
				Fixture1::equals(hbm::string::split(".12345678", "."), {"","12345678"});
				Fixture1::equals(hbm::string::split("1234.5678.", "."), {"1234","5678",""});
				Fixture1::equals(hbm::string::split("12345678.", "."), {"12345678",""});
				Fixture1::equals(hbm::string::split(".1234.5678.", "."), {"","1234","5678",""});
				Fixture1::equals(hbm::string::split("1234.5678", "."), {"1234","5678"});
				Fixture1::equals(hbm::string::split("12345678", "."), {"12345678"});
			}

			BOOST_AUTO_TEST_CASE( test_case_split_at_ends2 )
			{
				Fixture1::equals(hbm::string::split("...","."), {"","","",""});
				Fixture1::equals(hbm::string::split(".", "."), {"",""});
			}

			BOOST_AUTO_TEST_SUITE_END()
		}
	}
}
