/*
 *  Copyright (C) 2016-2021 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#pragma once

/*!
 * \brief The "system" directory of the frontend
 *
 * This directory can be used to store system specific content such as BIOSes,
 * configuration data, etc.
 */
#define LIBRETRO_SYSTEM_DIRECTORY_NAME  "system"

/*!
 * \brief The "save" directory of the frontend
 *
 * This directory can be used to store SRAM, memory cards, high scores, etc,
 * if the libretro core cannot use the regular memory interface
 * retro_get_memory_data().
 */
#define LIBRETRO_SAVE_DIRECTORY_NAME  "save"
