cmake_minimum_required(VERSION 3.13.1)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(peripheral_hids)

FILE(GLOB app_sources src/*.c)
target_sources(app PRIVATE ${app_sources})
zephyr_include_directories(${ZEPHYR_BASE}/include/power)
zephyr_include_directories(${ZEPHYR_BASE}/include/drivers)
include_directories(${CMAKE_SOURCE_DIR}/include)

#target_include_directories(TINYCBOR INTERFACE include/tinycbor)
#target_include_directories(TINYCBOR INTERFACE include)


FILE(GLOB fwk_sources ./../../modules/framework/source/*.c)
target_sources(app PRIVATE ${fwk_sources})
include_directories(${CMAKE_SOURCE_DIR}/../../modules/framework/include)
include_directories(${CMAKE_SOURCE_DIR}/framework_config)


