/*
 *  Copyright (C) 2026 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#include "FileMapping.h"

#if defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__)
#define HAS_FILE_MAPPING 1
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#if defined(__linux__)
#include <sys/vfs.h>
#else
#include <sys/mount.h>
#include <sys/param.h>
#endif
#endif

using namespace LIBRETRO;

#if defined(HAS_FILE_MAPPING)
namespace
{
bool IsLocalFilesystem(int fd)
{
  struct statfs fs;
  if (fstatfs(fd, &fs) != 0)
    return false;

#if defined(__linux__)
  switch (static_cast<uint32_t>(fs.f_type))
  {
    case 0x6969: // NFS
    case 0x517B: // SMB
    case 0xFF534D42: // CIFS
    case 0xFE534D42: // SMB2
    case 0x01021997: // 9P
    case 0x5346414F: // AFS
    case 0x00C36400: // Ceph
    case 0x65735546: // FUSE, which sshfs and desktop network shares use
      return false;
    default:
      return true;
  }
#else
  return (fs.f_flags & MNT_LOCAL) != 0;
#endif
}
} // namespace
#endif

CFileMapping::~CFileMapping()
{
  Unmap();
}

bool CFileMapping::Map(const std::string& path)
{
  Unmap();

#if defined(HAS_FILE_MAPPING)
  const int fd = open(path.c_str(), O_RDONLY | O_CLOEXEC);
  if (fd < 0)
    return false;

  void* data = MAP_FAILED;
  size_t size = 0;
  struct stat st;
  if (fstat(fd, &st) == 0 && S_ISREG(st.st_mode) && st.st_size > 0 && IsLocalFilesystem(fd))
  {
    size = static_cast<size_t>(st.st_size);
    // A file too big to address is left unmapped
    if (static_cast<off_t>(size) == st.st_size)
      data = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
  }

  // The mapping keeps its own hold on the file
  close(fd);

  if (data == MAP_FAILED)
    return false;

  m_data = static_cast<uint8_t*>(data);
  m_size = size;
  return true;
#else
  (void)path;
  return false;
#endif
}

void CFileMapping::Unmap()
{
#if defined(HAS_FILE_MAPPING)
  if (m_data != nullptr)
    munmap(m_data, m_size);
#endif
  m_data = nullptr;
  m_size = 0;
}
