include_directories(..)
#include_directories(../../..)
if(JSONCPP_FOUND)
  include_directories(${JSONCPP_INCLUDE_DIRS})
endif()

set( INTERFACE_INCLUDE_DIR ../include/devscan)

include_directories(${INTERFACE_INCLUDE_DIR})

SET( INTERFADE_HEADERS
    ${INTERFACE_INCLUDE_DIR}/configureclient.h
    ${INTERFACE_INCLUDE_DIR}/devicemonitor.h
    ${INTERFACE_INCLUDE_DIR}/receiver.h
    ${INTERFACE_INCLUDE_DIR}/receiver_if.h
)


#########################################################################
## SCANCLIENT LIBRARY
## Library used by all programs acting as client of the HBM scan protocol
#########################################################################

# SET(CMAKE_CXX_FLAGS "-std=c++11" CACHE STRING "" FORCE)

set(SOURCES_SCANCLIENT_OWN

  # concerning client software running on PC
  configureclient.cpp
  devicemonitor.cpp
  receiver.cpp
)

set(SOURCES_SCANCLIENT

  ${INTERFADE_HEADERS}
  # common for error handling
  ../../../hbm/exception/exception.hpp

  # common communication stuff
  ../../../hbm/communication/multicastserver.cpp
  ../../../hbm/communication/netadapter.cpp
  ../../../hbm/communication/netadapterlist.cpp
  ../../../hbm/communication/linux/netlink.cpp

  # common operating system abstraction
  ../../../hbm/sys/linux/eventloop.cpp
  ../../../hbm/sys/linux/timer.cpp
  ../../../hbm/sys/linux/notifier.cpp

  ../../../hbm/string/split.cpp

  ${SOURCES_SCANCLIENT_OWN}
)


add_library(scanclient-static STATIC ${SOURCES_SCANCLIENT})


# GCOV_PACKAGE is defined in devscan/client/test/CMakeLists.txt

if(GCOV_PACKAGE)
  SET_SOURCE_FILES_PROPERTIES(${SOURCES_SCANCLIENT_OWN} PROPERTIES COMPILE_FLAGS "--coverage")
endif()
