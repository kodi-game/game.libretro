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
#pragma once

#include "input/LibretroDevice.h"
#include "libretro.h"

#include "kodi/kodi_game_types.h"

#include <string>

class TiXmlNode;

namespace LIBRETRO
{
  class LibretroTranslator
  {
  public:
    static GAME_HW_CONTEXT_TYPE GetHWContextType(retro_hw_context_type type);
    static GAME_VIDEO_FORMAT    GetVideoFormat(retro_pixel_format format);
    static GAME_RUMBLE_EFFECT   GetRumbleEffect(retro_rumble_effect effect);
    static retro_mod            GetKeyModifiers(GAME_KEY_MOD modifiers);
    static libretro_device_t    GetDeviceType(const std::string& strType);
    static int                  GetFeatureIndex(const std::string& strFeatureName);
    static retro_key            GetKeyCode(uint32_t character);
    static const char*          GetKeyName(uint32_t keycode);
  };
}
