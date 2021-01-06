cmake_minimum_required(VERSION 3.13.1)

# Re-direct the directory where the 'boards' directory is found from
# $ZEPHYR_BASE to this directory (don't overlay our boards onto Zephyr tree).
get_filename_component(BOARD_DIR
                       ${CMAKE_CURRENT_LIST_DIR}/../../boards/arm/bt6
                       ABSOLUTE
)
list(APPEND BOARD_ROOT ${BOARD_DIR})

# Use static partition layout to ensure consistency between builds.
# This is to ensure settings storage will be at the same location after the DFU.
set(PM_STATIC_YML_FILE
  ${CMAKE_SOURCE_DIR}/pm_static_bt6.yml
)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(peripheral_IO)

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/../common/include)
include_directories(${CMAKE_SOURCE_DIR}/../common/framework_config)

target_sources(app PRIVATE
    ${CMAKE_SOURCE_DIR}/../common/source/AdcBt6.c
    ${CMAKE_SOURCE_DIR}/../common/source/Advertisement.c
    ${CMAKE_SOURCE_DIR}/../common/source/AttributeTable.c
    ${CMAKE_SOURCE_DIR}/../common/source/Attribute.c
    ${CMAKE_SOURCE_DIR}/../common/source/AttributeValidator.c
    ${CMAKE_SOURCE_DIR}/../common/source/BleTask.c
    ${CMAKE_SOURCE_DIR}/../common/source/BspSupport.c
    ${CMAKE_SOURCE_DIR}/../common/source/ControlTask.c
    ${CMAKE_SOURCE_DIR}/../common/source/main.c
    ${CMAKE_SOURCE_DIR}/../common/source/Sentrius_mgmt.c
    ${CMAKE_SOURCE_DIR}/../common/source/SensorTask.c
    ${CMAKE_SOURCE_DIR}/../common/source/UserInterfaceTask.c
    ${CMAKE_SOURCE_DIR}/../common/source/EventLog.c
)

if(CONFIG_TEST_MENU)
target_sources(app PRIVATE
    ${CMAKE_SOURCE_DIR}/../common/source/TestMenu.c
)
endif()

if(CONFIG_ATTR_SHELL)
target_sources(app PRIVATE
    ${CMAKE_SOURCE_DIR}/../common/source/AttributeShell.c
)
endif()
