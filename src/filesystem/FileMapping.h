/*
 *  Copyright (C) 2026 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#pragma once

#include <stddef.h>
#include <stdint.h>
#include <string>

namespace LIBRETRO
{
/*!
 * \brief A file mapped into memory, so a game can be handed to a core without
 *        being copied into a buffer first
 *
 * The mapping is copy-on-write: a core that writes to its game changes only
 * its own pages, never the file. Only a file on a local filesystem is mapped,
 * as reading a mapping whose file has gone, such as on a network share that
 * drops, ends the process.
 */
class CFileMapping
{
public:
  CFileMapping() = default;
  ~CFileMapping();

  CFileMapping(const CFileMapping&) = delete;
  CFileMapping& operator=(const CFileMapping&) = delete;

  /*!
   * \brief Map a whole file, in place of any mapping held before
   *
   * \return False if the file can't be mapped here, and nothing is held then
   */
  bool Map(const std::string& path);

  void Unmap();

  const uint8_t* Data() const { return m_data; }
  size_t Size() const { return m_size; }

private:
  uint8_t* m_data = nullptr;
  size_t m_size = 0;
};
} // namespace LIBRETRO
