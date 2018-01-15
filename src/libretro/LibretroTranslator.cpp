/*
 *      Copyright (C) 2015-2016 Team Kodi
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

#include "LibretroTranslator.h"

#include <algorithm>

using namespace LIBRETRO;

namespace LIBRETRO
{
  inline retro_mod operator|(retro_mod lhs, retro_mod rhs)
  {
    return static_cast<retro_mod>(static_cast<int>(lhs) | static_cast<int>(rhs));
  }
}

// --- Audo/video translation ----------------------------------------------

GAME_PIXEL_FORMAT LibretroTranslator::GetVideoFormat(retro_pixel_format format)
{
  switch (format)
  {
    case RETRO_PIXEL_FORMAT_0RGB1555: return GAME_PIXEL_FORMAT_0RGB1555;
    case RETRO_PIXEL_FORMAT_XRGB8888: return GAME_PIXEL_FORMAT_0RGB8888;
    case RETRO_PIXEL_FORMAT_RGB565:   return GAME_PIXEL_FORMAT_RGB565;
    case RETRO_PIXEL_FORMAT_UNKNOWN:
    default:
      break;
  }
  return GAME_PIXEL_FORMAT_UNKNOWN;
}

GAME_VIDEO_ROTATION LibretroTranslator::GetVideoRotation(unsigned int rotation)
{
  switch (rotation)
  {
  case 0: return GAME_VIDEO_ROTATION_0;
  case 1: return GAME_VIDEO_ROTATION_90;
  case 2: return GAME_VIDEO_ROTATION_180;
  case 3: return GAME_VIDEO_ROTATION_270;
  default:
    break;
  }
  return GAME_VIDEO_ROTATION_0;
}

// --- Hardware rendering translation --------------------------------------

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

// --- Input translation --------------------------------------------------

libretro_device_t LibretroTranslator::GetDeviceType(const std::string& strLibretroType)
{
  if (strLibretroType == "RETRO_DEVICE_JOYPAD")   return RETRO_DEVICE_JOYPAD;
  if (strLibretroType == "RETRO_DEVICE_MOUSE")    return RETRO_DEVICE_MOUSE;
  if (strLibretroType == "RETRO_DEVICE_KEYBOARD") return RETRO_DEVICE_KEYBOARD;
  if (strLibretroType == "RETRO_DEVICE_LIGHTGUN") return RETRO_DEVICE_LIGHTGUN;
  if (strLibretroType == "RETRO_DEVICE_ANALOG")   return RETRO_DEVICE_ANALOG;
  if (strLibretroType == "RETRO_DEVICE_POINTER")  return RETRO_DEVICE_POINTER;

  return RETRO_DEVICE_NONE;
}

const char* LibretroTranslator::GetDeviceName(libretro_device_t type)
{
  switch (type)
  {
  case RETRO_DEVICE_JOYPAD:   return "RETRO_DEVICE_JOYPAD";
  case RETRO_DEVICE_MOUSE:    return "RETRO_DEVICE_MOUSE";
  case RETRO_DEVICE_KEYBOARD: return "RETRO_DEVICE_KEYBOARD";
  case RETRO_DEVICE_LIGHTGUN: return "RETRO_DEVICE_LIGHTGUN";
  case RETRO_DEVICE_ANALOG:   return "RETRO_DEVICE_ANALOG";
  case RETRO_DEVICE_POINTER:  return "RETRO_DEVICE_POINTER";
  default:
    break;
  }

  return "";
}

namespace LIBRETRO
{
  struct LibretroFeature
  {
    const char *libretroId;
    int featureIndex;
  };

  using LibretroFeatures = std::vector<LibretroFeature>;
  using LibretroFeatureMap = std::map<libretro_device_t, LibretroFeatures>;

  const LibretroFeatureMap featureMap = {
    {
      RETRO_DEVICE_JOYPAD, {
        { "RETRO_DEVICE_ID_JOYPAD_A",       RETRO_DEVICE_ID_JOYPAD_A },
        { "RETRO_DEVICE_ID_JOYPAD_B",       RETRO_DEVICE_ID_JOYPAD_B },
        { "RETRO_DEVICE_ID_JOYPAD_X",       RETRO_DEVICE_ID_JOYPAD_X },
        { "RETRO_DEVICE_ID_JOYPAD_Y",       RETRO_DEVICE_ID_JOYPAD_Y },
        { "RETRO_DEVICE_ID_JOYPAD_START",   RETRO_DEVICE_ID_JOYPAD_START },
        { "RETRO_DEVICE_ID_JOYPAD_SELECT",  RETRO_DEVICE_ID_JOYPAD_SELECT },
        { "RETRO_DEVICE_ID_JOYPAD_UP",      RETRO_DEVICE_ID_JOYPAD_UP },
        { "RETRO_DEVICE_ID_JOYPAD_DOWN",    RETRO_DEVICE_ID_JOYPAD_DOWN },
        { "RETRO_DEVICE_ID_JOYPAD_RIGHT",   RETRO_DEVICE_ID_JOYPAD_RIGHT },
        { "RETRO_DEVICE_ID_JOYPAD_LEFT",    RETRO_DEVICE_ID_JOYPAD_LEFT },
        { "RETRO_DEVICE_ID_JOYPAD_L",       RETRO_DEVICE_ID_JOYPAD_L },
        { "RETRO_DEVICE_ID_JOYPAD_R",       RETRO_DEVICE_ID_JOYPAD_R },
        { "RETRO_DEVICE_ID_JOYPAD_L2",      RETRO_DEVICE_ID_JOYPAD_L2 },
        { "RETRO_DEVICE_ID_JOYPAD_R2",      RETRO_DEVICE_ID_JOYPAD_R2 },
        { "RETRO_DEVICE_ID_JOYPAD_L3",      RETRO_DEVICE_ID_JOYPAD_L3 },
        { "RETRO_DEVICE_ID_JOYPAD_R3",      RETRO_DEVICE_ID_JOYPAD_R3 },
      }
    },
    {
      RETRO_DEVICE_ANALOG, {
        { "RETRO_DEVICE_INDEX_ANALOG_LEFT",  RETRO_DEVICE_INDEX_ANALOG_LEFT },
        { "RETRO_DEVICE_INDEX_ANALOG_RIGHT", RETRO_DEVICE_INDEX_ANALOG_RIGHT },
      }
    },
    {
      RETRO_DEVICE_MOUSE, {
        { "RETRO_DEVICE_MOUSE",                     0 }, // Only 1 relative pointer, use ID 0
        { "RETRO_DEVICE_ID_MOUSE_LEFT",             RETRO_DEVICE_ID_MOUSE_LEFT },
        { "RETRO_DEVICE_ID_MOUSE_RIGHT",            RETRO_DEVICE_ID_MOUSE_RIGHT },
        { "RETRO_DEVICE_ID_MOUSE_WHEELUP",          RETRO_DEVICE_ID_MOUSE_WHEELUP },
        { "RETRO_DEVICE_ID_MOUSE_WHEELDOWN",        RETRO_DEVICE_ID_MOUSE_WHEELDOWN },
        { "RETRO_DEVICE_ID_MOUSE_MIDDLE",           RETRO_DEVICE_ID_MOUSE_MIDDLE },
        { "RETRO_DEVICE_ID_MOUSE_HORIZ_WHEELUP",    RETRO_DEVICE_ID_MOUSE_HORIZ_WHEELUP },
        { "RETRO_DEVICE_ID_MOUSE_HORIZ_WHEELDOWN",  RETRO_DEVICE_ID_MOUSE_HORIZ_WHEELDOWN },
      }
    },
    {
      RETRO_DEVICE_LIGHTGUN, {
        { "RETRO_DEVICE_LIGHTGUN",                  0 }, // Only 1 relative pointer, use ID 0
        { "RETRO_DEVICE_ID_LIGHTGUN_TRIGGER",       RETRO_DEVICE_ID_LIGHTGUN_TRIGGER },
        { "RETRO_DEVICE_ID_LIGHTGUN_CURSOR",        RETRO_DEVICE_ID_LIGHTGUN_CURSOR },
        { "RETRO_DEVICE_ID_LIGHTGUN_TURBO",         RETRO_DEVICE_ID_LIGHTGUN_TURBO },
        { "RETRO_DEVICE_ID_LIGHTGUN_PAUSE",         RETRO_DEVICE_ID_LIGHTGUN_PAUSE },
        { "RETRO_DEVICE_ID_LIGHTGUN_START",         RETRO_DEVICE_ID_LIGHTGUN_START },
        { "RETRO_DEVICE_ID_LIGHTGUN_AUX_A",         RETRO_DEVICE_ID_LIGHTGUN_AUX_A },
        { "RETRO_DEVICE_ID_LIGHTGUN_AUX_B",         RETRO_DEVICE_ID_LIGHTGUN_AUX_B },
        { "RETRO_DEVICE_ID_LIGHTGUN_SELECT",        RETRO_DEVICE_ID_LIGHTGUN_SELECT },
        { "RETRO_DEVICE_ID_LIGHTGUN_AUX_C",         RETRO_DEVICE_ID_LIGHTGUN_AUX_C },
        { "RETRO_DEVICE_ID_LIGHTGUN_DPAD_UP",       RETRO_DEVICE_ID_LIGHTGUN_DPAD_UP },
        { "RETRO_DEVICE_ID_LIGHTGUN_DPAD_DOWN",     RETRO_DEVICE_ID_LIGHTGUN_DPAD_DOWN },
        { "RETRO_DEVICE_ID_LIGHTGUN_DPAD_LEFT",     RETRO_DEVICE_ID_LIGHTGUN_DPAD_LEFT },
        { "RETRO_DEVICE_ID_LIGHTGUN_DPAD_RIGHT",    RETRO_DEVICE_ID_LIGHTGUN_DPAD_RIGHT },
        { "RETRO_DEVICE_ID_LIGHTGUN_IS_OFFSCREEN",  RETRO_DEVICE_ID_LIGHTGUN_IS_OFFSCREEN },
        { "RETRO_DEVICE_ID_LIGHTGUN_RELOAD",        RETRO_DEVICE_ID_LIGHTGUN_RELOAD },
      }
    }
  };
}

int LibretroTranslator::GetFeatureIndex(const std::string& strLibretroFeature)
{
  for (const auto &it : featureMap)
  {
    const auto &features = it.second;

    auto it2 = std::find_if(features.begin(), features.end(),
      [&strLibretroFeature](const LibretroFeature& feature)
      {
        return strLibretroFeature == feature.libretroId;
      });

    if (it2 != features.end())
      return it2->featureIndex;
  }

  return -1;
}

libretro_device_t LibretroTranslator::GetLibretroDevice(const std::string& strLibretroFeature)
{
  for (const auto &it : featureMap)
  {
    const libretro_device_t deviceType = it.first;
    const auto &features = it.second;

    auto it2 = std::find_if(features.begin(), features.end(),
      [&strLibretroFeature](const LibretroFeature& feature)
      {
        return strLibretroFeature == feature.libretroId;
      });

    if (it2 != features.end())
      return deviceType;
  }

  return RETRO_DEVICE_NONE;
}

const char* LibretroTranslator::GetFeatureName(libretro_device_t type, unsigned int index, unsigned int id)
{
  switch (type)
  {
  case RETRO_DEVICE_JOYPAD:
  {
    switch (id)
    {
    case RETRO_DEVICE_ID_JOYPAD_B:        return "RETRO_DEVICE_ID_JOYPAD_B";
    case RETRO_DEVICE_ID_JOYPAD_Y:        return "RETRO_DEVICE_ID_JOYPAD_Y";
    case RETRO_DEVICE_ID_JOYPAD_SELECT:   return "RETRO_DEVICE_ID_JOYPAD_SELECT";
    case RETRO_DEVICE_ID_JOYPAD_START:    return "RETRO_DEVICE_ID_JOYPAD_START";
    case RETRO_DEVICE_ID_JOYPAD_UP:       return "RETRO_DEVICE_ID_JOYPAD_UP";
    case RETRO_DEVICE_ID_JOYPAD_DOWN:     return "RETRO_DEVICE_ID_JOYPAD_DOWN";
    case RETRO_DEVICE_ID_JOYPAD_LEFT:     return "RETRO_DEVICE_ID_JOYPAD_LEFT";
    case RETRO_DEVICE_ID_JOYPAD_RIGHT:    return "RETRO_DEVICE_ID_JOYPAD_RIGHT";
    case RETRO_DEVICE_ID_JOYPAD_A:        return "RETRO_DEVICE_ID_JOYPAD_A";
    case RETRO_DEVICE_ID_JOYPAD_X:        return "RETRO_DEVICE_ID_JOYPAD_X";
    case RETRO_DEVICE_ID_JOYPAD_L:        return "RETRO_DEVICE_ID_JOYPAD_L";
    case RETRO_DEVICE_ID_JOYPAD_R:        return "RETRO_DEVICE_ID_JOYPAD_R";
    case RETRO_DEVICE_ID_JOYPAD_L2:       return "RETRO_DEVICE_ID_JOYPAD_L2";
    case RETRO_DEVICE_ID_JOYPAD_R2:       return "RETRO_DEVICE_ID_JOYPAD_R2";
    case RETRO_DEVICE_ID_JOYPAD_L3:       return "RETRO_DEVICE_ID_JOYPAD_L3";
    case RETRO_DEVICE_ID_JOYPAD_R3:       return "RETRO_DEVICE_ID_JOYPAD_R3";
    default:
      break;
    }
    break;
  }
  case RETRO_DEVICE_MOUSE:
  {
    switch (id)
    {
    case RETRO_DEVICE_ID_MOUSE_X:
    case RETRO_DEVICE_ID_MOUSE_Y:
      return "RETRO_DEVICE_MOUSE";

    case RETRO_DEVICE_ID_MOUSE_LEFT:             return "RETRO_DEVICE_ID_MOUSE_LEFT";
    case RETRO_DEVICE_ID_MOUSE_RIGHT:            return "RETRO_DEVICE_ID_MOUSE_RIGHT";
    case RETRO_DEVICE_ID_MOUSE_WHEELUP:          return "RETRO_DEVICE_ID_MOUSE_WHEELUP";
    case RETRO_DEVICE_ID_MOUSE_WHEELDOWN:        return "RETRO_DEVICE_ID_MOUSE_WHEELDOWN";
    case RETRO_DEVICE_ID_MOUSE_MIDDLE:           return "RETRO_DEVICE_ID_MOUSE_MIDDLE";
    case RETRO_DEVICE_ID_MOUSE_HORIZ_WHEELUP:    return "RETRO_DEVICE_ID_MOUSE_HORIZ_WHEELUP";
    case RETRO_DEVICE_ID_MOUSE_HORIZ_WHEELDOWN:  return "RETRO_DEVICE_ID_MOUSE_HORIZ_WHEELDOWN";
    default:
      break;
    }
    break;
  }
  case RETRO_DEVICE_KEYBOARD:
  {
    break; // TODO
  }
  case RETRO_DEVICE_LIGHTGUN:
  {
    switch (id)
    {
    case RETRO_DEVICE_ID_LIGHTGUN_X:
    case RETRO_DEVICE_ID_LIGHTGUN_Y:
      return "RETRO_DEVICE_LIGHTGUN";

    case RETRO_DEVICE_ID_LIGHTGUN_TRIGGER:  return "RETRO_DEVICE_ID_LIGHTGUN_TRIGGER";
    case RETRO_DEVICE_ID_LIGHTGUN_CURSOR:   return "RETRO_DEVICE_ID_LIGHTGUN_CURSOR";
    case RETRO_DEVICE_ID_LIGHTGUN_TURBO:    return "RETRO_DEVICE_ID_LIGHTGUN_TURBO";
    case RETRO_DEVICE_ID_LIGHTGUN_PAUSE:    return "RETRO_DEVICE_ID_LIGHTGUN_PAUSE";
    case RETRO_DEVICE_ID_LIGHTGUN_START:    return "RETRO_DEVICE_ID_LIGHTGUN_START";
    default:
      break;
    }
    break;
  }
  case RETRO_DEVICE_ANALOG:
  {
    switch (id)
    {
    case RETRO_DEVICE_ID_ANALOG_X:
    case RETRO_DEVICE_ID_ANALOG_Y:
    {
      switch (index)
      {
      case RETRO_DEVICE_INDEX_ANALOG_LEFT:  return "RETRO_DEVICE_INDEX_ANALOG_LEFT";
      case RETRO_DEVICE_INDEX_ANALOG_RIGHT: return "RETRO_DEVICE_INDEX_ANALOG_RIGHT";
      default:
        break;
      }
      break;
    }
    break;
    default:
      break;
    }
  }
  case RETRO_DEVICE_POINTER:
  {
    break; // TODO
  }
  default:
    break;
  }

  return "";
}

int LibretroTranslator::GetAxisID(const std::string& axisId)
{
  if (axisId == "RETRO_DEVICE_ID_ANALOG_X")         return RETRO_DEVICE_ID_ANALOG_X;
  else if (axisId == "RETRO_DEVICE_ID_ANALOG_Y")    return RETRO_DEVICE_ID_ANALOG_Y;
  else if (axisId == "RETRO_DEVICE_ID_MOUSE_X")     return RETRO_DEVICE_ID_MOUSE_X;
  else if (axisId == "RETRO_DEVICE_ID_MOUSE_Y")     return RETRO_DEVICE_ID_MOUSE_Y;
  else if (axisId == "RETRO_DEVICE_ID_LIGHTGUN_X")  return RETRO_DEVICE_ID_LIGHTGUN_X;
  else if (axisId == "RETRO_DEVICE_ID_LIGHTGUN_Y")  return RETRO_DEVICE_ID_LIGHTGUN_Y;
  else if (axisId == "RETRO_DEVICE_ID_POINTER_X")   return RETRO_DEVICE_ID_POINTER_X;
  else if (axisId == "RETRO_DEVICE_ID_POINTER_Y")   return RETRO_DEVICE_ID_POINTER_Y;

  return -1;
}

const char* LibretroTranslator::GetComponentName(libretro_device_t type, unsigned int index, unsigned int id)
{
  switch (type)
  {
  case RETRO_DEVICE_ANALOG:
  {
    switch (id)
    {
    case RETRO_DEVICE_ID_ANALOG_X:  return "RETRO_DEVICE_ID_ANALOG_X";
    case RETRO_DEVICE_ID_ANALOG_Y:  return "RETRO_DEVICE_ID_ANALOG_Y";
    default:
      break;
    }
    break;
  }
  case RETRO_DEVICE_MOUSE:
  {
    switch (id)
    {
    case RETRO_DEVICE_ID_MOUSE_X:  return "RETRO_DEVICE_ID_MOUSE_X";
    case RETRO_DEVICE_ID_MOUSE_Y:  return "RETRO_DEVICE_ID_MOUSE_Y";
    default:
      break;
    }
    break;
  }
  case RETRO_DEVICE_LIGHTGUN:
  {
    switch (id)
    {
    case RETRO_DEVICE_ID_LIGHTGUN_X:  return "RETRO_DEVICE_ID_LIGHTGUN_X";
    case RETRO_DEVICE_ID_LIGHTGUN_Y:  return "RETRO_DEVICE_ID_LIGHTGUN_Y";
    default:
      break;
    }
    break;
  }
  case RETRO_DEVICE_POINTER:
  {
    switch (id)
    {
    case RETRO_DEVICE_ID_POINTER_X:  return "RETRO_DEVICE_ID_POINTER_X";
    case RETRO_DEVICE_ID_POINTER_Y:  return "RETRO_DEVICE_ID_POINTER_Y";
    default:
      break;
    }
    break;
  }
  default:
    break;
  }

  return "";
}

std::string LibretroTranslator::GetMotorName(retro_rumble_effect effect)
{
  switch (effect)
  {
    case RETRO_RUMBLE_STRONG: return "RETRO_RUMBLE_STRONG";
    case RETRO_RUMBLE_WEAK:   return "RETRO_RUMBLE_WEAK";
    default:
      break;
  }
  return "";
}

retro_mod LibretroTranslator::GetKeyModifiers(GAME_KEY_MOD modifiers)
{
  retro_mod mods = RETROKMOD_NONE;

  if (modifiers & GAME_KEY_MOD_SHIFT)      mods = mods | RETROKMOD_SHIFT;
  if (modifiers & GAME_KEY_MOD_CTRL)       mods = mods | RETROKMOD_CTRL;
  if (modifiers & GAME_KEY_MOD_ALT)        mods = mods | RETROKMOD_ALT;
  if (modifiers & GAME_KEY_MOD_RALT)       mods = mods | RETROKMOD_ALT;
  if (modifiers & GAME_KEY_MOD_META)       mods = mods | RETROKMOD_META;
  if (modifiers & GAME_KEY_MOD_NUMLOCK)    mods = mods | RETROKMOD_NUMLOCK;
  if (modifiers & GAME_KEY_MOD_CAPSLOCK)   mods = mods | RETROKMOD_CAPSLOCK;
  if (modifiers & GAME_KEY_MOD_SCROLLOCK)  mods = mods | RETROKMOD_SCROLLOCK;

  return mods;
}

retro_key LibretroTranslator::GetKeyCode(XBMCVKey character)
{
  switch (character)
  {
    case XBMCVK_BACK:         return RETROK_BACKSPACE;
    case XBMCVK_TAB:          return RETROK_TAB;
    //case 0x0000:              return RETROK_CLEAR;
    case XBMCVK_RETURN:       return RETROK_RETURN;
    case XBMCVK_PAUSE:        return RETROK_PAUSE;
    case XBMCVK_ESCAPE:       return RETROK_ESCAPE;
    case XBMCVK_SPACE:        return RETROK_SPACE;
    case XBMCVK_EXCLAIM:      return RETROK_EXCLAIM;
    case XBMCVK_QUOTEDBL:     return RETROK_QUOTEDBL;
    case XBMCVK_HASH:         return RETROK_HASH;
    case XBMCVK_DOLLAR:       return RETROK_DOLLAR;
    case XBMCVK_AMPERSAND:    return RETROK_AMPERSAND;
    case XBMCVK_QUOTE:        return RETROK_QUOTE;
    case XBMCVK_LEFTPAREN:    return RETROK_LEFTPAREN;
    case XBMCVK_RIGHTPAREN:   return RETROK_RIGHTPAREN;
    case XBMCVK_ASTERISK:     return RETROK_ASTERISK;
    case XBMCVK_PLUS:         return RETROK_PLUS;
    case XBMCVK_COMMA:        return RETROK_COMMA;
    case XBMCVK_MINUS:        return RETROK_MINUS;
    case XBMCVK_PERIOD:       return RETROK_PERIOD;
    case XBMCVK_SLASH:        return RETROK_SLASH;
    case XBMCVK_0:            return RETROK_0;
    case XBMCVK_1:            return RETROK_1;
    case XBMCVK_2:            return RETROK_2;
    case XBMCVK_3:            return RETROK_3;
    case XBMCVK_4:            return RETROK_4;
    case XBMCVK_5:            return RETROK_5;
    case XBMCVK_6:            return RETROK_6;
    case XBMCVK_7:            return RETROK_7;
    case XBMCVK_8:            return RETROK_8;
    case XBMCVK_9:            return RETROK_9;
    case XBMCVK_COLON:        return RETROK_COLON;
    case XBMCVK_SEMICOLON:    return RETROK_SEMICOLON;
    case XBMCVK_LESS:         return RETROK_LESS;
    case XBMCVK_EQUALS:       return RETROK_EQUALS;
    case XBMCVK_GREATER:      return RETROK_GREATER;
    case XBMCVK_QUESTION:     return RETROK_QUESTION;
    case XBMCVK_AT:           return RETROK_AT;
    case XBMCVK_A:            return RETROK_a;
    case XBMCVK_B:            return RETROK_b;
    case XBMCVK_C:            return RETROK_c;
    case XBMCVK_D:            return RETROK_d;
    case XBMCVK_E:            return RETROK_e;
    case XBMCVK_F:            return RETROK_f;
    case XBMCVK_G:            return RETROK_g;
    case XBMCVK_H:            return RETROK_h;
    case XBMCVK_I:            return RETROK_i;
    case XBMCVK_J:            return RETROK_j;
    case XBMCVK_K:            return RETROK_k;
    case XBMCVK_L:            return RETROK_l;
    case XBMCVK_M:            return RETROK_m;
    case XBMCVK_N:            return RETROK_n;
    case XBMCVK_O:            return RETROK_o;
    case XBMCVK_P:            return RETROK_p;
    case XBMCVK_Q:            return RETROK_q;
    case XBMCVK_R:            return RETROK_r;
    case XBMCVK_S:            return RETROK_s;
    case XBMCVK_T:            return RETROK_t;
    case XBMCVK_U:            return RETROK_u;
    case XBMCVK_V:            return RETROK_v;
    case XBMCVK_W:            return RETROK_w;
    case XBMCVK_X:            return RETROK_x;
    case XBMCVK_Y:            return RETROK_y;
    case XBMCVK_Z:            return RETROK_z;
    case XBMCVK_LEFTBRACKET:  return RETROK_LEFTBRACKET;
    case XBMCVK_BACKSLASH:    return RETROK_BACKSLASH;
    case XBMCVK_RIGHTBRACKET: return RETROK_RIGHTBRACKET;
    case XBMCVK_CARET:        return RETROK_CARET;
    case XBMCVK_UNDERSCORE:   return RETROK_UNDERSCORE;
    case XBMCVK_BACKQUOTE:    return RETROK_BACKQUOTE;
    case XBMCVK_DELETE:       return RETROK_DELETE;

    case XBMCVK_NUMPAD0:      return RETROK_KP0;
    case XBMCVK_NUMPAD1:      return RETROK_KP1;
    case XBMCVK_NUMPAD2:      return RETROK_KP2;
    case XBMCVK_NUMPAD3:      return RETROK_KP3;
    case XBMCVK_NUMPAD4:      return RETROK_KP4;
    case XBMCVK_NUMPAD5:      return RETROK_KP5;
    case XBMCVK_NUMPAD6:      return RETROK_KP6;
    case XBMCVK_NUMPAD7:      return RETROK_KP7;
    case XBMCVK_NUMPAD8:      return RETROK_KP8;
    case XBMCVK_NUMPAD9:      return RETROK_KP9;
    case XBMCVK_NUMPADPERIOD: return RETROK_KP_PERIOD;
    case XBMCVK_NUMPADDIVIDE: return RETROK_KP_DIVIDE;
    case XBMCVK_NUMPADTIMES:  return RETROK_KP_MULTIPLY;
    case XBMCVK_NUMPADMINUS:  return RETROK_KP_MINUS;
    case XBMCVK_NUMPADPLUS:   return RETROK_KP_PLUS;
    case XBMCVK_NUMPADENTER:  return RETROK_KP_ENTER;
    //case 0x0000:              return RETROK_KP_EQUALS;
    case XBMCVK_UP:           return RETROK_UP;
    case XBMCVK_DOWN:         return RETROK_DOWN;
    case XBMCVK_LEFT:         return RETROK_RIGHT;
    case XBMCVK_RIGHT:        return RETROK_LEFT;
    case XBMCVK_INSERT:       return RETROK_INSERT;
    case XBMCVK_HOME:         return RETROK_HOME;
    case XBMCVK_END:          return RETROK_END;
    case XBMCVK_PAGEUP:       return RETROK_PAGEUP;
    case XBMCVK_PAGEDOWN:     return RETROK_PAGEDOWN;

    case XBMCVK_F1:           return RETROK_F1;
    case XBMCVK_F2:           return RETROK_F2;
    case XBMCVK_F3:           return RETROK_F3;
    case XBMCVK_F4:           return RETROK_F4;
    case XBMCVK_F5:           return RETROK_F5;
    case XBMCVK_F6:           return RETROK_F6;
    case XBMCVK_F7:           return RETROK_F7;
    case XBMCVK_F8:           return RETROK_F8;
    case XBMCVK_F9:           return RETROK_F9;
    case XBMCVK_F10:          return RETROK_F10;
    case XBMCVK_F11:          return RETROK_F11;
    case XBMCVK_F12:          return RETROK_F12;
    case XBMCVK_F13:          return RETROK_F13;
    case XBMCVK_F14:          return RETROK_F14;
    case XBMCVK_F15:          return RETROK_F15;

    case XBMCVK_NUMLOCK:      return RETROK_NUMLOCK;
    case XBMCVK_CAPSLOCK:     return RETROK_CAPSLOCK;
    case XBMCVK_SCROLLLOCK:   return RETROK_SCROLLOCK;
    case XBMCVK_RSHIFT:       return RETROK_RSHIFT;
    case XBMCVK_LSHIFT:       return RETROK_LSHIFT;
    case XBMCVK_RCONTROL:     return RETROK_RCTRL;
    case XBMCVK_LCONTROL:     return RETROK_LCTRL;
    case XBMCVK_RMENU:        return RETROK_RALT;
    case XBMCVK_LMENU:        return RETROK_LALT;
    case XBMCVK_RWIN:         return RETROK_RMETA;
    case XBMCVK_LWIN:         return RETROK_LMETA;
    //case 0x0000:              return RETROK_LSUPER;
    //case 0x0000:              return RETROK_RSUPER;
    //case 0x0000:              return RETROK_MODE;
    //case 0x0000:              return RETROK_COMPOSE;

    //case 0x0000:              return RETROK_HELP;
    case XBMCVK_PRINTSCREEN:  return RETROK_PRINT;
    //case 0x0000:              return RETROK_SYSREQ;
    //case 0x0000:              return RETROK_BREAK;
    case XBMCVK_BROWSER_HOME: return RETROK_MENU;
    case XBMCVK_POWER:        return RETROK_POWER;
    //case 0x0000:              return RETROK_EURO;
    //case 0x0000:              return RETROK_UNDO;

    default:
      break;
  }
  return RETROK_UNKNOWN;
}

const char* LibretroTranslator::GetKeyName(XBMCVKey code)
{
  switch (code)
  {
    case XBMCVK_BACK:         return "backspace";
    case XBMCVK_TAB:          return "tab";
    //case 0x0000:              return "clear";
    case XBMCVK_RETURN:       return "return";
    case XBMCVK_PAUSE:        return "pause";
    case XBMCVK_ESCAPE:       return "escape";
    case XBMCVK_SPACE:        return "space";
    case XBMCVK_EXCLAIM:      return "\"";
    case XBMCVK_QUOTEDBL:     return "&";
    case XBMCVK_HASH:         return "#";
    case XBMCVK_DOLLAR:       return "$";
    case XBMCVK_AMPERSAND:    return "&";
    case XBMCVK_QUOTE:        return "'";
    case XBMCVK_LEFTPAREN:    return "(";
    case XBMCVK_RIGHTPAREN:   return ")";
    case XBMCVK_ASTERISK:     return "*";
    case XBMCVK_PLUS:         return "+";
    case XBMCVK_COMMA:        return ",";
    case XBMCVK_MINUS:        return "-";
    case XBMCVK_PERIOD:       return ".";
    case XBMCVK_SLASH:        return "-";
    case XBMCVK_0:            return "0";
    case XBMCVK_1:            return "1";
    case XBMCVK_2:            return "2";
    case XBMCVK_3:            return "3";
    case XBMCVK_4:            return "4";
    case XBMCVK_5:            return "5";
    case XBMCVK_6:            return "6";
    case XBMCVK_7:            return "7";
    case XBMCVK_8:            return "8";
    case XBMCVK_9:            return "9";
    case XBMCVK_COLON:        return ":";
    case XBMCVK_SEMICOLON:    return ";";
    case XBMCVK_LESS:         return "<";
    case XBMCVK_EQUALS:       return "=";
    case XBMCVK_GREATER:      return ">";
    case XBMCVK_QUESTION:     return "?";
    case XBMCVK_AT:           return "@";
    case XBMCVK_A:            return "A";
    case XBMCVK_B:            return "B";
    case XBMCVK_C:            return "C";
    case XBMCVK_D:            return "D";
    case XBMCVK_E:            return "E";
    case XBMCVK_F:            return "F";
    case XBMCVK_G:            return "G";
    case XBMCVK_H:            return "H";
    case XBMCVK_I:            return "I";
    case XBMCVK_J:            return "J";
    case XBMCVK_K:            return "K";
    case XBMCVK_L:            return "L";
    case XBMCVK_M:            return "M";
    case XBMCVK_N:            return "N";
    case XBMCVK_O:            return "O";
    case XBMCVK_P:            return "P";
    case XBMCVK_Q:            return "Q";
    case XBMCVK_R:            return "R";
    case XBMCVK_S:            return "S";
    case XBMCVK_T:            return "T";
    case XBMCVK_U:            return "U";
    case XBMCVK_V:            return "V";
    case XBMCVK_W:            return "W";
    case XBMCVK_X:            return "X";
    case XBMCVK_Y:            return "Y";
    case XBMCVK_Z:            return "Z";
    case XBMCVK_LEFTBRACKET:  return "]";
    case XBMCVK_BACKSLASH:    return "\\";
    case XBMCVK_RIGHTBRACKET: return "]";
    case XBMCVK_CARET:        return "^";
    case XBMCVK_UNDERSCORE:   return "_";
    case XBMCVK_BACKQUOTE:    return "`";
    case XBMCVK_DELETE:       return "delete";

    case XBMCVK_NUMPAD0:      return "keypad0";
    case XBMCVK_NUMPAD1:      return "keypad1";
    case XBMCVK_NUMPAD2:      return "keypad2";
    case XBMCVK_NUMPAD3:      return "keypad3";
    case XBMCVK_NUMPAD4:      return "keypad4";
    case XBMCVK_NUMPAD5:      return "keypad5";
    case XBMCVK_NUMPAD6:      return "keypad6";
    case XBMCVK_NUMPAD7:      return "keypad7";
    case XBMCVK_NUMPAD8:      return "keypad8";
    case XBMCVK_NUMPAD9:      return "keypad9";
    case XBMCVK_NUMPADPERIOD: return "keypadperiod";
    case XBMCVK_NUMPADDIVIDE: return "keypaddivide";
    case XBMCVK_NUMPADTIMES:  return "keypadmultiply";
    case XBMCVK_NUMPADMINUS:  return "keypadminus";
    case XBMCVK_NUMPADPLUS:   return "keypadplus";
    case XBMCVK_NUMPADENTER:  return "keypadenter";
    //case 0x0000:            return "keypadequals";
    case XBMCVK_UP:           return "up";
    case XBMCVK_DOWN:         return "down";
    case XBMCVK_LEFT:         return "right";
    case XBMCVK_RIGHT:        return "left";
    case XBMCVK_INSERT:       return "insert";
    case XBMCVK_HOME:         return "home";
    case XBMCVK_END:          return "end";
    case XBMCVK_PAGEUP:       return "pageup";
    case XBMCVK_PAGEDOWN:     return "pagedown";

    case XBMCVK_F1:           return "f1";
    case XBMCVK_F2:           return "f2";
    case XBMCVK_F3:           return "f3";
    case XBMCVK_F4:           return "f4";
    case XBMCVK_F5:           return "f5";
    case XBMCVK_F6:           return "f6";
    case XBMCVK_F7:           return "f7";
    case XBMCVK_F8:           return "f8";
    case XBMCVK_F9:           return "f9";
    case XBMCVK_F10:          return "f10";
    case XBMCVK_F11:          return "f11";
    case XBMCVK_F12:          return "f12";
    case XBMCVK_F13:          return "f13";
    case XBMCVK_F14:          return "f14";
    case XBMCVK_F15:          return "f15";

    case XBMCVK_NUMLOCK:      return "numlock";
    case XBMCVK_CAPSLOCK:     return "capslock";
    case XBMCVK_SCROLLLOCK:   return "scrolllock";
    case XBMCVK_RSHIFT:       return "rshift";
    case XBMCVK_LSHIFT:       return "lshift";
    case XBMCVK_RCONTROL:     return "rctrl";
    case XBMCVK_LCONTROL:     return "lctrl";
    case XBMCVK_RMENU:        return "ralt";
    case XBMCVK_LMENU:        return "lalt";
    case XBMCVK_RWIN:         return "rmeta";
    case XBMCVK_LWIN:         return "lmeta";
    //case 0x0000:              return "lsuper";
    //case 0x0000:              return "rsuper";
    //case 0x0000:              return "mode";
    //case 0x0000:              return "compose";

    //case 0x0000:              return "help";
    case XBMCVK_PRINTSCREEN:  return "print";
    //case 0x0000:              return "sysreq";
    //case 0x0000:              return "break";
    case XBMCVK_BROWSER_HOME: return "menu";
    case XBMCVK_POWER:        return "power";
    //case 0x0000:              return "euro";
    //case 0x0000:              return "undo";

    default:
      break;
  }
  return "";
}
