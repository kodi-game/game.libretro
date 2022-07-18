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
   * Provide a bridge for frontend callbacks given to the client. The bridging
   * function is a simple wrapper that invokes the callback in the frontend.
   * These functions are given to the libretro client in EnvironmentCallback().
   */
  class CFrontendBridge
  {
  public:
    // Forward to Kodi add-on API
    static void LogFrontend(retro_log_level level, const char *fmt, ...);

    // Forward to Kodi game API
    static void VideoRefresh(const void* data, unsigned int width, unsigned int height, size_t pitch);
    static void AudioFrame(int16_t left, int16_t right);
    static size_t AudioFrames(const int16_t* data, size_t frames);
    static void InputPoll(void);
    static int16_t InputState(unsigned int port, unsigned int device, unsigned int index, unsigned int id);
    static uintptr_t HwGetCurrentFramebuffer(void);
    static retro_proc_address_t HwGetProcAddress(const char *sym);
    static bool RumbleSetState(unsigned port, retro_rumble_effect effect, uint16_t strength);
    static void LedSetState(int led, int state);
    static bool SensorSetState(unsigned port, retro_sensor_action action, unsigned rate);
    static float SensorGetInput(unsigned port, unsigned id);
    static bool StartCamera(void);
    static void StopCamera(void);
    static retro_time_t PerfGetTimeUsec(void);
    static retro_perf_tick_t PerfGetCounter(void);
    static uint64_t PerfGetCpuFeatures(void);
    static void PerfLog(void);
    static void PerfRegister(retro_perf_counter *counter);
    static void PerfStart(retro_perf_counter *counter);
    static void PerfStop(retro_perf_counter *counter);
    static bool StartLocation(void);
    static void StopLocation(void);
    static bool GetLocation(double *lat, double *lon, double *horiz_accuracy, double *vert_accuracy);
    static void SetLocationInterval(unsigned interval_ms, unsigned interval_distance);
    static void LocationInitialized(void);
    static void LocationDeinitialized(void);

    // Forward to Kodi VFS API
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
