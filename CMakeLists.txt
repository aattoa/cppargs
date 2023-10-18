cmake_minimum_required(VERSION 3.24)
project(cppargs CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(${PROJECT_NAME} STATIC)
target_sources(${PROJECT_NAME}
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}/cppargs.hpp
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}/parse.cpp
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}/exception.cpp
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}/parameters.cpp
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}/arguments.cpp)
target_include_directories(${PROJECT_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME})

option(CPPARGS_BUILD_TESTS "Build cppargs tests" ON)
if(${CPPARGS_BUILD_TESTS})
    enable_testing()
    add_subdirectory(test)
endif()
