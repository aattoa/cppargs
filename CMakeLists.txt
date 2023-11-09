cmake_minimum_required(VERSION 3.24)
project(cppargs CXX)

if (NOT DEFINED CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 20)
endif ()
if (NOT DEFINED CMAKE_CXX_EXTENSIONS)
    set(CMAKE_CXX_EXTENSIONS OFF)
endif ()

add_library(${PROJECT_NAME} STATIC)
target_sources(${PROJECT_NAME}
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}/cppargs.hpp
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}/parse.cpp
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}/exception.cpp
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}/parameters.cpp)
target_include_directories(${PROJECT_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME})

if (MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE "/W4")
else ()
    target_compile_options(${PROJECT_NAME} PRIVATE "-Wall" "-Wextra" "-Wpedantic")
endif ()

option(CPPARGS_BUILD_TESTS "Build cppargs tests" OFF)
if (${CPPARGS_BUILD_TESTS})
    enable_testing()
    add_subdirectory(tests)
endif ()
