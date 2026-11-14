set(BUILD_GMOCK OFF)
set(INSTALL_GTEST OFF)

find_package(GTest 1.17 QUIET)

if(NOT GTest_FOUND)
  message(STATUS "Google Test not found or version too old, fetching GTest")
  FetchContent_Declare(googletest
      GIT_REPOSITORY https://github.com/google/googletest.git
      GIT_TAG        v1.17.0
  )
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)
endif()

if(NOT TARGET GTest::gtest_main)
    message(FATAL_ERROR "Google Test not found")
endif()

include(GoogleTest)

add_executable(calendar_converter_test1 calendar_converter_test1.cpp)
target_link_libraries(calendar_converter_test1 PRIVATE ${PROJECT_NAME} GTest::gtest_main)
target_compile_features(calendar_converter_test1 PRIVATE cxx_std_20)
gtest_discover_tests(calendar_converter_test1)

add_executable(calendar_converter_test2 calendar_converter_test2.cpp)
target_link_libraries(calendar_converter_test2 PRIVATE ${PROJECT_NAME} GTest::gtest_main)
target_compile_features(calendar_converter_test2 PRIVATE cxx_std_20)
gtest_discover_tests(calendar_converter_test2)

add_executable(calendar_converter_test3 calendar_converter_test3.cpp)
target_link_libraries(calendar_converter_test3 PRIVATE ${PROJECT_NAME} GTest::gtest_main)
target_compile_features(calendar_converter_test3 PRIVATE cxx_std_20)
gtest_discover_tests(calendar_converter_test3)
