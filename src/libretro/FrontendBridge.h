/*
 *  Copyright (C) 2014-2021 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#pragma once

#include "libretro.h"

namespace LIBRETRO
{
  /*!
   * Provide a bridge for frontend callbacks given to the client. The bridging
   * function is a simple wrapper that invokes the callback in the frontend.
   * These functions are given to the libretro client in EnvironmentCallback().
   */
  class CFrontendBridge
  {
  public:
    // Forward to Kodi add-on API
    static void LogFrontend(retro_log_level level, const char *fmt, ...);

    // Forward to Kodi game API
    static void VideoRefresh(const void* data, unsigned int width, unsigned int height, size_t pitch);
    static void AudioFrame(int16_t left, int16_t right);
    static size_t AudioFrames(const int16_t* data, size_t frames);
    static void InputPoll(void);
    static int16_t InputState(unsigned int port, unsigned int device, unsigned int index, unsigned int id);
    static uintptr_t HwGetCurrentFramebuffer(void);
    static retro_proc_address_t HwGetProcAddress(const char *sym);
    static bool RumbleSetState(unsigned port, retro_rumble_effect effect, uint16_t strength);
    static bool SensorSetState(unsigned port, retro_sensor_action action, unsigned rate);
    static float SensorGetInput(unsigned port, unsigned id);
    static bool StartCamera(void);
    static void StopCamera(void);
    static retro_time_t PerfGetTimeUsec(void);
    static retro_perf_tick_t PerfGetCounter(void);
    static uint64_t PerfGetCpuFeatures(void);
    static void PerfLog(void);
    static void PerfRegister(retro_perf_counter *counter);
    static void PerfStart(retro_perf_counter *counter);
    static void PerfStop(retro_perf_counter *counter);
    static bool StartLocation(void);
    static void StopLocation(void);
    static bool GetLocation(double *lat, double *lon, double *horiz_accuracy, double *vert_accuracy);
    static void SetLocationInterval(unsigned interval_ms, unsigned interval_distance);
    static void LocationInitialized(void);
    static void LocationDeinitialized(void);
  };
} // namespace LIBRETRO
