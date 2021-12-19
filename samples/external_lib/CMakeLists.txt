# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
#cmake_minimum_required(VERSION 3.16.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(external_lib)

list(INSERT OVERLAY_CONFIG 0 ../common/overlay-ot-defaults.conf)

target_sources(app PRIVATE src/main.c)

# Create a wrapper CMake library that our app can link with
#add_compile_definitions(REDNODEBUS_LIB_IMPORT)
#add_library(rnb_nrf_lib STATIC IMPORTED GLOBAL)
#set_target_properties(rnb_nrf_lib PROPERTIES IMPORTED_LOCATION             ${REDNODELABS_DIR}/rnb_static_lib/librnb_nrf_lib.a)
#set_target_properties(rnb_lib PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${rnb_inc_dir})

#target_link_libraries(app PUBLIC rnb_nrf_lib)
