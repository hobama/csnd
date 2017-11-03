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

ExternalProject_Add(azure-iot-sdk-c
  # サブモジュールの取得に失敗することがある
  GIT_REPOSITORY https://github.com/Azure/azure-iot-sdk-c
  GIT_TAG lts_07_2017
  INSTALL_DIR ${DEPEND_LIB_ROOT}
  SOURCE_DIR ${DEPEND_LIB_ROOT}/src/azure-iot-sdk-c
  CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${DEPEND_LIB_ROOT}
  )
# cert.c コンパイル用
target_include_directories(${PROJECT_NAME} PRIVATE ${DEPEND_LIB_ROOT}/src/azure-iot-sdk-c/certs)

add_dependencies(${PROJECT_NAME} spdlog thread-pool azure-iot-sdk-c)

if (${AZURE_IOT_PROTOCOL} STREQUAL "AMQP")
  target_link_libraries(${PROJECT_NAME} INTERFACE
    iothub_client_amqp_transport
    iothub_client_amqp_ws_transport
    uamqp)
  add_definitions(-DAZURE_IOT_AMQP)
elseif (${AZURE_IOT_PROTOCOL} STREQUAL "MQTT")
  target_link_libraries(${PROJECT_NAME} INTERFACE
    iothub_client_mqtt_transport
    iothub_client_mqtt_ws_transport
    umqtt)
  add_definitions(-DAZURE_IOT_MQTT)
else (${AZURE_IOT_PROTOCOL} STREQUAL "AMQP")
  target_link_libraries(${PROJECT_NAME} INTERFACE iothub_client_http_transport)
endif (${AZURE_IOT_PROTOCOL} STREQUAL "AMQP")

target_link_libraries(${PROJECT_NAME} INTERFACE
  iothub_client
  aziotsharedutil
  ssl
  crypto
  curl
  uuid
  )

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
