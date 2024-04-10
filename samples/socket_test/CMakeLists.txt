# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

list(APPEND DTS_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../..)
list(APPEND BOARD_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../..)

string(REPLACE " " ";" OVERLAY_CONFIG_LIST "${OVERLAY_CONFIG}")
if("overlay-ot-rnb.conf" IN_LIST OVERLAY_CONFIG_LIST)
  include(${CMAKE_CURRENT_SOURCE_DIR}/../common/CMakeLists.txt)
endif()

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(socket_test)

target_sources(app PRIVATE src/socket-test.c)

target_include_directories(app PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../include)

target_sources_ifdef(CONFIG_NET_UDP app PRIVATE src/udp.c)
target_include_directories_ifdef(CONFIG_REDNODEBUS app PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/../common/src)
target_sources_ifdef(CONFIG_REDNODEBUS app PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/../common/src/rnb_utils.c
  ${CMAKE_CURRENT_SOURCE_DIR}/../common/src/rnb_leds.c
  ${CMAKE_CURRENT_SOURCE_DIR}/../common/src/rnb_pa_lna.c)
target_sources_ifdef(CONFIG_REDNODERANGING app PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/../common/src/rnb_ranging_port.c
  ${CMAKE_CURRENT_SOURCE_DIR}/../common/src/rnb_ranging_spi.c
  ${CMAKE_CURRENT_SOURCE_DIR}/../common/src/rnb_ranging_sleep.c)
