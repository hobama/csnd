cmake_minimum_required(VERSION 2.8.12)
project (sender_iothub)

set(AZURE_IOT_PROTOCOL "HTTP")

#link_directories(${DEPEND_LIB_ROOT}/lib)

add_library(${PROJECT_NAME} STATIC iothub.cpp)

target_include_directories(${PROJECT_NAME} PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/..
  ${CMAKE_SOURCE_DIR}/common
  ${DEPEND_LIB_ROOT}/include
  )

#include(ExternalProject)

ExternalProject_Add(azure-iothub
  GIT_REPOSITORY https://github.com/sgr/azure-iothub
  INSTALL_DIR ${DEPEND_LIB_ROOT}
  CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${DEPEND_LIB_ROOT}
  )

add_dependencies(${PROJECT_NAME} spdlog azure-iothub)
target_link_libraries(${PROJECT_NAME} azure_iothub ssl crypto curl uuid)

## test
set(TEST_NAME sender_iothub_test)
add_executable(${TEST_NAME} iothub_test.cpp)
# add_test(
#   NAME ${TEST_NAME}
#   COMMAND $<TARGET_FILE:${TEST_NAME}>
#   CONFIGURATIONS Debug
#   )
target_include_directories(${TEST_NAME} PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/..
  ${CMAKE_SOURCE_DIR}/common
  ${DEPEND_LIB_ROOT}/include
  )

add_dependencies(${TEST_NAME} ${PROJECT_NAME} fmt)
target_link_libraries(${TEST_NAME} ${PROJECT_NAME} pthread)
