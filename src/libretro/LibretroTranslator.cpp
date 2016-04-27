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
  if (strFeatureName == "strong")       return RETRO_RUMBLE_STRONG;
  if (strFeatureName == "weak")         return RETRO_RUMBLE_WEAK;

  return -1;
}

std::string LibretroTranslator::GetMotorName(retro_rumble_effect effect)
{
  switch (effect)
  {
    case RETRO_RUMBLE_STRONG: return "strong";
    case RETRO_RUMBLE_WEAK:   return "weak";
    default:
      break;
  }
  return "";
}

retro_key LibretroTranslator::GetKeyCode(uint32_t character)
{
  switch (character)
  {
    case 0x0000: return RETROK_UNKNOWN;
    case 0x0008: return RETROK_BACKSPACE;
    case 0x0009: return RETROK_TAB;
    case 0x000c: return RETROK_CLEAR;
    case 0x000d: return RETROK_RETURN;
    case 0x0013: return RETROK_PAUSE;
    case 0x001b: return RETROK_ESCAPE;
    case 0x0020: return RETROK_SPACE;
    case 0x0021: return RETROK_EXCLAIM;
    case 0x0022: return RETROK_QUOTEDBL;
    case 0x0023: return RETROK_HASH;
    case 0x0024: return RETROK_DOLLAR;
    case 0x0026: return RETROK_AMPERSAND;
    case 0x0027: return RETROK_QUOTE;
    case 0x0028: return RETROK_LEFTPAREN;
    case 0x0029: return RETROK_RIGHTPAREN;
    case 0x002a: return RETROK_ASTERISK;
    case 0x002b: return RETROK_PLUS;
    case 0x002c: return RETROK_COMMA;
    case 0x002d: return RETROK_MINUS;
    case 0x002e: return RETROK_PERIOD;
    case 0x002f: return RETROK_SLASH;
    case 0x0030: return RETROK_0;
    case 0x0031: return RETROK_1;
    case 0x0032: return RETROK_2;
    case 0x0033: return RETROK_3;
    case 0x0034: return RETROK_4;
    case 0x0035: return RETROK_5;
    case 0x0036: return RETROK_6;
    case 0x0037: return RETROK_7;
    case 0x0038: return RETROK_8;
    case 0x0039: return RETROK_9;
    case 0x003a: return RETROK_COLON;
    case 0x003b: return RETROK_SEMICOLON;
    case 0x003c: return RETROK_LESS;
    case 0x003d: return RETROK_EQUALS;
    case 0x003e: return RETROK_GREATER;
    case 0x003f: return RETROK_QUESTION;
    case 0x0040: return RETROK_AT;
    case 0x0041: return RETROK_a;
    case 0x0042: return RETROK_b;
    case 0x0043: return RETROK_c;
    case 0x0044: return RETROK_d;
    case 0x0045: return RETROK_e;
    case 0x0046: return RETROK_f;
    case 0x0047: return RETROK_g;
    case 0x0048: return RETROK_h;
    case 0x0049: return RETROK_i;
    case 0x004a: return RETROK_j;
    case 0x004b: return RETROK_k;
    case 0x004c: return RETROK_l;
    case 0x004d: return RETROK_m;
    case 0x004e: return RETROK_n;
    case 0x004f: return RETROK_o;
    case 0x0050: return RETROK_p;
    case 0x0051: return RETROK_q;
    case 0x0052: return RETROK_r;
    case 0x0053: return RETROK_s;
    case 0x0054: return RETROK_t;
    case 0x0055: return RETROK_u;
    case 0x0056: return RETROK_v;
    case 0x0057: return RETROK_w;
    case 0x0058: return RETROK_x;
    case 0x0059: return RETROK_y;
    case 0x005a: return RETROK_z;
    case 0x005b: return RETROK_LEFTBRACKET;
    case 0x005c: return RETROK_BACKSLASH;
    case 0x005d: return RETROK_RIGHTBRACKET;
    case 0x005e: return RETROK_CARET;
    case 0x005f: return RETROK_UNDERSCORE;
    case 0x0060: return RETROK_BACKQUOTE;
    case 0x0061: return RETROK_a;
    case 0x0062: return RETROK_b;
    case 0x0063: return RETROK_c;
    case 0x0064: return RETROK_d;
    case 0x0065: return RETROK_e;
    case 0x0066: return RETROK_f;
    case 0x0067: return RETROK_g;
    case 0x0068: return RETROK_h;
    case 0x0069: return RETROK_i;
    case 0x006a: return RETROK_j;
    case 0x006b: return RETROK_k;
    case 0x006c: return RETROK_l;
    case 0x006d: return RETROK_m;
    case 0x006e: return RETROK_n;
    case 0x006f: return RETROK_o;
    case 0x0070: return RETROK_p;
    case 0x0071: return RETROK_q;
    case 0x0072: return RETROK_r;
    case 0x0073: return RETROK_s;
    case 0x0074: return RETROK_t;
    case 0x0075: return RETROK_u;
    case 0x0076: return RETROK_v;
    case 0x0077: return RETROK_w;
    case 0x0078: return RETROK_x;
    case 0x0079: return RETROK_y;
    case 0x007a: return RETROK_z;
    case 0x007f: return RETROK_DELETE;

    case 0x0100: return RETROK_KP0;
    case 0x0101: return RETROK_KP1;
    case 0x0102: return RETROK_KP2;
    case 0x0103: return RETROK_KP3;
    case 0x0104: return RETROK_KP4;
    case 0x0105: return RETROK_KP5;
    case 0x0106: return RETROK_KP6;
    case 0x0107: return RETROK_KP7;
    case 0x0108: return RETROK_KP8;
    case 0x0109: return RETROK_KP9;
    case 0x010a: return RETROK_KP_PERIOD;
    case 0x010b: return RETROK_KP_DIVIDE;
    case 0x010c: return RETROK_KP_MULTIPLY;
    case 0x010d: return RETROK_KP_MINUS;
    case 0x010e: return RETROK_KP_PLUS;
    case 0x010f: return RETROK_KP_ENTER;
    case 0x0110: return RETROK_KP_EQUALS;
    case 0x0111: return RETROK_UP;
    case 0x0112: return RETROK_DOWN;
    case 0x0113: return RETROK_RIGHT;
    case 0x0114: return RETROK_LEFT;
    case 0x0115: return RETROK_INSERT;
    case 0x0116: return RETROK_HOME;
    case 0x0117: return RETROK_END;
    case 0x0118: return RETROK_PAGEUP;
    case 0x0119: return RETROK_PAGEDOWN;

    case 0x011a: return RETROK_F1;
    case 0x011b: return RETROK_F2;
    case 0x011c: return RETROK_F3;
    case 0x011d: return RETROK_F4;
    case 0x011e: return RETROK_F5;
    case 0x011f: return RETROK_F6;
    case 0x0120: return RETROK_F7;
    case 0x0121: return RETROK_F8;
    case 0x0122: return RETROK_F9;
    case 0x0123: return RETROK_F10;
    case 0x0124: return RETROK_F11;
    case 0x0125: return RETROK_F12;
    case 0x0126: return RETROK_F13;
    case 0x0127: return RETROK_F14;
    case 0x0128: return RETROK_F15;

    case 0x012c: return RETROK_NUMLOCK;
    case 0x012d: return RETROK_CAPSLOCK;
    case 0x012e: return RETROK_SCROLLOCK;
    case 0x012f: return RETROK_RSHIFT;
    case 0x0130: return RETROK_LSHIFT;
    case 0x0131: return RETROK_RCTRL;
    case 0x0132: return RETROK_LCTRL;
    case 0x0133: return RETROK_RALT;
    case 0x0134: return RETROK_LALT;
    case 0x0135: return RETROK_RMETA;
    case 0x0136: return RETROK_LMETA;
    case 0x0137: return RETROK_LSUPER;
    case 0x0138: return RETROK_RSUPER;
    case 0x0139: return RETROK_MODE;
    case 0x013a: return RETROK_COMPOSE;

    case 0x013b: return RETROK_HELP;
    case 0x013c: return RETROK_PRINT;
    case 0x013d: return RETROK_SYSREQ;
    case 0x013e: return RETROK_BREAK;
    case 0x013f: return RETROK_MENU;
    case 0x0140: return RETROK_POWER;
    case 0x0141: return RETROK_EURO;
    case 0x0142: return RETROK_UNDO;

    default:
      break;
  }
  return RETROK_UNKNOWN;
}

