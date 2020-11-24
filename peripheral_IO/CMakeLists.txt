cmake_minimum_required(VERSION 3.13.1)

# Re-direct the directory where the 'boards' directory is found from
# $ZEPHYR_BASE to this directory (don't overlay our boards onto Zephyr tree).
get_filename_component(BOARD_DIR
                       ${CMAKE_CURRENT_LIST_DIR}/../../boards/arm/bt6
                       ABSOLUTE
)
list(APPEND BOARD_ROOT ${BOARD_DIR})

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(peripheral_IO)

include_directories(${CMAKE_SOURCE_DIR}/../common/include)
include_directories(${CMAKE_SOURCE_DIR}/../common/framework_config)

target_sources(app PRIVATE
    ${CMAKE_SOURCE_DIR}/../common/source/AdcBt6.c
    ${CMAKE_SOURCE_DIR}/../common/source/AttributeFunctions.c
    ${CMAKE_SOURCE_DIR}/../common/source/BleTask.c
    ${CMAKE_SOURCE_DIR}/../common/source/BspSupport.c
    ${CMAKE_SOURCE_DIR}/../common/source/CborBuilder.c
    ${CMAKE_SOURCE_DIR}/../common/source/ControlTask.c
    ${CMAKE_SOURCE_DIR}/../common/source/LedPwm.c
    ${CMAKE_SOURCE_DIR}/../common/source/main.c
    ${CMAKE_SOURCE_DIR}/../common/source/ProtocolTask.c
    ${CMAKE_SOURCE_DIR}/../common/source/Sentrius_mgmt.c
    ${CMAKE_SOURCE_DIR}/../common/source/Sentrius_mgmtHandlers.c
    ${CMAKE_SOURCE_DIR}/../common/source/SystemUartTask.c
    ${CMAKE_SOURCE_DIR}/../common/source/UserCommTask.c
    ${CMAKE_SOURCE_DIR}/../common/source/UserInterfaceTask.c
)

if(CONFIG_TEST_MENU)
target_sources(app PRIVATE
    ${CMAKE_SOURCE_DIR}/../common/source/TestMenu.c
)
endif()

include_directories(${CMAKE_SOURCE_DIR}/include)

target_sources(app PRIVATE
)

if(CONFIG_XXXX)
include_directories(${CMAKE_SOURCE_DIR}/xxxx/include)
target_sources(app PRIVATE
    ${CMAKE_SOURCE_DIR}/xxx/source/
    ${CMAKE_SOURCE_DIR}/xx/source/
)
endif()
