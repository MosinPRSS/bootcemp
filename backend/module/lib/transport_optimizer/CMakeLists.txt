cmake_minimum_required(VERSION 3.24)

project(transport_optimizer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# Собираем только необходимую C++-часть OR-Tools.
set(BUILD_DEPS ON CACHE BOOL "" FORCE)
set(BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(BUILD_FLATZINC OFF CACHE BOOL "" FORCE)
set(USE_COINOR OFF CACHE BOOL "" FORCE)
set(USE_SCIP OFF CACHE BOOL "" FORCE)
set(USE_HIGHS OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    ortools
    GIT_REPOSITORY https://github.com/google/or-tools.git
    GIT_TAG v9.15
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(ortools)

find_package(nlohmann_json CONFIG REQUIRED)

add_library(
    transport_optimizer STATIC
    Bootcamp_IT_ONE/optimizer.cpp
)

target_link_libraries(
    transport_optimizer
    PUBLIC
    ortools::ortools
    nlohmann_json::nlohmann_json
)
if(MSVC)
    target_compile_options(transport_optimizer PRIVATE /utf-8)
endif()

target_include_directories(transport_optimizer PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Bootcamp_IT_ONE")
option(TRANSPORT_BUILD_TEST_RUNNER "Build JSON regression runner" OFF)
if(TRANSPORT_BUILD_TEST_RUNNER)
    add_executable(transport_optimizer_runner tests/json_runner.cpp)
    target_link_libraries(transport_optimizer_runner PRIVATE transport_optimizer)
    if(MSVC)
        target_compile_options(transport_optimizer_runner PRIVATE /utf-8)
    endif()
endif()