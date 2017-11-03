cmake_minimum_required(VERSION 2.8.12)
project (serializer_avroc)

#link_directories(${DEPEND_LIB_ROOT}/lib)

add_library(${PROJECT_NAME} STATIC avroc.cpp)

target_include_directories(${PROJECT_NAME} PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/..
  ${CMAKE_SOURCE_DIR}/common
  )

#include(ExternalProject)

ExternalProject_Add(avro-c
  GIT_REPOSITORY https://github.com/apache/avro
  GIT_TAG release-1.8.2
  DOWNLOAD_COMMAND "" # avro-cpp§«ºË∆¿∫—
  INSTALL_DIR ${DEPEND_LIB_ROOT}
  SOURCE_DIR ${DEPEND_LIB_ROOT}/src/avro
  #SOURCE_SUBDIR lang/c++ # cmake 3.7 or later
  CONFIGURE_COMMAND ${CMAKE_COMMAND} -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR> <SOURCE_DIR>/lang/c # Tell CMake to use subdirectory as source.
  CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${DEPEND_LIB_ROOT}
  )
add_dependencies(${PROJECT_NAME} avro-c)
ExternalProject_Get_Property(avro-c SOURCE_DIR)
target_include_directories(${PROJECT_NAME} PRIVATE ${SOURCE_DIR}/lang/c/src)
target_link_libraries(${PROJECT_NAME} INTERFACE ${DEPEND_LIB_ROOT}/lib/libavro.a)
target_link_libraries(${PROJECT_NAME} PRIVATE jansson z lzma snappy)

## test
set(TEST_NAME avroc_test)
add_executable(${TEST_NAME} avroc_test.cpp)
add_test(
  NAME ${TEST_NAME}
  COMMAND $<TARGET_FILE:${TEST_NAME}>
  CONFIGURATIONS Debug
  )
add_dependencies(${TEST_NAME} ${PROJECT_NAME})
target_link_libraries(${TEST_NAME} PRIVATE pthread ${PROJECT_NAME})
