/*
 *  Copyright (C) 2014-2021 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#include "FrontendVFS.h"

#include <kodi/Filesystem.h>
#include <limits>

using namespace LIBRETRO;

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

  // Set file size
  if (size != nullptr)
  {
    // What to return if size > 2 GiB?
    if (statFile.GetSize() <= std::numeric_limits<int32_t>::max())
      *size = static_cast<int32_t>(statFile.GetSize());
  }

  // Return bitmask on success
  return returnBitmask;
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
