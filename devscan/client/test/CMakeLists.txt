project("hbm_device_scan_test" CXX)

cmake_minimum_required(VERSION 2.8)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/../../../cmake)

# enable coverage inspection:
# used by: /devscan/client/lib/CMakeLists.txt
# compile: --coverage
# link: gcov
# option(GCOV_PACKAGE "GCOV" ON)

include(QuantumxBoost OPTIONAL)

find_package(Boost COMPONENTS unit_test_framework)

if(NOT Boost_FOUND)
    set(BOOST_LIBRARYDIR /opt/boost/1.52.0/lib)
    find_package(Boost COMPONENTS unit_test_framework)
endif(Boost_FOUND)

if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    list(APPEND ADDITIONAL_LIBS ${Boost_LIBRARIES})
endif(Boost_FOUND)


include_directories(../../..) #hbm/commuc...
include_directories(../..) #devscan
include_directories(../include)
include_directories(../../../jsoncpp/include)

add_subdirectory(../../../jsoncpp ${PROJECT_BINARY_DIR}/jsoncpp)

#add_definitions("-std=c++0x --coverage")

add_subdirectory(../lib ${PROJECT_BINARY_DIR}/lib)


set(SOURCES_SCANCLIENTTEST
    scanclienttest.cpp
)

add_executable( scanclienttest ${SOURCES_SCANCLIENTTEST} )

target_link_libraries(
    scanclienttest
    jsoncpp_lib
    scanclient-static
	gcov
    ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
)


set(SOURCES_DEVICEMONITORTEST
    devicemonitortest.cpp
)


add_executable( devicemonitor.test ${SOURCES_DEVICEMONITORTEST} )

target_link_libraries(
	devicemonitor.test
    jsoncpp_lib
    scanclient-static
	gcov
	${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
)


enable_testing()
# add_test(scanclienttest scanclienttest)
add_test(devicemonitortest devicemonitor.test
	--report_level=no
	--log_level=all
	--output_format=xml
	--log_sink=${CMAKE_BINARY_DIR}/devicemonitor_test.xml
)
