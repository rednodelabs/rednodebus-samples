# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

set(OT_NCP_VENDOR_HOOK_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

list(APPEND DTS_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../..)
list(APPEND BOARD_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../..)

string(REPLACE " " ";" OVERLAY_CONFIG_LIST "${OVERLAY_CONFIG}")
if("overlay-ot-rnb.conf" IN_LIST OVERLAY_CONFIG_LIST)
  include(${CMAKE_CURRENT_SOURCE_DIR}/../common/CMakeLists.txt)
endif()

if("overlay-usb-rnb.conf" IN_LIST OVERLAY_CONFIG_LIST)
  list(APPEND DTC_OVERLAY_FILE ${CMAKE_CURRENT_SOURCE_DIR}/boards/${BOARD}_usb.overlay)
endif()

include(CheckGit.cmake)
CheckCoprocessorGitSetup()

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(coprocessor)

target_sources(app PRIVATE src/main.c src/nrf_802154_radio_wrapper.c ${CMAKE_BINARY_DIR}/generated/rnb_coprocessor_version.c)

target_include_directories_ifdef(CONFIG_REDNODEBUS app PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/../common/src)
target_sources_ifdef(CONFIG_REDNODEBUS app PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/../common/src/rnb_leds.c
  ${CMAKE_CURRENT_SOURCE_DIR}/../common/src/rnb_pa_lna.c)

target_include_directories(app PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../include)
