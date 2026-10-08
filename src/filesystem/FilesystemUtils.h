/*
 *  Copyright (C) 2026 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#pragma once

#include <string>

namespace LIBRETRO
{
/*!
 * \brief Helpers for the paths handed to libretro cores
 */
class CFilesystemUtils
{
public:
  /*!
   * \brief A directory the way libretro cores expect it
   *
   * Cores join a file name onto a directory with a separator of their own, so
   * the directory is given without a trailing one. A root keeps its separator,
   * as "/" would otherwise become empty and "D:\" drive-relative.
   */
  static std::string CoreDirectory(std::string path);
};
} // namespace LIBRETRO
