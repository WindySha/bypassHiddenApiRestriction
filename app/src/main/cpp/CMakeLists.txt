cmake_minimum_required(VERSION 3.10.2)

project("test-lib")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden")


# import lib from local-project
string(TOLOWER ${CMAKE_BUILD_TYPE} BUILD_TYPE_DIRNAME)
set(LIB_BASE ${CMAKE_CURRENT_SOURCE_DIR}/../../../../bypassRestriction)
add_library(bypassRestriction::bypassRestriction SHARED IMPORTED)
set_target_properties(bypassRestriction::bypassRestriction PROPERTIES
        IMPORTED_LOCATION ${LIB_BASE}/build/intermediates/stripped_native_libs/${BUILD_TYPE_DIRNAME}/out/lib/${ANDROID_ABI}/libbypassRestriction.so
        INTERFACE_INCLUDE_DIRECTORIES ${LIB_BASE}/src/main/cpp/include)

# import lib from maven
#find_package(bypassRestriction REQUIRED CONFIG)

add_library(test-lib
        SHARED
        main.cpp)

find_library(log-lib
        log)

target_link_libraries(test-lib
        bypassRestriction::bypassRestriction
        ${log-lib})