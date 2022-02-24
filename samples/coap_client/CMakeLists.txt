# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

string(REPLACE " " ";" OVERLAY_CONFIG_LIST ${OVERLAY_CONFIG})
if("overlay-ot-rnb.conf" IN_LIST OVERLAY_CONFIG_LIST)
include(../common/CMakeLists.txt)
endif()

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(openthread_coap_client)

if("overlay-ot-rnb.conf" IN_LIST OVERLAY_CONFIG_LIST)
target_compile_definitions_ifdef(CONFIG_NET_L2_OPENTHREAD ot-config INTERFACE "OPENTHREAD_CONFIG_CHILD_SUPERVISION_MSG_NO_ACK_REQUEST=1")
endif()

# APP START
target_sources(app PRIVATE src/coap_client.c
			   src/coap_client_utils.c)

target_sources_ifdef(CONFIG_REDNODEBUS app PRIVATE src/rnb_utils.c)

target_include_directories(app PUBLIC ../coap_server/interface)
# APP END

target_sources_ifdef(CONFIG_BT_NUS app PRIVATE src/ble_utils.c)
