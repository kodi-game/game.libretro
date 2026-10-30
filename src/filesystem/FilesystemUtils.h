/*
 *  Copyright (C) 2026 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#pragma once

#include <map>
#include <string>

#include <kodi/Filesystem.h>

namespace LIBRETRO
{
/*!
 * \brief Helpers for paths and files, and for the paths handed to libretro cores
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

  /*!
   * \brief A file's path in a folder, with one separator between them
   */
  static std::string AddFileToFolder(const std::string& folder, const std::string& file);

  /*!
   * \brief Whether two paths name the same folder
   *
   * Kodi separates a Windows path's folders with backslashes, so either
   * separator matches the other.
   */
  static bool SameFolder(std::string a, std::string b);

  /*!
   * \brief List every file in a folder and in the folders inside it
   *
   * \param folder The folder to list
   * \param[in,out] files Each file, by its path inside the folder with '/'
   *                      between folders. A path already in the map is kept,
   *                      so a folder listed earlier takes precedence.
   */
  static void ListFiles(const std::string& folder,
                        std::map<std::string, kodi::vfs::CDirEntry>& files);

  /*!
   * \brief Create the folders a path runs through inside a folder
   *
   * \param folder The folder the path is inside
   * \param path A file's path inside it, with '/' between folders
   *
   * \return False if a folder couldn't be created
   */
  static bool CreateFolders(const std::string& folder, const std::string& path);

  /*!
   * \brief Copy a file over another without ever leaving it half written
   *
   * The copy is made beside the target and moved over it, so a file copied
   * only partly never takes its place, and the file it replaces is only
   * removed once the copy is in place.
   */
  static bool CopyInto(const std::string& source, const std::string& target);

  /*!
   * \brief Read a whole file as text
   *
   * \return False if the file doesn't exist or couldn't be read to its end
   */
  static bool ReadText(const std::string& path, std::string& text);

  /*!
   * \brief Write text to a file, replacing what it held
   */
  static bool WriteText(const std::string& path, const std::string& text);
};
} // namespace LIBRETRO
