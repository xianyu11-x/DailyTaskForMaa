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

add_subdirectory(co_async)
add_subdirectory(src)
