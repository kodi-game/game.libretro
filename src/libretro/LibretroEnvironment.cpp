/*
 *  Copyright (C) 2014-2021 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#include "LibretroEnvironment.h"
#include "ClientBridge.h"
#include "FrontendBridge.h"
#include "libretro-common/libretro.h"
#include "LibretroDLL.h"
#include "LibretroTranslator.h"
#include "input/InputManager.h"
#include "log/Log.h"
#include "settings/Settings.h"
#include "video/VideoGeometry.h"
#include "client.h"

#include <kodi/General.h>

using namespace LIBRETRO;

namespace LIBRETRO
{
  bool EnvCallback(unsigned cmd, void* data)
  {
    return CLibretroEnvironment::Get().EnvironmentCallback(cmd, data);
  }
}

CLibretroEnvironment::CLibretroEnvironment(void) :
  m_addon(nullptr),
  m_client(nullptr),
  m_clientBridge(nullptr),
  m_videoFormat(GAME_PIXEL_FORMAT_0RGB1555), // Default libretro format
  m_videoRotation(GAME_VIDEO_ROTATION_0)
{
}

CLibretroEnvironment& CLibretroEnvironment::Get(void)
{
  static CLibretroEnvironment _instance;
  return _instance;
}

void CLibretroEnvironment::InitializeEnvironment(CGameLibRetro* addon,
                                                 CLibretroDLL* client,
                                                 CClientBridge* clientBridge)
{
  m_addon = addon;
  m_client = client;
  m_clientBridge = clientBridge;

  m_videoStream.Initialize(m_addon);
  m_audioStream.Initialize(m_addon);

  m_settings.Initialize(m_addon);
  m_resources.Initialize(m_addon);

  // Install environment callback
  m_client->retro_set_environment(EnvCallback);
}

void CLibretroEnvironment::InitializeCallbacks()
{
  // Install remaining callbacks
  m_client->retro_set_video_refresh(CFrontendBridge::VideoRefresh);
  m_client->retro_set_audio_sample(CFrontendBridge::AudioFrame);
  m_client->retro_set_audio_sample_batch(CFrontendBridge::AudioFrames);
  m_client->retro_set_input_poll(CFrontendBridge::InputPoll);
  m_client->retro_set_input_state(CFrontendBridge::InputState);
}

void CLibretroEnvironment::Deinitialize()
{
  CloseStreams();

  m_resources.Deinitialize();
  m_settings.Deinitialize();
}

void CLibretroEnvironment::CloseStreams()
{
  m_videoStream.Deinitialize();
  m_audioStream.Deinitialize();
}

void CLibretroEnvironment::UpdateVideoGeometry(const retro_game_geometry &geometry)
{
  CVideoGeometry videoGeometry(geometry);
  m_videoStream.SetGeometry(videoGeometry);
}

void CLibretroEnvironment::SetSetting(const std::string& name, const std::string& value)
{
  m_settings.SetCurrentValue(name, value);
}

std::string CLibretroEnvironment::GetResourcePath(const char* relPath)
{
  return m_resources.GetFullPath(relPath);
}

void CLibretroEnvironment::OnFrameEnd()
{
  m_videoStream.OnFrameEnd();
}

bool CLibretroEnvironment::EnvironmentCallback(unsigned int cmd, void *data)
{
  if (!m_addon || !m_clientBridge)
    return false;

  switch (cmd)
  {
  case RETRO_ENVIRONMENT_SET_ROTATION:
    {
      unsigned* typedData = reinterpret_cast<unsigned*>(data);
      if (typedData)
        m_videoRotation = LibretroTranslator::GetVideoRotation(*typedData);
      break;
    }
  case RETRO_ENVIRONMENT_GET_OVERSCAN:
    {
      bool* typedData = reinterpret_cast<bool*>(data);
      if (typedData)
        *typedData = !CSettings::Get().CropOverscan();
      break;
    }
  case RETRO_ENVIRONMENT_GET_CAN_DUPE:
    {
      bool* typedData = reinterpret_cast<bool*>(data);
      if (typedData)
        *typedData = true;
      break;
    }
  case RETRO_ENVIRONMENT_SET_MESSAGE:
    {
      // Sets a message to be displayed. Generally not for trivial messages.
      const retro_message* typedData = reinterpret_cast<const retro_message*>(data);
      if (typedData)
      {
        const char* msg = typedData->msg;
        kodi::QueueFormattedNotification(QUEUE_INFO, msg);
      }
      break;
    }
  case RETRO_ENVIRONMENT_SHUTDOWN:
    {
      m_addon->CloseGame();
      break;
    }
  case RETRO_ENVIRONMENT_SET_PERFORMANCE_LEVEL:
    {
      const unsigned* typedData = reinterpret_cast<const unsigned*>(data);
      // Removed from Game API
      (void)typedData;
      break;
    }
  case RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY:
    {
      const char** typedData = reinterpret_cast<const char**>(data);
      if (typedData)
      {
        *typedData = m_resources.GetSystemDir();
      }
      break;
    }
  case RETRO_ENVIRONMENT_SET_PIXEL_FORMAT:
    {
      const retro_pixel_format* typedData = reinterpret_cast<const retro_pixel_format*>(data);
      if (!typedData)
        return false;

      dsyslog("Setting libretro pixel format \"%s\"", LibretroTranslator::VideoFormatToString(*typedData));

      m_videoFormat = LibretroTranslator::GetVideoFormat(*typedData);

      break;
    }
  case RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS:
    {
      const retro_input_descriptor* typedData = reinterpret_cast<const retro_input_descriptor*>(data);
      if (typedData)
        CInputManager::Get().LogInputDescriptors(typedData);
      break;
    }
  case RETRO_ENVIRONMENT_SET_KEYBOARD_CALLBACK:
    {
      const retro_keyboard_callback* typedData = reinterpret_cast<const retro_keyboard_callback*>(data);
      if (typedData)
      {
        // Store callback from libretro client
        m_clientBridge->SetKeyboardEvent(typedData->callback);
      }
      break;
    }
  case RETRO_ENVIRONMENT_SET_DISK_CONTROL_INTERFACE:
    {
      const retro_disk_control_callback *typedData = reinterpret_cast<const retro_disk_control_callback*>(data);
      if (typedData)
      {
        // Disk control interface not implemented
        return false;
      }
      break;
    }
  case RETRO_ENVIRONMENT_SET_HW_RENDER:
    {
      retro_hw_render_callback* typedData = reinterpret_cast<retro_hw_render_callback*>(data);
      if (typedData)
      {
        // Translate struct and report hw info to frontend
        game_stream_hw_framebuffer_properties hw_info;
        hw_info.context_type       = LibretroTranslator::GetHWContextType(typedData->context_type);
        hw_info.depth              = typedData->depth;
        hw_info.stencil            = typedData->stencil;
        hw_info.bottom_left_origin = typedData->bottom_left_origin;
        hw_info.version_major      = typedData->version_major;
        hw_info.version_minor      = typedData->version_minor;
        hw_info.cache_context      = typedData->cache_context;
        hw_info.debug_context      = typedData->debug_context;
        if (!m_videoStream.EnableHardwareRendering(hw_info))
          return false;

        // Store callbacks from libretro client
        m_clientBridge->SetHwContextReset(typedData->context_reset);
        m_clientBridge->SetHwContextDestroy(typedData->context_destroy);

        // Expose frontend callbacks to libretro client
        typedData->get_current_framebuffer = CFrontendBridge::HwGetCurrentFramebuffer;
        typedData->get_proc_address        = CFrontendBridge::HwGetProcAddress;
      }
      break;
    }
  case RETRO_ENVIRONMENT_GET_VARIABLE:
    {
      retro_variable* typedData = reinterpret_cast<retro_variable*>(data);
      if (typedData)
      {
        const char* strKey = typedData->key;
        if (strKey == nullptr)
          return false;

        typedData->value = m_settings.GetCurrentValue(strKey);

        // Assume libretro core is retrieving all variables at a time
        m_settings.SetUnchanged();
       }
       break;
    }
  case RETRO_ENVIRONMENT_SET_VARIABLES:
    {
      const retro_variable* typedData = reinterpret_cast<const retro_variable*>(data);
      if (typedData)
      {
        m_settings.SetAllSettings(typedData);
      }
      break;
    }
  case RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE:
    {
      bool* typedData = reinterpret_cast<bool*>(data);
      if (typedData)
      {
        *typedData = m_settings.Changed();
      }
      break;
    }
  case RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME:
    {
      const bool* typedData = reinterpret_cast<const bool*>(data);
      if (typedData)
      {
        const bool bSupportsNoGame = *typedData;
        if (bSupportsNoGame)
          kodi::Log(ADDON_LOG_DEBUG, "Libretro client supports loading with no game");
      }
      break;
    }
  case RETRO_ENVIRONMENT_GET_LIBRETRO_PATH:
    {
      const char** typedData = reinterpret_cast<const char**>(data);
      if (typedData)
      {
        *typedData = m_resources.GetContentDirectory();
      }
      break;
    }
  case RETRO_ENVIRONMENT_SET_AUDIO_CALLBACK:
    {
      const retro_audio_callback *typedData = reinterpret_cast<const retro_audio_callback*>(data);
      if (typedData)
      {
        // Store callbacks from libretro client
        m_clientBridge->SetAudioAvailable(typedData->callback);
        m_clientBridge->SetAudioEnable(typedData->set_state);
      }
      break;
    }
  case RETRO_ENVIRONMENT_SET_FRAME_TIME_CALLBACK:
    {
      const retro_frame_time_callback *typedData = reinterpret_cast<const retro_frame_time_callback*>(data);
      if (typedData)
      {
        // Store callbacks from libretro client.
        m_clientBridge->SetFrameTime(typedData->callback);
        return false;
      }
      break;
    }
  case RETRO_ENVIRONMENT_GET_RUMBLE_INTERFACE:
    {
      retro_rumble_interface* typedData = reinterpret_cast<retro_rumble_interface*>(data);
      if (typedData)
      {
        // Expose callback to libretro core
        typedData->set_rumble_state = CFrontendBridge::RumbleSetState;
      }
      break;
    }
  case RETRO_ENVIRONMENT_GET_INPUT_DEVICE_CAPABILITIES:
    {
      uint64_t* typedData = reinterpret_cast<uint64_t*>(data);
      if (typedData)
        *typedData = CInputManager::Get().GetDeviceCaps();
      break;
    }
  case RETRO_ENVIRONMENT_GET_SENSOR_INTERFACE:
    {
      retro_sensor_interface* typedData = reinterpret_cast<retro_sensor_interface*>(data);
      if (typedData)
      {
        // Expose callbacks to libretro core
        typedData->set_sensor_state = CFrontendBridge::SensorSetState;
        typedData->get_sensor_input = CFrontendBridge::SensorGetInput;
      }
      break;
    }
  case RETRO_ENVIRONMENT_GET_CAMERA_INTERFACE:
    {
      retro_camera_callback* typedData = reinterpret_cast<retro_camera_callback*>(data);
      if (typedData)
      {
        // Camera interface not implemented
        return false;
      }
      break;
    }
  case RETRO_ENVIRONMENT_GET_LOG_INTERFACE:
    {
      retro_log_callback* typedData = reinterpret_cast<retro_log_callback*>(data);
      if (typedData)
      {
        // Expose callback to libretro core
        typedData->log = CFrontendBridge::LogFrontend; // libretro logging forwards to Kodi add-on log function
      }
      break;
    }
  case RETRO_ENVIRONMENT_GET_PERF_INTERFACE:
    {
      retro_perf_callback* typedData = reinterpret_cast<retro_perf_callback*>(data);
      if (typedData)
      {
        // Expose callbacks to libretro core
        typedData->get_time_usec    = CFrontendBridge::PerfGetTimeUsec;
        typedData->get_cpu_features = CFrontendBridge::PerfGetCpuFeatures;
        typedData->get_perf_counter = CFrontendBridge::PerfGetCounter;
        typedData->perf_register    = CFrontendBridge::PerfRegister;
        typedData->perf_start       = CFrontendBridge::PerfStart;
        typedData->perf_stop        = CFrontendBridge::PerfStop;
        typedData->perf_log         = CFrontendBridge::PerfLog;
      }
      break;
    }
  case RETRO_ENVIRONMENT_GET_LOCATION_INTERFACE:
    {
      retro_location_callback* typedData = reinterpret_cast<retro_location_callback*>(data);
      if (typedData)
      {
        // Expose callbacks to libretro core
        typedData->start         = CFrontendBridge::StartLocation;
        typedData->stop          = CFrontendBridge::StopLocation;
        typedData->get_position  = CFrontendBridge::GetLocation;
        typedData->set_interval  = CFrontendBridge::SetLocationInterval;
        typedData->initialized   = CFrontendBridge::LocationInitialized;
        typedData->deinitialized = CFrontendBridge::LocationDeinitialized;
      }
      break;
    }
  case RETRO_ENVIRONMENT_GET_CONTENT_DIRECTORY:
    {
      const char** typedData = reinterpret_cast<const char**>(data);
      if (typedData)
      {
        *typedData = m_resources.GetContentDirectory();
      }
      break;
    }
  case RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY:
    {
      const char** typedData = reinterpret_cast<const char**>(data);
      if (typedData)
      {
        *typedData = m_resources.GetSaveDirectory();
      }
      break;
    }
  case RETRO_ENVIRONMENT_SET_SYSTEM_AV_INFO:
    {
      const retro_system_av_info* typedData = reinterpret_cast<const retro_system_av_info*>(data);
      if (!typedData)
        return false;

      CVideoGeometry videoGeometry(typedData->geometry);
      m_videoStream.SetGeometry(videoGeometry);

      //! @todo Reopen streams if geometry changes

      //! @todo Report updating timing info to frontend
      const double fps = typedData->timing.fps;
      const double sampleRate = typedData->timing.sample_rate;

      break;
    }
  case RETRO_ENVIRONMENT_SET_PROC_ADDRESS_CALLBACK:
  {
    const retro_get_proc_address_interface* typedData = reinterpret_cast<const retro_get_proc_address_interface*>(data);
    if (typedData)
    {
      // get_proc_address() interface not implemented
      return false;
    }
    break;
  }
  case RETRO_ENVIRONMENT_SET_SUBSYSTEM_INFO:
  {
    const retro_subsystem_info* typedData = reinterpret_cast<const retro_subsystem_info*>(data);
    if (typedData)
    {
      // Not implemented
      return false;
    }
    break;
  }
  case RETRO_ENVIRONMENT_SET_CONTROLLER_INFO:
    {
      const retro_controller_info* typedData = reinterpret_cast<const retro_controller_info*>(data);
      if (typedData)
      {
        CInputManager::Get().SetControllerInfo(typedData);
      }
      break;
    }
  case RETRO_ENVIRONMENT_SET_MEMORY_MAPS:
  {
    const retro_memory_map* typedData = reinterpret_cast<const retro_memory_map*>(data);
    if (typedData)
      m_mmap.Initialize(*typedData);

    break;
  }
  case RETRO_ENVIRONMENT_SET_GEOMETRY:
  {
    const retro_game_geometry* typedData = reinterpret_cast<const retro_game_geometry*>(data);
    if (typedData)
    {
      // Not implemented
      return false;
    }
    break;
  }
  case RETRO_ENVIRONMENT_GET_USERNAME:
  {
    const char** typedData = reinterpret_cast<const char**>(data);
    if (typedData)
    {
      // Not implemented
      return false;
    }
    break;
  }
  case RETRO_ENVIRONMENT_GET_LANGUAGE:
  {
    unsigned int* typedData = reinterpret_cast<unsigned int*>(data);
    if (typedData)
    {
      // Not implemented
      return false;
    }
    break;
  }
  case RETRO_ENVIRONMENT_GET_CURRENT_SOFTWARE_FRAMEBUFFER:
  {
    retro_framebuffer* typedData = reinterpret_cast<retro_framebuffer*>(data);
    if (typedData)
    {
      // Get framebuffer params from core
      const unsigned int accessFlags = typedData->access_flags;
      const unsigned int width = typedData->width;
      const unsigned int height = typedData->height;

      // Reading framebuffers not supported
      if (accessFlags & RETRO_MEMORY_ACCESS_READ)
        return false;

      game_stream_sw_framebuffer_buffer framebuffer{};
      if (!m_videoStream.GetSwFramebuffer(width, height, m_videoFormat, framebuffer))
        return false;

      // Report framebuffer info to frontend
      typedData->data = framebuffer.data;
      typedData->pitch = framebuffer.size / height;
      typedData->format = LibretroTranslator::GetLibretroVideoFormat(framebuffer.format);
      typedData->memory_flags = 0;
    }
    break;
  }
  case RETRO_ENVIRONMENT_SET_HW_SHARED_CONTEXT:
  {
    // Not implemented
    return false;
  }
  case RETRO_ENVIRONMENT_GET_VFS_INTERFACE:
  {
    const uint32_t supported_vfs_version = 3;

    retro_vfs_interface_info* typedData = reinterpret_cast<retro_vfs_interface_info*>(data);
    if (typedData)
    {
      if (typedData->required_interface_version <= supported_vfs_version)
      {
        static retro_vfs_interface vfsInterface = {
          CFrontendBridge::GetPath,
          CFrontendBridge::OpenFile,
          CFrontendBridge::CloseFile,
          CFrontendBridge::FileSize,
          CFrontendBridge::GetPosition,
          CFrontendBridge::Seek,
          CFrontendBridge::ReadFile,
          CFrontendBridge::WriteFile,
          CFrontendBridge::FlushFile,
          CFrontendBridge::RemoveFile,
          CFrontendBridge::RenameFile,
          CFrontendBridge::Truncate,
          CFrontendBridge::Stat,
          CFrontendBridge::MakeDirectory,
          CFrontendBridge::OpenDirectory,
          CFrontendBridge::ReadDirectory,
          CFrontendBridge::GetDirectoryName,
          CFrontendBridge::IsDirectory,
          CFrontendBridge::CloseDirectory,
        };

        typedData->required_interface_version = supported_vfs_version;
        typedData->iface = &vfsInterface;
      }
    }
    break;
  }
  case RETRO_ENVIRONMENT_GET_LED_INTERFACE:
  {
    retro_led_interface* typedData = reinterpret_cast<retro_led_interface*>(data);
    if (typedData)
    {
      // Expose callback to libretro core
      typedData->set_led_state = CFrontendBridge::LedSetState;
    }
    break;
  }
  case RETRO_ENVIRONMENT_GET_HW_RENDER_INTERFACE:
  {
    const retro_hw_render_interface* typedData = reinterpret_cast<const retro_hw_render_interface*>(data);
    if (typedData)
    {
      // Not implemented
      return false;
    }
    break;
  }
  case RETRO_ENVIRONMENT_SET_SUPPORT_ACHIEVEMENTS:
  {
    const bool* typedData = reinterpret_cast<const bool*>(data);
    if (typedData)
    {
      // Not implemented
      return false;
    }
    break;
  }
  case RETRO_ENVIRONMENT_SET_HW_RENDER_CONTEXT_NEGOTIATION_INTERFACE:
  {
    const retro_hw_render_context_negotiation_interface* typedData = reinterpret_cast<const retro_hw_render_context_negotiation_interface*>(data);
    if (typedData)
    {
      // Not implemented
      return false;
    }
    break;
  }
  case RETRO_ENVIRONMENT_SET_SERIALIZATION_QUIRKS:
  {
    const retro_hw_render_context_negotiation_interface* typedData = reinterpret_cast<const retro_hw_render_context_negotiation_interface*>(data);
    if (typedData)
    {
      // Not implemented
      return false;
    }
    break;
  }
  default:
    return false;
  }

  return true;
}

const CMemoryMap& CLibretroEnvironment::GetMemoryMap()
{
  return m_mmap;
}
