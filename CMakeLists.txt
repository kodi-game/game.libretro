project(game.libretro)

cmake_minimum_required(VERSION 2.6)
enable_language(CXX)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR})

find_package(kodi REQUIRED)
find_package(kodiplatform REQUIRED)
find_package(p8-platform REQUIRED)

include_directories(${INCLUDES}
                    ${platform_INCLUDE_DIRS}
                    ${kodiplatform_INCLUDE_DIRS}
                    ${KODI_INCLUDE_DIR}
                    ${PROJECT_SOURCE_DIR}/src)

list(APPEND DEPLIBS ${kodiplatform_LIBRARIES} ${platform_LIBRARIES})

set(LIBRETRO_SOURCES src/client.cpp
                     src/ClientBridge.cpp
                     src/FrontendBridge.cpp
                     src/GameInfoLoader.cpp
                     src/input/ButtonMapper.cpp
                     src/input/InputManager.cpp
                     src/input/LibretroDevice.cpp
                     src/LibretroDLL.cpp
                     src/LibretroEnvironment.cpp
                     src/LibretroTranslator.cpp
                     src/log/Log.cpp
                     src/log/LogAddon.cpp
                     src/log/LogConsole.cpp
                     src/settings/Settings.cpp)

build_addon(game.libretro LIBRETRO DEPLIBS)

include(CPack)
