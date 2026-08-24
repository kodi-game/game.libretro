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

void CLibretroEnvironment::OnFrameBegin()
{
  m_videoStream.OnFrameBegin();
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
      unsigned* typedData = static_cast<unsigned*>(data);
      if (typedData)
        m_videoRotation = LibretroTranslator::GetVideoRotation(*typedData);
      break;
    }
  case RETRO_ENVIRONMENT_GET_OVERSCAN:
    {
      bool* typedData = static_cast<bool*>(data);
      if (typedData)
        *typedData = !CSettings::Get().CropOverscan();
      break;
    }
  case RETRO_ENVIRONMENT_GET_CAN_DUPE:
    {
      bool* typedData = static_cast<bool*>(data);
      if (typedData)
        *typedData = true;
      break;
    }
  case RETRO_ENVIRONMENT_SET_MESSAGE:
    {
      // Sets a message to be displayed. Generally not for trivial messages.
      const retro_message* typedData = static_cast<const retro_message*>(data);
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
      const unsigned* typedData = static_cast<const unsigned*>(data);
      // Removed from Game API
      (void)typedData;
      break;
    }
  case RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY:
    {
      const char** typedData = static_cast<const char**>(data);
      if (typedData)
      {
        *typedData = m_resources.GetSystemDir();
      }
      break;
    }
  case RETRO_ENVIRONMENT_SET_PIXEL_FORMAT:
    {
      const retro_pixel_format* typedData = static_cast<const retro_pixel_format*>(data);
      if (!typedData)
        return false;

      dsyslog("Setting libretro pixel format \"%s\"", LibretroTranslator::VideoFormatToString(*typedData));

      m_videoFormat = LibretroTranslator::GetVideoFormat(*typedData);

      break;
    }
  case RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS:
    {
      const retro_input_descriptor* typedData = static_cast<const retro_input_descriptor*>(data);
      if (typedData)
        CInputManager::Get().LogInputDescriptors(typedData);
      break;
    }
  case RETRO_ENVIRONMENT_SET_KEYBOARD_CALLBACK:
    {
      const retro_keyboard_callback* typedData = static_cast<const retro_keyboard_callback*>(data);
      if (typedData)
      {
        // Store callback from libretro client
        m_clientBridge->SetKeyboardEvent(typedData->callback);
      }
      break;
    }
  case RETRO_ENVIRONMENT_SET_DISK_CONTROL_INTERFACE:
    {
      const retro_disk_control_callback *typedData = static_cast<const retro_disk_control_callback*>(data);
      if (typedData)
      {
        // Store callbacks from libretro client
        m_clientBridge->SetSetEjectState(typedData->set_eject_state);
        m_clientBridge->SetGetEjectState(typedData->get_eject_state);
        m_clientBridge->SetGetImageIndex(typedData->get_image_index);
        m_clientBridge->SetSetImageIndex(typedData->set_image_index);
        m_clientBridge->SetGetImageCount(typedData->get_num_images);
        m_clientBridge->SetReplaceImageIndex(typedData->replace_image_index);
        m_clientBridge->SetAddImageIndex(typedData->add_image_index);
      }
      break;
    }
  case RETRO_ENVIRONMENT_SET_HW_RENDER:
    {
      retro_hw_render_callback* typedData = static_cast<retro_hw_render_callback*>(data);
      if (typedData)
      {
        // Translate struct and report hw info to frontend
        game_hw_rendering_properties hw_info;
        hw_info.context_type       = LibretroTranslator::GetHWContextType(typedData->context_type);
        hw_info.depth              = typedData->depth;
        hw_info.stencil            = typedData->stencil;
        hw_info.bottom_left_origin = typedData->bottom_left_origin;
        hw_info.version_major      = typedData->version_major;
        hw_info.version_minor      = typedData->version_minor;
        hw_info.cache_context      = typedData->cache_context;
        hw_info.debug_context      = typedData->debug_context;

        // Set up the video stream
        if (!m_videoStream.EnableHardwareRendering())
          return false;

        // Store callbacks from libretro client
        m_clientBridge->SetHwContextReset(typedData->context_reset);
        m_clientBridge->SetHwContextDestroy(typedData->context_destroy);

        // Expose frontend callbacks to libretro client
        typedData->get_current_framebuffer = CFrontendBridge::HwGetCurrentFramebuffer;
        typedData->get_proc_address        = CFrontendBridge::HwGetProcAddress;

        // Now that hooks are installed, enable HW rendering in the frontend
        if (!m_addon->EnableHardwareRendering(hw_info))
          return false;
      }
      break;
    }
  case RETRO_ENVIRONMENT_GET_VARIABLE:
    {
      retro_variable* typedData = static_cast<retro_variable*>(data);
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
      const retro_variable* typedData = static_cast<const retro_variable*>(data);
      if (typedData)
      {
        m_settings.SetAllSettings(typedData);
      }
      break;
    }
  case RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE:
    {
      bool* typedData = static_cast<bool*>(data);
      if (typedData)
      {
        *typedData = m_settings.Changed();
      }
      break;
    }
  case RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME:
    {
      const bool* typedData = static_cast<const bool*>(data);
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
      const char** typedData = static_cast<const char**>(data);
      if (typedData)
      {
        *typedData = m_resources.GetContentDirectory();
      }
      break;
    }
  case RETRO_ENVIRONMENT_SET_FRAME_TIME_CALLBACK:
    {
      const retro_frame_time_callback *typedData = static_cast<const retro_frame_time_callback*>(data);
      if (typedData)
      {
        // Store callbacks from libretro client.
        m_clientBridge->SetFrameTime(typedData->callback);
        return false;
      }
      break;
    }
  case RETRO_ENVIRONMENT_SET_AUDIO_CALLBACK:
    {
      const retro_audio_callback *typedData = static_cast<const retro_audio_callback*>(data);
      if (typedData)
      {
        // Store callbacks from libretro client
        m_clientBridge->SetAudioAvailable(typedData->callback);
        m_clientBridge->SetAudioEnable(typedData->set_state);
      }
      break;
    }
  case RETRO_ENVIRONMENT_GET_RUMBLE_INTERFACE:
    {
      retro_rumble_interface* typedData = static_cast<retro_rumble_interface*>(data);
      if (typedData)
      {
        // Expose callback to libretro core
        typedData->set_rumble_state = CFrontendBridge::RumbleSetState;
      }
      break;
    }
  case RETRO_ENVIRONMENT_GET_INPUT_DEVICE_CAPABILITIES:
    {
      uint64_t* typedData = static_cast<uint64_t*>(data);
      if (typedData)
        *typedData = CInputManager::Get().GetDeviceCaps();
      break;
    }
  case RETRO_ENVIRONMENT_GET_SENSOR_INTERFACE:
    {
      retro_sensor_interface* typedData = static_cast<retro_sensor_interface*>(data);
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
      retro_camera_callback* typedData = static_cast<retro_camera_callback*>(data);
      if (typedData)
      {
        // Camera interface not implemented
        return false;
      }
      break;
    }
  case RETRO_ENVIRONMENT_GET_LOG_INTERFACE:
    {
      retro_log_callback* typedData = static_cast<retro_log_callback*>(data);
      if (typedData)
      {
        // Expose callback to libretro core
        typedData->log = CFrontendBridge::LogFrontend; // libretro logging forwards to Kodi add-on log function
      }
      break;
    }
  case RETRO_ENVIRONMENT_GET_PERF_INTERFACE:
    {
      retro_perf_callback* typedData = static_cast<retro_perf_callback*>(data);
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
      retro_location_callback* typedData = static_cast<retro_location_callback*>(data);
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
  case RETRO_ENVIRONMENT_GET_CORE_ASSETS_DIRECTORY:
    {
      const char** typedData = static_cast<const char**>(data);
      if (typedData)
      {
        *typedData = m_resources.GetContentDirectory();
      }
      break;
    }
  case RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY:
    {
      const char** typedData = static_cast<const char**>(data);
      if (typedData)
      {
        *typedData = m_resources.GetSaveDirectory();
      }
      break;
    }
  case RETRO_ENVIRONMENT_SET_SYSTEM_AV_INFO:
    {
      const retro_system_av_info* typedData = static_cast<const retro_system_av_info*>(data);
      if (!typedData)
        return false;

      UpdateVideoGeometry(typedData->geometry);

      const double fps = typedData->timing.fps;
      const double sampleRate = typedData->timing.sample_rate;

      m_videoTiming.SetFrameRate(fps);
      m_audioTiming.SetSampleRate(sampleRate);

      game_system_timing timingInfo{};
      timingInfo.fps = fps;
      timingInfo.sample_rate = sampleRate;
      m_addon->SetGameTiming(timingInfo);

      break;
    }
  case RETRO_ENVIRONMENT_SET_PROC_ADDRESS_CALLBACK:
  {
    const retro_get_proc_address_interface* typedData = static_cast<const retro_get_proc_address_interface*>(data);
    if (typedData)
    {
      // get_proc_address() interface not implemented
      return false;
    }
    break;
  }
  case RETRO_ENVIRONMENT_SET_SUBSYSTEM_INFO:
  {
    const retro_subsystem_info* typedData = static_cast<const retro_subsystem_info*>(data);
    if (typedData)
    {
      // Not implemented
      return false;
    }
    break;
  }
  case RETRO_ENVIRONMENT_SET_CONTROLLER_INFO:
    {
      const retro_controller_info* typedData = static_cast<const retro_controller_info*>(data);
      if (typedData)
      {
        CInputManager::Get().SetControllerInfo(typedData);
      }
      break;
    }
  case RETRO_ENVIRONMENT_SET_MEMORY_MAPS:
  {
    const retro_memory_map* typedData = static_cast<const retro_memory_map*>(data);
    if (typedData)
      m_mmap.Initialize(*typedData);

    break;
  }
  case RETRO_ENVIRONMENT_SET_GEOMETRY:
  {
    const retro_game_geometry* typedData = static_cast<const retro_game_geometry*>(data);
    if (typedData)
    {
      UpdateVideoGeometry(*typedData);
    }
    break;
  }
  case RETRO_ENVIRONMENT_GET_USERNAME:
  {
    const char** typedData = static_cast<const char**>(data);
    if (typedData)
    {
      // Not implemented
      return false;
    }
    break;
  }
  case RETRO_ENVIRONMENT_GET_LANGUAGE:
  {
    unsigned int* typedData = static_cast<unsigned int*>(data);
    if (typedData)
    {
      // Not implemented
      return false;
    }
    break;
  }
  case RETRO_ENVIRONMENT_GET_CURRENT_SOFTWARE_FRAMEBUFFER:
  {
    retro_framebuffer* typedData = static_cast<retro_framebuffer*>(data);
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
  case RETRO_ENVIRONMENT_GET_HW_RENDER_INTERFACE:
  {
    const retro_hw_render_interface* typedData = static_cast<const retro_hw_render_interface*>(data);
    if (typedData)
    {
      // Not implemented
      return false;
    }
    break;
  }
  case RETRO_ENVIRONMENT_SET_SUPPORT_ACHIEVEMENTS:
  {
    const bool* typedData = static_cast<const bool*>(data);
    if (typedData)
    {
      const bool supportsAchievements = *typedData;

      if (supportsAchievements)
        kodi::Log(ADDON_LOG_INFO, "This core supports achievements");
      else
        kodi::Log(ADDON_LOG_INFO, "This core doesn't support achievements");
    }
    break;
  }
  case RETRO_ENVIRONMENT_SET_HW_RENDER_CONTEXT_NEGOTIATION_INTERFACE:
  {
    const retro_hw_render_context_negotiation_interface* typedData = static_cast<const retro_hw_render_context_negotiation_interface*>(data);
    if (typedData)
    {
      // Not implemented
      return false;
    }
    break;
  }
  case RETRO_ENVIRONMENT_SET_SERIALIZATION_QUIRKS:
  {
    uint64_t* typedData = static_cast<uint64_t*>(data);
    if (typedData)
    {
      uint64_t quirks = *typedData;

      kodi::Log(ADDON_LOG_INFO, "------------------------------------------------------------");
      kodi::Log(ADDON_LOG_INFO, "Libretro serialization quirks:");

      if (quirks & RETRO_SERIALIZATION_QUIRK_INCOMPLETE)
        kodi::Log(ADDON_LOG_INFO, "  INCOMPLETE - Serialized state is incomplete in some way");

      if (quirks & RETRO_SERIALIZATION_QUIRK_MUST_INITIALIZE)
        kodi::Log(ADDON_LOG_INFO, "  MUST_INITIALIZE - Some initialization time is required");

      if (quirks & RETRO_SERIALIZATION_QUIRK_CORE_VARIABLE_SIZE)
        kodi::Log(ADDON_LOG_INFO, "  VARIABLE_SIZE - Serialization size may change within a session");

      // TODO: We don't support variable serialization size
      quirks &= ~RETRO_SERIALIZATION_QUIRK_FRONT_VARIABLE_SIZE;

      if (quirks & RETRO_SERIALIZATION_QUIRK_SINGLE_SESSION)
        kodi::Log(ADDON_LOG_INFO, "  SINGLE_SESSION - State can only be loaded during the same session");

      if (quirks & RETRO_SERIALIZATION_QUIRK_ENDIAN_DEPENDENT)
        kodi::Log(ADDON_LOG_INFO, "  ENDIAN_DEPENDENT - State cannot be loaded with a different system endianness");

      if (quirks & RETRO_SERIALIZATION_QUIRK_PLATFORM_DEPENDENT)
        kodi::Log(ADDON_LOG_INFO, "  PLATFORM_DEPENDENT - State needs the same platform, such as for word size dependence");

      kodi::Log(ADDON_LOG_INFO, "------------------------------------------------------------");

      *typedData = quirks;
    }
    break;
  }
  case RETRO_ENVIRONMENT_SET_HW_SHARED_CONTEXT:
  {
    // Parameter is ignored
    (void)data;

    // Not implemented
    return false;
  }
  case RETRO_ENVIRONMENT_GET_VFS_INTERFACE:
  {
    const uint32_t supported_vfs_version = 3;

    retro_vfs_interface_info* typedData = static_cast<retro_vfs_interface_info*>(data);
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
    retro_led_interface* typedData = static_cast<retro_led_interface*>(data);
    if (typedData)
    {
      // Expose callback to libretro core
      typedData->set_led_state = CFrontendBridge::LedSetState;
    }
    break;
  }
  case RETRO_ENVIRONMENT_GET_AUDIO_VIDEO_ENABLE:
  {
    retro_av_enable_flags* typedData = static_cast<retro_av_enable_flags*>(data);

    // Not implemented
    (void)typedData;
    return false;
  }
  case RETRO_ENVIRONMENT_GET_MIDI_INTERFACE:
  {
    retro_midi_interface* typedData = static_cast<retro_midi_interface*>(data);

    // Not implemented
    (void)typedData;
    return false;
  }
  case RETRO_ENVIRONMENT_GET_FASTFORWARDING:
  {
    bool* typedData = static_cast<bool*>(data);
    if (typedData != nullptr)
    {
      *typedData = CVideoTiming::IsFastForwarding(m_addon->GetPlaybackSpeed());
    }
    break;
  }
  case RETRO_ENVIRONMENT_GET_TARGET_REFRESH_RATE:
  {
    float* typedData = static_cast<float*>(data);

    // Not implemented
    (void)typedData;
    return false;
  }
  case RETRO_ENVIRONMENT_GET_INPUT_BITMASKS:
  {
    // Parameter is ignored
    (void)data;

    // Served in CFrontendBridge::InputState, which assembles the mask from the
    // individual button states. Saying yes only tells a core it may ask; a core
    // that asks anyway without checking -- LRPS2 does -- is answered either way.
    return true;
  }
  case RETRO_ENVIRONMENT_GET_CORE_OPTIONS_VERSION:
  {
    unsigned int* typedData = static_cast<unsigned int*>(data);
    if (typedData == nullptr)
      return false;

    // Version 2. A core told 0 here falls back to SET_VARIABLES, which most do
    // through libretro's own boilerplate -- but not all, and one that does not
    // ends up with none of its settings registered at all.
    *typedData = 2;
    break;
  }
  case RETRO_ENVIRONMENT_SET_CORE_OPTIONS:
  {
    const retro_core_option_definition* typedData =
        static_cast<const retro_core_option_definition*>(data);
    if (typedData == nullptr)
      return false;

    m_settings.SetAllSettings(typedData);
    break;
  }
  case RETRO_ENVIRONMENT_SET_CORE_OPTIONS_INTL:
  {
    const retro_core_options_intl* typedData = static_cast<const retro_core_options_intl*>(data);
    if (typedData == nullptr || typedData->us == nullptr)
      return false;

    // Only the American English set is taken. The translated one carries the
    // same keys and values, differing in text this add-on does not display.
    m_settings.SetAllSettings(typedData->us);
    break;
  }
  case RETRO_ENVIRONMENT_SET_CORE_OPTIONS_DISPLAY:
  {
    const retro_core_option_display* typedData = static_cast<const retro_core_option_display*>(data);

    // Not implemented
    (void)typedData;
    return false;
  }
  case RETRO_ENVIRONMENT_GET_PREFERRED_HW_RENDER:
  {
    retro_hw_context_type* typedData = static_cast<retro_hw_context_type*>(data);

    // Not implemented
    (void)typedData;
    return false;
  }
  case RETRO_ENVIRONMENT_GET_DISK_CONTROL_INTERFACE_VERSION:
  {
    unsigned int* typedData = static_cast<unsigned int*>(data);
    if (typedData != nullptr)
    {
      // Extended interface is supported
      *typedData = 1;
    }
    break;
  }
  case RETRO_ENVIRONMENT_SET_DISK_CONTROL_EXT_INTERFACE:
  {
    const retro_disk_control_ext_callback* typedData = static_cast<const retro_disk_control_ext_callback*>(data);
    if (typedData)
    {
      // Store callbacks from libretro client
      m_clientBridge->SetSetEjectState(typedData->set_eject_state);
      m_clientBridge->SetGetEjectState(typedData->get_eject_state);
      m_clientBridge->SetGetImageIndex(typedData->get_image_index);
      m_clientBridge->SetSetImageIndex(typedData->set_image_index);
      m_clientBridge->SetGetImageCount(typedData->get_num_images);
      m_clientBridge->SetReplaceImageIndex(typedData->replace_image_index);
      m_clientBridge->SetAddImageIndex(typedData->add_image_index);
      m_clientBridge->SetSetInitialImage(typedData->set_initial_image);
      m_clientBridge->SetGetImagePath(typedData->get_image_path);
      m_clientBridge->SetGetImageLabel(typedData->get_image_label);
    }
    break;
  }
  case RETRO_ENVIRONMENT_GET_MESSAGE_INTERFACE_VERSION:
  {
    unsigned int* typedData = static_cast<unsigned int*>(data);

    if (typedData != nullptr)
    {
      // Only legacy message interface is currently supported
      *typedData = 0;
    }

    break;
  }
  case RETRO_ENVIRONMENT_SET_MESSAGE_EXT:
  {
    const retro_message_ext* typedData = static_cast<const retro_message_ext*>(data);

    // Not implemented
    (void)typedData;
    return false;
  }
  case RETRO_ENVIRONMENT_GET_INPUT_MAX_USERS:
  {
    unsigned int* typedData = static_cast<unsigned int*>(data);

    // Not implemented
    (void)typedData;
    return false;
  }
  case RETRO_ENVIRONMENT_SET_AUDIO_BUFFER_STATUS_CALLBACK:
  {
    const retro_audio_buffer_status_callback* typedData = static_cast<const retro_audio_buffer_status_callback*>(data);

    // Not implemented
    (void)typedData;
    return false;
  }
  case RETRO_ENVIRONMENT_SET_MINIMUM_AUDIO_LATENCY:
  {
    const unsigned int* typedData = static_cast<const unsigned int*>(data);

    // Not implemented
    (void)typedData;
    return false;
  }
  case RETRO_ENVIRONMENT_SET_FASTFORWARDING_OVERRIDE:
  {
    const retro_fastforwarding_override* typedData = static_cast<const retro_fastforwarding_override*>(data);

    // Not implemented
    (void)typedData;
    return false;
  }
  case RETRO_ENVIRONMENT_SET_CONTENT_INFO_OVERRIDE:
  {
    const retro_system_content_info_override* typedData = static_cast<const retro_system_content_info_override*>(data);

    // Not implemented
    (void)typedData;
    return false;
  }
  case RETRO_ENVIRONMENT_GET_GAME_INFO_EXT:
  {
    const retro_game_info_ext** typedData = static_cast<const retro_game_info_ext**>(data);

    // Not implemented
    (void)typedData;
    return false;
  }
  case RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2:
  {
    const retro_core_options_v2* typedData = static_cast<const retro_core_options_v2*>(data);
    if (typedData == nullptr || typedData->definitions == nullptr)
      return false;

    // Categories are for grouping in a frontend's own settings UI, which this
    // add-on does not build -- the settings are Kodi's
    m_settings.SetAllSettings(typedData->definitions);
    break;
  }
  case RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2_INTL:
  {
    const retro_core_options_v2_intl* typedData =
        static_cast<const retro_core_options_v2_intl*>(data);
    if (typedData == nullptr || typedData->us == nullptr ||
        typedData->us->definitions == nullptr)
      return false;

    m_settings.SetAllSettings(typedData->us->definitions);
    break;
  }
  case RETRO_ENVIRONMENT_SET_CORE_OPTIONS_UPDATE_DISPLAY_CALLBACK:
  {
    const retro_core_options_update_display_callback* typedData = static_cast<const retro_core_options_update_display_callback*>(data);

    // Not implemented
    (void)typedData;
    return false;
  }
  case RETRO_ENVIRONMENT_SET_VARIABLE:
  {
    const retro_variable* typedData = static_cast<const retro_variable*>(data);

    // Not implemented
    (void)typedData;
    return false;
  }
  case RETRO_ENVIRONMENT_GET_THROTTLE_STATE:
  {
    retro_throttle_state* typedData = static_cast<retro_throttle_state*>(data);
    if (typedData != nullptr)
    {
      m_videoTiming.GetThrottleState(m_addon->GetPlaybackSpeed(), *typedData);
    }
    break;
  }
  case RETRO_ENVIRONMENT_GET_SAVESTATE_CONTEXT:
  {
    retro_savestate_context* typedData = static_cast<retro_savestate_context*>(data);

    if (typedData != nullptr)
    {
      // Least restrictive, and fits Kodi's current context
      *typedData = RETRO_SAVESTATE_CONTEXT_NORMAL;
    }

    break;
  }
  case RETRO_ENVIRONMENT_GET_HW_RENDER_CONTEXT_NEGOTIATION_INTERFACE_SUPPORT:
  {
    retro_hw_render_context_negotiation_interface* typedData = static_cast<retro_hw_render_context_negotiation_interface*>(data);

    // Not implemented
    (void)typedData;
    return false;
  }
  case RETRO_ENVIRONMENT_GET_JIT_CAPABLE:
  {
    bool* typedData = static_cast<bool*>(data);

    if (typedData != nullptr)
    {
      // No reason to not allow JIT
      *typedData = true;
    }

    break;
  }
  case RETRO_ENVIRONMENT_GET_MICROPHONE_INTERFACE:
  {
    retro_microphone_interface* typedData = static_cast<retro_microphone_interface*>(data);

    // Not implemented
    (void)typedData;
    return false;
  }
  case RETRO_ENVIRONMENT_GET_DEVICE_POWER:
  {
    retro_device_power* typedData = static_cast<retro_device_power*>(data);

    // Not implemented
    (void)typedData;
    return false;
  }
  case RETRO_ENVIRONMENT_SET_NETPACKET_INTERFACE:
  {
    const retro_netpacket_callback* typedData = static_cast<const retro_netpacket_callback*>(data);

    // Not implemented
    (void)typedData;
    return false;
  }
  case RETRO_ENVIRONMENT_GET_PLAYLIST_DIRECTORY:
  {
    const char** typedData = static_cast<const char**>(data);

    // Not implemented
    (void)typedData;
    return false;
  }
#if defined(RETRO_ENVIRONMENT_GET_FILE_BROWSER_START_DIRECTORY)
  case RETRO_ENVIRONMENT_GET_FILE_BROWSER_START_DIRECTORY:
  {
    const char** typedData = static_cast<const char**>(data);

    // Not implemented
    (void)typedData;
    return false;
  }
#endif
#if defined(RETRO_ENVIRONMENT_GET_TARGET_SAMPLE_RATE)
  case RETRO_ENVIRONMENT_GET_TARGET_SAMPLE_RATE:
  {
    unsigned int* typedData = static_cast<unsigned int*>(data);

    // Not implemented
    (void)typedData;
    return false;
  }
#endif
#if defined(RETRO_ENVIRONMENT_GET_NETPLAY_CLIENT_INDEX)
  case RETRO_ENVIRONMENT_GET_NETPLAY_CLIENT_INDEX:
  {
    unsigned int* typedData = static_cast<unsigned int*>(data);

    // Not implemented
    (void)typedData;
    return false;
  }
#endif
  default:
    // Documentation says that unrecognized commands should always return false
    return false;
  }

  return true;
}

const CMemoryMap& CLibretroEnvironment::GetMemoryMap()
{
  return m_mmap;
}
