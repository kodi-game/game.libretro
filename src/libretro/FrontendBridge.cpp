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
#include "client.h"

#include <algorithm>
#include <assert.h>
#include <kodi/General.h>

using namespace LIBRETRO;

#define S16NE_FRAMESIZE  4 // int16 L + int16 R

#define MAX_RUMBLE_STRENGTH  0xffff

#ifndef CONSTRAIN
  // Credit: https://stackoverflow.com/questions/8941262/constrain-function-port-from-arduino
  #define CONSTRAIN(amt, low, high)  ((amt) < (low) ? (low) : ((amt) > (high) ? (high) : (amt)))
#endif

void CFrontendBridge::LogFrontend(retro_log_level level, const char *fmt, ...)
{
  AddonLog xbmcLevel;
  switch (level)
  {
  case RETRO_LOG_DEBUG: xbmcLevel = ADDON_LOG_DEBUG; break;
  case RETRO_LOG_INFO:  xbmcLevel = ADDON_LOG_INFO;  break;
  case RETRO_LOG_WARN:  xbmcLevel = ADDON_LOG_ERROR; break;
  case RETRO_LOG_ERROR: xbmcLevel = ADDON_LOG_ERROR; break;
  default:              xbmcLevel = ADDON_LOG_ERROR; break;
  }

  char buffer[16384];
  va_list args;
  va_start(args, fmt);
  vsprintf(buffer, fmt, args);
  va_end(args);

  kodi::Log(xbmcLevel, buffer);
}

void CFrontendBridge::VideoRefresh(const void* data, unsigned int width, unsigned int height, size_t pitch)
{
  if (data == RETRO_HW_FRAME_BUFFER_VALID)
  {
    CLibretroEnvironment::Get().Video().RenderHwFrame();
  }
  else if (data == nullptr)
  {
    // Libretro is sending a frame dupe command
    CLibretroEnvironment::Get().Video().DupeFrame();
  }
  else
  {
    CLibretroEnvironment::Get().Video().AddFrame(static_cast<const uint8_t*>(data),
                                                 static_cast<unsigned int>(pitch * height),
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
                                                      static_cast<unsigned int>(frames * S16NE_FRAMESIZE));

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
  case RETRO_DEVICE_KEYBOARD:
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
    float value = 0.0f; // Axis value between -1 and 1

    if (index == RETRO_DEVICE_INDEX_ANALOG_BUTTON)
    {
      value = CInputManager::Get().AnalogButtonState(port, id);
    }
    else
    {
      float x, y;
      if (CInputManager::Get().AnalogStickState(port, index, x, y))
      {
        if (id == RETRO_DEVICE_ID_ANALOG_X)
        {
          value = x;
        }
        else if (id == RETRO_DEVICE_ID_ANALOG_Y)
        {
          value = -y; // y axis is inverted
        }
      }
    }

    const float normalized = (value + 1.0f) / 2.0f;
    const int clamped = std::max(0, std::min(0xffff, static_cast<int>(normalized * 0xffff)));
    inputState = clamped - 0x8000;
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
  if (!CLibretroEnvironment::Get().GetAddon())
    return 0;

  return CLibretroEnvironment::Get().Video().GetHwFramebuffer();
}

retro_proc_address_t CFrontendBridge::HwGetProcAddress(const char *sym)
{
  if (!CLibretroEnvironment::Get().GetAddon())
    return nullptr;

  return CLibretroEnvironment::Get().GetAddon()->HwGetProcAddress(sym);
}

bool CFrontendBridge::RumbleSetState(unsigned int port, retro_rumble_effect effect, uint16_t strength)
{
  if (!CLibretroEnvironment::Get().GetAddon())
    return false;

  std::string controllerId  = CInputManager::Get().ControllerID(port);
  std::string address       = CInputManager::Get().GetAddress(port);
  std::string libretroMotor = LibretroTranslator::GetMotorName(effect);
  std::string featureName   = CButtonMapper::Get().GetControllerFeature(controllerId, libretroMotor);
  float       magnitude     = static_cast<float>(strength) / MAX_RUMBLE_STRENGTH;

  if (controllerId.empty() || address.empty() || featureName.empty())
    return false;

  game_input_event eventStruct;
  eventStruct.type            = GAME_INPUT_EVENT_MOTOR;
  eventStruct.controller_id   = controllerId.c_str();
  eventStruct.port_address    = address.c_str();
  eventStruct.port_type       = GAME_PORT_CONTROLLER;
  eventStruct.feature_name    = featureName.c_str();
  eventStruct.motor.magnitude = CONSTRAIN(magnitude, 0.0f, 1.0f);

  CLibretroEnvironment::Get().GetAddon()->KodiInputEvent(eventStruct);
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
