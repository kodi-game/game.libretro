/*
 *      Copyright (C) 2015 Team XBMC
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

#include "LibretroTranslator.h"

using namespace LIBRETRO;

namespace LIBRETRO
{
  inline retro_mod operator|(retro_mod lhs, retro_mod rhs)
  {
    return static_cast<retro_mod>(static_cast<int>(lhs) | static_cast<int>(rhs));
  }
}

GAME_HW_CONTEXT_TYPE LibretroTranslator::GetHWContextType(retro_hw_context_type type)
{
  switch (type)
  {
    case RETRO_HW_CONTEXT_OPENGL:      return GAME_HW_CONTEXT_OPENGL;
    case RETRO_HW_CONTEXT_OPENGLES2:   return GAME_HW_CONTEXT_OPENGLES2;
    case RETRO_HW_CONTEXT_OPENGL_CORE: return GAME_HW_CONTEXT_OPENGL_CORE;
    case RETRO_HW_CONTEXT_OPENGLES3:   return GAME_HW_CONTEXT_OPENGLES3;
    case RETRO_HW_CONTEXT_NONE:
    case RETRO_HW_CONTEXT_DUMMY:
    default:
      break;
  }
  return GAME_HW_CONTEXT_NONE;
}

GAME_RENDER_FORMAT LibretroTranslator::GetRenderFormat(retro_pixel_format format)
{
  switch (format)
  {
    case RETRO_PIXEL_FORMAT_0RGB1555: return GAME_RENDER_FMT_0RGB1555;
    case RETRO_PIXEL_FORMAT_XRGB8888: return GAME_RENDER_FMT_0RGB8888;
    case RETRO_PIXEL_FORMAT_RGB565:   return GAME_RENDER_FMT_RGB565;
    case RETRO_PIXEL_FORMAT_UNKNOWN:
    default:
      break;
  }
  return GAME_RENDER_FMT_NONE;
}

GAME_RUMBLE_EFFECT LibretroTranslator::GetRumbleEffect(retro_rumble_effect effect)
{
  switch (effect)
  {
    case RETRO_RUMBLE_STRONG: return GAME_RUMBLE_STRONG;
    case RETRO_RUMBLE_WEAK:   return GAME_RUMBLE_WEAK;
    case RETRO_RUMBLE_DUMMY:
    default:
      break;
  }
  return GAME_RUMBLE_STRONG;
}

retro_mod LibretroTranslator::GetKeyModifiers(GAME_KEY_MOD modifiers)
{
  retro_mod mods = RETROKMOD_NONE;

  if (modifiers & GAME_KEY_MOD_SHIFT)      mods = mods | RETROKMOD_SHIFT;
  if (modifiers & GAME_KEY_MOD_CTRL)       mods = mods | RETROKMOD_CTRL;
  if (modifiers & GAME_KEY_MOD_ALT)        mods = mods | RETROKMOD_ALT;
  if (modifiers & GAME_KEY_MOD_META)       mods = mods | RETROKMOD_META;
  if (modifiers & GAME_KEY_MOD_NUMLOCK)    mods = mods | RETROKMOD_NUMLOCK;
  if (modifiers & GAME_KEY_MOD_CAPSLOCK)   mods = mods | RETROKMOD_CAPSLOCK;
  if (modifiers & GAME_KEY_MOD_SCROLLOCK)  mods = mods | RETROKMOD_SCROLLOCK;

  return mods;
}

libretro_device_t LibretroTranslator::GetDeviceType(const std::string& strType)
{
  if (strType == "joypad")   return RETRO_DEVICE_JOYPAD;
  if (strType == "mouse")    return RETRO_DEVICE_MOUSE;
  if (strType == "keyboard") return RETRO_DEVICE_KEYBOARD;
  if (strType == "lightgun") return RETRO_DEVICE_LIGHTGUN;
  if (strType == "analog")   return RETRO_DEVICE_ANALOG;
  if (strType == "pointer")  return RETRO_DEVICE_POINTER;

  return RETRO_DEVICE_NONE;
}

int LibretroTranslator::GetFeatureIndex(const std::string& strFeatureName)
{
  if (strFeatureName == "a")            return RETRO_DEVICE_ID_JOYPAD_A;
  if (strFeatureName == "b")            return RETRO_DEVICE_ID_JOYPAD_B;
  if (strFeatureName == "x")            return RETRO_DEVICE_ID_JOYPAD_X;
  if (strFeatureName == "y")            return RETRO_DEVICE_ID_JOYPAD_Y;
  if (strFeatureName == "start")        return RETRO_DEVICE_ID_JOYPAD_START;
  if (strFeatureName == "select")       return RETRO_DEVICE_ID_JOYPAD_SELECT;
  if (strFeatureName == "up")           return RETRO_DEVICE_ID_JOYPAD_UP;
  if (strFeatureName == "down")         return RETRO_DEVICE_ID_JOYPAD_DOWN;
  if (strFeatureName == "right")        return RETRO_DEVICE_ID_JOYPAD_RIGHT;
  if (strFeatureName == "left")         return RETRO_DEVICE_ID_JOYPAD_LEFT;
  if (strFeatureName == "l")            return RETRO_DEVICE_ID_JOYPAD_L;
  if (strFeatureName == "r")            return RETRO_DEVICE_ID_JOYPAD_R;
  if (strFeatureName == "l2")           return RETRO_DEVICE_ID_JOYPAD_L2;
  if (strFeatureName == "r2")           return RETRO_DEVICE_ID_JOYPAD_R2;
  if (strFeatureName == "l3")           return RETRO_DEVICE_ID_JOYPAD_L3;
  if (strFeatureName == "r3")           return RETRO_DEVICE_ID_JOYPAD_R3;
  if (strFeatureName == "leftstick")    return RETRO_DEVICE_INDEX_ANALOG_LEFT;
  if (strFeatureName == "rightstick")   return RETRO_DEVICE_INDEX_ANALOG_RIGHT;
  if (strFeatureName == "leftmouse")    return RETRO_DEVICE_ID_MOUSE_LEFT;
  if (strFeatureName == "rightmouse")   return RETRO_DEVICE_ID_MOUSE_RIGHT;
  if (strFeatureName == "trigger")      return RETRO_DEVICE_ID_LIGHTGUN_TRIGGER;
  if (strFeatureName == "cursor")       return RETRO_DEVICE_ID_LIGHTGUN_CURSOR;
  if (strFeatureName == "turbo")        return RETRO_DEVICE_ID_LIGHTGUN_TURBO;
  if (strFeatureName == "pause")        return RETRO_DEVICE_ID_LIGHTGUN_PAUSE;
  if (strFeatureName == "start")        return RETRO_DEVICE_ID_LIGHTGUN_START;

  return -1;
}
