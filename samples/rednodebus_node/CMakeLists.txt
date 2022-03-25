# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.13.1)

string(REPLACE " " ";" OVERLAY_CONFIG_LIST ${OVERLAY_CONFIG})
if("overlay-ot-rnb.conf" IN_LIST OVERLAY_CONFIG_LIST)
include(../common/CMakeLists.txt)
endif()

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(rednodebus_node)

if("overlay-ot-rnb.conf" IN_LIST OVERLAY_CONFIG_LIST)
target_compile_definitions_ifdef(CONFIG_NET_L2_OPENTHREAD ot-config INTERFACE "OPENTHREAD_CONFIG_CHILD_SUPERVISION_MSG_NO_ACK_REQUEST=1")
endif()

target_sources(app PRIVATE src/main.c)
target_include_directories(app PUBLIC ../../include)
target_sources_ifdef(CONFIG_REDNODEBUS app PRIVATE src/rnb_utils.c)
