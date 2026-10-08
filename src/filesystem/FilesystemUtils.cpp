/*
 *  Copyright (C) 2026 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#include "FilesystemUtils.h"

using namespace LIBRETRO;

std::string CFilesystemUtils::CoreDirectory(std::string path)
{
  while (path.size() > 1 && (path.back() == '/' || path.back() == '\\') &&
         !(path.size() == 3 && path[1] == ':'))
    path.pop_back();

  return path;
}
