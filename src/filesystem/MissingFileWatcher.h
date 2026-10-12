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
 * \brief Notes the last file in a folder that the core looks for and doesn't
 *        find, such as a BIOS in the system directory
 *
 * A core that fails to load usually gives up right after the file it needed,
 * while the ones it looked for before may be optional. A file it finds in the
 * folder afterwards means it moved on, so that clears it.
 */
class CMissingFileWatcher
{
public:
  /*!
   * \brief Watches a folder for what the core looks for on the calling thread,
   *        for as long as the scope lasts
   */
  class Scope
  {
  public:
    Scope(CMissingFileWatcher& watcher, const std::string& folder);
    ~Scope();

    Scope(const Scope&) = delete;
    Scope& operator=(const Scope&) = delete;

  private:
    CMissingFileWatcher* m_previous;
  };

  /*!
   * \brief Forgets the missing file, before a game is loaded
   *
   * Not done per scope, as a core may only look for its BIOS on the first of
   * several attempts at loading one game.
   */
  void Clear() { m_missingFile.clear(); }

  /*!
   * \brief The last missing file's path relative to the folder, or empty
   */
  const std::string& MissingFile() const { return m_missingFile; }

  /*!
   * \brief Note that the core looked for a file through the VFS
   */
  static void NoteFile(const std::string& path, bool found);

private:
  void Note(const std::string& path, bool found);

  // The libretro VFS callbacks carry no context, so a file goes to the watcher
  // of the thread the core looked for it on
  static thread_local CMissingFileWatcher* s_current;

  std::string m_folder;
  std::string m_missingFile;
};
} // namespace LIBRETRO
