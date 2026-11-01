#
# MobCam - Native Camera Plugin for OBS Studio
# Copyright (C) 2026 Yash Rayjada (YBR Digital)
# SPDX-License-Identifier: GPL-2.0-or-later
#

cmake_minimum_required(VERSION 3.22)
project(mobcam VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)

# Output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

set(MOBCAM_SOURCES
    src/plugin-main.c
    src/mobcam-source.c
    src/ui/mobcam-properties.c
    src/ui/web_control_server.c
    src/cjson/cJSON.c
    src/net/control_client.c
    src/net/media_client.c
    src/net/discovery_listener.c
    src/decode/h264_decoder.c
    src/decode/aac_decoder.c
    src/decode/watermark_overlay.c
)

if(WIN32)
    list(APPEND MOBCAM_SOURCES src/mobcam.rc)
endif()

add_library(mobcam MODULE ${MOBCAM_SOURCES})

# Target properties
set_target_properties(mobcam PROPERTIES
    PREFIX ""
    OUTPUT_NAME "mobcam"
)

# Include directories
target_include_directories(mobcam PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/deps/libobs"
    "${CMAKE_CURRENT_SOURCE_DIR}/deps/obs-deps/include"
)

# Compile definitions
target_compile_definitions(mobcam PRIVATE
    UNICODE
    _UNICODE
    _CRT_SECURE_NO_WARNINGS
)

# Link directories
target_link_directories(mobcam PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/deps/libobs"
    "${CMAKE_CURRENT_SOURCE_DIR}/deps/obs-deps/lib"
    "${CMAKE_CURRENT_SOURCE_DIR}/deps/obs-deps/bin"
    "${CMAKE_CURRENT_SOURCE_DIR}/deps/obs-deps/win64/bin"
    "${CMAKE_CURRENT_SOURCE_DIR}/deps/obs-deps/win64/lib"
)

# Link libraries
target_link_libraries(mobcam PRIVATE
    obs
    avcodec-62
    avutil-60
    ws2_32
)

# Installation rule to local OBS plugins directory
if(DEFINED ENV{ProgramData})
    file(TO_CMAKE_PATH "$ENV{ProgramData}" PROGRAM_DATA_PATH)
    set(DEFAULT_OBS_PLUGIN_DIR "${PROGRAM_DATA_PATH}/obs-studio/plugins/mobcam/bin/64bit")
    install(TARGETS mobcam DESTINATION "${DEFAULT_OBS_PLUGIN_DIR}")
    install(DIRECTORY data/ DESTINATION "${PROGRAM_DATA_PATH}/obs-studio/plugins/mobcam/data")
endif()
