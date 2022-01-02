cmake_minimum_required(VERSION 3.5)
project(imagedecoder.heif)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR})

find_package(Kodi REQUIRED)
find_package(de265 REQUIRED)
find_package(HEIF REQUIRED)
find_package(Threads REQUIRED)
find_package(TinyXML2 REQUIRED)

include_directories(${KODI_INCLUDE_DIR}/.. # Hack way with "/..", need bigger Kodi cmake rework to match right include ways (becomes done in future)
                    ${de265_INCLUDE_DIRS}
                    ${heif_INCLUDE_DIRS}
                    ${TINYXML2_INCLUDE_DIRS}
                    ${PROJECT_SOURCE_DIR}/lib)

set(HEIF_SOURCES src/HeifPicture.cpp
                 lib/TinyEXIF/TinyEXIF.cpp)

set(HEIF_HEADERS src/HeifPicture.h
                 lib/TinyEXIF/TinyEXIF.h)

set(DEPLIBS ${HEIF_LIBRARIES} ${de265_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${TINYXML2_LIBRARIES})

build_addon(imagedecoder.heif HEIF DEPLIBS)

include(CPack)
