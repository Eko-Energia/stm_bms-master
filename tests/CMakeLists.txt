cmake_minimum_required(VERSION 3.16)
project(BMS_Master_UnitTests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

enable_testing()

include(FetchContent)

# Prefer vendored GoogleTest (tests/third_party) so configure works offline / without git SSL.
set(BMS_GTEST_VENDORED_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/googletest-1.14.0")
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

if(EXISTS "${BMS_GTEST_VENDORED_DIR}/CMakeLists.txt")
  message(STATUS "Using vendored GoogleTest at ${BMS_GTEST_VENDORED_DIR}")
  add_subdirectory("${BMS_GTEST_VENDORED_DIR}" "${CMAKE_BINARY_DIR}/googletest-build")
else()
  message(STATUS "Vendored GoogleTest not found — FetchContent from GitHub zip")
  FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
  )
  FetchContent_MakeAvailable(googletest)
endif()

add_library(bms_host_logic
  host/bms_protocol.cpp
  host/bms_can_rx_processor.cpp
  host/bms_fan_controller.cpp
  host/bms_peripherals_anchor.cpp
)
target_include_directories(bms_host_logic
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/host
    ${CMAKE_CURRENT_SOURCE_DIR}/stubs
    ${CMAKE_CURRENT_SOURCE_DIR}/mocks
)

add_executable(bms_unit_tests
  unit/test_protocol.cpp
  unit/test_can_rx_processor.cpp
  unit/test_fan_controller.cpp
  unit/test_mocks_integration.cpp
  unit/test_can1_can2.cpp
  unit/test_pwm.cpp
  unit/test_adc_dma.cpp
)
target_link_libraries(bms_unit_tests
  PRIVATE
    bms_host_logic
    GTest::gtest_main
    GTest::gmock
)

include(GoogleTest)
gtest_discover_tests(bms_unit_tests
  DISCOVERY_TIMEOUT 60
  DISCOVERY_MODE PRE_TEST
)
