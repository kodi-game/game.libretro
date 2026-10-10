/*
 *  Copyright (C) 2014-2021 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#include "FrontendVFS.h"

#include <kodi/Filesystem.h>
#include <algorithm>
#include <cstring>
#include <limits>

using namespace LIBRETRO;

namespace
{
  // A copy step's budget when the core leaves it to the frontend, as in RetroArch
  constexpr int64_t DEFAULT_COPY_STEP = 4 * 1024 * 1024;

  constexpr size_t COPY_BUFFER_SIZE = 64 * 1024;
}

const char *CFrontendVFS::GetPath(retro_vfs_file_handle *stream)
{
  if (stream == nullptr)
    return "";

  FileHandle *fileHandle = reinterpret_cast<FileHandle*>(stream);

  return fileHandle->path.c_str();
}

retro_vfs_file_handle *CFrontendVFS::OpenFile(const char *path, unsigned mode, unsigned hints)
{
  // Return NULL for error
  if (path == nullptr)
    return nullptr;

  std::unique_ptr<FileHandle> fileHandle(new FileHandle{ path });
  fileHandle->file.reset(new kodi::vfs::CFile);

  const bool bReadOnly = (mode == RETRO_VFS_FILE_ACCESS_READ);
  if (bReadOnly)
  {
    unsigned int flags = 0;

    // TODO
    //flags |= ADDON_READ_TRUNCATED;

    if (hints & RETRO_VFS_FILE_ACCESS_HINT_FREQUENT_ACCESS)
      flags |= ADDON_READ_CACHED;

    if (!fileHandle->file->OpenFile(fileHandle->path, flags))
      return nullptr;
  }
  else
  {
    // Discard contents and overwrite existing file unless "update existing" is
    // specified
    const bool bOverwrite = !(mode & RETRO_VFS_FILE_ACCESS_UPDATE_EXISTING);

    if (!fileHandle->file->OpenFileForWrite(fileHandle->path, bOverwrite))
      return nullptr;
  }

  // Return the opaque file handle on success
  return reinterpret_cast<retro_vfs_file_handle*>(fileHandle.release());
}

int CFrontendVFS::CloseFile(retro_vfs_file_handle *stream)
{
  // Return -1 on failure
  if (stream == nullptr)
    return -1;

  FileHandle *fileHandle = reinterpret_cast<FileHandle*>(stream);

  fileHandle->file->Close();
  delete fileHandle;

  // Return 0 on success
  return 0;
}

int64_t CFrontendVFS::FileSize(retro_vfs_file_handle *stream)
{
  // Return -1 for error
  if (stream == nullptr)
    return -1;

  FileHandle *fileHandle = reinterpret_cast<FileHandle*>(stream);

  const int64_t fileSize = fileHandle->file->GetLength();

  if (fileSize < 0)
    return -1;

  // Return the file size on success
  return fileSize;
}

int64_t CFrontendVFS::GetPosition(retro_vfs_file_handle *stream)
{
  // Return -1 for error
  if (stream == nullptr)
    return -1;

  FileHandle *fileHandle = reinterpret_cast<FileHandle*>(stream);

  const int64_t currentPosition = fileHandle->file->GetPosition();

  if (currentPosition < 0)
    return -1;

  // Return the current read / write position for the file
  return currentPosition;
}

int64_t CFrontendVFS::Seek(retro_vfs_file_handle *stream, int64_t offset, int seek_position)
{
  // Return -1 for error
  if (stream == nullptr)
    return -1;

  FileHandle *fileHandle = reinterpret_cast<FileHandle*>(stream);

  int whence = -1;

  switch (seek_position)
  {
  case RETRO_VFS_SEEK_POSITION_START:
    whence = SEEK_SET;
    break;
  case RETRO_VFS_SEEK_POSITION_CURRENT:
    whence = SEEK_CUR;
    break;
  case RETRO_VFS_SEEK_POSITION_END:
    whence = SEEK_END;
    break;
  default:
    break;
  }

  if (whence == -1)
    return -1;

  if (fileHandle->file->Seek(offset, whence) < 0)
    return -1;

  return 0;
}

int64_t CFrontendVFS::ReadFile(retro_vfs_file_handle *stream, void *s, uint64_t len)
{
  // Return -1 for error
  if (stream == nullptr)
    return -1;

  FileHandle *fileHandle = reinterpret_cast<FileHandle*>(stream);

  const ssize_t bytesRead = fileHandle->file->Read(s, static_cast<size_t>(len));

  if (bytesRead < 0)
    return -1;

  // Return 0 if no bytes are available to read (end of file was reached) or
  // undetectable error occurred
  if (bytesRead == 0)
    return 0;

  // Return the number of bytes read
  return static_cast<int64_t>(bytesRead);
}

int64_t CFrontendVFS::WriteFile(retro_vfs_file_handle *stream, const void *s, uint64_t len)
{
  // Return -1 for error
  if (stream == nullptr)
    return -1;

  FileHandle *fileHandle = reinterpret_cast<FileHandle*>(stream);

  const ssize_t bytesWritten = fileHandle->file->Write(s, static_cast<size_t>(len));

  if (bytesWritten < 0)
    return -1;

  // Return 0 if no bytes were written and no detectable error occurred
  if (bytesWritten == 0)
    return 0;

  // Return the number of bytes written
  return static_cast<int64_t>(bytesWritten);
}

int CFrontendVFS::FlushFile(retro_vfs_file_handle *stream)
{
  // Return -1 on failure
  if (stream == nullptr)
    return -1;

  FileHandle *fileHandle = reinterpret_cast<FileHandle*>(stream);

  fileHandle->file->Flush();

  // Return 0 on success
  return 0;
}

int CFrontendVFS::RemoveFile(const char *path)
{
  // Return -1 on failure
  if (path == nullptr)
    return -1;

  if (!kodi::vfs::DeleteFile(path))
    return -1;

  // Return 0 on success
  return 0;
}

int CFrontendVFS::RenameFile(const char *old_path, const char *new_path)
{
  // Return -1 on failure
  if (old_path == nullptr || new_path == nullptr)
    return -1;

  if (!kodi::vfs::RenameFile(old_path, new_path))
    return -1;

  // Return 0 on success
  return 0;
}

int64_t CFrontendVFS::Truncate(retro_vfs_file_handle *stream, int64_t length)
{
  // Return -1 on error
  if (stream == nullptr)
    return -1;

  FileHandle *fileHandle = reinterpret_cast<FileHandle*>(stream);

  if (fileHandle->file->Truncate(length) < 0)
    return -1;

  // Return 0 on success
  return 0;
}

int CFrontendVFS::Stat(const char *path, int32_t *size)
{
  int64_t fileSize = 0;
  const int returnBitmask = StatPath(path, &fileSize);

  // Set file size
  if (size != nullptr && (returnBitmask & RETRO_VFS_STAT_IS_VALID))
  {
    // What to return if size > 2 GiB?
    if (fileSize <= std::numeric_limits<int32_t>::max())
      *size = static_cast<int32_t>(fileSize);
  }

  return returnBitmask;
}

int CFrontendVFS::Stat64(const char *path, int64_t *size)
{
  return StatPath(path, size);
}

int CFrontendVFS::StatPath(const char *path, int64_t *size)
{
  int returnBitmask = 0;

  // Return mask with no flags set if the path was not valid
  if (path == nullptr)
    return returnBitmask;

  kodi::vfs::FileStatus statFile;
  if (!kodi::vfs::StatFile(path, statFile))
    return returnBitmask;

  // Set bitmask flags
  returnBitmask |= RETRO_VFS_STAT_IS_VALID;

  if (statFile.GetIsDirectory())
    returnBitmask |= RETRO_VFS_STAT_IS_DIRECTORY;

  if (statFile.GetIsCharacter())
    returnBitmask |= RETRO_VFS_STAT_IS_CHARACTER_SPECIAL;

  if (size != nullptr)
    *size = static_cast<int64_t>(statFile.GetSize());

  // Return bitmask on success
  return returnBitmask;
}

int CFrontendVFS::SetReadOnly(const char *path, int readonly)
{
  // Kodi's VFS can't change a file's permissions
  return -1;
}

int CFrontendVFS::GetModificationTime(const char *path, int64_t *mtime)
{
  if (path == nullptr)
    return -1;

  kodi::vfs::FileStatus statFile;
  if (!kodi::vfs::StatFile(path, statFile))
    return -1;

  if (mtime != nullptr)
    *mtime = static_cast<int64_t>(statFile.GetModificationTime());

  return 0;
}

int CFrontendVFS::SetModificationTime(const char *path, int64_t mtime)
{
  // Kodi's VFS can't set a modification time
  return -1;
}

retro_vfs_copy_handle *CFrontendVFS::CopyBegin(const char *src, const char *dst, unsigned flags)
{
  if (src == nullptr || dst == nullptr || *src == '\0' || *dst == '\0' || std::strcmp(src, dst) == 0)
    return nullptr;

  int64_t sourceSize = 0;
  const int sourceBitmask = StatPath(src, &sourceSize);
  if (!(sourceBitmask & RETRO_VFS_STAT_IS_VALID) ||
      (sourceBitmask & (RETRO_VFS_STAT_IS_DIRECTORY | RETRO_VFS_STAT_IS_CHARACTER_SPECIAL)))
    return nullptr;

  std::unique_ptr<CopyHandle> copyHandle(new CopyHandle{ dst });
  copyHandle->bytesTotal = sourceSize;
  copyHandle->source.reset(new kodi::vfs::CFile);

  // Opened before the destination is touched, so a failed copy can't cost
  // the file it would have replaced
  if (!copyHandle->source->OpenFile(src))
    return nullptr;

  const int destinationBitmask = StatPath(dst, nullptr);
  if (destinationBitmask & RETRO_VFS_STAT_IS_VALID)
  {
    if ((destinationBitmask & RETRO_VFS_STAT_IS_DIRECTORY) || !(flags & RETRO_VFS_COPY_OVERWRITE))
      return nullptr;

    if (!kodi::vfs::DeleteFile(dst))
      return nullptr;
  }
  else if (!CreateParentDirectories(dst))
  {
    return nullptr;
  }

  copyHandle->target.reset(new kodi::vfs::CFile);
  if (!copyHandle->target->OpenFileForWrite(dst, true))
  {
    copyHandle.reset();
    kodi::vfs::DeleteFile(dst);
    return nullptr;
  }

  return reinterpret_cast<retro_vfs_copy_handle*>(copyHandle.release());
}

int CFrontendVFS::CopyStep(retro_vfs_copy_handle *handle, int64_t max_bytes, int64_t *bytes_done, int64_t *bytes_total)
{
  if (handle == nullptr)
    return RETRO_VFS_COPY_FAILED;

  CopyHandle *copyHandle = reinterpret_cast<CopyHandle*>(handle);

  if (copyHandle->status == RETRO_VFS_COPY_RUNNING)
  {
    int64_t budget = max_bytes > 0 ? max_bytes : DEFAULT_COPY_STEP;
    copyHandle->buffer.resize(COPY_BUFFER_SIZE);

    while (budget > 0)
    {
      const size_t chunk = static_cast<size_t>(std::min<int64_t>(budget, copyHandle->buffer.size()));
      const ssize_t bytesRead = copyHandle->source->Read(copyHandle->buffer.data(), chunk);
      if (bytesRead == 0 && copyHandle->bytesDone >= copyHandle->bytesTotal)
      {
        copyHandle->source.reset();
        copyHandle->target.reset();
        copyHandle->status = RETRO_VFS_COPY_DONE;
        break;
      }

      // A source that ends before its size was reached fails the copy too
      if (bytesRead <= 0 || copyHandle->target->Write(copyHandle->buffer.data(), bytesRead) != bytesRead)
      {
        // Close both ends so the partial copy can be removed
        copyHandle->source.reset();
        copyHandle->target.reset();
        kodi::vfs::DeleteFile(copyHandle->destination);
        copyHandle->status = RETRO_VFS_COPY_FAILED;
        break;
      }

      copyHandle->bytesDone += bytesRead;
      budget -= bytesRead;
    }
  }

  if (bytes_done != nullptr)
    *bytes_done = copyHandle->bytesDone;
  if (bytes_total != nullptr)
    *bytes_total = copyHandle->bytesTotal;

  return copyHandle->status;
}

int CFrontendVFS::CopyClose(retro_vfs_copy_handle *handle)
{
  if (handle == nullptr)
    return -1;

  std::unique_ptr<CopyHandle> copyHandle(reinterpret_cast<CopyHandle*>(handle));

  if (copyHandle->status == RETRO_VFS_COPY_DONE)
    return 0;

  // A copy that is still running is cancelled
  if (copyHandle->status == RETRO_VFS_COPY_RUNNING)
  {
    copyHandle->source.reset();
    copyHandle->target.reset();
    kodi::vfs::DeleteFile(copyHandle->destination);
  }

  return -1;
}

bool CFrontendVFS::CreateParentDirectories(const std::string &path)
{
  std::string parent = kodi::vfs::GetDirectoryName(path);
  kodi::vfs::RemoveSlashAtEnd(parent);

  if (parent.empty() || parent == path || kodi::vfs::DirectoryExists(parent))
    return true;

  if (!CreateParentDirectories(parent))
    return false;

  return kodi::vfs::CreateDirectory(parent) || kodi::vfs::DirectoryExists(parent);
}

int CFrontendVFS::MakeDirectory(const char *dir)
{
  // Return -1 on unknown failure
  if (dir == nullptr)
    return -1;

  if (!kodi::vfs::CreateDirectory(dir))
  {
    // Return -2 if already exists
    if (kodi::vfs::DirectoryExists(dir))
      return 2;

    return -1;
  }

  // Return 0 on success
  return 0;
}

retro_vfs_dir_handle *CFrontendVFS::OpenDirectory(const char *dir, bool include_hidden)
{
  // Return NULL for error
  if (dir == nullptr)
    return nullptr;

  std::unique_ptr<DirectoryHandle> directoryHandle(new DirectoryHandle{ dir });

  // Return the opaque dir handle
  return reinterpret_cast<retro_vfs_dir_handle*>(directoryHandle.release());
}

bool CFrontendVFS::ReadDirectory(retro_vfs_dir_handle *dirstream)
{
  // What to return on error?
  if (dirstream == nullptr)
    return false;

  DirectoryHandle *directoryHandle = reinterpret_cast<DirectoryHandle*>(dirstream);

  if (!directoryHandle->bOpen)
  {
    if (kodi::vfs::GetDirectory(directoryHandle->path, "", directoryHandle->items))
    {
      directoryHandle->bOpen = true;

      // Simulate a read
      directoryHandle->currentPosition = directoryHandle->nextPosition = directoryHandle->items.begin();

      // Simulate a dir entry pointer increment
      if (directoryHandle->nextPosition != directoryHandle->items.end())
        ++directoryHandle->nextPosition;
    }
  }
  else
  {
    // Simulate a read
    directoryHandle->currentPosition = directoryHandle->nextPosition;

    // Simulate a dir entry pointer increment
    if (directoryHandle->nextPosition != directoryHandle->items.end())
      ++directoryHandle->nextPosition;
  }

  // Return false if already on the last entry
  if (directoryHandle->currentPosition == directoryHandle->items.end())
    return false;

  // Return true on success
  return true;
}

const char *CFrontendVFS::GetDirectoryName(retro_vfs_dir_handle *dirstream)
{
  // Return NULL for error
  if (dirstream == nullptr)
    return nullptr;

  DirectoryHandle *directoryHandle = reinterpret_cast<DirectoryHandle*>(dirstream);

  if (directoryHandle->currentPosition == directoryHandle->items.end())
    return nullptr;

  // The returned string pointer must be valid until the next call to
  // ReadDirectory() or CloseDirectory()
  return directoryHandle->currentPosition->Label().c_str();
}

bool CFrontendVFS::IsDirectory(retro_vfs_dir_handle *dirstream)
{
  // Return false on error
  if (dirstream == nullptr)
    return false;

  DirectoryHandle *directoryHandle = reinterpret_cast<DirectoryHandle*>(dirstream);

  if (directoryHandle->currentPosition == directoryHandle->items.end())
    return false;

  return directoryHandle->currentPosition->IsFolder();
}

int CFrontendVFS::CloseDirectory(retro_vfs_dir_handle *dirstream)
{
  // Return -1 on failure
  if (dirstream == nullptr)
    return -1;

  DirectoryHandle *directoryHandle = reinterpret_cast<DirectoryHandle*>(dirstream);

  delete directoryHandle;

  // Return 0 on success
  return 0;
}

int CFrontendVFS::DirectoryEntryStat(retro_vfs_dir_handle *dirstream, int64_t *size, int64_t *mtime)
{
  // Return 0 if there is no entry to report on
  if (dirstream == nullptr)
    return 0;

  DirectoryHandle *directoryHandle = reinterpret_cast<DirectoryHandle*>(dirstream);
  if (!directoryHandle->bOpen || directoryHandle->currentPosition == directoryHandle->items.end())
    return 0;

  // A copy, as CDirEntry::DateTime() isn't const
  kodi::vfs::CDirEntry entry = *directoryHandle->currentPosition;

  int returnBitmask = RETRO_VFS_STAT_IS_VALID;
  if (entry.IsFolder())
    returnBitmask |= RETRO_VFS_STAT_IS_DIRECTORY;

  if (size != nullptr)
    *size = entry.IsFolder() ? 0 : entry.Size();
  if (mtime != nullptr)
    *mtime = static_cast<int64_t>(entry.DateTime());

  return returnBitmask;
}
