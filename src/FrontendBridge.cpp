/*
 *      Copyright (C) 2014 Team XBMC
 *      http://xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include "FrontendBridge.h"
#include "LibretroEnvironment.h"
#include "libXBMC_addon.h"
#include "libXBMC_game.h"

using namespace ADDON;
using namespace LIBRETRO;

void CFrontendBridge::LogFrontend(retro_log_level level, const char *fmt, ...)
{
  if (!CLibretroEnvironment::GetXBMC())
    return;

  addon_log_t xbmcLevel;
  switch (level)
  {
  case RETRO_LOG_DEBUG: xbmcLevel = LOG_DEBUG; break;
  case RETRO_LOG_INFO:  xbmcLevel = LOG_INFO;  break;
  case RETRO_LOG_WARN:  xbmcLevel = LOG_ERROR; break;
  case RETRO_LOG_ERROR: xbmcLevel = LOG_ERROR; break;
  default:              xbmcLevel = LOG_ERROR; break;
  }

  char buffer[16384];
  va_list args;
  va_start(args, fmt);
  vsprintf(buffer, fmt, args);
  va_end(args);

  CLibretroEnvironment::GetXBMC()->Log(xbmcLevel, buffer);
}

void CFrontendBridge::VideoRefresh(const void* data, unsigned width, unsigned height, size_t pitch)
{
  if (!CLibretroEnvironment::GetFrontend())
    return;

  return CLibretroEnvironment::GetFrontend()->VideoRefresh(data, width, height, pitch, CLibretroEnvironment::GetPixelFormat());
}

void CFrontendBridge::AudioSample(int16_t left, int16_t right)
{
  if (!CLibretroEnvironment::GetFrontend())
    return;

  return CLibretroEnvironment::GetFrontend()->AudioSample(left, right);
}

size_t CFrontendBridge::AudioSampleBatch(const int16_t* data, size_t frames)
{
  if (!CLibretroEnvironment::GetFrontend())
    return 0;

  return CLibretroEnvironment::GetFrontend()->AudioSampleBatch(data, frames);
}

void CFrontendBridge::InputPoll(void)
{
  // Not needed
}

int16_t CFrontendBridge::InputState(unsigned port, unsigned device, unsigned index, unsigned id)
{
  if (!CLibretroEnvironment::GetFrontend())
    return 0;

  return CLibretroEnvironment::GetFrontend()->InputState(port, device, index, id);
}

uintptr_t CFrontendBridge::HwGetCurrentFramebuffer(void)
{
  if (!CLibretroEnvironment::GetFrontend())
    return 0;

  return CLibretroEnvironment::GetFrontend()->HwGetCurrentFramebuffer();
}

retro_proc_address_t CFrontendBridge::HwGetProcAddress(const char *sym)
{
  if (!CLibretroEnvironment::GetFrontend())
    return NULL;

  return CLibretroEnvironment::GetFrontend()->HwGetProcAddress(sym);
}

bool CFrontendBridge::RumbleSetState(unsigned port, retro_rumble_effect effect, uint16_t strength)
{
  if (!CLibretroEnvironment::GetFrontend())
    return false;

  return CLibretroEnvironment::GetFrontend()->RumbleSetState(port, (GAME_RUMBLE_EFFECT)effect, strength);
}

bool CFrontendBridge::SensorSetState(unsigned port, retro_sensor_action action, unsigned rate)
{
  if (!CLibretroEnvironment::GetFrontend())
    return false;

  return CLibretroEnvironment::GetFrontend()->SensorSetState(port, (GAME_SENSOR_ACTION)action, rate);
}

float CFrontendBridge::SensorGetInput(unsigned port, unsigned id)
{
  if (!CLibretroEnvironment::GetFrontend())
    return 0.0f;

  return CLibretroEnvironment::GetFrontend()->SensorGetInput(port, id);
}

bool CFrontendBridge::CameraStart(void)
{
  if (!CLibretroEnvironment::GetFrontend())
    return false;

  return CLibretroEnvironment::GetFrontend()->CameraStart();
}

void CFrontendBridge::CameraStop(void)
{
  if (!CLibretroEnvironment::GetFrontend())
    return;

  CLibretroEnvironment::GetFrontend()->CameraStop();
}

retro_time_t CFrontendBridge::PerfGetTimeUsec(void)
{
  if (!CLibretroEnvironment::GetFrontend())
    return 0;

  return CLibretroEnvironment::GetFrontend()->PerfGetTimeUsec();
}

retro_perf_tick_t CFrontendBridge::PerfGetCounter(void)
{
  if (!CLibretroEnvironment::GetFrontend())
    return 0;

  return CLibretroEnvironment::GetFrontend()->PerfGetCounter();
}

uint64_t CFrontendBridge::PerfGetCpuFeatures(void)
{
  if (!CLibretroEnvironment::GetFrontend())
    return 0;

  return CLibretroEnvironment::GetFrontend()->PerfGetCpuFeatures();
}

void CFrontendBridge::PerfLog(void)
{
  if (!CLibretroEnvironment::GetFrontend())
    return;

  CLibretroEnvironment::GetFrontend()->PerfLog();
}

void CFrontendBridge::PerfRegister(retro_perf_counter *counter)
{
  if (!CLibretroEnvironment::GetFrontend())
    return;

  CLibretroEnvironment::GetFrontend()->PerfRegister(reinterpret_cast<game_perf_counter*>(counter));
}

void CFrontendBridge::PerfStart(retro_perf_counter *counter)
{
  if (!CLibretroEnvironment::GetFrontend())
    return;

  CLibretroEnvironment::GetFrontend()->PerfStart(reinterpret_cast<game_perf_counter*>(counter));
}

void CFrontendBridge::PerfStop(retro_perf_counter *counter)
{
  if (!CLibretroEnvironment::GetFrontend())
    return;

  CLibretroEnvironment::GetFrontend()->PerfStop(reinterpret_cast<game_perf_counter*>(counter));
}

bool CFrontendBridge::LocationStart(void)
{
  if (!CLibretroEnvironment::GetFrontend())
    return false;

  return CLibretroEnvironment::GetFrontend()->LocationStart();
}

void CFrontendBridge::LocationStop(void)
{
  if (!CLibretroEnvironment::GetFrontend())
    return;

  CLibretroEnvironment::GetFrontend()->LocationStop();
}

bool CFrontendBridge::LocationGetPosition(double *lat, double *lon, double *horiz_accuracy, double *vert_accuracy)
{
  if (!CLibretroEnvironment::GetFrontend())
    return false;

  return CLibretroEnvironment::GetFrontend()->LocationGetPosition(lat, lon, horiz_accuracy, vert_accuracy);
}

void CFrontendBridge::LocationSetInterval(unsigned interval_ms, unsigned interval_distance)
{
  if (!CLibretroEnvironment::GetFrontend())
    return;

  CLibretroEnvironment::GetFrontend()->LocationSetInterval(interval_ms, interval_distance);
}

void CFrontendBridge::LocationInitialized(void)
{
  if (!CLibretroEnvironment::GetFrontend())
    return;

  CLibretroEnvironment::GetFrontend()->LocationInitialized();
}

void CFrontendBridge::LocationDeinitialized(void)
{
  if (!CLibretroEnvironment::GetFrontend())
    return;

  CLibretroEnvironment::GetFrontend()->LocationDeinitialized();
}
