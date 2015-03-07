project(game.libretro)

cmake_minimum_required(VERSION 2.6)
enable_language(CXX)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR})

find_package(kodi REQUIRED)
find_package(kodiplatform REQUIRED)

include_directories(${INCLUDES}
                    ${kodiplatform_INCLUDE_DIRS}
                    ${KODI_INCLUDE_DIR}
                    ${PROJECT_SOURCE_DIR}/src)

list(APPEND DEPLIBS ${kodiplatform_LIBRARIES})

set(LIBRETRO_SOURCES src/client.cpp
                     src/ClientBridge.cpp
                     src/FrontendBridge.cpp
                     src/GameInfoLoader.cpp
                     src/LibretroDLL.cpp
                     src/LibretroEnvironment.cpp
                     src/LibretroTranslator.cpp)

build_addon(game.libretro LIBRETRO DEPLIBS)

include(CPack)
