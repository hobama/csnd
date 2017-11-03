cmake_minimum_required(VERSION 2.8.12)
project (sensor_onavi)

#link_directories(${DEPEND_LIB_ROOT}/lib)

add_library(${PROJECT_NAME} onavi.cpp)

target_include_directories(${PROJECT_NAME} PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/..
  ${CMAKE_SOURCE_DIR}/common
  ${DEPEND_LIB_ROOT}/include
  )

## test
set(TEST_NAME onavi_test)
add_executable(${TEST_NAME} onavi_test.cpp)
# add_test(
#   NAME ${TEST_NAME}
#   COMMAND $<TARGET_FILE:${TEST_NAME}>
#   CONFIGURATIONS Debug
#   )
target_link_libraries(${TEST_NAME} ${PROJECT_NAME})
add_dependencies(${TEST_NAME} ${PROJECT_NAME})
