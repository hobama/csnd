cmake_minimum_required(VERSION 2.8.12)
project (csnd)

link_directories(${DEPEND_LIB_ROOT}/lib)

add_executable(${PROJECT_NAME} config.cpp main.cpp sampler.cpp)
add_dependencies(${PROJECT_NAME} detector_slr recorder sensor_onavi sender_iothub)

if (USE_AVROCPP)
  add_dependencies(${PROJECT_NAME} serializer_avrocpp)
  add_definitions(-DUSE_AVROCPP=${USE_AVROCPP})
else(USE_AVROCPP)
  add_dependencies(${PROJECT_NAME} serializer_avroc)
endif(USE_AVROCPP)

ExternalProject_Add(jbeder-yaml-cpp
  GIT_REPOSITORY https://github.com/jbeder/yaml-cpp
  GIT_TAG yaml-cpp-0.5.3
  INSTALL_DIR ${DEPEND_LIB_ROOT}
  CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${DEPEND_LIB_ROOT} -DYAML_CPP_BUILD_TOOLS=OFF -DYAML_CPP_BUILD_CONTRIB=OFF
  )
add_dependencies(${PROJECT_NAME} jbeder-yaml-cpp)

target_include_directories(${PROJECT_NAME} PRIVATE
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/common
  ${DEPEND_LIB_ROOT}/include
  # $<TARGET_PROPERTY:detector_slr,INTERFACE_INCLUDE_DIRECTORIES>
  # $<TARGET_PROPERTY:recorder,INTERFACE_INCLUDE_DIRECTORIES>
  # $<TARGET_PROPERTY:sensor_onavi,INTERFACE_INCLUDE_DIRECTORIES>
  # $<TARGET_PROPERTY:sender_iothub,INTERFACE_INCLUDE_DIRECTORIES>
  )

target_link_libraries(${PROJECT_NAME} pthread yaml-cpp sender_iothub sensor_onavi detector_slr recorder)

if (USE_AVROCPP)
  target_include_directories(${PROJECT_NAME} PRIVATE $<TARGET_PROPERTY:serializer_avrocpp,INTERFACE_INCLUDE_DIRECTORIES>)
  target_link_libraries(${PROJECT_NAME} serializer_avrocpp)
else(USE_AVROCPP)
  target_include_directories(${PROJECT_NAME} PRIVATE $<TARGET_PROPERTY:serializer_avroc,INTERFACE_INCLUDE_DIRECTORIES>)
  target_link_libraries(${PROJECT_NAME} serializer_avroc)
endif(USE_AVROCPP)

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
