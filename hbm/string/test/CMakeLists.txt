project("hbm_utility_test" CXX)

cmake_minimum_required(VERSION 2.8)


set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/../../../cmake)


SET(CMAKE_CXX_FLAGS "-std=c++0x" CACHE STRING "" FORCE)

option(WITH_CPPCHECK "CppCheck" OFF)

if(WITH_CPPCHECK)
  find_package(cppcheck)
  include(CppcheckTargets)
endif()

message(STATUS "-----------------------------------------------------------")
message(STATUS "CppCheck            : " ${WITH_CPPCHECK})
message(STATUS "-----------------------------------------------------------")


include(QuantumxBoost)

find_package(Boost COMPONENTS unit_test_framework)
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
  list(APPEND ADDITIONAL_LIBS ${Boost_LIBRARIES})
endif(Boost_FOUND)

# set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/../../../cmake)


# enable coverage inspection:
# used by: /devscan/client/lib/CMakeLists.txt
# compile: --coverage
# link: gcov
option(GCOV_PACKAGE "GCOV" ON)


include_directories(..)


#add_definitions("-std=c++0x --coverage")


set(SOURCES_SPLITTEST
	splittest.cpp
	../split.cpp
)

add_executable( splittest ${SOURCES_SPLITTEST} )

target_link_libraries( splittest
	gcov
    ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
)

enable_testing()
add_test(splittest splittest.test
	--report_level=no
	--log_level=all
	--output_format=xml
	--log_sink=${CMAKE_BINARY_DIR}/splittest.xml
)

set(SOURCES_REPLACETEST
	replacetest.cpp
	../replace.cpp
)

add_executable( replacetest ${SOURCES_REPLACETEST} )

target_link_libraries( replacetest
	gcov
    ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
)

enable_testing()
add_test(replacetest replacetest.test
	--report_level=no
	--log_level=all
	--output_format=xml
	--log_sink=${CMAKE_BINARY_DIR}/replacetest.xml
)

set(SOURCES_TRIMTEST
	trimtest.cpp
	../trim.cpp
)

add_executable( trimtest ${SOURCES_TRIMTEST} )

target_link_libraries( trimtest
	gcov
	${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
)

enable_testing()
add_test(trimtest trimtest.test
	--report_level=no
	--log_level=all
	--output_format=xml
	--log_sink=${CMAKE_BINARY_DIR}/trimtest.xml
)
