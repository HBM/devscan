project("hbm_scan_client" CXX)

cmake_minimum_required(VERSION 2.8)

SET(CMAKE_CXX_FLAGS "-std=c++0x" CACHE STRING "" FORCE)

option(WITH_CPPCHECK "CppCheck" OFF)

if(WITH_CPPCHECK)
  find_package(cppcheck)
  include(CppcheckTargets)
endif()

message(STATUS "-----------------------------------------------------------")
message(STATUS "CppCheck            : " ${WITH_CPPCHECK})
message(STATUS "-----------------------------------------------------------")

include_directories(..)
include_directories(../..)
include_directories(../../jsoncpp/include)
include_directories(include)

add_subdirectory(../../jsoncpp ${PROJECT_BINARY_DIR}/jsoncpp)

#add_subdirectory(test ${PROJECT_BINARY_DIR}/test)

# the library with all client specific stuff
add_subdirectory(lib ${PROJECT_BINARY_DIR}/lib)

###################################################################
## SCANCLIENT_COMPACT
## Receive all announcements for a specified time and print them in
## a compact form
###################################################################
set(SOURCES_SCANCLIENT_COMPACT
  scanclientcompact.cpp
  )

add_executable( scanclientcompact.bin ${SOURCES_SCANCLIENT_COMPACT} )

if(CPPCHECK_FOUND)
  add_cppcheck_sources(scanclientcompact.bin ALL ${SOURCES_SCANCLIENT_COMPACT})
  add_xml_cppcheck(scanclientcompact.bin ALL )
endif()

target_link_libraries( scanclientcompact.bin
  scanclient-static
  jsoncpp_lib
  )

###################################################################
## SCANCLIENT_PRINT
## Dump all new, changed and removed announcements until program
## is stopped
###################################################################
set(SOURCES_SCANCLIENT_PRINT
  scanclientprint.cpp
  )

add_executable( scanclientprint.bin ${SOURCES_SCANCLIENT_PRINT} )
target_link_libraries( scanclientprint.bin
  scanclient-static
  jsoncpp_lib
  )

if(CPPCHECK_FOUND)
  add_cppcheck_sources(scanclientprint.bin ALL ${SOURCES_SCANCLIENT_PRINT})
  add_xml_cppcheck(scanclientprint.bin)
endif(CPPCHECK_FOUND)


###################################################################
## SCANCLIENT_NOTIFIER
## Notify all new, changed and removed announcements until program
## is stopped
###################################################################
set(SOURCES_SCANCLIENT_NOTIFIER
  scanclientnotifier.cpp
)

add_executable( scanclientnotifier.bin ${SOURCES_SCANCLIENT_NOTIFIER} )
target_link_libraries( scanclientnotifier.bin
scanclient-static
  jsoncpp_lib
)

if(CPPCHECK_FOUND)
  add_cppcheck_sources(scanclientnotifier.bin ALL ${SOURCES_SCANCLIENT_NOTIFIER})
  add_xml_cppcheck(scanclientnotifier.bin)
endif(CPPCHECK_FOUND)


###################################################################
## CONFIGUREINTERFACE
## Send network configuration requests via the HBM scan protocol.
## See speicifaction for details.
###################################################################
set(SOURCES_CONFIGUREINTERFACE
  configureinterface.cpp
  )

add_executable( configureinterface.bin ${SOURCES_CONFIGUREINTERFACE} )
target_link_libraries( configureinterface.bin
  scanclient-static
  jsoncpp_lib
  )

if(CPPCHECK_FOUND)
  add_cppcheck_sources(configureinterface.bin ALL ${SOURCES_CONFIGUREINTERFACE})
  add_xml_cppcheck(configureinterface.bin)
endif(CPPCHECK_FOUND)

