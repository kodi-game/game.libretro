/*
 *      Copyright (C) 2014-2016 Team Kodi
 *      http://kodi.tv
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
 *  along with this Program; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include "FrontendBridge.h"
#include "LibretroEnvironment.h"
#include "LibretroTranslator.h"
#include "input/ButtonMapper.h"
#include "input/InputManager.h"

#include "libXBMC_addon.h"
#include "libKODI_game.h"

#include <assert.h>

using namespace ADDON;
using namespace LIBRETRO;

#define S16NE_FRAMESIZE  4 // int16 L + int16 R

#define MAX_RUMBLE_STRENGTH  0xffff

#ifndef CONSTRAIN
  // Credit: https://stackoverflow.com/questions/8941262/constrain-function-port-from-arduino
  #define CONSTRAIN(amt, low, high)  ((amt) < (low) ? (low) : ((amt) > (high) ? (high) : (amt)))
#endif

void CFrontendBridge::LogFrontend(retro_log_level level, const char *fmt, ...)
{
  if (!CLibretroEnvironment::Get().GetXBMC())
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

  CLibretroEnvironment::Get().GetXBMC()->Log(xbmcLevel, buffer);
}

void CFrontendBridge::VideoRefresh(const void* data, unsigned int width, unsigned int height, size_t pitch)
{
  if (data == RETRO_HW_FRAME_BUFFER_VALID)
  {
    if (CLibretroEnvironment::Get().GetFrontend())
      CLibretroEnvironment::Get().GetFrontend()->RenderFrame();
  }
  else if (data == nullptr)
  {
    // Libretro is sending a frame dupe command
  }
  else
  {
    CLibretroEnvironment::Get().Video().AddFrame(static_cast<const uint8_t*>(data),
                                                 pitch * height,
                                                 width,
                                                 height,
                                                 CLibretroEnvironment::Get().GetVideoFormat(),
                                                 CLibretroEnvironment::Get().GetVideoRotation());
  }
}

void CFrontendBridge::AudioFrame(int16_t left, int16_t right)
{
  CLibretroEnvironment::Get().Audio().AddFrame_S16NE(left, right);
}

size_t CFrontendBridge::AudioFrames(const int16_t* data, size_t frames)
{
  CLibretroEnvironment::Get().Audio().AddFrames_S16NE(reinterpret_cast<const uint8_t*>(data),
                                                      frames * S16NE_FRAMESIZE);

  return frames;
}

void CFrontendBridge::InputPoll(void)
{
  // Not needed
}

int16_t CFrontendBridge::InputState(unsigned int port, unsigned int device, unsigned int index, unsigned int id)
{
  int16_t inputState = 0;

  // According to libretro.h, device should already be masked, but just in case
  device &= RETRO_DEVICE_MASK;

  switch (device)
  {
  case RETRO_DEVICE_JOYPAD:
  //case RETRO_DEVICE_KEYBOARD: // TODO
    inputState = CInputManager::Get().ButtonState(device, port, id) ? 1 : 0;
    break;

  case RETRO_DEVICE_MOUSE:
  case RETRO_DEVICE_LIGHTGUN:
    static_assert(RETRO_DEVICE_ID_MOUSE_X == RETRO_DEVICE_ID_LIGHTGUN_X, "RETRO_DEVICE_ID_MOUSE_X != RETRO_DEVICE_ID_LIGHTGUN_X");
    static_assert(RETRO_DEVICE_ID_MOUSE_Y == RETRO_DEVICE_ID_LIGHTGUN_Y, "RETRO_DEVICE_ID_MOUSE_Y != RETRO_DEVICE_ID_LIGHTGUN_Y");

    switch (id)
    {
      case RETRO_DEVICE_ID_MOUSE_X:
        inputState = CInputManager::Get().DeltaX(device, port);
        break;
      case RETRO_DEVICE_ID_MOUSE_Y:
        inputState = CInputManager::Get().DeltaY(device, port);
        break;
      default:
      {
        inputState = CInputManager::Get().ButtonState(device, port, id) ? 1 : 0;
        break;
      }
    }
    break;

  case RETRO_DEVICE_ANALOG:
  {
    float x, y;
    if (CInputManager::Get().AnalogStickState(port, index, x, y))
    {
      if (id == RETRO_DEVICE_ID_ANALOG_X)
      {
        const float normalized = (x + 1.0f) / 2.0f;
        inputState = (int)(normalized * 0xffff) - 0x8000;
      }
      else if (id == RETRO_DEVICE_ID_ANALOG_Y)
      {
        const float normalized = (-y + 1.0f) / 2.0f; // y axis is inverted
        inputState = (int)(normalized * 0xffff) - 0x8000;
      }
    }
    break;
  }

  case RETRO_DEVICE_POINTER:
  {
    float x, y;
    if (CInputManager::Get().AbsolutePointerState(port, index, x, y))
    {
      if (id == RETRO_DEVICE_ID_POINTER_X)
      {
        inputState = (int)(x * 0x7fff);
      }
      else if (id == RETRO_DEVICE_ID_POINTER_Y)
      {
        inputState = (int)(y * 0x7fff);
      }
      else if (id == RETRO_DEVICE_ID_POINTER_PRESSED)
      {
        inputState = 1;
      }
    }
    break;
  }

  default:
    break;
  }

  return inputState;
}

uintptr_t CFrontendBridge::HwGetCurrentFramebuffer(void)
{
  if (!CLibretroEnvironment::Get().GetFrontend())
    return 0;

  return CLibretroEnvironment::Get().GetFrontend()->HwGetCurrentFramebuffer();
}

retro_proc_address_t CFrontendBridge::HwGetProcAddress(const char *sym)
{
  if (!CLibretroEnvironment::Get().GetFrontend())
    return nullptr;

  return CLibretroEnvironment::Get().GetFrontend()->HwGetProcAddress(sym);
}

bool CFrontendBridge::RumbleSetState(unsigned int port, retro_rumble_effect effect, uint16_t strength)
{
  if (!CLibretroEnvironment::Get().GetFrontend())
    return false;

  std::string controllerId  = CInputManager::Get().ControllerID(port);
  std::string libretroMotor = LibretroTranslator::GetMotorName(effect);
  std::string featureName   = CButtonMapper::Get().GetControllerFeature(controllerId, libretroMotor);
  float       magnitude     = static_cast<float>(strength) / MAX_RUMBLE_STRENGTH;

  game_input_event eventStruct;
  eventStruct.type            = GAME_INPUT_EVENT_MOTOR;
  eventStruct.port            = port;
  eventStruct.controller_id   = controllerId.c_str();
  eventStruct.feature_name    = featureName.c_str();
  eventStruct.motor.magnitude = CONSTRAIN(magnitude, 0.0f, 1.0f);

  CLibretroEnvironment::Get().GetFrontend()->InputEvent(eventStruct);
  return true;
}

bool CFrontendBridge::SensorSetState(unsigned port, retro_sensor_action action, unsigned rate)
{
  const bool bEnabled = (action == RETRO_SENSOR_ACCELEROMETER_ENABLE);

  CInputManager::Get().EnableAnalogSensors(port, bEnabled);

  return true;
}

float CFrontendBridge::SensorGetInput(unsigned port, unsigned id)
{
  float axisState = 0.0f;

  float x, y, z;
  if (CInputManager::Get().AccelerometerState(port, x, y, z))
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

bool CFrontendBridge::StartCamera(void)
{
  return false; // Not implemented
}

void CFrontendBridge::StopCamera(void)
{
  // Not implemented
}

retro_time_t CFrontendBridge::PerfGetTimeUsec(void)
{
  return 0; // Not implemented
}

retro_perf_tick_t CFrontendBridge::PerfGetCounter(void)
{
  return 0; // Not implemented
}

uint64_t CFrontendBridge::PerfGetCpuFeatures(void)
{
  return 0; // Not implemented
}

void CFrontendBridge::PerfLog(void)
{
  // Not implemented
}

void CFrontendBridge::PerfRegister(retro_perf_counter *counter)
{
  // Not implemented
}

void CFrontendBridge::PerfStart(retro_perf_counter *counter)
{
  // Not implemented
}

void CFrontendBridge::PerfStop(retro_perf_counter *counter)
{
  // Not implemented
}

bool CFrontendBridge::StartLocation(void)
{
  return false; // Not implemented
}

void CFrontendBridge::StopLocation(void)
{
  // Not implemented
}

bool CFrontendBridge::GetLocation(double *lat, double *lon, double *horiz_accuracy, double *vert_accuracy)
{
  return false; // Not implemented
}

void CFrontendBridge::SetLocationInterval(unsigned interval_ms, unsigned interval_distance)
{
  // Not implemented
}

void CFrontendBridge::LocationInitialized(void)
{
  // Not implemented
}

void CFrontendBridge::LocationDeinitialized(void)
{
  // Not implemented
}
