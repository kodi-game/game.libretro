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
#pragma once

#include "input/LibretroDevice.h"
#include "libretro.h"

#include "kodi_game_types.h"
#include "XBMC_vkeys.h"

#include <string>

class TiXmlNode;

namespace LIBRETRO
{
  /*!
   * \brief Translates data types from the Game API to the corresponding one in libretro (and vice versa).
   *
   * This class is stateless.
   */
  class LibretroTranslator
  {
    LibretroTranslator() = delete;

  public:
    // --- Audo/video translation ----------------------------------------------

    /*!
     * \brief Translate video format (libretro to Game API).
     * \param format The video format to translate.
     * \return Translated video format.
     */
    static GAME_PIXEL_FORMAT GetVideoFormat(retro_pixel_format format);

    /*!
     * \brief Translate video rotation (libretro to Game API).
     * \param rotation The video rotation to translate as set by RETRO_ENVIRONMENT_SET_ROTATION.
     * \return Translated video rotation.
     */
    static GAME_VIDEO_ROTATION GetVideoRotation(unsigned int rotation);

    // --- Hardware rendering translation --------------------------------------

    /*!
     * \brief Translate HW context type (libretro to Game API)
     * \param type The HW context type to translate (e.g. OpenGL, OpenGLES).
     * \return Translated HW context type.
     */
    static GAME_HW_CONTEXT_TYPE GetHWContextType(retro_hw_context_type type);

    // --- Input translation --------------------------------------------------

    /*!
     * \brief Translate device type (libretro buttonmap "type" field) from version 1
     * \param strType The device type to translate from a buttonmap at version 1
     * \return The translated device type
     */
    static libretro_device_t GetDeviceTypeV1(const std::string& strType);

    /*!
     * \brief Translate device type (Game API to libretro).
     * \param strType The device type to translate.
     * \return Translated device values.
     */
    static libretro_device_t GetDeviceTypeV2(const std::string& strLibretroType);

    /*!
     * \brief Translate device type (libretro) to string representation (e.g. for logging).
     * \param type The device type to stringify.
     * \return String representation of device type.
     */
    static const char* GetDeviceName(libretro_device_t type);

    /*!
     * \brief Translate button/feature name (libretro buttonmap "mapto" field) from version 1 to version 2
     * \param strFeatureName The feature name to translate from a buttonmap at version 1.
     * \return The feature name for the buttonmap at version 2+.
     */
    static std::string GetFeatureV2(const std::string& strLibretroFeature);

    /*!
     * \brief Translate button/feature name (libretro buttonmap "mapto" field) to libretro index value.
     * \param strFeatureName The feature name to translate.
     * \return Translated button/feature id.
     */
    static int GetFeatureIndexV2(const std::string& strLibretroFeature);

    /*!
     * \brief Translate button/feature name (libretro buttonmap "mapto" field) to libretro index value.
     * \param strFeatureName The feature name to translate.
     * \return Translated button/feature id.
     */
    static libretro_device_t GetLibretroDevice(const std::string& strLibretroFeature);

    /*!
     * \brief Translate identifiers to feature name (libretro buttonmap "mapto" field).
     * \param type The libretro device type
     * \param index The libretro index
     * \param id The libretro ID
     * \return Translated feature name
     */
    static const char* GetFeatureName(libretro_device_t type, unsigned int index, unsigned int id);

    /*!
     * \brief Translate identifiers to component name of a feature, e.g. individual axes
     * \param type The libretro device type
     * \param index The libretro index
     * \param id The libretro ID
     * \return Translated name of a feature's component
     */
    static const char* GetComponentName(libretro_device_t type, unsigned int index, unsigned int id);

    /*!
     * \brief Translate libretro axis ID (libretro buttonmap "axis" field) to axis ID value in libretro.h
     * \param axisId The axis ID
     * \return Translated value of the axis ID
     */
    static int GetAxisID(const std::string& axisId);

    /*!
     * \brief Translate rumble motor name (libretro) to string representation (e.g. for logging).
     * \param effect The rumble motor name to translate.
     * \return String representation of rumble motor name.
     */
    static std::string GetMotorName(retro_rumble_effect effect);

    /*!
     * \brief Translate key modifiers (Game API to libretro).
     * \param modifiers The key modifiers to translate (e.g. Shift, Ctrl).
     * \return Translated key modifiers.
     */
    static retro_mod GetKeyModifiers(GAME_KEY_MOD modifiers);

    /*!
     * \brief Translate keycode (Game API to libretro).
     * \param character The character to translate.
     * \return Translated character.
     */
    static retro_key GetKeyCode(XBMCVKey character);

    /*!
     * \brief Translate keycode (Game API) to string representation (e.g. for logging pressed buttons).
     * \param character The character to translate.
     * \return String representation of character.
     */
    static const char* GetKeyName(XBMCVKey keycode);
  };
}
