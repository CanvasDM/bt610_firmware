cmake_minimum_required(VERSION 3.13.1)

set(FWK_APP_ID_FILE_LIST
    ${CMAKE_SOURCE_DIR}/src/framework_config/framework_ids_list.h
)

set(FWK_APP_MSG_FILE_LIST
    ${CMAKE_SOURCE_DIR}/src/framework_config/framework_msg_codes_list.h
)
set(mcuboot_CONF_FILE
    ${CMAKE_SOURCE_DIR}/mcuboot_bt610.conf
)
set(mcuboot_ROOT_APP_DIR
    ${CMAKE_SOURCE_DIR}/src
)
set(mcuboot_ROOT_PROJECT_APP_DIR
    ${CMAKE_SOURCE_DIR}
)
set(PM_STATIC_YML_FILE
    ${CMAKE_SOURCE_DIR}/pm_static.sb.bt610.yml
)

set(b0_OVERLAY_CONFIG
    ${CMAKE_SOURCE_DIR}/b0_bt610.conf
)

# The component-generated files for attributes
set(ATTR_CUSTOM_PATH_BASE ${CMAKE_SOURCE_DIR}/components/attributes/bt610)

# Version information generation
# Generate build ID based on UTC timestamp
string(TIMESTAMP build_time "%s" UTC)
set(APP_VERSION_BUILD ${build_time})
set(gen_dir ${CMAKE_BINARY_DIR}/zephyr/include/generated/app)
set(ver_file ${gen_dir}/app_version.h)
configure_file(${CMAKE_SOURCE_DIR}/src/version/app_version.h.in ${ver_file})
# Get application version
file(READ ${ver_file} ver)
string(REGEX MATCH "APP_VERSION_MAJOR ([0-9]+)" _ ${ver})
set(ver_major ${CMAKE_MATCH_1})
string(REGEX MATCH "APP_VERSION_MINOR ([0-9]+)" _ ${ver})
set(ver_minor ${CMAKE_MATCH_1})
string(REGEX MATCH "APP_VERSION_PATCH ([0-9]+)" _ ${ver})
set(ver_patch ${CMAKE_MATCH_1})
set(app_version ${ver_major}.${ver_minor}.${ver_patch}+${build_time})
message("app version: ${app_version}")

# Set application version for mcuboot
set(mcuboot_ver_conf ${CMAKE_BINARY_DIR}/generated_mcuboot_image_version.conf)
set(MCUBOOT_IMAGE_VERSION ${app_version})

# This configuration file will get automatically merged when building.
# No need to append to OVERLAY_CONFIG
configure_file(${CMAKE_SOURCE_DIR}/src/version/mcuboot_image_version.conf.in ${mcuboot_ver_conf})

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(bt610_dm_firmware)

# Build the API
execute_process(
    COMMAND
    ${PYTHON_EXECUTABLE}
    ${CMAKE_SOURCE_DIR}/../modules/lib/laird_connect/attributes/generator/api.py
    ${CMAKE_SOURCE_DIR}
    bt610
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    RESULT_VARIABLE api_result
)
if(NOT "${api_result}" STREQUAL 0)
    message(FATAL_ERROR "Failed to build API")
endif()

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/src/framework_config)
include_directories(${CMAKE_SOURCE_DIR}/src/version)
target_include_directories(app PRIVATE ${gen_dir})

target_sources(app PRIVATE
    ${CMAKE_SOURCE_DIR}/src/AdcBt6.c
    ${CMAKE_SOURCE_DIR}/src/Advertisement.c
    ${CMAKE_SOURCE_DIR}/src/BleTask.c
    ${CMAKE_SOURCE_DIR}/src/BspSupport.c
    ${CMAKE_SOURCE_DIR}/src/ControlTask.c
    ${CMAKE_SOURCE_DIR}/src/EventTask.c
    ${CMAKE_SOURCE_DIR}/src/LEDs.c
    ${CMAKE_SOURCE_DIR}/src/main.c
    ${CMAKE_SOURCE_DIR}/src/NonInit.c
    ${CMAKE_SOURCE_DIR}/src/SensorTask.c
    ${CMAKE_SOURCE_DIR}/src/UserInterfaceTask.c
    ${CMAKE_SOURCE_DIR}/src/Flags.c
    ${ATTR_CUSTOM_PATH_BASE}/src/attr_custom_validator.c
)

if(CONFIG_FS_MGMT_FILE_ACCESS_HOOK)
    target_sources(app PRIVATE
        ${CMAKE_SOURCE_DIR}/src/FileAccess.c
    )
endif()

if(CONFIG_MCUMGR_CMD_PRODUCTION_MGMT)
    target_sources(app PRIVATE
        ${CMAKE_SOURCE_DIR}/src/production_mgmt.c
    )
endif()

if(CONFIG_TEST_MENU)
    target_sources(app PRIVATE
        ${CMAKE_SOURCE_DIR}/src/TestMenu.c
    )
endif()
