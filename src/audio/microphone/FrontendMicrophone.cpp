/*
 *  Copyright (C) 2026 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#include "FrontendMicrophone.h"

#include "IMicrophone.h"

#include <kodi/General.h>

#include <array>
#include <memory>
#include <utility>

using namespace LIBRETRO;

namespace
{
  // RetroArch's default
  constexpr unsigned int DEFAULT_RATE = 48000;

  // The libretro API lets a core open several microphones, but RetroArch opens
  // only one at a time, so no core counts on more. Four leaves room for one per
  // player.
  constexpr size_t MAX_MICROPHONES = 4;

  struct Microphone
  {
    bool bOpen = false;
    bool bCapturing = false;
    unsigned int rate = 0;
    std::unique_ptr<IMicrophone> device;
  };

  // Handles point into this array, so one the core keeps after CloseAll() is
  // rejected rather than dangling
  std::array<Microphone, MAX_MICROPHONES> microphones;

  Microphone* GetMicrophone(const retro_microphone_t* handle)
  {
    for (Microphone& microphone : microphones)
    {
      if (handle == reinterpret_cast<const retro_microphone_t*>(&microphone))
        return microphone.bOpen ? &microphone : nullptr;
    }
    return nullptr;
  }

  retro_microphone_t* RETRO_CALLCONV OpenMic(const retro_microphone_params_t* params)
  {
    for (Microphone& microphone : microphones)
    {
      if (!microphone.bOpen)
      {
        std::unique_ptr<IMicrophone> device = CreateMicrophone();
        if (!device)
          return nullptr;

        microphone.bOpen = true;
        microphone.rate = (params != nullptr && params->rate != 0) ? params->rate : DEFAULT_RATE;
        microphone.device = std::move(device);
        return reinterpret_cast<retro_microphone_t*>(&microphone);
      }
    }

    kodi::Log(ADDON_LOG_ERROR, "Can't open more than %zu microphones", microphones.size());
    return nullptr;
  }

  void RETRO_CALLCONV CloseMic(retro_microphone_t* handle)
  {
    Microphone* microphone = GetMicrophone(handle);
    if (microphone != nullptr)
      *microphone = Microphone();
  }

  bool RETRO_CALLCONV GetParams(const retro_microphone_t* handle, retro_microphone_params_t* params)
  {
    const Microphone* microphone = GetMicrophone(handle);
    if (microphone == nullptr || params == nullptr)
      return false;

    params->rate = microphone->rate;
    return true;
  }

  bool RETRO_CALLCONV SetMicState(retro_microphone_t* handle, bool state)
  {
    Microphone* microphone = GetMicrophone(handle);
    if (microphone == nullptr)
      return false;

    if (state == microphone->bCapturing)
      return true;

    if (state)
    {
      if (!microphone->device->Start(microphone->rate))
        return false;
    }
    else
    {
      microphone->device->Stop();
    }

    microphone->bCapturing = state;
    return true;
  }

  bool RETRO_CALLCONV GetMicState(const retro_microphone_t* handle)
  {
    const Microphone* microphone = GetMicrophone(handle);
    return microphone != nullptr && microphone->bCapturing;
  }

  int RETRO_CALLCONV ReadMic(retro_microphone_t* handle, int16_t* samples, size_t num_samples)
  {
    Microphone* microphone = GetMicrophone(handle);
    if (microphone == nullptr || !microphone->bCapturing || samples == nullptr)
      return -1;

    return microphone->device->Read(samples, num_samples);
  }
} // namespace

bool CFrontendMicrophone::GetInterface(retro_microphone_interface& microphoneInterface)
{
  if (!CanCapture())
    return false;

  microphoneInterface.interface_version = RETRO_MICROPHONE_INTERFACE_VERSION;
  microphoneInterface.open_mic = OpenMic;
  microphoneInterface.close_mic = CloseMic;
  microphoneInterface.get_params = GetParams;
  microphoneInterface.set_mic_state = SetMicState;
  microphoneInterface.get_mic_state = GetMicState;
  microphoneInterface.read_mic = ReadMic;
  return true;
}

void CFrontendMicrophone::CloseAll()
{
  for (Microphone& microphone : microphones)
    microphone = Microphone();
}
