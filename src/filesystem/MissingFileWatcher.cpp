/*
 *  Copyright (C) 2026 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#include "MissingFileWatcher.h"

using namespace LIBRETRO;

namespace
{
bool IsSeparator(char c)
{
  return c == '/' || c == '\\';
}
} // namespace

thread_local CMissingFileWatcher* CMissingFileWatcher::s_current = nullptr;

CMissingFileWatcher::Scope::Scope(CMissingFileWatcher& watcher, const std::string& folder)
  : m_previous(s_current)
{
  watcher.m_folder = folder;
  // A root keeps its separator
  while (watcher.m_folder.size() > 1 && IsSeparator(watcher.m_folder.back()))
    watcher.m_folder.pop_back();
  s_current = &watcher;
}

CMissingFileWatcher::Scope::~Scope()
{
  s_current->m_folder.clear();
  s_current = m_previous;
}

void CMissingFileWatcher::NoteFile(const std::string& path, bool found)
{
  if (s_current != nullptr)
    s_current->Note(path, found);
}

void CMissingFileWatcher::Note(const std::string& path, bool found)
{
  if (m_folder.empty() || path.compare(0, m_folder.size(), m_folder) != 0)
    return;

  // Only a file inside the folder, not the folder itself or one beside it
  size_t start = m_folder.size();
  if (!IsSeparator(m_folder.back()) && (start == path.size() || !IsSeparator(path[start])))
    return;
  start = path.find_first_not_of("/\\", start);
  if (start == std::string::npos)
    return;

  if (found)
    m_missingFile.clear();
  else
    m_missingFile = path.substr(start);
}
