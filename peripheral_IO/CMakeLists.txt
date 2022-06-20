cmake_minimum_required(VERSION 3.13.1)

set(FWK_APP_ID_FILE_LIST
    ${CMAKE_SOURCE_DIR}/../common/framework_config/framework_ids_list.h
)

set(FWK_APP_MSG_FILE_LIST
    ${CMAKE_SOURCE_DIR}/../common/framework_config/framework_msg_codes_list.h
)

# Enable floating point support in MCU MGR. Needs to be enabled here
# for visibility in underlying MCU MGR files.
add_definitions(-DFLOAT_SUPPORT=1)

# Prepend the additional boards directory but still allow the original Zephyr
# boards directory to be used with lower priority
list(PREPEND BOARD_ROOT ${CMAKE_CURRENT_LIST_DIR}/../../)

# The component-generated files for attributes
set(ATTR_CUSTOM_PATH_BASE ${CMAKE_SOURCE_DIR}/../components/attributes)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(peripheral_IO)

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/../common/include)
include_directories(${CMAKE_SOURCE_DIR}/../common/framework_config)

target_sources(app PRIVATE
    ${CMAKE_SOURCE_DIR}/../common/source/AdcBt6.c
    ${CMAKE_SOURCE_DIR}/../common/source/Advertisement.c
    ${CMAKE_SOURCE_DIR}/../common/source/AggregationCount.c
    ${CMAKE_SOURCE_DIR}/../common/source/BleTask.c
    ${CMAKE_SOURCE_DIR}/../common/source/BspSupport.c
    ${CMAKE_SOURCE_DIR}/../common/source/ControlTask.c
    ${CMAKE_SOURCE_DIR}/../common/source/EventTask.c
    ${CMAKE_SOURCE_DIR}/../common/source/FileAccess.c
    ${CMAKE_SOURCE_DIR}/../common/source/HalfFloatDecoder.c
    ${CMAKE_SOURCE_DIR}/../common/source/LEDs.c
    ${CMAKE_SOURCE_DIR}/../common/source/main.c
    ${CMAKE_SOURCE_DIR}/../common/source/NonInit.c
    ${CMAKE_SOURCE_DIR}/../common/source/SensorTask.c
    ${CMAKE_SOURCE_DIR}/../common/source/UserInterfaceTask.c
    ${ATTR_CUSTOM_PATH_BASE}/src/attr_custom_validator_bluetooth_flags.c
    )

if(CONFIG_MCUMGR_CMD_SENTRIUS_MGMT)
target_sources(app PRIVATE
    ${CMAKE_SOURCE_DIR}/../common/source/Sentrius_mgmt.c
)
endif()

if(CONFIG_TEST_MENU)
target_sources(app PRIVATE
    ${CMAKE_SOURCE_DIR}/../common/source/TestMenu.c
)
endif()
