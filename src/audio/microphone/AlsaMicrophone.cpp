/*
 *  Copyright (C) 2026 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#include "IMicrophone.h"

#include <kodi/General.h>

#include <dlfcn.h>

#include <algorithm>
#include <cerrno>
#include <chrono>
#include <climits>

using namespace LIBRETRO;

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
  constexpr int SND_PCM_FORMAT_S16 = __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__ ? 3 : 2;

  // Routed by a sound server, if there is one, to the user's default source
  constexpr const char* DEVICE = "default";

  constexpr unsigned int LATENCY_US = 100000;

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

  /*!
   * \brief Captures from ALSA's default device
   */
  class CAlsaMicrophone : public IMicrophone
  {
  public:
    ~CAlsaMicrophone() override { Stop(); }

    bool Start(unsigned int rate) override
    {
      snd_pcm_t* pcm = nullptr;
      int err = alsa->pcm_open(&pcm, DEVICE, SND_PCM_STREAM_CAPTURE, SND_PCM_NONBLOCK);
      if (err == 0)
      {
        err = alsa->pcm_set_params(pcm, SND_PCM_FORMAT_S16, SND_PCM_ACCESS_RW_INTERLEAVED, 1, rate,
                                   1, LATENCY_US);
        if (err == 0)
          err = alsa->pcm_start(pcm);
        if (err != 0)
          alsa->pcm_close(pcm);
      }

      if (err != 0)
      {
        kodi::Log(ADDON_LOG_ERROR, "Failed to capture from ALSA device \"%s\" at %u Hz: %s", DEVICE,
                  rate, alsa->strerror(err));
        return false;
      }

      kodi::Log(ADDON_LOG_DEBUG, "Capturing from ALSA device \"%s\" at %u Hz", DEVICE, rate);
      m_pcm = pcm;
      m_lastRead = std::chrono::steady_clock::now();
      return true;
    }

    void Stop() override
    {
      if (m_pcm != nullptr)
      {
        alsa->pcm_close(m_pcm);
        m_pcm = nullptr;
      }
    }

    int Read(int16_t* samples, size_t count) override
    {
      if (m_pcm == nullptr)
        return -1;

      // A sound server's device keeps the start of a gap longer than its buffer,
      // such as while the game was paused, instead of reporting an overrun
      const auto now = std::chrono::steady_clock::now();
      if (now - m_lastRead > std::chrono::microseconds(LATENCY_US))
      {
        const long backlog = alsa->pcm_avail(m_pcm);
        if (backlog > 0)
          alsa->pcm_forward(m_pcm, backlog);
      }
      m_lastRead = now;

      const long frames = alsa->pcm_readi(m_pcm, samples, std::min<size_t>(count, INT_MAX));
      if (frames >= 0)
        return static_cast<int>(frames);

      if (frames == -EAGAIN)
        return 0;

      if (alsa->pcm_recover(m_pcm, static_cast<int>(frames), 1) == 0 && alsa->pcm_start(m_pcm) == 0)
        return 0;

      return -1;
    }

  private:
    snd_pcm_t* m_pcm = nullptr;
    std::chrono::steady_clock::time_point m_lastRead;
  };
} // namespace

std::unique_ptr<IMicrophone> LIBRETRO::CreateMicrophone()
{
  if (!LoadAlsa())
    return {};

  return std::make_unique<CAlsaMicrophone>();
}

bool LIBRETRO::CanCapture()
{
  if (!LoadAlsa())
    return false;

  // The default device can be one that only plays, such as HDMI
  snd_pcm_t* pcm = nullptr;
  const int err = alsa->pcm_open(&pcm, DEVICE, SND_PCM_STREAM_CAPTURE, SND_PCM_NONBLOCK);
  if (err != 0)
  {
    kodi::Log(ADDON_LOG_INFO, "Microphones are unavailable: ALSA device \"%s\" can't capture: %s",
              DEVICE, alsa->strerror(err));
    return false;
  }

  alsa->pcm_close(pcm);
  return true;
}
