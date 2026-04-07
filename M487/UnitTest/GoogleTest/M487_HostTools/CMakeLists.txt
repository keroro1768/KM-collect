# CMakeLists.txt for M487 Host Tools Tests
# Builds GoogleTest-based unit tests for host-side tools

cmake_minimum_required(VERSION 3.14)
project(M487_HostTools_Tests)

# C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Fetch GoogleTest
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.12.1
)
# For Windows: prevent overriding the project's /Wn flag by including GTest's
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Enable testing
enable_testing()

# Source files
set(SOURCES
    test_hidtool.cpp
    test_msc_protocol.cpp
)

# Create test executable
add_executable(m487_host_tests ${SOURCES})

# Link GoogleTest
target_link_libraries(m487_host_tests
    gtest
    gtest_main
)

# GoogleTest specific settings
include(GoogleTest)

# Discover tests automatically
gtest_discover_tests(m487_host_tests)

# Optional: Enable verbose output
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(m487_host_tests PRIVATE GTEST_VERBOSE=1)
endif()

# Copy test data files if needed
# file(COPY test_data DESTINATION ${CMAKE_BINARY_DIR})

# Build options
option(BUILD_COVERAGE "Build with code coverage" OFF)
if(BUILD_COVERAGE)
    target_compile_options(m487_host_tests PRIVATE
        --coverage
        -fprofile-arcs
        -ftest-coverage
    )
    target_link_options(m487_host_tests PRIVATE
        --coverage
        -fprofile-arcs
        -ftest-coverage
    )
endif()
