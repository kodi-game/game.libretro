/*
 *  Copyright (C) 2026 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <map>
#include <string>

namespace LIBRETRO
{
/*!
 * \brief What an emulator last copied into the shared system folder
 *
 * It tells an earlier copy apart from a file that was put in the shared folder
 * some other way, or changed after it was copied.
 */
struct CSharedSystemRecord
{
  //! The shared folder the files were copied into
  std::string sharedDirectory;

  //! The emulator's version and its files' names, sizes and times when copied
  uint64_t fingerprint = 0;

  //! Each file copied, by its path in the system folder, with its content hash
  std::map<std::string, uint64_t> copies;

  /*!
   * \brief Read a record written by Format()
   *
   * \return False if the text is not a record, leaving this one empty
   */
  bool Parse(const std::string& text);

  std::string Format() const;
};

/*!
 * \brief What to do with one of an emulator's system files
 */
enum class SharedFileAction
{
  COPY, //!< The shared folder doesn't have the file
  NONE, //!< The shared folder has this file already
  REPLACE, //!< The shared folder has an earlier copy of the file
  KEEP, //!< The shared folder's file came from elsewhere, or was changed after copying
};

/*!
 * \param target The content hash of the file in the shared folder, or nullptr if there is none
 * \param source The content hash of the emulator's file
 * \param copied The content hash of the last copy made, or nullptr if none was
 */
SharedFileAction DecideSharedFile(const uint64_t* target, uint64_t source, const uint64_t* copied);

//! The 64-bit FNV-1a offset basis, the hash of no bytes
constexpr uint64_t HASH_START = 14695981039346656037ULL;

/*!
 * \brief 64-bit FNV-1a of \p size bytes, continuing from \p hash
 */
uint64_t HashBytes(const void* data, size_t size, uint64_t hash = HASH_START);
} // namespace LIBRETRO
