cmake_minimum_required(VERSION 3.10)

set(CMAKE_POLICY_VERSION_MINIMUM 3.10) # force subprojects
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_ROOT}")
set(CMAKE_C_STANDARD 99)

project(anglefix LANGUAGES C)

add_subdirectory(${PROJECT_ROOT}/extern/VDF VDF)

add_executable(anglefix
    main.c
    anglefixer.c
    vmf.c
    math.c
    util.c
    build-preset.c
)

target_include_directories(anglefix
    PRIVATE "${PROJECT_ROOT}/extern/VDF"
)

target_link_libraries(anglefix PRIVATE vdf m)

enable_testing()

if(NOT CMAKE_SYSTEM_NAME STREQUAL "Windows")
    add_executable(test_anglefix
        test.c
        anglefixer.c
        vmf.c
        math.c
        util.c
    )

    target_include_directories(test_anglefix
        PRIVATE "${PROJECT_ROOT}/extern/VDF"
    )

    target_link_libraries(test_anglefix PRIVATE vdf m)

    add_dependencies(test_anglefix anglefix)

    add_test(NAME test1
        COMMAND sh -c
            "./test_anglefix tests/1.vmf tests/1-anglefixed.vmf"
        WORKING_DIRECTORY "$<TARGET_FILE_DIR:test_anglefix>"
    )

    add_custom_command(TARGET anglefix POST_BUILD
        COMMAND "${CMAKE_COMMAND}" --build "${CMAKE_BINARY_DIR}" --target test_anglefix
        COMMAND "${CMAKE_CTEST_COMMAND}" --test-dir "${CMAKE_BINARY_DIR}" --output-on-failure
        COMMENT "Building test executable and running tests..."
    )
endif()
