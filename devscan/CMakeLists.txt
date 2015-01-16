project("hbm_scan" CXX)

cmake_minimum_required(VERSION 2.8)

if(HBM_TARGETS)
  option(scanclient_package            "devscan client software" OFF)
else(HBM_TARGETS)
  option(scanclient_package            "devscan client software" ON)
endif(HBM_TARGETS)


add_subdirectory(device ${PROJECT_BINARY_DIR}/device)

if(scanclient_package)
  add_subdirectory(client ${PROJECT_BINARY_DIR}/client)
endif(scanclient_package)
