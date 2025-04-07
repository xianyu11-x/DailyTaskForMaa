cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(MAAbackend LANGUAGES CXX)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CO_ASYNC_DEBUG ON) 
endif()
set(CO_ASYNC_INVALFIX ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  ${CMAKE_SOURCE_DIR}/bin)

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/src)

add_subdirectory(co_async)
add_subdirectory(src)

add_custom_target(copy_cfg ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/cfg ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/cfg
    DEPENDS ${CMAKE_SOURCE_DIR}/cfg
)

add_dependencies(MAAbackend copy_cfg)