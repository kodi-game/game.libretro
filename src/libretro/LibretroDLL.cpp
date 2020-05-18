/*
 *  Copyright (C) 2014-2020 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#include "LibretroDLL.h"
#include "log/Log.h"

#include <kodi/addon-instance/Game.h>

#include <dlfcn.h>

#include <assert.h>

using namespace LIBRETRO;

CLibretroDLL::CLibretroDLL(void) :
  retro_set_environment(nullptr),
  retro_set_video_refresh(nullptr),
  retro_set_audio_sample(nullptr),
  retro_set_audio_sample_batch(nullptr),
  retro_set_input_poll(nullptr),
  retro_set_input_state(nullptr),
  retro_init(nullptr),
  retro_deinit(nullptr),
  retro_api_version(nullptr),
  retro_get_system_info(nullptr),
  retro_get_system_av_info(nullptr),
  retro_set_controller_port_device(nullptr),
  retro_reset(nullptr),
  retro_run(nullptr),
  retro_serialize_size(nullptr),
  retro_serialize(nullptr),
  retro_unserialize(nullptr),
  retro_cheat_reset(nullptr),
  retro_cheat_set(nullptr),
  retro_load_game(nullptr),
  retro_load_game_special(nullptr),
  retro_unload_game(nullptr),
  retro_get_region(nullptr),
  retro_get_memory_data(nullptr),
  retro_get_memory_size(nullptr),
  m_libretroClient(nullptr)
{
}

void CLibretroDLL::Unload(void)
{
  if (m_libretroClient)
  {
    dlclose(m_libretroClient);
    m_libretroClient = nullptr;
  }

  m_strPath.clear();
}

// Convert functionPtr to a string literal
#define LIBRETRO_REGISTER_SYMBOL(dll, functionPtr)  RegisterSymbol(dll, functionPtr, #functionPtr)

// Register symbols from DLL, cast to type T and store in member variable
template <typename T>
bool RegisterSymbol(void* dll, T& functionPtr, const char* strFunctionPtr)
{
  return (functionPtr = (T)dlsym(dll, strFunctionPtr)) != nullptr;
}

bool CLibretroDLL::Load(const std::string& gameClientDllPath)
{
  Unload();

  m_strPath = gameClientDllPath;
  m_libretroClient = dlopen(m_strPath.c_str(), RTLD_LAZY);
  if (m_libretroClient == nullptr)
  {
    esyslog("Unable to load: %s", dlerror());
    return false;
  }

  try
  {
    if (!LIBRETRO_REGISTER_SYMBOL(m_libretroClient, retro_set_environment)) throw false;
    if (!LIBRETRO_REGISTER_SYMBOL(m_libretroClient, retro_set_video_refresh)) throw false;
    if (!LIBRETRO_REGISTER_SYMBOL(m_libretroClient, retro_set_audio_sample)) throw false;
    if (!LIBRETRO_REGISTER_SYMBOL(m_libretroClient, retro_set_audio_sample_batch)) throw false;
    if (!LIBRETRO_REGISTER_SYMBOL(m_libretroClient, retro_set_input_poll)) throw false;
    if (!LIBRETRO_REGISTER_SYMBOL(m_libretroClient, retro_set_input_state)) throw false;
    if (!LIBRETRO_REGISTER_SYMBOL(m_libretroClient, retro_init)) throw false;
    if (!LIBRETRO_REGISTER_SYMBOL(m_libretroClient, retro_deinit)) throw false;
    if (!LIBRETRO_REGISTER_SYMBOL(m_libretroClient, retro_api_version)) throw false;
    if (!LIBRETRO_REGISTER_SYMBOL(m_libretroClient, retro_get_system_info)) throw false;
    if (!LIBRETRO_REGISTER_SYMBOL(m_libretroClient, retro_get_system_av_info)) throw false;
    if (!LIBRETRO_REGISTER_SYMBOL(m_libretroClient, retro_set_controller_port_device)) throw false;
    if (!LIBRETRO_REGISTER_SYMBOL(m_libretroClient, retro_reset)) throw false;
    if (!LIBRETRO_REGISTER_SYMBOL(m_libretroClient, retro_run)) throw false;
    if (!LIBRETRO_REGISTER_SYMBOL(m_libretroClient, retro_serialize_size)) throw false;
    if (!LIBRETRO_REGISTER_SYMBOL(m_libretroClient, retro_serialize)) throw false;
    if (!LIBRETRO_REGISTER_SYMBOL(m_libretroClient, retro_unserialize)) throw false;
    if (!LIBRETRO_REGISTER_SYMBOL(m_libretroClient, retro_cheat_reset)) throw false;
    if (!LIBRETRO_REGISTER_SYMBOL(m_libretroClient, retro_cheat_set)) throw false;
    if (!LIBRETRO_REGISTER_SYMBOL(m_libretroClient, retro_load_game)) throw false;
    if (!LIBRETRO_REGISTER_SYMBOL(m_libretroClient, retro_load_game_special)) throw false;
    if (!LIBRETRO_REGISTER_SYMBOL(m_libretroClient, retro_unload_game)) throw false;
    if (!LIBRETRO_REGISTER_SYMBOL(m_libretroClient, retro_get_region)) throw false;
    if (!LIBRETRO_REGISTER_SYMBOL(m_libretroClient, retro_get_memory_data)) throw false;
    if (!LIBRETRO_REGISTER_SYMBOL(m_libretroClient, retro_get_memory_size)) throw false;
  }
  catch (const bool& bSuccess)
  {
    esyslog("Unable to assign function: %s", dlerror());
    return bSuccess;
  }

  return true;
}
