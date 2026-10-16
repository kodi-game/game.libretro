/*
 *  Copyright (C) 2026 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#include "FrontendMicrophone.h"

using namespace LIBRETRO;

#if defined(__linux__) && !defined(__ANDROID__)

#include <kodi/General.h>

#include <dlfcn.h>

#include <algorithm>
#include <array>
#include <cerrno>
#include <chrono>
#include <climits>

namespace
{
  // libasound is loaded at run time, so it isn't needed to build or run the add-on
  struct snd_pcm_t;

  struct Alsa
  {
    int (*pcm_open)(snd_pcm_t**, const char*, int, int);
    int (*pcm_set_params)(snd_pcm_t*, int, int, unsigned int, unsigned int, int, unsigned int);
    int (*pcm_start)(snd_pcm_t*);
    long (*pcm_readi)(snd_pcm_t*, void*, unsigned long);
    long (*pcm_avail)(snd_pcm_t*);
    long (*pcm_forward)(snd_pcm_t*, unsigned long);
    int (*pcm_recover)(snd_pcm_t*, int, int);
    int (*pcm_close)(snd_pcm_t*);
    const char* (*strerror)(int);
  };

  // Values from alsa/pcm.h
  constexpr int SND_PCM_STREAM_CAPTURE = 1;
  constexpr int SND_PCM_NONBLOCK = 1;
  constexpr int SND_PCM_ACCESS_RW_INTERLEAVED = 3;
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
  constexpr int SND_PCM_FORMAT_S16 = 3;
#else
  constexpr int SND_PCM_FORMAT_S16 = 2;
#endif

  // Routed by a sound server, if there is one, to the user's default source
  constexpr const char* DEVICE = "default";

  // RetroArch's default
  constexpr unsigned int DEFAULT_RATE = 48000;

  constexpr unsigned int LATENCY_US = 100000;

  struct Microphone
  {
    bool bOpen = false;
    unsigned int rate = 0;
    snd_pcm_t* pcm = nullptr;
    std::chrono::steady_clock::time_point lastRead;
  };

  // Handles point into this array, so one the core keeps after CloseAll() is
  // rejected rather than dangling
  std::array<Microphone, 4> microphones;

  const Alsa* alsa = nullptr;

  bool LoadAlsa()
  {
    static bool bTried = false;
    if (bTried)
      return alsa != nullptr;
    bTried = true;

    void* lib = dlopen("libasound.so.2", RTLD_NOW | RTLD_LOCAL);
    if (lib == nullptr)
    {
      kodi::Log(ADDON_LOG_ERROR, "Microphones are unavailable: %s", dlerror());
      return false;
    }

    static Alsa functions;
    functions.pcm_open = reinterpret_cast<decltype(functions.pcm_open)>(dlsym(lib, "snd_pcm_open"));
    functions.pcm_set_params = reinterpret_cast<decltype(functions.pcm_set_params)>(dlsym(lib, "snd_pcm_set_params"));
    functions.pcm_start = reinterpret_cast<decltype(functions.pcm_start)>(dlsym(lib, "snd_pcm_start"));
    functions.pcm_readi = reinterpret_cast<decltype(functions.pcm_readi)>(dlsym(lib, "snd_pcm_readi"));
    functions.pcm_avail = reinterpret_cast<decltype(functions.pcm_avail)>(dlsym(lib, "snd_pcm_avail"));
    functions.pcm_forward = reinterpret_cast<decltype(functions.pcm_forward)>(dlsym(lib, "snd_pcm_forward"));
    functions.pcm_recover = reinterpret_cast<decltype(functions.pcm_recover)>(dlsym(lib, "snd_pcm_recover"));
    functions.pcm_close = reinterpret_cast<decltype(functions.pcm_close)>(dlsym(lib, "snd_pcm_close"));
    functions.strerror = reinterpret_cast<decltype(functions.strerror)>(dlsym(lib, "snd_strerror"));

    if (!functions.pcm_open || !functions.pcm_set_params || !functions.pcm_start ||
        !functions.pcm_readi || !functions.pcm_avail || !functions.pcm_forward ||
        !functions.pcm_recover || !functions.pcm_close || !functions.strerror)
    {
      kodi::Log(ADDON_LOG_ERROR, "Microphones are unavailable: libasound.so.2 is missing functions");
      dlclose(lib);
      return false;
    }

    alsa = &functions;
    return true;
  }

  Microphone* GetMicrophone(const retro_microphone_t* handle)
  {
    for (Microphone& microphone : microphones)
    {
      if (handle == reinterpret_cast<const retro_microphone_t*>(&microphone))
        return microphone.bOpen ? &microphone : nullptr;
    }
    return nullptr;
  }

  bool StartCapture(Microphone& microphone)
  {
    snd_pcm_t* pcm = nullptr;
    int err = alsa->pcm_open(&pcm, DEVICE, SND_PCM_STREAM_CAPTURE, SND_PCM_NONBLOCK);
    if (err == 0)
    {
      err = alsa->pcm_set_params(pcm, SND_PCM_FORMAT_S16, SND_PCM_ACCESS_RW_INTERLEAVED, 1,
                                 microphone.rate, 1, LATENCY_US);
      if (err == 0)
        err = alsa->pcm_start(pcm);
      if (err != 0)
        alsa->pcm_close(pcm);
    }

    if (err != 0)
    {
      kodi::Log(ADDON_LOG_ERROR, "Failed to capture from ALSA device \"%s\" at %u Hz: %s", DEVICE,
                microphone.rate, alsa->strerror(err));
      return false;
    }

    kodi::Log(ADDON_LOG_DEBUG, "Capturing from ALSA device \"%s\" at %u Hz", DEVICE,
              microphone.rate);
    microphone.pcm = pcm;
    microphone.lastRead = std::chrono::steady_clock::now();
    return true;
  }

  void StopCapture(Microphone& microphone)
  {
    if (microphone.pcm != nullptr)
    {
      alsa->pcm_close(microphone.pcm);
      microphone.pcm = nullptr;
    }
  }

  retro_microphone_t* RETRO_CALLCONV OpenMic(const retro_microphone_params_t* params)
  {
    if (!LoadAlsa())
      return nullptr;

    for (Microphone& microphone : microphones)
    {
      if (!microphone.bOpen)
      {
        microphone.bOpen = true;
        microphone.rate = (params != nullptr && params->rate != 0) ? params->rate : DEFAULT_RATE;
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
    {
      StopCapture(*microphone);
      *microphone = Microphone();
    }
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

    if (state == (microphone->pcm != nullptr))
      return true;

    if (state)
      return StartCapture(*microphone);

    StopCapture(*microphone);
    return true;
  }

  bool RETRO_CALLCONV GetMicState(const retro_microphone_t* handle)
  {
    const Microphone* microphone = GetMicrophone(handle);
    return microphone != nullptr && microphone->pcm != nullptr;
  }

  int RETRO_CALLCONV ReadMic(retro_microphone_t* handle, int16_t* samples, size_t num_samples)
  {
    Microphone* microphone = GetMicrophone(handle);
    if (microphone == nullptr || microphone->pcm == nullptr || samples == nullptr)
      return -1;

    // A sound server's device keeps the start of a gap longer than its buffer,
    // such as while the game was paused, instead of reporting an overrun
    const auto now = std::chrono::steady_clock::now();
    if (now - microphone->lastRead > std::chrono::microseconds(LATENCY_US))
    {
      const long backlog = alsa->pcm_avail(microphone->pcm);
      if (backlog > 0)
        alsa->pcm_forward(microphone->pcm, backlog);
    }
    microphone->lastRead = now;

    const long frames = alsa->pcm_readi(microphone->pcm, samples,
                                        std::min<size_t>(num_samples, INT_MAX));
    if (frames >= 0)
      return static_cast<int>(frames);

    if (frames == -EAGAIN)
      return 0;

    if (alsa->pcm_recover(microphone->pcm, static_cast<int>(frames), 1) == 0 &&
        alsa->pcm_start(microphone->pcm) == 0)
      return 0;

    return -1;
  }
} // namespace

bool CFrontendMicrophone::GetInterface(retro_microphone_interface& microphoneInterface)
{
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
  {
    StopCapture(microphone);
    microphone = Microphone();
  }
}

#else

bool CFrontendMicrophone::GetInterface(retro_microphone_interface& /* microphoneInterface */)
{
  return false;
}

void CFrontendMicrophone::CloseAll()
{
}

#endif
