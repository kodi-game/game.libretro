cmake_minimum_required(VERSION 3.5...4.0)
project(game_libretro_reload_test LANGUAGES CXX)

find_package(Kodi REQUIRED)
find_path(LIBRETRO_INCLUDE_DIR libretro.h PATH_SUFFIXES libretro-common REQUIRED)
set(GAME_LIBRETRO_LIBRARY "" CACHE FILEPATH "Built game.libretro library to test")
if(NOT EXISTS "${GAME_LIBRETRO_LIBRARY}")
  message(FATAL_ERROR "Set GAME_LIBRETRO_LIBRARY to the built game.libretro library")
endif()

add_library(reload_core SHARED ReloadCore.cpp)
target_include_directories(reload_core PRIVATE ${LIBRETRO_INCLUDE_DIR})
add_executable(test_reload TestReload.cpp)
target_include_directories(test_reload PRIVATE ${KODI_INCLUDE_DIR}/.. ${LIBRETRO_INCLUDE_DIR})
target_link_libraries(test_reload PRIVATE ${CMAKE_DL_LIBS})
target_compile_features(test_reload PRIVATE cxx_std_11)

enable_testing()
add_test(NAME retained_instance_reload
         COMMAND test_reload "${GAME_LIBRETRO_LIBRARY}" $<TARGET_FILE:reload_core>)
set_tests_properties(retained_instance_reload PROPERTIES TIMEOUT 10)
