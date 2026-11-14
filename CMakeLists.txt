cmake_minimum_required(VERSION 3.31)

project(calendar_converter)
option(BUILD_TESTS "build tests" ${PROJECT_IS_TOP_LEVEL})
set(BUILD_SHARED_LIBS OFF)

include(FetchContent)
FetchContent_Declare(
    ez3i
    GIT_REPOSITORY https://github.com/abramov7613/extracted_z3_integer.git
    GIT_TAG        v1.1.0
)
FetchContent_MakeAvailable(ez3i)

add_library(${PROJECT_NAME} STATIC
  calendar_converter.h
  calendar_converter.cpp
  flexint.h
  flexint.cpp
)

target_link_libraries(${PROJECT_NAME} PUBLIC ez3i)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

target_include_directories(${PROJECT_NAME}
  PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
)

if(BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