const char* LibretroTranslator::GetKeyName(uint32_t code)
{
  switch (code)
  {
    case 0x0008: return "backspace";
    case 0x0009: return "tab";
    case 0x000c: return "clear";
    case 0x000d: return "return";
    case 0x0013: return "pause";
    case 0x001b: return "escape";
    case 0x0020: return "space";
    case 0x0021: return "\"";
    case 0x0022: return "&";
    case 0x0023: return "#";
    case 0x0024: return "$";
    case 0x0026: return "&";
    case 0x0027: return "'";
    case 0x0028: return "(";
    case 0x0029: return ")";
    case 0x002a: return "*";
    case 0x002b: return "+";
    case 0x002c: return ",";
    case 0x002d: return "-";
    case 0x002e: return ".";
    case 0x002f: return "-";
    case 0x0030: return "0";
    case 0x0031: return "1";
    case 0x0032: return "2";
    case 0x0033: return "3";
    case 0x0034: return "4";
    case 0x0035: return "5";
    case 0x0036: return "6";
    case 0x0037: return "7";
    case 0x0038: return "8";
    case 0x0039: return "9";
    case 0x003a: return ":";
    case 0x003b: return ";";
    case 0x003c: return "<";
    case 0x003d: return "=";
    case 0x003e: return ">";
    case 0x003f: return "?";
    case 0x0040: return "@";
    case 0x0041: return "A";
    case 0x0042: return "B";
    case 0x0043: return "C";
    case 0x0044: return "D";
    case 0x0045: return "E";
    case 0x0046: return "F";
    case 0x0047: return "G";
    case 0x0048: return "H";
    case 0x0049: return "I";
    case 0x004a: return "J";
    case 0x004b: return "K";
    case 0x004c: return "L";
    case 0x004d: return "M";
    case 0x004e: return "N";
    case 0x004f: return "O";
    case 0x0050: return "P";
    case 0x0051: return "Q";
    case 0x0052: return "R";
    case 0x0053: return "S";
    case 0x0054: return "T";
    case 0x0055: return "U";
    case 0x0056: return "V";
    case 0x0057: return "W";
    case 0x0058: return "X";
    case 0x0059: return "Y";
    case 0x005a: return "Z";
    case 0x005b: return "]";
    case 0x005c: return "\\";
    case 0x005d: return "]";
    case 0x005e: return "^";
    case 0x005f: return "_";
    case 0x0060: return "`";
    case 0x0061: return "a";
    case 0x0062: return "b";
    case 0x0063: return "c";
    case 0x0064: return "d";
    case 0x0065: return "e";
    case 0x0066: return "f";
    case 0x0067: return "g";
    case 0x0068: return "h";
    case 0x0069: return "i";
    case 0x006a: return "j";
    case 0x006b: return "k";
    case 0x006c: return "l";
    case 0x006d: return "m";
    case 0x006e: return "n";
    case 0x006f: return "o";
    case 0x0070: return "p";
    case 0x0071: return "q";
    case 0x0072: return "r";
    case 0x0073: return "s";
    case 0x0074: return "t";
    case 0x0075: return "u";
    case 0x0076: return "v";
    case 0x0077: return "w";
    case 0x0078: return "x";
    case 0x0079: return "y";
    case 0x007a: return "z";
    case 0x007f: return "delete";

    case 0x0100: return "keypad0";
    case 0x0101: return "keypad1";
    case 0x0102: return "keypad2";
    case 0x0103: return "keypad3";
    case 0x0104: return "keypad4";
    case 0x0105: return "keypad5";
    case 0x0106: return "keypad6";
    case 0x0107: return "keypad7";
    case 0x0108: return "keypad8";
    case 0x0109: return "keypad9";
    case 0x010a: return "keypadperiod";
    case 0x010b: return "keypaddivide";
    case 0x010c: return "keypadmultiply";
    case 0x010d: return "keypadminus";
    case 0x010e: return "keypadplus";
    case 0x010f: return "keypadenter";
    case 0x0110: return "keypadequals";
    case 0x0111: return "up";
    case 0x0112: return "down";
    case 0x0113: return "right";
    case 0x0114: return "left";
    case 0x0115: return "insert";
    case 0x0116: return "home";
    case 0x0117: return "end";
    case 0x0118: return "pageup";
    case 0x0119: return "pagedown";

    case 0x011a: return "f1";
    case 0x011b: return "f2";
    case 0x011c: return "f3";
    case 0x011d: return "f4";
    case 0x011e: return "f5";
    case 0x011f: return "f6";
    case 0x0120: return "f7";
    case 0x0121: return "f8";
    case 0x0122: return "f9";
    case 0x0123: return "f10";
    case 0x0124: return "f11";
    case 0x0125: return "f12";
    case 0x0126: return "f13";
    case 0x0127: return "f14";
    case 0x0128: return "f15";

    case 0x012c: return "numlock";
    case 0x012d: return "capslock";
    case 0x012e: return "scrolllock";
    case 0x012f: return "rshift";
    case 0x0130: return "lshift";
    case 0x0131: return "rctrl";
    case 0x0132: return "lctrl";
    case 0x0133: return "ralt";
    case 0x0134: return "lalt";
    case 0x0135: return "rmeta";
    case 0x0136: return "lmeta";
    case 0x0137: return "lsuper";
    case 0x0138: return "rsuper";
    case 0x0139: return "mode";
    case 0x013a: return "compose";

    case 0x013b: return "help";
    case 0x013c: return "print";
    case 0x013d: return "sysreq";
    case 0x013e: return "break";
    case 0x013f: return "menu";
    case 0x0140: return "power";
    case 0x0141: return "euro";
    case 0x0142: return "undo";

    default:
      break;
  }
  return "";
}
