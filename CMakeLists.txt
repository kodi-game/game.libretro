cmake_minimum_required(VERSION 3.5...4.0)
project(game.libretro)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

find_package(Kodi REQUIRED)
find_package(LibretroCommon REQUIRED)
find_package(Rcheevos REQUIRED)
find_package(TinyXML REQUIRED)

add_definitions(${RCHEEVOS_DEFINITIONS})

include_directories(${KODI_INCLUDE_DIR}
                    ${KODI_INCLUDE_DIR}/..
                    ${PROJECT_SOURCE_DIR}/src
                    ${LIBRETRO_COMMON_INCLUDE_DIRS}
                    ${TINYXML_INCLUDE_DIRS}
                    ${RCHEEVOS_INCLUDE_DIRS})

list(APPEND DEPLIBS ${TINYXML_LIBRARIES})
list(APPEND DEPLIBS ${RCHEEVOS_LIBRARIES})

if(WIN32)
  find_package(dlfcn-win32 REQUIRED)
  list(APPEND DEPLIBS ${dlfcn-win32_LIBRARIES})
  include_directories(${dlfcn-win32_INCLUDE_DIRS})
endif()

set(LIBRETRO_SOURCES src/client.cpp
                     src/audio/AudioStream.cpp
                     src/audio/AudioTiming.cpp
                     src/audio/SingleFrameAudio.cpp
                     src/cheevos/Cheevos.cpp
                     src/cheevos/CheevosEnvironment.cpp
                     src/cheevos/CheevosFrontendBridge.cpp
                     src/cheevos/CheevosUtils.cpp
                     src/GameInfoLoader.cpp
                     src/input/ButtonMapper.cpp
                     src/input/ControllerLayout.cpp
                     src/input/ControllerTopology.cpp
                     src/input/DefaultControllerTranslator.cpp
                     src/input/DefaultKeyboardTranslator.cpp
                     src/input/InputManager.cpp
                     src/input/InputTranslator.cpp
                     src/input/LibretroDevice.cpp
                     src/input/LibretroDeviceInput.cpp
                     src/libretro/ClientBridge.cpp
                     src/libretro/FrontendBridge.cpp
                     src/libretro/LibretroDLL.cpp
                     src/libretro/LibretroEnvironment.cpp
                     src/libretro/LibretroResources.cpp
                     src/libretro/LibretroTranslator.cpp
                     src/libretro/MemoryMap.cpp
                     src/log/Log.cpp
                     src/log/LogAddon.cpp
                     src/log/LogConsole.cpp
                     src/settings/LanguageGenerator.cpp
                     src/settings/LibretroSetting.cpp
                     src/settings/LibretroSettings.cpp
                     src/settings/Settings.cpp
                     src/settings/SettingsGenerator.cpp
                     src/utils/Base64.cpp
                     src/utils/Timer.cpp
                     src/video/VideoGeometry.cpp
                     src/video/VideoStream.cpp
                     src/video/VideoTiming.cpp
)

set(LIBRETRO_HEADERS src/GameInfoLoader.h
                     src/audio/AudioStream.h
                     src/audio/AudioTiming.h
                     src/audio/SingleFrameAudio.h
                     src/input/ButtonMapper.h
                     src/cheevos/Cheevos.h
                     src/cheevos/CheevosUtils.h
                     src/input/ControllerLayout.h
                     src/input/ControllerTopology.h
                     src/input/DefaultControllerDefines.h
                     src/input/DefaultControllerTranslator.h
                     src/input/DefaultKeyboardDefines.h
                     src/input/DefaultKeyboardTranslator.h
                     src/input/InputDefinitions.h
                     src/input/InputManager.h
                     src/input/InputTranslator.h
                     src/input/InputTypes.h
                     src/input/LibretroDevice.h
                     src/input/LibretroDeviceInput.h
                     src/libretro/ClientBridge.h
                     src/libretro/FrontendBridge.h
                     src/libretro/LibretroDefines.h
                     src/libretro/LibretroDLL.h
                     src/libretro/LibretroEnvironment.h
                     src/libretro/LibretroResources.h
                     src/libretro/LibretroTranslator.h
                     src/libretro/MemoryMap.h
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
                     src/utils/Base64.h
                     src/utils/Timer.h
                     src/video/VideoGeometry.h
                     src/video/VideoStream.h
                     src/video/VideoTiming.h
)

# The frontend gains the achievement indicator callbacks in Game API 8.1.0.
# Against an older API they are not there, so build without forwarding them and
# leave the frontend showing nothing. Detected rather than version-tested,
# because the two land together and either may arrive first.
include(CheckStructHasMember)
check_struct_has_member("AddonToKodiFuncTable_Game"
                        RCOnProgressIndicator
                        "${KODI_INCLUDE_DIR}/c-api/addon-instance/game.h"
                        HAVE_GAME_RC_INDICATORS
                        LANGUAGE CXX)

build_addon(${PROJECT_NAME} LIBRETRO DEPLIBS)

if(HAVE_GAME_RC_INDICATORS)
  target_compile_definitions(${PROJECT_NAME} PRIVATE HAVE_GAME_RC_INDICATORS)
endif()

if(ENABLE_INTERNAL_LIBRETROCOMMON)
  add_dependencies(${PROJECT_NAME} libretro-common)
endif()
if(ENABLE_INTERNAL_RCHEEVOS)
  add_dependencies(${PROJECT_NAME} rcheevos)
endif()

include(CPack)
