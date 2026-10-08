/*
 *  Copyright (C) 2014-2021 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#pragma once

#include "libretro-common/libretro.h"

#include <memory>
#include <string>
#include <vector>

namespace kodi
{
namespace vfs
{
  class CDirEntry;
  class CFile;
}
}

namespace LIBRETRO
{
  /*!
   * \brief The libretro VFS interface, forwarded to Kodi's VFS
   */
  class CFrontendVFS
  {
  public:
    static const char *GetPath(retro_vfs_file_handle *stream);
    static retro_vfs_file_handle *OpenFile(const char *path, unsigned mode, unsigned hints);
    static int CloseFile(retro_vfs_file_handle *stream);
    static int64_t FileSize(retro_vfs_file_handle *stream);
    static int64_t GetPosition(retro_vfs_file_handle *stream);
    static int64_t Seek(retro_vfs_file_handle *stream, int64_t offset, int seek_position);
    static int64_t ReadFile(retro_vfs_file_handle *stream, void *s, uint64_t len);
    static int64_t WriteFile(retro_vfs_file_handle *stream, const void *s, uint64_t len);
    static int FlushFile(retro_vfs_file_handle *stream);
    static int RemoveFile(const char *path);
    static int RenameFile(const char *old_path, const char *new_path);
    static int64_t Truncate(retro_vfs_file_handle *stream, int64_t length);
    static int Stat(const char *path, int32_t *size);
    static int MakeDirectory(const char *dir);
    static retro_vfs_dir_handle *OpenDirectory(const char *dir, bool include_hidden);
    static bool ReadDirectory(retro_vfs_dir_handle *dirstream);
    static const char *GetDirectoryName(retro_vfs_dir_handle *dirstream);
    static bool IsDirectory(retro_vfs_dir_handle *dirstream);
    static int CloseDirectory(retro_vfs_dir_handle *dirstream);

  private:
    struct FileHandle
    {
      std::string path;
      std::unique_ptr<kodi::vfs::CFile> file;
    };

    struct DirectoryHandle
    {
      std::string path;
      bool bOpen = false;
      std::vector<kodi::vfs::CDirEntry> items;
      std::vector<kodi::vfs::CDirEntry>::const_iterator currentPosition;
      std::vector<kodi::vfs::CDirEntry>::const_iterator nextPosition;
    };
  };
} // namespace LIBRETRO
