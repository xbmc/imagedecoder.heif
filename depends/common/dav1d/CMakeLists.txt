cmake_minimum_required(VERSION 3.5)
project(dav1d)

find_package(Python3 REQUIRED)

if(NOT CPU AND CMAKE_SYSTEM_PROCESSOR)
  set(CPU ${CMAKE_SYSTEM_PROCESSOR})
endif()

include(ExternalProject)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(MESON_BUILD_TYPE debug)
else()
  set(MESON_BUILD_TYPE release)
endif()

list(APPEND configure_command ${CMAKE_COMMAND} -E env)
list(APPEND configure_command ANDROID_NDK=${TOOLCHAIN})
list(APPEND configure_command AR=${CMAKE_AR})
list(APPEND configure_command CC=${CMAKE_C_COMPILER})
list(APPEND configure_command CFLAGS=${CMAKE_C_FLAGS})
list(APPEND configure_command CPPFLAGS=${CMAKE_CPP_FLAGS})
list(APPEND configure_command CXX=${CMAKE_CXX_COMPILER})
list(APPEND configure_command CXXFLAGS=${CMAKE_CXX_FLAGS})
list(APPEND configure_command INSTALL_PREFIX=)
list(APPEND configure_command LDFLAGS=${CMAKE_LD_FLAGS})
list(APPEND configure_command RANLIB=${CMAKE_RANLIB})
list(APPEND configure_command meson)
list(APPEND configure_command --prefix=${CMAKE_INSTALL_PREFIX})
list(APPEND configure_command --libdir=lib)
list(APPEND configure_command --buildtype=${MESON_BUILD_TYPE})
list(APPEND configure_command --default-library=static)
list(APPEND configure_command -Denable_asm=true)
list(APPEND configure_command -Denable_tools=false)
list(APPEND configure_command -Denable_examples=false)
list(APPEND configure_command -Denable_tests=false)

externalproject_add(dav1d
                    SOURCE_DIR ${CMAKE_SOURCE_DIR}
                    UPDATE_COMMAND ""
                    CONFIGURE_COMMAND ${configure_command} . build
                    BUILD_COMMAND ninja -C ${CMAKE_SOURCE_DIR}/build
                    INSTALL_COMMAND ninja -C ${CMAKE_SOURCE_DIR}/build -v install
                    BUILD_IN_SOURCE 1)

install(CODE "execute_process(COMMAND make install_sw WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})")
