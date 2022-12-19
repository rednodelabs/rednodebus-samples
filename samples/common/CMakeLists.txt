#
# Copyright (c) 2022 RedNodeLabs
#

cmake_minimum_required(VERSION 3.20.0)

list(INSERT OVERLAY_CONFIG 0 ${CMAKE_CURRENT_SOURCE_DIR}/../common/overlay-rnb-defaults.conf)

list(INSERT OVERLAY_CONFIG 0 ${CMAKE_CURRENT_SOURCE_DIR}/../common/overlay-ot-defaults.conf)
