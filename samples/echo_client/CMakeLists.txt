# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

string(REPLACE " " ";" OVERLAY_CONFIG_LIST ${OVERLAY_CONFIG})
if("overlay-ot-rnb.conf" IN_LIST OVERLAY_CONFIG_LIST)
include(../common/CMakeLists.txt)
endif()

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(sockets_echo_client)


if("overlay-ot-rnb.conf" IN_LIST OVERLAY_CONFIG_LIST)
target_compile_definitions_ifdef(CONFIG_NET_L2_OPENTHREAD ot-config INTERFACE "OPENTHREAD_CONFIG_CHILD_SUPERVISION_MSG_NO_ACK_REQUEST=1")
endif()

if(CONFIG_MBEDTLS_KEY_EXCHANGE_PSK_ENABLED AND
    (CONFIG_NET_SAMPLE_PSK_HEADER_FILE STREQUAL "dummy_psk.h"))
  add_custom_target(development_psk
    COMMAND ${CMAKE_COMMAND} -E echo "----------------------------------------------------------"
    COMMAND ${CMAKE_COMMAND} -E echo "--- WARNING: Using dummy PSK! Only suitable for        ---"
    COMMAND ${CMAKE_COMMAND} -E echo "--- development. Set NET_SAMPLE_PSK_HEADER_FILE to use ---"
    COMMAND ${CMAKE_COMMAND} -E echo "--- own pre-shared key.                                ---"
    COMMAND ${CMAKE_COMMAND} -E echo "----------------------------------------------------------"
  )
  add_dependencies(app development_psk)
endif()

target_sources(                     app PRIVATE src/echo-client.c)
target_include_directories(app PUBLIC ../../include)
target_sources_ifdef(CONFIG_NET_UDP app PRIVATE src/udp.c)
target_sources_ifdef(CONFIG_NET_TCP app PRIVATE src/tcp.c)
target_sources_ifdef(CONFIG_NET_VLAN app PRIVATE src/vlan.c)
target_sources_ifdef(CONFIG_REDNODEBUS app PRIVATE src/rnb_utils.c)

include(${ZEPHYR_BASE}/samples/net/common/common.cmake)

set(gen_dir ${ZEPHYR_BINARY_DIR}/include/generated/)

generate_inc_file_for_target(
    app
    src/echo-apps-cert.der
    ${gen_dir}/echo-apps-cert.der.inc
    )
