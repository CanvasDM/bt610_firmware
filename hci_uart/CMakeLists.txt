# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.13.1)

# Re-direct the directory where the 'boards' directory is found from
# $ZEPHYR_BASE to this directory (don't overlay our boards onto Zephyr tree).
get_filename_component(BOARD_DIR
                       ${CMAKE_CURRENT_LIST_DIR}/../../boards/arm/bt6
                       ABSOLUTE
)
list(APPEND BOARD_ROOT ${BOARD_DIR})

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(hci_uart)

target_sources(app PRIVATE src/main.c)
