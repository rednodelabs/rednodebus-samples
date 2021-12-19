#
# Copyright (c) 2020 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

cmake_minimum_required(VERSION 3.13.1)

set(OT_NCP_VENDOR_HOOK_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

string(REPLACE " " ";" OVERLAY_CONFIG_LIST ${OVERLAY_CONFIG})

include(../common/CMakeLists.txt)

include(CheckGit.cmake)
CheckCoprocessorGitSetup()

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(openthread_coprocessor)

if("overlay-rcp-rnb.conf" IN_LIST OVERLAY_CONFIG_LIST)
target_compile_definitions_ifdef(CONFIG_NET_L2_OPENTHREAD ot-config INTERFACE "OPENTHREAD_CONFIG_CHILD_SUPERVISION_MSG_NO_ACK_REQUEST=1")
endif()

# NORDIC SDK APP START
target_sources(app PRIVATE src/main.c src/nrf_802154_radio_wrapper.c ${CMAKE_BINARY_DIR}/generated/rnb_coprocessor_version.c)
# NORDIC SDK APP END
