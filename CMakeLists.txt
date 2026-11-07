cmake_minimum_required(VERSION 3.31)

project(calendar_converter)
option(BUILD_TESTS "build tests" ${PROJECT_IS_TOP_LEVEL})
set(BUILD_SHARED_LIBS OFF)

include(FetchContent)
FetchContent_Declare(
    ez3i
    GIT_REPOSITORY https://github.com/abramov7613/extracted_z3_integer.git
    GIT_TAG        v1.0.4
)
FetchContent_MakeAvailable(ez3i)

add_library(${PROJECT_NAME} STATIC
  calendar_converter.h
  calendar_converter.cpp
)

target_link_libraries(${PROJECT_NAME} PUBLIC ez3i)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

target_include_directories(${PROJECT_NAME}
  PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
)

if(BUILD_TESTS)
  enable_testing()
  set(BUILD_GMOCK OFF)
  set(INSTALL_GTEST OFF)
  FetchContent_Declare(
      googletest
      URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip
  )
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)
  include(GoogleTest)

  add_executable(calendar_converter_test1 tests/calendar_converter_test1.cpp)
  target_link_libraries(calendar_converter_test1 PRIVATE ${PROJECT_NAME} GTest::gtest_main)
  target_compile_features(calendar_converter_test1 PRIVATE cxx_std_20)
  gtest_discover_tests(calendar_converter_test1)

  add_executable(calendar_converter_test2 tests/calendar_converter_test2.cpp)
  target_link_libraries(calendar_converter_test2 PRIVATE ${PROJECT_NAME} GTest::gtest_main)
  target_compile_features(calendar_converter_test2 PRIVATE cxx_std_20)
  gtest_discover_tests(calendar_converter_test2)

  add_executable(calendar_converter_test3 tests/calendar_converter_test3.cpp)
  target_link_libraries(calendar_converter_test3 PRIVATE ${PROJECT_NAME} GTest::gtest_main)
  target_compile_features(calendar_converter_test3 PRIVATE cxx_std_20)
  gtest_discover_tests(calendar_converter_test3)
endif()
