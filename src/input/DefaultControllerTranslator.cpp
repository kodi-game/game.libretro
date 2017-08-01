/*
 *      Copyright (C) 2017 Team Kodi
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

#include "DefaultControllerTranslator.h"
#include "DefaultControllerDefines.h"
#include "libretro/libretro.h"

using namespace LIBRETRO;

int CDefaultControllerTranslator::GetLibretroIndex(const std::string &strFeatureName)
{
  if (strFeatureName == DEFAULT_CONTROLLER_FEATURE_A)             return RETRO_DEVICE_ID_JOYPAD_A;
  if (strFeatureName == DEFAULT_CONTROLLER_FEATURE_B)             return RETRO_DEVICE_ID_JOYPAD_B;
  if (strFeatureName == DEFAULT_CONTROLLER_FEATURE_X)             return RETRO_DEVICE_ID_JOYPAD_X;
  if (strFeatureName == DEFAULT_CONTROLLER_FEATURE_Y)             return RETRO_DEVICE_ID_JOYPAD_Y;
  if (strFeatureName == DEFAULT_CONTROLLER_FEATURE_START)         return RETRO_DEVICE_ID_JOYPAD_START;
  if (strFeatureName == DEFAULT_CONTROLLER_FEATURE_BACK)          return RETRO_DEVICE_ID_JOYPAD_SELECT;
  if (strFeatureName == DEFAULT_CONTROLLER_FEATURE_LEFT_BUMPER)   return RETRO_DEVICE_ID_JOYPAD_L;
  if (strFeatureName == DEFAULT_CONTROLLER_FEATURE_RIGHT_BUMPER)  return RETRO_DEVICE_ID_JOYPAD_R;
  if (strFeatureName == DEFAULT_CONTROLLER_FEATURE_LEFT_THUMB)    return RETRO_DEVICE_ID_JOYPAD_L3;
  if (strFeatureName == DEFAULT_CONTROLLER_FEATURE_RIGHT_THUMB)   return RETRO_DEVICE_ID_JOYPAD_R3;
  if (strFeatureName == DEFAULT_CONTROLLER_FEATURE_UP)            return RETRO_DEVICE_ID_JOYPAD_UP;
  if (strFeatureName == DEFAULT_CONTROLLER_FEATURE_DOWN)          return RETRO_DEVICE_ID_JOYPAD_DOWN;
  if (strFeatureName == DEFAULT_CONTROLLER_FEATURE_RIGHT)         return RETRO_DEVICE_ID_JOYPAD_RIGHT;
  if (strFeatureName == DEFAULT_CONTROLLER_FEATURE_LEFT)          return RETRO_DEVICE_ID_JOYPAD_LEFT;
  if (strFeatureName == DEFAULT_CONTROLLER_FEATURE_LEFT_TRIGGER)  return RETRO_DEVICE_ID_JOYPAD_L2;
  if (strFeatureName == DEFAULT_CONTROLLER_FEATURE_RIGHT_TRIGGER) return RETRO_DEVICE_ID_JOYPAD_R2;
  if (strFeatureName == DEFAULT_CONTROLLER_FEATURE_LEFT_STICK)    return RETRO_DEVICE_INDEX_ANALOG_LEFT;
  if (strFeatureName == DEFAULT_CONTROLLER_FEATURE_RIGHT_STICK)   return RETRO_DEVICE_INDEX_ANALOG_RIGHT;
  if (strFeatureName == DEFAULT_CONTROLLER_FEATURE_LEFT_MOTOR)    return RETRO_RUMBLE_STRONG;
  if (strFeatureName == DEFAULT_CONTROLLER_FEATURE_RIGHT_MOTOR)   return RETRO_RUMBLE_WEAK;

  return -1;
}

std::string CDefaultControllerTranslator::GetControllerFeature(const std::string &strLibretroFeature)
{
  if (strLibretroFeature == "RETRO_DEVICE_ID_JOYPAD_A")        return DEFAULT_CONTROLLER_FEATURE_A;
  if (strLibretroFeature == "RETRO_DEVICE_ID_JOYPAD_B")        return DEFAULT_CONTROLLER_FEATURE_B;
  if (strLibretroFeature == "RETRO_DEVICE_ID_JOYPAD_X")        return DEFAULT_CONTROLLER_FEATURE_X;
  if (strLibretroFeature == "RETRO_DEVICE_ID_JOYPAD_Y")        return DEFAULT_CONTROLLER_FEATURE_Y;
  if (strLibretroFeature == "RETRO_DEVICE_ID_JOYPAD_START")    return DEFAULT_CONTROLLER_FEATURE_START;
  if (strLibretroFeature == "RETRO_DEVICE_ID_JOYPAD_SELECT")   return DEFAULT_CONTROLLER_FEATURE_BACK;
  if (strLibretroFeature == "RETRO_DEVICE_ID_JOYPAD_UP")       return DEFAULT_CONTROLLER_FEATURE_UP;
  if (strLibretroFeature == "RETRO_DEVICE_ID_JOYPAD_DOWN")     return DEFAULT_CONTROLLER_FEATURE_DOWN;
  if (strLibretroFeature == "RETRO_DEVICE_ID_JOYPAD_RIGHT")    return DEFAULT_CONTROLLER_FEATURE_RIGHT;
  if (strLibretroFeature == "RETRO_DEVICE_ID_JOYPAD_LEFT")     return DEFAULT_CONTROLLER_FEATURE_LEFT;
  if (strLibretroFeature == "RETRO_DEVICE_ID_JOYPAD_L")        return DEFAULT_CONTROLLER_FEATURE_LEFT_BUMPER;
  if (strLibretroFeature == "RETRO_DEVICE_ID_JOYPAD_R")        return DEFAULT_CONTROLLER_FEATURE_RIGHT_BUMPER;
  if (strLibretroFeature == "RETRO_DEVICE_ID_JOYPAD_L2")       return DEFAULT_CONTROLLER_FEATURE_LEFT_TRIGGER;
  if (strLibretroFeature == "RETRO_DEVICE_ID_JOYPAD_R2")       return DEFAULT_CONTROLLER_FEATURE_RIGHT_TRIGGER;
  if (strLibretroFeature == "RETRO_DEVICE_ID_JOYPAD_L3")       return DEFAULT_CONTROLLER_FEATURE_LEFT_THUMB;
  if (strLibretroFeature == "RETRO_DEVICE_ID_JOYPAD_R3")       return DEFAULT_CONTROLLER_FEATURE_RIGHT_THUMB;
  if (strLibretroFeature == "RETRO_DEVICE_INDEX_ANALOG_LEFT")  return DEFAULT_CONTROLLER_FEATURE_LEFT_STICK;
  if (strLibretroFeature == "RETRO_DEVICE_INDEX_ANALOG_RIGHT") return DEFAULT_CONTROLLER_FEATURE_RIGHT_STICK;
  if (strLibretroFeature == "RETRO_RUMBLE_STRONG")             return DEFAULT_CONTROLLER_FEATURE_LEFT_MOTOR;
  if (strLibretroFeature == "RETRO_RUMBLE_WEAK")               return DEFAULT_CONTROLLER_FEATURE_RIGHT_MOTOR;

  return "";
}
