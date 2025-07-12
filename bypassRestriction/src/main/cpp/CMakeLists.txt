cmake_minimum_required(VERSION 4.0.3)

project("bypassRestriction")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden")

set(bypass_dlfcn_root_dir ${CMAKE_CURRENT_SOURCE_DIR}/bypass_dl)
add_subdirectory(${bypass_dlfcn_root_dir} bypass_dlfcn)

add_library(bypassRestriction
        SHARED
        main.cpp)

include_directories(
        include/
)

find_library(log-lib
        log)

target_link_libraries(bypassRestriction
        bypass_dlfcn
        ${log-lib})