cmake_minimum_required(VERSION 2.8.12)
project (csnd)

option(USE_AVROCPP "Use avro c++ library (If it's set OFF, use c library)" ON)
#set(USE_AVROCPP OFF)

#### c++11
if (CMAKE_VERSION VERSION_LESS "3.1")
  if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set (CMAKE_CXX_FLAGS "-std=gnu++11 ${CMAKE_CXX_FLAGS}")
  endif ()
else ()
  set (CMAKE_CXX_STANDARD 11)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  set(CMAKE_CXX_EXTENSIONS OFF)
endif ()

set(CMAKE_CXX_FLAGS_DEBUG "-ggdb -gstabs ${CMAKE_CXX_FLAGS_DEBUG}")

#### external projects (common)
include(ExternalProject)
set(DEPEND_LIB_ROOT ${PROJECT_BINARY_DIR}/Depends)
include_directories(${DEPEND_LIB_ROOT}/include)
link_directories(${DEPEND_LIB_ROOT}/lib)
link_directories(${DEPEND_LIB_ROOT}/lib64)

ExternalProject_Add(argagg
  GIT_REPOSITORY https://github.com/vietjtnguyen/argagg
  GIT_TAG 0.4.6
  INSTALL_DIR ${DEPEND_LIB_ROOT}
  CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${DEPEND_LIB_ROOT}
  )

ExternalProject_Add(fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt
  GIT_TAG 4.0.0
  INSTALL_DIR ${DEPEND_LIB_ROOT}
  CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${DEPEND_LIB_ROOT} -DFMT_DOC=OFF -DFMT_TEST=OFF
  )

if(NOT EXISTS /usr/include/spdlog)
  # spdlog は cmake 3.1 以上が必要
  ExternalProject_Add(spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog
    GIT_TAG v0.14.0
    INSTALL_DIR ${DEPEND_LIB_ROOT}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${DEPEND_LIB_ROOT} -DSPDLOG_BUILD_TESTING=OFF
    )
endif()

ExternalProject_Add(thread-pool
  GIT_REPOSITORY https://github.com/progschj/ThreadPool
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
  SOURCE_DIR ${DEPEND_LIB_ROOT}/src/ThreadPool
  INSTALL_COMMAND install -p -m 0644 -D ${DEPEND_LIB_ROOT}/src/ThreadPool/ThreadPool.h ${DEPEND_LIB_ROOT}/include/ThreadPool.h
  )

#### installation
install(DIRECTORY DESTINATION ${CMAKE_INSTALL_PREFIX}/etc)
install(FILES
  ${CMAKE_SOURCE_DIR}/doc/csnd.service.example
  ${CMAKE_SOURCE_DIR}/doc/csnd.yml.example
  DESTINATION etc)
#install(DIRECTORY DESTINATION ${CMAKE_INSTALL_PREFIX}/log)
#install(DIRECTORY DESTINATION ${CMAKE_INSTALL_PREFIX}/out)
#install(DIRECTORY DESTINATION ${CMAKE_INSTALL_PREFIX}/run)

# install(DIRECTORY ${DEPEND_LIB_ROOT}/bin DESTINATION ${CMAKE_INSTALL_PREFIX}
#   PATTERN "*"
#   PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ)
# install(DIRECTORY ${DEPEND_LIB_ROOT}/include DESTINATION ${CMAKE_INSTALL_PREFIX})
# if (EXISTS ${DEPEND_LIB_ROOT}/lib)
#   install(DIRECTORY ${DEPEND_LIB_ROOT}/lib DESTINATION ${CMAKE_INSTALL_PREFIX})
# endif()
# if (EXISTS ${DEPEND_LIB_ROOT}/lib64)
#   install(DIRECTORY ${DEPEND_LIB_ROOT}/lib64 DESTINATION ${CMAKE_INSTALL_PREFIX})
# endif()

#### subdirectories
add_subdirectory(detector)
add_subdirectory(recorder)
add_subdirectory(sensor)
add_subdirectory(serializer)
add_subdirectory(sender)
add_subdirectory(src)
