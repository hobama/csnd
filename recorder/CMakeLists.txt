cmake_minimum_required(VERSION 2.8.12)
project (recorder)

add_library(${PROJECT_NAME} recorder.cpp recorder_local_posix.cpp)

add_dependencies(${PROJECT_NAME} thread-pool)

target_include_directories(${PROJECT_NAME} PRIVATE
  ${DEPEND_LIB_ROOT}/include
  ${CMAKE_SOURCE_DIR}/common
  )
