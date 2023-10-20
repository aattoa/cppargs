find_package(Catch2 3 REQUIRED)

add_executable(test-${PROJECT_NAME} test.cpp)
target_include_directories(test-${PROJECT_NAME}
    PRIVATE ${PROJECT_SOURCE_DIR}/${PROJECT_NAME})
target_link_libraries(test-${PROJECT_NAME}
    PRIVATE ${PROJECT_NAME}
    PRIVATE Catch2::Catch2WithMain)

if (MSVC)
    target_compile_options(test-${PROJECT_NAME} PRIVATE "/W4")
else ()
    target_compile_options(test-${PROJECT_NAME} PRIVATE "-Wall" "-Wextra" "-Wpedantic")
endif ()

include(Catch)
catch_discover_tests(test-${PROJECT_NAME})
