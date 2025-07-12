cmake_minimum_required(VERSION 4.0.3)

project("bypass_dlfcn")

set(bypass_dlfcn_sources
        ${CMAKE_CURRENT_SOURCE_DIR}/bypass_dlopen.c
        )

include_directories(
        ../include/
)

add_library(
        bypass_dlfcn
        STATIC    # You can change it to SHARED, to build an seperate so into the apk
        ${bypass_dlfcn_sources}
)

target_link_libraries(
        bypass_dlfcn
        log)