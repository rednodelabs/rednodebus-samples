# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

string(REPLACE " " ";" OVERLAY_CONFIG_LIST ${OVERLAY_CONFIG})
if("overlay-ot-rnb.conf" IN_LIST OVERLAY_CONFIG_LIST)
include(../common/CMakeLists.txt)
endif()

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(accelerometer)

if("overlay-ot-rnb.conf" IN_LIST OVERLAY_CONFIG_LIST)
target_compile_definitions_ifdef(CONFIG_NET_L2_OPENTHREAD ot-config INTERFACE "OPENTHREAD_CONFIG_CHILD_SUPERVISION_MSG_NO_ACK_REQUEST=1")
endif()

# APP START
target_sources(app PRIVATE src/accelerometer.c)

target_include_directories(app PUBLIC ../../include)

target_sources_ifdef(CONFIG_NET_UDP app PRIVATE src/udp.c)

target_include_directories_ifdef(CONFIG_REDNODEBUS app PUBLIC ../common/src)
target_sources_ifdef(CONFIG_REDNODEBUS app PRIVATE
  ../common/src/rnb_utils.c
  ../common/src/rnb_leds.c)
  # APP END

include(${ZEPHYR_BASE}/samples/net/common/common.cmake)

set(gen_dir ${ZEPHYR_BINARY_DIR}/include/generated/)
