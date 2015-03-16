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
#include "InputManager.h"
#include "LibretroEnvironment.h"
#include "LibretroTranslator.h"

#include "kodi/libXBMC_addon.h"
#include "kodi/libXBMC_game.h"

using namespace ADDON;
using namespace LIBRETRO;

void CFrontendBridge::LogFrontend(retro_log_level level, const char *fmt, ...)
{
  if (!ENVIRONMENT.GetXBMC())
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

  ENVIRONMENT.GetXBMC()->Log(xbmcLevel, buffer);
}

void CFrontendBridge::VideoRefresh(const void* data, unsigned width, unsigned height, size_t pitch)
{
  if (!ENVIRONMENT.GetFrontend())
    return;

  ENVIRONMENT.GetFrontend()->VideoFrame(ENVIRONMENT.GetRenderFormat(), width, height, static_cast<const uint8_t*>(data));
}

size_t CFrontendBridge::AudioFrames(const int16_t* data, size_t frames)
{
  if (!ENVIRONMENT.GetFrontend())
    return 0;

  return ENVIRONMENT.GetFrontend()->AudioFrames(GAME_AUDIO_FMT_S16NE, frames, reinterpret_cast<const uint8_t*>(data));
}

void CFrontendBridge::InputPoll(void)
{
  // Not needed
}

int16_t CFrontendBridge::InputState(unsigned port, unsigned device, unsigned index, unsigned id)
{
  int16_t inputState = 0;

  device &= RETRO_DEVICE_MASK;

  switch (device)
  {
  case RETRO_DEVICE_JOYPAD:
    return CInputManager::Get().DigitalButtonState(port, id) ? 1 : 0;

  case RETRO_DEVICE_MOUSE:
    switch (id)
    {
      case RETRO_DEVICE_ID_MOUSE_LEFT:
      case RETRO_DEVICE_ID_MOUSE_RIGHT:
      {
        const unsigned int buttonIndex = id - RETRO_DEVICE_ID_MOUSE_LEFT;
        inputState = CInputManager::Get().DigitalButtonState(port, buttonIndex) ? 1 : 0;
        break;
      }
      case RETRO_DEVICE_ID_MOUSE_X:
        inputState = CInputManager::Get().MouseDeltaX(port);
        break;
      case RETRO_DEVICE_ID_MOUSE_Y:
        inputState = CInputManager::Get().MouseDeltaY(port);
        break;
      default:
        break;
    }
    break;

  case RETRO_DEVICE_KEYBOARD:
    // TODO
    break;

  case RETRO_DEVICE_LIGHTGUN:
    // TODO
    break;

  case RETRO_DEVICE_ANALOG:
  {
    float x, y;
    if (CInputManager::Get().AnalogStickState(port, index, x, y))
    {
      if (id == RETRO_DEVICE_ID_ANALOG_X)
      {
        const float normalized = x + 1.0f / 2.0f;
        inputState = (int)(normalized * 0xffff) - 0x8000;
      }
      else if (id == RETRO_DEVICE_ID_ANALOG_Y)
      {
        const float normalized = -y + 1.0f / 2.0f; // y axis is inverted
        inputState = (int)(normalized * 0xffff) - 0x8000;
      }
    }
    break;
  }

  case RETRO_DEVICE_POINTER:
    // TODO
    break;

  default:
    break;
  }

  return inputState;
}

uintptr_t CFrontendBridge::HwGetCurrentFramebuffer(void)
{
  if (!ENVIRONMENT.GetFrontend())
    return 0;

  return ENVIRONMENT.GetFrontend()->HwGetCurrentFramebuffer();
}

retro_proc_address_t CFrontendBridge::HwGetProcAddress(const char *sym)
{
  if (!ENVIRONMENT.GetFrontend())
    return NULL;

  return ENVIRONMENT.GetFrontend()->HwGetProcAddress(sym);
}

bool CFrontendBridge::RumbleSetState(unsigned port, retro_rumble_effect effect, uint16_t strength)
{
  if (!ENVIRONMENT.GetFrontend())
    return false;

  return ENVIRONMENT.GetFrontend()->RumbleSetState(port, LibretroTranslator::GetRumbleEffect(effect), strength);
}

