cmake_minimum_required(VERSION 2.8.12)
project (detector_slr)

add_library(${PROJECT_NAME} slr.cpp)

target_include_directories(${PROJECT_NAME} PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/..
  ${CMAKE_SOURCE_DIR}/common
  ${DEPEND_LIB_ROOT}/include
  $<TARGET_PROPERTY:sender_iothub,INTERFACE_INCLUDE_DIRECTORIES>
  )
add_dependencies(${PROJECT_NAME} thread-pool)

## test
set(TEST_CB_NAME cb_test)
add_executable(${TEST_CB_NAME} cb_test.cpp)
# add_test(
#   NAME ${TEST_CB_NAME}
#   COMMAND $<TARGET_FILE:${TEST_CB_NAME}>
#   CONFIGURATIONS Debug
#   )
target_include_directories(${TEST_CB_NAME} PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/..
  ${CMAKE_SOURCE_DIR}/common
  ${DEPEND_LIB_ROOT}/include
  )
add_dependencies(${TEST_CB_NAME} fmt)
target_link_libraries(${TEST_CB_NAME} ${DEPEND_LIB_ROOT}/lib/libfmt.a)

set(TEST_NAME slr_test)
add_executable(${TEST_NAME} slr_test.cpp ${CMAKE_SOURCE_DIR}/src/sampler.cpp)
# add_test(
#   NAME ${TEST_NAME}
#   COMMAND $<TARGET_FILE:${TEST_NAME}>
#   CONFIGURATIONS Debug
#   )
target_include_directories(${TEST_NAME} PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/..
  ${CMAKE_SOURCE_DIR}/common
  ${CMAKE_SOURCE_DIR}/src
  ${DEPEND_LIB_ROOT}/include
  $<TARGET_PROPERTY:sensor_onavi,INTERFACE_INCLUDE_DIRECTORIES>
  )
add_dependencies(${TEST_NAME} sensor_onavi fmt)
target_link_libraries(${TEST_NAME} PRIVATE pthread ${PROJECT_NAME} sensor_onavi)
