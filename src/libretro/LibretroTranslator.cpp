/*
 *  Copyright (C) 2015-2021 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
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

retro_pixel_format LibretroTranslator::GetLibretroVideoFormat(GAME_PIXEL_FORMAT format)
{
  switch (format)
  {
    case GAME_PIXEL_FORMAT_0RGB1555: return RETRO_PIXEL_FORMAT_0RGB1555;
    case GAME_PIXEL_FORMAT_0RGB8888: return RETRO_PIXEL_FORMAT_XRGB8888;
    case GAME_PIXEL_FORMAT_RGB565:   return RETRO_PIXEL_FORMAT_RGB565;
    default:
      break;
  }
  return RETRO_PIXEL_FORMAT_UNKNOWN;
}

const char *LibretroTranslator::VideoFormatToString(retro_pixel_format format)
{
  switch (format)
  {
    case RETRO_PIXEL_FORMAT_0RGB1555: return "0RGB1555";
    case RETRO_PIXEL_FORMAT_XRGB8888: return "XRGB8888";
    case RETRO_PIXEL_FORMAT_RGB565:   return "RGB565";
    default:
      break;
  }
  return "";
}

GAME_VIDEO_ROTATION LibretroTranslator::GetVideoRotation(unsigned int rotation)
{
  switch (rotation)
  {
  case 0: return GAME_VIDEO_ROTATION_0;
  case 1: return GAME_VIDEO_ROTATION_90_CCW;
  case 2: return GAME_VIDEO_ROTATION_180_CCW;
  case 3: return GAME_VIDEO_ROTATION_270_CCW;
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
    case RETRO_HW_CONTEXT_OPENGLES_VERSION: return GAME_HW_CONTEXT_OPENGLES_VERSION;
    case RETRO_HW_CONTEXT_VULKAN:      return GAME_HW_CONTEXT_VULKAN;
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
        { "RETRO_RUMBLE_STRONG",            RETRO_RUMBLE_STRONG },
        { "RETRO_RUMBLE_WEAK",              RETRO_RUMBLE_WEAK },
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
    },
    {
      RETRO_DEVICE_KEYBOARD, {
        { "RETROK_BACKSPACE",      RETROK_BACKSPACE },
        { "RETROK_TAB",            RETROK_TAB },
        { "RETROK_CLEAR",          RETROK_CLEAR },
        { "RETROK_RETURN",         RETROK_RETURN },
        { "RETROK_PAUSE",          RETROK_PAUSE },
        { "RETROK_ESCAPE",         RETROK_ESCAPE },
        { "RETROK_SPACE",          RETROK_SPACE },
        { "RETROK_EXCLAIM",        RETROK_EXCLAIM },
        { "RETROK_QUOTEDBL",       RETROK_QUOTEDBL },
        { "RETROK_HASH",           RETROK_HASH },
        { "RETROK_DOLLAR",         RETROK_DOLLAR },
        { "RETROK_AMPERSAND",      RETROK_AMPERSAND },
        { "RETROK_QUOTE",          RETROK_QUOTE },
        { "RETROK_LEFTPAREN",      RETROK_LEFTPAREN },
        { "RETROK_RIGHTPAREN",     RETROK_RIGHTPAREN },
        { "RETROK_ASTERISK",       RETROK_ASTERISK },
        { "RETROK_PLUS",           RETROK_PLUS },
        { "RETROK_COMMA",          RETROK_COMMA },
        { "RETROK_MINUS",          RETROK_MINUS },
        { "RETROK_PERIOD",         RETROK_PERIOD },
        { "RETROK_SLASH",          RETROK_SLASH },
        { "RETROK_0",              RETROK_0 },
        { "RETROK_1",              RETROK_1 },
        { "RETROK_2",              RETROK_2 },
        { "RETROK_3",              RETROK_3 },
        { "RETROK_4",              RETROK_4 },
        { "RETROK_5",              RETROK_5 },
        { "RETROK_6",              RETROK_6 },
        { "RETROK_7",              RETROK_7 },
        { "RETROK_8",              RETROK_8 },
        { "RETROK_9",              RETROK_9 },
        { "RETROK_COLON",          RETROK_COLON },
        { "RETROK_SEMICOLON",      RETROK_SEMICOLON },
        { "RETROK_LESS",           RETROK_LESS },
        { "RETROK_EQUALS",         RETROK_EQUALS },
        { "RETROK_GREATER",        RETROK_GREATER },
        { "RETROK_QUESTION",       RETROK_QUESTION },
        { "RETROK_AT",             RETROK_AT },
        { "RETROK_LEFTBRACKET",    RETROK_LEFTBRACKET },
        { "RETROK_BACKSLASH",      RETROK_BACKSLASH },
        { "RETROK_RIGHTBRACKET",   RETROK_RIGHTBRACKET },
        { "RETROK_CARET",          RETROK_CARET },
        { "RETROK_UNDERSCORE",     RETROK_UNDERSCORE },
        { "RETROK_BACKQUOTE",      RETROK_BACKQUOTE },
        { "RETROK_a",              RETROK_a },
        { "RETROK_b",              RETROK_b },
        { "RETROK_c",              RETROK_c },
        { "RETROK_d",              RETROK_d },
        { "RETROK_e",              RETROK_e },
        { "RETROK_f",              RETROK_f },
        { "RETROK_g",              RETROK_g },
        { "RETROK_h",              RETROK_h },
        { "RETROK_i",              RETROK_i },
        { "RETROK_j",              RETROK_j },
        { "RETROK_k",              RETROK_k },
        { "RETROK_l",              RETROK_l },
        { "RETROK_m",              RETROK_m },
        { "RETROK_n",              RETROK_n },
        { "RETROK_o",              RETROK_o },
        { "RETROK_p",              RETROK_p },
        { "RETROK_q",              RETROK_q },
        { "RETROK_r",              RETROK_r },
        { "RETROK_s",              RETROK_s },
        { "RETROK_t",              RETROK_t },
        { "RETROK_u",              RETROK_u },
        { "RETROK_v",              RETROK_v },
        { "RETROK_w",              RETROK_w },
        { "RETROK_x",              RETROK_x },
        { "RETROK_y",              RETROK_y },
        { "RETROK_z",              RETROK_z },
        { "RETROK_LEFTBRACE",      RETROK_LEFTBRACE },
        { "RETROK_BAR",            RETROK_BAR },
        { "RETROK_RIGHTBRACE",     RETROK_RIGHTBRACE },
        { "RETROK_TILDE",          RETROK_TILDE },
        { "RETROK_DELETE",         RETROK_DELETE },
        { "RETROK_KP0",            RETROK_KP0 },
        { "RETROK_KP1",            RETROK_KP1 },
        { "RETROK_KP2",            RETROK_KP2 },
        { "RETROK_KP3",            RETROK_KP3 },
        { "RETROK_KP4",            RETROK_KP4 },
        { "RETROK_KP5",            RETROK_KP5 },
        { "RETROK_KP6",            RETROK_KP6 },
        { "RETROK_KP7",            RETROK_KP7 },
        { "RETROK_KP8",            RETROK_KP8 },
        { "RETROK_KP9",            RETROK_KP9 },
        { "RETROK_KP_PERIOD",      RETROK_KP_PERIOD },
        { "RETROK_KP_DIVIDE",      RETROK_KP_DIVIDE },
        { "RETROK_KP_MULTIPLY",    RETROK_KP_MULTIPLY },
        { "RETROK_KP_MINUS",       RETROK_KP_MINUS },
        { "RETROK_KP_PLUS",        RETROK_KP_PLUS },
        { "RETROK_KP_ENTER",       RETROK_KP_ENTER },
        { "RETROK_KP_EQUALS",      RETROK_KP_EQUALS },
        { "RETROK_UP",             RETROK_UP },
        { "RETROK_DOWN",           RETROK_DOWN },
        { "RETROK_RIGHT",          RETROK_RIGHT },
        { "RETROK_LEFT",           RETROK_LEFT },
        { "RETROK_INSERT",         RETROK_INSERT },
        { "RETROK_HOME",           RETROK_HOME },
        { "RETROK_END",            RETROK_END },
        { "RETROK_PAGEUP",         RETROK_PAGEUP },
        { "RETROK_PAGEDOWN",       RETROK_PAGEDOWN },
        { "RETROK_F1",             RETROK_F1 },
        { "RETROK_F2",             RETROK_F2 },
        { "RETROK_F3",             RETROK_F3 },
        { "RETROK_F4",             RETROK_F4 },
        { "RETROK_F5",             RETROK_F5 },
        { "RETROK_F6",             RETROK_F6 },
        { "RETROK_F7",             RETROK_F7 },
        { "RETROK_F8",             RETROK_F8 },
        { "RETROK_F9",             RETROK_F9 },
        { "RETROK_F10",            RETROK_F10 },
        { "RETROK_F11",            RETROK_F11 },
        { "RETROK_F12",            RETROK_F12 },
        { "RETROK_F13",            RETROK_F13 },
        { "RETROK_F14",            RETROK_F14 },
        { "RETROK_F15",            RETROK_F15 },
        { "RETROK_NUMLOCK",        RETROK_NUMLOCK },
        { "RETROK_CAPSLOCK",       RETROK_CAPSLOCK },
        { "RETROK_SCROLLOCK",      RETROK_SCROLLOCK },
        { "RETROK_RSHIFT",         RETROK_RSHIFT },
        { "RETROK_LSHIFT",         RETROK_LSHIFT },
        { "RETROK_RCTRL",          RETROK_RCTRL },
        { "RETROK_LCTRL",          RETROK_LCTRL },
        { "RETROK_RALT",           RETROK_RALT },
        { "RETROK_LALT",           RETROK_LALT },
        { "RETROK_RMETA",          RETROK_RMETA },
        { "RETROK_LMETA",          RETROK_LMETA },
        { "RETROK_LSUPER",         RETROK_LSUPER },
        { "RETROK_RSUPER",         RETROK_RSUPER },
        { "RETROK_MODE",           RETROK_MODE },
        { "RETROK_COMPOSE",        RETROK_COMPOSE },
        { "RETROK_HELP",           RETROK_HELP },
        { "RETROK_PRINT",          RETROK_PRINT },
        { "RETROK_SYSREQ",         RETROK_SYSREQ },
        { "RETROK_BREAK",          RETROK_BREAK },
        { "RETROK_MENU",           RETROK_MENU },
        { "RETROK_POWER",          RETROK_POWER },
        { "RETROK_EURO",           RETROK_EURO },
        { "RETROK_UNDO",           RETROK_UNDO },
      }
    },
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
    switch (id)
    {
    case RETROK_BACKSPACE:      return "RETROK_BACKSPACE";
    case RETROK_TAB:            return "RETROK_TAB";
    case RETROK_CLEAR:          return "RETROK_CLEAR";
    case RETROK_RETURN:         return "RETROK_RETURN";
    case RETROK_PAUSE:          return "RETROK_PAUSE";
    case RETROK_ESCAPE:         return "RETROK_ESCAPE";
    case RETROK_SPACE:          return "RETROK_SPACE";
    case RETROK_EXCLAIM:        return "RETROK_EXCLAIM";
    case RETROK_QUOTEDBL:       return "RETROK_QUOTEDBL";
    case RETROK_HASH:           return "RETROK_HASH";
    case RETROK_DOLLAR:         return "RETROK_DOLLAR";
    case RETROK_AMPERSAND:      return "RETROK_AMPERSAND";
    case RETROK_QUOTE:          return "RETROK_QUOTE";
    case RETROK_LEFTPAREN:      return "RETROK_LEFTPAREN";
    case RETROK_RIGHTPAREN:     return "RETROK_RIGHTPAREN";
    case RETROK_ASTERISK:       return "RETROK_ASTERISK";
    case RETROK_PLUS:           return "RETROK_PLUS";
    case RETROK_COMMA:          return "RETROK_COMMA";
    case RETROK_MINUS:          return "RETROK_MINUS";
    case RETROK_PERIOD:         return "RETROK_PERIOD";
    case RETROK_SLASH:          return "RETROK_SLASH";
    case RETROK_0:              return "RETROK_0";
    case RETROK_1:              return "RETROK_1";
    case RETROK_2:              return "RETROK_2";
    case RETROK_3:              return "RETROK_3";
    case RETROK_4:              return "RETROK_4";
    case RETROK_5:              return "RETROK_5";
    case RETROK_6:              return "RETROK_6";
    case RETROK_7:              return "RETROK_7";
    case RETROK_8:              return "RETROK_8";
    case RETROK_9:              return "RETROK_9";
    case RETROK_COLON:          return "RETROK_COLON";
    case RETROK_SEMICOLON:      return "RETROK_SEMICOLON";
    case RETROK_LESS:           return "RETROK_LESS";
    case RETROK_EQUALS:         return "RETROK_EQUALS";
    case RETROK_GREATER:        return "RETROK_GREATER";
    case RETROK_QUESTION:       return "RETROK_QUESTION";
    case RETROK_AT:             return "RETROK_AT";
    case RETROK_LEFTBRACKET:    return "RETROK_LEFTBRACKET";
    case RETROK_BACKSLASH:      return "RETROK_BACKSLASH";
    case RETROK_RIGHTBRACKET:   return "RETROK_RIGHTBRACKET";
    case RETROK_CARET:          return "RETROK_CARET";
    case RETROK_UNDERSCORE:     return "RETROK_UNDERSCORE";
    case RETROK_BACKQUOTE:      return "RETROK_BACKQUOTE";
    case RETROK_a:              return "RETROK_a";
    case RETROK_b:              return "RETROK_b";
    case RETROK_c:              return "RETROK_c";
    case RETROK_d:              return "RETROK_d";
    case RETROK_e:              return "RETROK_e";
    case RETROK_f:              return "RETROK_f";
    case RETROK_g:              return "RETROK_g";
    case RETROK_h:              return "RETROK_h";
    case RETROK_i:              return "RETROK_i";
    case RETROK_j:              return "RETROK_j";
    case RETROK_k:              return "RETROK_k";
    case RETROK_l:              return "RETROK_l";
    case RETROK_m:              return "RETROK_m";
    case RETROK_n:              return "RETROK_n";
    case RETROK_o:              return "RETROK_o";
    case RETROK_p:              return "RETROK_p";
    case RETROK_q:              return "RETROK_q";
    case RETROK_r:              return "RETROK_r";
    case RETROK_s:              return "RETROK_s";
    case RETROK_t:              return "RETROK_t";
    case RETROK_u:              return "RETROK_u";
    case RETROK_v:              return "RETROK_v";
    case RETROK_w:              return "RETROK_w";
    case RETROK_x:              return "RETROK_x";
    case RETROK_y:              return "RETROK_y";
    case RETROK_z:              return "RETROK_z";
    case RETROK_LEFTBRACE:      return "RETROK_LEFTBRACE";
    case RETROK_BAR:            return "RETROK_BAR";
    case RETROK_RIGHTBRACE:     return "RETROK_RIGHTBRACE";
    case RETROK_TILDE:          return "RETROK_TILDE";
    case RETROK_DELETE:         return "RETROK_DELETE";
    case RETROK_KP0:            return "RETROK_KP0";
    case RETROK_KP1:            return "RETROK_KP1";
    case RETROK_KP2:            return "RETROK_KP2";
    case RETROK_KP3:            return "RETROK_KP3";
    case RETROK_KP4:            return "RETROK_KP4";
    case RETROK_KP5:            return "RETROK_KP5";
    case RETROK_KP6:            return "RETROK_KP6";
    case RETROK_KP7:            return "RETROK_KP7";
    case RETROK_KP8:            return "RETROK_KP8";
    case RETROK_KP9:            return "RETROK_KP9";
    case RETROK_KP_PERIOD:      return "RETROK_KP_PERIOD";
    case RETROK_KP_DIVIDE:      return "RETROK_KP_DIVIDE";
    case RETROK_KP_MULTIPLY:    return "RETROK_KP_MULTIPLY";
    case RETROK_KP_MINUS:       return "RETROK_KP_MINUS";
    case RETROK_KP_PLUS:        return "RETROK_KP_PLUS";
    case RETROK_KP_ENTER:       return "RETROK_KP_ENTER";
    case RETROK_KP_EQUALS:      return "RETROK_KP_EQUALS";
    case RETROK_UP:             return "RETROK_UP";
    case RETROK_DOWN:           return "RETROK_DOWN";
    case RETROK_RIGHT:          return "RETROK_RIGHT";
    case RETROK_LEFT:           return "RETROK_LEFT";
    case RETROK_INSERT:         return "RETROK_INSERT";
    case RETROK_HOME:           return "RETROK_HOME";
    case RETROK_END:            return "RETROK_END";
    case RETROK_PAGEUP:         return "RETROK_PAGEUP";
    case RETROK_PAGEDOWN:       return "RETROK_PAGEDOWN";
    case RETROK_F1:             return "RETROK_F1";
    case RETROK_F2:             return "RETROK_F2";
    case RETROK_F3:             return "RETROK_F3";
    case RETROK_F4:             return "RETROK_F4";
    case RETROK_F5:             return "RETROK_F5";
    case RETROK_F6:             return "RETROK_F6";
    case RETROK_F7:             return "RETROK_F7";
    case RETROK_F8:             return "RETROK_F8";
    case RETROK_F9:             return "RETROK_F9";
    case RETROK_F10:            return "RETROK_F10";
    case RETROK_F11:            return "RETROK_F11";
    case RETROK_F12:            return "RETROK_F12";
    case RETROK_F13:            return "RETROK_F13";
    case RETROK_F14:            return "RETROK_F14";
    case RETROK_F15:            return "RETROK_F15";
    case RETROK_NUMLOCK:        return "RETROK_NUMLOCK";
    case RETROK_CAPSLOCK:       return "RETROK_CAPSLOCK";
    case RETROK_SCROLLOCK:      return "RETROK_SCROLLOCK";
    case RETROK_RSHIFT:         return "RETROK_RSHIFT";
    case RETROK_LSHIFT:         return "RETROK_LSHIFT";
    case RETROK_RCTRL:          return "RETROK_RCTRL";
    case RETROK_LCTRL:          return "RETROK_LCTRL";
    case RETROK_RALT:           return "RETROK_RALT";
    case RETROK_LALT:           return "RETROK_LALT";
    case RETROK_RMETA:          return "RETROK_RMETA";
    case RETROK_LMETA:          return "RETROK_LMETA";
    case RETROK_LSUPER:         return "RETROK_LSUPER";
    case RETROK_RSUPER:         return "RETROK_RSUPER";
    case RETROK_MODE:           return "RETROK_MODE";
    case RETROK_COMPOSE:        return "RETROK_COMPOSE";
    case RETROK_HELP:           return "RETROK_HELP";
    case RETROK_PRINT:          return "RETROK_PRINT";
    case RETROK_SYSREQ:         return "RETROK_SYSREQ";
    case RETROK_BREAK:          return "RETROK_BREAK";
    case RETROK_MENU:           return "RETROK_MENU";
    case RETROK_POWER:          return "RETROK_POWER";
    case RETROK_EURO:           return "RETROK_EURO";
    case RETROK_UNDO:           return "RETROK_UNDO";
    default:
      break;
    }
    break;
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
    switch (index)
    {
    case RETRO_DEVICE_INDEX_ANALOG_LEFT:   return "RETRO_DEVICE_INDEX_ANALOG_LEFT";
    case RETRO_DEVICE_INDEX_ANALOG_RIGHT:  return "RETRO_DEVICE_INDEX_ANALOG_RIGHT";
    case RETRO_DEVICE_INDEX_ANALOG_BUTTON: return GetFeatureName(RETRO_DEVICE_JOYPAD, index, id);
    default:
      break;
    }
    break;
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
  if (modifiers & GAME_KEY_MOD_META)       mods = mods | RETROKMOD_META;
  // GAME_KEY_MOD_SUPER not implemented in libretro API
  if (modifiers & GAME_KEY_MOD_NUMLOCK)    mods = mods | RETROKMOD_NUMLOCK;
  if (modifiers & GAME_KEY_MOD_CAPSLOCK)   mods = mods | RETROKMOD_CAPSLOCK;
  if (modifiers & GAME_KEY_MOD_SCROLLOCK)  mods = mods | RETROKMOD_SCROLLOCK;

  return mods;
}
