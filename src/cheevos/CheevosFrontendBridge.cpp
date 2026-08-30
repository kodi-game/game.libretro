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

int64_t CCheevosFrontendBridge::GetPosition(void* file_handle)
{
  if (file_handle == nullptr)
    return -1;

  FileHandle *fileHandle = static_cast<FileHandle*>(file_handle);

  // Report the position we have tracked rather than asking the VFS, which
  // cannot always answer. See Seek() for why that distinction matters.
  return fileHandle->position;
}

void CCheevosFrontendBridge::Seek(void* file_handle, int64_t offset, int origin)
{
  if (file_handle == nullptr)
    return;

  FileHandle *fileHandle = static_cast<FileHandle*>(file_handle);

  // Resolve the destination to an absolute offset here rather than handing the
  // origin to the VFS.
  //
  // rcheevos has no call for a file's size: it seeks to the end and asks where
  // it landed. Not every VFS backend can seek, though. vfs.libarchive fails
  // inside a compressed entry and, worse, stores the failure as its position,
  // so the answer that comes back is negative. rc_hash_whole_file() takes that
  // as the size, and `remaining = (size_t)-1` has it hash sixteen exabytes of
  // stale buffer -- an unbreakable loop that hangs the add-on for good, since
  // the read return value it would need to notice is discarded.
  //
  // GetLength() those backends do answer, so ask that for SEEK_END, and track
  // the position ourselves so nothing negative can reach rcheevos.
  int64_t position = offset;

  switch (origin)
  {
  case 0: // SEEK_SET
    break;
  case 1: // SEEK_CUR
    position += fileHandle->position;
    break;
  case 2: // SEEK_END
  {
    const int64_t length = fileHandle->file->GetLength();
    if (length < 0)
      return;
    position += length;
    break;
  }
  default:
    return;
  }

  if (position < 0)
    return;

  const int64_t reached = fileHandle->file->Seek(position, SEEK_SET);
  if (reached >= 0)
  {
    fileHandle->position = reached;
    return;
  }

  // rcheevos uses SEEK_END only to measure the file before seeking elsewhere.
  if (origin == SEEK_END && offset == 0)
  {
    fileHandle->position = position;
    return;
  }

  std::unique_ptr<kodi::vfs::CFile> reopened(new kodi::vfs::CFile);
  if (!reopened->OpenFile(fileHandle->path, 0))
    return;

  char buffer[64 * 1024];
  int64_t remaining = position;
  while (remaining > 0)
  {
    const size_t chunkSize = remaining < static_cast<int64_t>(sizeof(buffer))
                                 ? static_cast<size_t>(remaining)
                                 : sizeof(buffer);
    const ssize_t bytesRead = reopened->Read(buffer, chunkSize);
    if (bytesRead <= 0)
      return;

    remaining -= bytesRead;
  }

  fileHandle->file->Close();
  fileHandle->file = std::move(reopened);
  fileHandle->position = position;
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

  fileHandle->position += bytesRead;

  // Return the number of bytes read
  return static_cast<size_t>(bytesRead);
}
