/*
 *  Copyright (C) 2020 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#include "CheevosFrontendBridge.h"

#include <kodi/Filesystem.h>
#include <stdio.h>

using namespace LIBRETRO;

void *CCheevosFrontendBridge::OpenFile(const char* path_utf8)
{
  // Return NULL for error
  if (path_utf8 == nullptr)
    return nullptr;

  // TODO: Handle UTF-8?
  std::unique_ptr<FileHandle> fileHandle(new FileHandle{ path_utf8 });
  fileHandle->file.reset(new kodi::vfs::CFile);

  if (!fileHandle->file->OpenFile(fileHandle->path, 0))
    return nullptr;

  // Return the opaque file handle on success
  return static_cast<void*>(fileHandle.release());
}

void CCheevosFrontendBridge::CloseFile(void* file_handle)
{
  if (file_handle == nullptr)
    return;

  FileHandle *fileHandle = static_cast<FileHandle*>(file_handle);

  fileHandle->file->Close();
  delete fileHandle;
}

size_t CCheevosFrontendBridge::GetPosition(void* file_handle)
{
  // Return 0 for error
  if (file_handle == nullptr)
    return 0;

  FileHandle *fileHandle = static_cast<FileHandle*>(file_handle);

  const int64_t currentPosition = fileHandle->file->GetPosition();

  if (currentPosition < 0)
    return 0;

  // Return the current read / write position for the file
  return currentPosition;
}

void CCheevosFrontendBridge::Seek(void* file_handle, size_t offset, int origin)
{
  if (file_handle == nullptr)
    return;

  FileHandle *fileHandle = static_cast<FileHandle*>(file_handle);

  int whence = -1;

  switch (origin)
  {
  case 0:
    whence = SEEK_SET;
    break;
  case 1:
    whence = SEEK_CUR;
    break;
  case 2:
    whence = SEEK_END;
    break;
  default:
    break;
  }

  if (whence == -1)
    return;

  fileHandle->file->Seek(offset, whence);
}

size_t CCheevosFrontendBridge::ReadFile(void* file_handle, void* buffer, size_t requested_bytes)
{
  // Return 0 for error
  if (file_handle == nullptr)
    return 0;

  FileHandle *fileHandle = static_cast<FileHandle*>(file_handle);

  const ssize_t bytesRead = fileHandle->file->Read(buffer, requested_bytes);

  if (bytesRead < 0)
    return 0;

  // Return 0 if no bytes are available to read (end of file was reached) or
  // undetectable error occurred
  if (bytesRead == 0)
    return 0;

  // Return the number of bytes read
  return static_cast<size_t>(bytesRead);
}
