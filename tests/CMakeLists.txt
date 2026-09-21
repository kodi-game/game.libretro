cmake_minimum_required(VERSION 3.5...4.0)
project(game_libretro_tests LANGUAGES CXX)

enable_testing()
find_path(LIBRETRO_INCLUDE_DIR libretro.h PATH_SUFFIXES libretro-common REQUIRED)

add_library(memory_model STATIC ../src/memory/MemoryMap.cpp ../src/memory/LibretroMemory.cpp)
target_include_directories(memory_model PUBLIC ../src ${LIBRETRO_INCLUDE_DIR})
target_compile_features(memory_model PUBLIC cxx_std_11)
add_executable(test_memory_map TestMemoryMap.cpp)
target_link_libraries(test_memory_map PRIVATE memory_model)
add_test(NAME memory_map COMMAND test_memory_map)
set_tests_properties(memory_map PROPERTIES TIMEOUT 10)

# Use the addon's already-built, pinned rcheevos dependency. Never download it.
find_path(RCHEEVOS_INCLUDE_DIR rc_libretro.h PATH_SUFFIXES rcheevos)
find_library(RCHEEVOS_LIBRARY NAMES rcheevoslib)
if(RCHEEVOS_INCLUDE_DIR AND RCHEEVOS_LIBRARY)
  message(STATUS "RA memory test headers: ${RCHEEVOS_INCLUDE_DIR}")
  message(STATUS "RA memory test library: ${RCHEEVOS_LIBRARY}")
  get_filename_component(RCHEEVOS_INCLUDE_PARENT "${RCHEEVOS_INCLUDE_DIR}" DIRECTORY)
  add_executable(test_cheevos_memory TestCheevosMemory.cpp ../src/memory/CheevosMemory.cpp)
  target_include_directories(test_cheevos_memory PRIVATE
                             ${RCHEEVOS_INCLUDE_DIR} ${RCHEEVOS_INCLUDE_PARENT})
  target_compile_definitions(test_cheevos_memory PRIVATE RC_STATIC)
  target_link_libraries(test_cheevos_memory PRIVATE memory_model ${RCHEEVOS_LIBRARY})
  add_test(NAME cheevos_memory COMMAND test_cheevos_memory)
  set_tests_properties(cheevos_memory PROPERTIES TIMEOUT 10)
else()
  message(STATUS "RA memory tests disabled: set RCHEEVOS_INCLUDE_DIR and RCHEEVOS_LIBRARY to v12.5.0")
endif()

set(GAME_LIBRETRO_LIBRARY "" CACHE FILEPATH "Built game.libretro library for retained-instance tests")
if(GAME_LIBRETRO_LIBRARY)
  if(NOT EXISTS "${GAME_LIBRETRO_LIBRARY}")
    message(FATAL_ERROR "GAME_LIBRETRO_LIBRARY does not exist: ${GAME_LIBRETRO_LIBRARY}")
  endif()
  find_package(Kodi REQUIRED)

  add_library(reload_core SHARED ReloadCore.cpp)
  target_include_directories(reload_core PRIVATE ${LIBRETRO_INCLUDE_DIR})
  add_executable(test_reload TestReload.cpp)
  target_include_directories(test_reload PRIVATE ${KODI_INCLUDE_DIR}/.. ${LIBRETRO_INCLUDE_DIR})
  target_link_libraries(test_reload PRIVATE ${CMAKE_DL_LIBS})
  target_compile_features(test_reload PRIVATE cxx_std_11)

  add_test(NAME retained_instance_reload
           COMMAND test_reload "${GAME_LIBRETRO_LIBRARY}" $<TARGET_FILE:reload_core>)
  add_test(NAME hardware_stream_lifecycle
           COMMAND test_reload "${GAME_LIBRETRO_LIBRARY}" $<TARGET_FILE:reload_core> hardware)
  set_tests_properties(retained_instance_reload hardware_stream_lifecycle PROPERTIES TIMEOUT 10)
  foreach(scenario preferred_gl preferred_gles preferred_none retained_hardware_software
                   hardware_geometry_growth failed_load_content failed_load_standalone
                   failed_load_memory_retry memory_map_reload memory_map_runtime_replace
                   memory_map_failed_load settings_core_switch)
    add_test(NAME ${scenario}
             COMMAND test_reload "${GAME_LIBRETRO_LIBRARY}" $<TARGET_FILE:reload_core> ${scenario})
    set_tests_properties(${scenario} PROPERTIES TIMEOUT 10)
  endforeach()
endif()
