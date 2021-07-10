cmake_minimum_required(VERSION 3.10.2)

project("bypass_dlfcn")

enable_language(ASM)

if (${ANDROID_ABI} STREQUAL "armeabi-v7a")
    set(dlfcn_trampoline_asm ${CMAKE_CURRENT_SOURCE_DIR}/dlfcn_trampoline32.S)
elseif (${ANDROID_ABI} STREQUAL "arm64-v8a")
    set(dlfcn_trampoline_asm ${CMAKE_CURRENT_SOURCE_DIR}/dlfcn_trampoline64.S)
endif ()

set(bypass_dlfcn_sources
        ${CMAKE_CURRENT_SOURCE_DIR}/bypass_dlfcn.c
        ${CMAKE_CURRENT_SOURCE_DIR}/efl_parser.c)

include_directories(
        ../include/
)

add_library(
        bypass_dlfcn
        STATIC    # You can change it to SHARED, to build an seperate so into the apk
        ${dlfcn_trampoline_asm}
        ${bypass_dlfcn_sources}
)

target_link_libraries(
        bypass_dlfcn
        log)