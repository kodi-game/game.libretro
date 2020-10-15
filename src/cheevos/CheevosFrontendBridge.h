/*
 *  Copyright (C) 2020 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#pragma once

#include <memory>
#include <rcheevos/rhash.h>
#include <string>

namespace kodi
{
namespace vfs
{
  class CFile;
}
}

namespace LIBRETRO
{
  /*!
   * Provide a bridge for frontend callbacks given to the rcheevos library.
   *
   * These functions are given to the rcheevos library in
   * CheevosEnvironment::Initialize().
   */
  class CCheevosFrontendBridge
  {
  public:
    // Forward to Kodi VFS API
    static void *OpenFile(const char* path_utf8);
    static void CloseFile(void* file_handle);
    static size_t GetPosition(void* file_handle);
    static void Seek(void* file_handle, size_t offset, int origin);
    static size_t ReadFile(void* file_handle, void* buffer, size_t requested_bytes);

  private:
    struct FileHandle
    {
      std::string path;
      std::unique_ptr<kodi::vfs::CFile> file;
    };
  };
} // namespace LIBRETRO
