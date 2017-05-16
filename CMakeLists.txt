cmake_minimum_required(VERSION 3.1)
project(game.libretro)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR})

find_package(Kodi REQUIRED)
find_package(kodiplatform REQUIRED)
find_package(p8-platform REQUIRED)

include_directories(${KODI_INCLUDE_DIR}
                    ${kodiplatform_INCLUDE_DIRS}
                    ${p8-platform_INCLUDE_DIRS}
                    ${PROJECT_SOURCE_DIR}/src)

list(APPEND DEPLIBS ${kodiplatform_LIBRARIES} ${p8-platform_LIBRARIES})

set(LIBRETRO_SOURCES src/client.cpp
                     src/audio/AudioStream.cpp
                     src/audio/SingleFrameAudio.cpp
                     src/GameInfoLoader.cpp
                     src/input/ButtonMapper.cpp
                     src/input/InputManager.cpp
                     src/input/LibretroDevice.cpp
                     src/input/LibretroDeviceInput.cpp
                     src/libretro/ClientBridge.cpp
                     src/libretro/FrontendBridge.cpp
                     src/libretro/LibretroDLL.cpp
                     src/libretro/LibretroEnvironment.cpp
                     src/libretro/LibretroResources.cpp
                     src/libretro/LibretroTranslator.cpp
                     src/log/Log.cpp
                     src/log/LogAddon.cpp
                     src/log/LogConsole.cpp
                     src/settings/LanguageGenerator.cpp
                     src/settings/LibretroSetting.cpp
                     src/settings/LibretroSettings.cpp
                     src/settings/Settings.cpp
                     src/settings/SettingsGenerator.cpp
                     src/utils/PathUtils.cpp
                     src/video/VideoStream.cpp)

set(LIBRETRO_HEADERS src/GameInfoLoader.h
                     src/audio/AudioStream.h
                     src/audio/SingleFrameAudio.h
                     src/input/ButtonMapper.h
                     src/input/InputDefinitions.h
                     src/input/InputManager.h
                     src/input/LibretroDevice.h
                     src/input/LibretroDeviceInput.h
                     src/libretro/ClientBridge.h
                     src/libretro/FrontendBridge.h
                     src/libretro/LibretroDefines.h
                     src/libretro/LibretroDLL.h
                     src/libretro/LibretroEnvironment.h
                     src/libretro/libretro.h
                     src/libretro/LibretroResources.h
                     src/libretro/LibretroTranslator.h
                     src/log/ILog.h
                     src/log/LogAddon.h
                     src/log/LogConsole.h
                     src/log/Log.h
                     src/settings/LanguageGenerator.h
                     src/settings/LibretroSetting.h
                     src/settings/LibretroSettings.h
                     src/settings/SettingsGenerator.h
                     src/settings/Settings.h
                     src/settings/SettingsTypes.h
                     src/utils/PathUtils.h
                     src/video/VideoStream.h)

build_addon(${PROJECT_NAME} LIBRETRO DEPLIBS)