bool CFrontendBridge::SensorSetState(unsigned port, retro_sensor_action action, unsigned rate)
{
  const bool bEnabled = (action == RETRO_SENSOR_ACCELEROMETER_ENABLE);

  CInputManager::Get().EnableSource(bEnabled, port, GAME_INPUT_EVENT_ACCELEROMETER, 0);

  return true;
}

float CFrontendBridge::SensorGetInput(unsigned port, unsigned id)
{
  float axisState = 0.0f;

  float x, y, z;
  if (CInputManager::Get().AccelerometerState(port, 0, x, y, z))
  {
    switch (id)
    {
    case RETRO_SENSOR_ACCELEROMETER_X:
      axisState = x;
      break;
    case RETRO_SENSOR_ACCELEROMETER_Y:
      axisState = y;
      break;
    case RETRO_SENSOR_ACCELEROMETER_Z:
      axisState = z;
      break;
    default:
      break;
    }
  }

  return axisState;
}

bool CFrontendBridge::CameraStart(void)
{
  if (!ENVIRONMENT.GetFrontend())
    return false;

  return ENVIRONMENT.GetFrontend()->CameraStart();
}

void CFrontendBridge::CameraStop(void)
{
  if (!ENVIRONMENT.GetFrontend())
    return;

  ENVIRONMENT.GetFrontend()->CameraStop();
}

retro_time_t CFrontendBridge::PerfGetTimeUsec(void)
{
  if (!ENVIRONMENT.GetFrontend())
    return 0;

  return ENVIRONMENT.GetFrontend()->PerfGetTimeUsec();
}

retro_perf_tick_t CFrontendBridge::PerfGetCounter(void)
{
  if (!ENVIRONMENT.GetFrontend())
    return 0;

  return ENVIRONMENT.GetFrontend()->PerfGetCounter();
}

uint64_t CFrontendBridge::PerfGetCpuFeatures(void)
{
  if (!ENVIRONMENT.GetFrontend())
    return 0;

  return ENVIRONMENT.GetFrontend()->PerfGetCpuFeatures();
}

void CFrontendBridge::PerfLog(void)
{
  if (!ENVIRONMENT.GetFrontend())
    return;

  ENVIRONMENT.GetFrontend()->PerfLog();
}

void CFrontendBridge::PerfRegister(retro_perf_counter *counter)
{
  if (!ENVIRONMENT.GetFrontend())
    return;

  ENVIRONMENT.GetFrontend()->PerfRegister(reinterpret_cast<game_perf_counter*>(counter));
}

void CFrontendBridge::PerfStart(retro_perf_counter *counter)
{
  if (!ENVIRONMENT.GetFrontend())
    return;

  ENVIRONMENT.GetFrontend()->PerfStart(reinterpret_cast<game_perf_counter*>(counter));
}

void CFrontendBridge::PerfStop(retro_perf_counter *counter)
{
  if (!ENVIRONMENT.GetFrontend())
    return;

  ENVIRONMENT.GetFrontend()->PerfStop(reinterpret_cast<game_perf_counter*>(counter));
}

bool CFrontendBridge::LocationStart(void)
{
  if (!ENVIRONMENT.GetFrontend())
    return false;

  return ENVIRONMENT.GetFrontend()->LocationStart();
}

void CFrontendBridge::LocationStop(void)
{
  if (!ENVIRONMENT.GetFrontend())
    return;

  ENVIRONMENT.GetFrontend()->LocationStop();
}

bool CFrontendBridge::LocationGetPosition(double *lat, double *lon, double *horiz_accuracy, double *vert_accuracy)
{
  if (!ENVIRONMENT.GetFrontend())
    return false;

  return ENVIRONMENT.GetFrontend()->LocationGetPosition(lat, lon, horiz_accuracy, vert_accuracy);
}

void CFrontendBridge::LocationSetInterval(unsigned interval_ms, unsigned interval_distance)
{
  if (!ENVIRONMENT.GetFrontend())
    return;

  ENVIRONMENT.GetFrontend()->LocationSetInterval(interval_ms, interval_distance);
}

void CFrontendBridge::LocationInitialized(void)
{
  if (!ENVIRONMENT.GetFrontend())
    return;

  ENVIRONMENT.GetFrontend()->LocationInitialized();
}

void CFrontendBridge::LocationDeinitialized(void)
{
  if (!ENVIRONMENT.GetFrontend())
    return;

  ENVIRONMENT.GetFrontend()->LocationDeinitialized();
}
