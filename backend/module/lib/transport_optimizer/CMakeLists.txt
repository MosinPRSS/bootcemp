cmake_minimum_required(VERSION 3.24)

project(transport_optimizer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Используем установленный в системе OR-Tools.
find_package(ortools CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

add_library(
    transport_optimizer SHARED
    Bootcamp_IT_ONE/optimizer.cpp
    Bootcamp_IT_ONE/bridge.cpp
)

target_link_libraries(
    transport_optimizer
    PUBLIC
    ortools::ortools
    nlohmann_json::nlohmann_json
)

target_include_directories(
    transport_optimizer
    PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/Bootcamp_IT_ONE"
)

if(MSVC)
    target_compile_options(
        transport_optimizer
        PRIVATE
        /utf-8
    )
endif()

option(
    TRANSPORT_BUILD_TEST_RUNNER
    "Build JSON regression runner"
    OFF
)

if(TRANSPORT_BUILD_TEST_RUNNER)

    add_executable(
        transport_optimizer_runner
        tests/json_runner.cpp
    )

    target_link_libraries(
        transport_optimizer_runner
        PRIVATE
        transport_optimizer
    )

    if(MSVC)
        target_compile_options(
            transport_optimizer_runner
            PRIVATE
            /utf-8
        )
    endif()

endif()