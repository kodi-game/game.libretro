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
