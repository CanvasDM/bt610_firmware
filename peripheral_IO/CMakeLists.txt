cmake_minimum_required(VERSION 3.13.1)

set(FWK_APP_ID_FILE_LIST
    ${CMAKE_SOURCE_DIR}/../common/framework_config/framework_ids_list.h
)

set(FWK_APP_MSG_FILE_LIST
    ${CMAKE_SOURCE_DIR}/../common/framework_config/framework_msg_codes_list.h
)
set(mcuboot_CONF_FILE
    ${CMAKE_SOURCE_DIR}/mcuboot_bt610.conf
)
set(mcuboot_ROOT_APP_DIR
    ${CMAKE_SOURCE_DIR}/../common
)
set(mcuboot_ROOT_PROJECT_APP_DIR
    ${CMAKE_SOURCE_DIR}/../peripheral_IO
)
set(PM_STATIC_YML_FILE
    ${CMAKE_SOURCE_DIR}/pm_static.sb.bt610.yml
)

set(b0_OVERLAY_CONFIG
    ${CMAKE_SOURCE_DIR}/b0_bt610.conf
)

# The component-generated files for attributes
set(ATTR_CUSTOM_PATH_BASE ${CMAKE_SOURCE_DIR}/../components/attributes/bt610)

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
    ${CMAKE_SOURCE_DIR}/../common/source/LEDs.c
    ${CMAKE_SOURCE_DIR}/../common/source/main.c
    ${CMAKE_SOURCE_DIR}/../common/source/NonInit.c
    ${CMAKE_SOURCE_DIR}/../common/source/SensorTask.c
    ${CMAKE_SOURCE_DIR}/../common/source/UserInterfaceTask.c
    ${ATTR_CUSTOM_PATH_BASE}/src/attr_custom_validator_bluetooth_flags.c
    ${ATTR_CUSTOM_PATH_BASE}/src/attr_custom_validator_event_filter_flags.c
    ${ATTR_CUSTOM_PATH_BASE}/src/attr_custom_validator.c
)

if(CONFIG_FS_MGMT_FILE_ACCESS_HOOK)
    target_sources(app PRIVATE
        ${CMAKE_SOURCE_DIR}/../common/source/FileAccess.c
    )
endif()

if(CONFIG_MCUMGR_CMD_PRODUCTION_MGMT)
    target_sources(app PRIVATE
        ${CMAKE_SOURCE_DIR}/../common/source/production_mgmt.c
    )
endif()

if(CONFIG_TEST_MENU)
    target_sources(app PRIVATE
        ${CMAKE_SOURCE_DIR}/../common/source/TestMenu.c
    )
endif()
