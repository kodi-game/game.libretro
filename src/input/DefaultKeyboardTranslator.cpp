/*
 *  Copyright (C) 2018-2021 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#include "DefaultKeyboardTranslator.h"
#include "DefaultKeyboardDefines.h"
#include "libretro-common/libretro.h"

using namespace LIBRETRO;

int CDefaultKeyboardTranslator::GetLibretroIndex(const std::string &strFeatureName)
{
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_BACKSPACE)     return RETROK_BACKSPACE;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_TAB)           return RETROK_TAB;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_CLEAR)         return RETROK_CLEAR;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_ENTER)         return RETROK_RETURN;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_PAUSE)         return RETROK_PAUSE;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_ESCAPE)        return RETROK_ESCAPE;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_SPACE)         return RETROK_SPACE;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_EXCLAIM)       return RETROK_EXCLAIM;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_DOUBLEQUOTE)   return RETROK_QUOTEDBL;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_HASH)          return RETROK_HASH;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_DOLLAR)        return RETROK_DOLLAR;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_AMPERSAND)     return RETROK_AMPERSAND;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_QUOTE)         return RETROK_QUOTE;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_LEFTPAREN)     return RETROK_LEFTPAREN;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_RIGHTPAREN)    return RETROK_RIGHTPAREN;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_ASTERISK)      return RETROK_ASTERISK;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_PLUS)          return RETROK_PLUS;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_COMMA)         return RETROK_COMMA;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_MINUS)         return RETROK_MINUS;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_PERIOD)        return RETROK_PERIOD;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_SLASH)         return RETROK_SLASH;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_0)             return RETROK_0;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_1)             return RETROK_1;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_2)             return RETROK_2;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_3)             return RETROK_3;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_4)             return RETROK_4;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_5)             return RETROK_5;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_6)             return RETROK_6;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_7)             return RETROK_7;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_8)             return RETROK_8;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_9)             return RETROK_9;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_COLON)         return RETROK_COLON;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_SEMICOLON)     return RETROK_SEMICOLON;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_LESS)          return RETROK_LESS;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_EQUALS)        return RETROK_EQUALS;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_GREATER)       return RETROK_GREATER;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_QUESTION)      return RETROK_QUESTION;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_AT)            return RETROK_AT;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_LEFTBRACKET)   return RETROK_LEFTBRACKET;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_BACKSLASH)     return RETROK_BACKSLASH;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_RIGHTBRACKET)  return RETROK_RIGHTBRACKET;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_CARET)         return RETROK_CARET;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_UNDERSCORE)    return RETROK_UNDERSCORE;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_GRAVE)         return RETROK_BACKQUOTE;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_A)             return RETROK_a;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_B)             return RETROK_b;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_C)             return RETROK_c;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_D)             return RETROK_d;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_E)             return RETROK_e;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_F)             return RETROK_f;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_G)             return RETROK_g;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_H)             return RETROK_h;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_I)             return RETROK_i;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_J)             return RETROK_j;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_K)             return RETROK_k;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_L)             return RETROK_l;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_M)             return RETROK_m;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_N)             return RETROK_n;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_O)             return RETROK_o;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_P)             return RETROK_p;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_Q)             return RETROK_q;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_R)             return RETROK_r;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_S)             return RETROK_s;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_T)             return RETROK_t;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_U)             return RETROK_u;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_V)             return RETROK_v;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_W)             return RETROK_w;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_X)             return RETROK_x;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_Y)             return RETROK_y;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_Z)             return RETROK_z;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_LEFTBRACE)     return RETROK_LEFTBRACE;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_BAR)           return RETROK_BAR;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_RIGHTBRACE)    return RETROK_RIGHTBRACE;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_TILDE)         return RETROK_TILDE;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_DELETE)        return RETROK_DELETE;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_KP0)           return RETROK_KP0;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_KP1)           return RETROK_KP1;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_KP2)           return RETROK_KP2;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_KP3)           return RETROK_KP3;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_KP4)           return RETROK_KP4;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_KP5)           return RETROK_KP5;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_KP6)           return RETROK_KP6;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_KP7)           return RETROK_KP7;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_KP8)           return RETROK_KP8;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_KP9)           return RETROK_KP9;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_KPPERIOD)      return RETROK_KP_PERIOD;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_KPDIVIDE)      return RETROK_KP_DIVIDE;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_KPMULTIPLY)    return RETROK_KP_MULTIPLY;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_KPMINUS)       return RETROK_KP_MINUS;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_KPPLUS)        return RETROK_KP_PLUS;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_KPENTER)       return RETROK_KP_ENTER;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_KPEQUALS)      return RETROK_KP_EQUALS;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_UP)            return RETROK_UP;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_DOWN)          return RETROK_DOWN;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_RIGHT)         return RETROK_RIGHT;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_LEFT)          return RETROK_LEFT;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_INSERT)        return RETROK_INSERT;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_HOME)          return RETROK_HOME;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_END)           return RETROK_END;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_PAGEUP)        return RETROK_PAGEUP;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_PAGEDOWN)      return RETROK_PAGEDOWN;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_F1)            return RETROK_F1;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_F2)            return RETROK_F2;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_F3)            return RETROK_F3;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_F4)            return RETROK_F4;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_F5)            return RETROK_F5;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_F6)            return RETROK_F6;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_F7)            return RETROK_F7;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_F8)            return RETROK_F8;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_F9)            return RETROK_F9;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_F10)           return RETROK_F10;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_F11)           return RETROK_F11;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_F12)           return RETROK_F12;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_F13)           return RETROK_F13;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_F14)           return RETROK_F14;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_F15)           return RETROK_F15;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_NUMLOCK)       return RETROK_NUMLOCK;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_CAPSLOCK)      return RETROK_CAPSLOCK;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_SCROLLLOCK)    return RETROK_SCROLLOCK;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_RIGHTSHIFT)    return RETROK_RSHIFT;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_LEFTSHIFT)     return RETROK_LSHIFT;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_RIGHTCTRL)     return RETROK_RCTRL;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_LEFTCTRL)      return RETROK_LCTRL;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_RIGHTALT)      return RETROK_RALT;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_LEFTALT)       return RETROK_LALT;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_RIGHTMETA)     return RETROK_RMETA;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_LEFTMETA)      return RETROK_LMETA;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_RIGHTSUPER)    return RETROK_RSUPER;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_LEFTSUPER)     return RETROK_LSUPER;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_MODE)          return RETROK_MODE;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_COMPOSE)       return RETROK_COMPOSE;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_HELP)          return RETROK_HELP;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_PRINTSCREEN)   return RETROK_PRINT;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_SYSREQ)        return RETROK_SYSREQ;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_BREAK)         return RETROK_BREAK;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_MENU)          return RETROK_MENU;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_POWER)         return RETROK_POWER;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_EURO)          return RETROK_EURO;
  if (strFeatureName == DEFAULT_KEYBOARD_FEATURE_UNDO)          return RETROK_UNDO;

  return -1;
}
