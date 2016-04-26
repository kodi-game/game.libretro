project(game.libretro)

cmake_minimum_required(VERSION 2.6)
enable_language(CXX)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR})

find_package(kodi REQUIRED)
find_package(kodiplatform REQUIRED)
find_package(p8-platform REQUIRED)

include_directories(${INCLUDES}
                    ${p8-platform_INCLUDE_DIRS}
                    ${kodiplatform_INCLUDE_DIRS}
                    ${KODI_INCLUDE_DIR}
                    ${PROJECT_SOURCE_DIR}/src)

list(APPEND DEPLIBS ${kodiplatform_LIBRARIES} ${p8-platform_LIBRARIES})

set(LIBRETRO_SOURCES src/client.cpp
                     src/audio/AudioStream.cpp
                     src/audio/SingleFrameAudio.cpp
                     src/GameInfoLoader.cpp
                     src/GameLoop.cpp
                     src/input/ButtonMapper.cpp
                     src/input/InputManager.cpp
                     src/input/LibretroDevice.cpp
                     src/libretro/ClientBridge.cpp
                     src/libretro/FrontendBridge.cpp
                     src/libretro/LibretroDLL.cpp
                     src/libretro/LibretroEnvironment.cpp
                     src/libretro/LibretroTranslator.cpp
                     src/log/Log.cpp
                     src/log/LogAddon.cpp
                     src/log/LogConsole.cpp
                     src/settings/Settings.cpp
                     src/video/VideoStream.cpp)

build_addon(game.libretro LIBRETRO DEPLIBS)

include(CPack)
