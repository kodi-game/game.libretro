/*
 *  Copyright (C) 2014-2021 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#include "FrontendBridge.h"
#include "LibretroEnvironment.h"
#include "LibretroTranslator.h"
#include "input/ButtonMapper.h"
#include "input/InputManager.h"
#include "client.h"

#include <algorithm>
#include <assert.h>
#include <kodi/Filesystem.h>
#include <kodi/General.h>
#include <limits>
#include <stdio.h>

using namespace LIBRETRO;

#define S16NE_FRAMESIZE  4 // int16 L + int16 R

#define MAX_RUMBLE_STRENGTH  0xffff

#ifndef CONSTRAIN
  // Credit: https://stackoverflow.com/questions/8941262/constrain-function-port-from-arduino
  #define CONSTRAIN(amt, low, high)  ((amt) < (low) ? (low) : ((amt) > (high) ? (high) : (amt)))
#endif

void CFrontendBridge::LogFrontend(retro_log_level level, const char *fmt, ...)
{
  AddonLog xbmcLevel;
  switch (level)
  {
  case RETRO_LOG_DEBUG: xbmcLevel = ADDON_LOG_DEBUG; break;
  case RETRO_LOG_INFO:  xbmcLevel = ADDON_LOG_INFO;  break;
  case RETRO_LOG_WARN:  xbmcLevel = ADDON_LOG_ERROR; break;
  case RETRO_LOG_ERROR: xbmcLevel = ADDON_LOG_ERROR; break;
  default:              xbmcLevel = ADDON_LOG_ERROR; break;
  }

  char buffer[16384];
  va_list args;
  va_start(args, fmt);
  vsprintf(buffer, fmt, args);
  va_end(args);

  kodi::Log(xbmcLevel, buffer);
}

void CFrontendBridge::VideoRefresh(const void* data, unsigned int width, unsigned int height, size_t pitch)
{
  if (data == RETRO_HW_FRAME_BUFFER_VALID)
  {
    CLibretroEnvironment::Get().Video().RenderHwFrame();
  }
  else if (data == nullptr)
  {
    // Libretro is sending a frame dupe command
    CLibretroEnvironment::Get().Video().DupeFrame();
  }
  else
  {
    CLibretroEnvironment::Get().Video().AddFrame(static_cast<const uint8_t*>(data),
                                                 static_cast<unsigned int>(pitch * height),
                                                 width,
                                                 height,
                                                 CLibretroEnvironment::Get().GetVideoFormat(),
                                                 CLibretroEnvironment::Get().GetVideoRotation());
  }
}

void CFrontendBridge::AudioFrame(int16_t left, int16_t right)
{
  CLibretroEnvironment::Get().Audio().AddFrame_S16NE(left, right);
}

size_t CFrontendBridge::AudioFrames(const int16_t* data, size_t frames)
{
  CLibretroEnvironment::Get().Audio().AddFrames_S16NE(reinterpret_cast<const uint8_t*>(data),
                                                      static_cast<unsigned int>(frames * S16NE_FRAMESIZE));

  return frames;
}

void CFrontendBridge::InputPoll(void)
{
  // Not needed
}

int16_t CFrontendBridge::InputState(unsigned int port, unsigned int device, unsigned int index, unsigned int id)
{
  int16_t inputState = 0;

  // According to libretro.h, device should already be masked, but just in case
  device &= RETRO_DEVICE_MASK;

  switch (device)
  {
  case RETRO_DEVICE_JOYPAD:
  case RETRO_DEVICE_KEYBOARD:
    inputState = CInputManager::Get().ButtonState(device, port, id) ? 1 : 0;
    break;

  case RETRO_DEVICE_MOUSE:
  case RETRO_DEVICE_LIGHTGUN:
    static_assert(RETRO_DEVICE_ID_MOUSE_X == RETRO_DEVICE_ID_LIGHTGUN_X, "RETRO_DEVICE_ID_MOUSE_X != RETRO_DEVICE_ID_LIGHTGUN_X");
    static_assert(RETRO_DEVICE_ID_MOUSE_Y == RETRO_DEVICE_ID_LIGHTGUN_Y, "RETRO_DEVICE_ID_MOUSE_Y != RETRO_DEVICE_ID_LIGHTGUN_Y");

    switch (id)
    {
      case RETRO_DEVICE_ID_MOUSE_X:
        inputState = CInputManager::Get().DeltaX(device, port);
        break;
      case RETRO_DEVICE_ID_MOUSE_Y:
        inputState = CInputManager::Get().DeltaY(device, port);
        break;
      default:
      {
        inputState = CInputManager::Get().ButtonState(device, port, id) ? 1 : 0;
        break;
      }
    }
    break;

  case RETRO_DEVICE_ANALOG:
  {
    float value = 0.0f; // Axis value between -1 and 1

    if (index == RETRO_DEVICE_INDEX_ANALOG_BUTTON)
    {
      value = CInputManager::Get().AnalogButtonState(port, id);
    }
    else
    {
      float x, y;
      if (CInputManager::Get().AnalogStickState(port, index, x, y))
      {
        if (id == RETRO_DEVICE_ID_ANALOG_X)
        {
          value = x;
        }
        else if (id == RETRO_DEVICE_ID_ANALOG_Y)
        {
          value = -y; // y axis is inverted
        }
      }
    }

    const float normalized = (value + 1.0f) / 2.0f;
    const int clamped = std::max(0, std::min(0xffff, static_cast<int>(normalized * 0xffff)));
    inputState = clamped - 0x8000;
    break;
  }

  case RETRO_DEVICE_POINTER:
  {
    float x, y;
    if (CInputManager::Get().AbsolutePointerState(port, index, x, y))
    {
      if (id == RETRO_DEVICE_ID_POINTER_X)
      {
        inputState = (int)(x * 0x7fff);
      }
      else if (id == RETRO_DEVICE_ID_POINTER_Y)
      {
        inputState = (int)(y * 0x7fff);
      }
      else if (id == RETRO_DEVICE_ID_POINTER_PRESSED)
      {
        inputState = 1;
      }
    }
    break;
  }

  default:
    break;
  }

  return inputState;
}

uintptr_t CFrontendBridge::HwGetCurrentFramebuffer(void)
{
  if (!CLibretroEnvironment::Get().GetAddon())
    return 0;

  return CLibretroEnvironment::Get().Video().GetHwFramebuffer();
}

retro_proc_address_t CFrontendBridge::HwGetProcAddress(const char *sym)
{
  if (!CLibretroEnvironment::Get().GetAddon())
    return nullptr;

  return CLibretroEnvironment::Get().GetAddon()->HwGetProcAddress(sym);
}

bool CFrontendBridge::RumbleSetState(unsigned int port, retro_rumble_effect effect, uint16_t strength)
{
  if (!CLibretroEnvironment::Get().GetAddon())
    return false;

  std::string controllerId  = CInputManager::Get().ControllerID(port);
  std::string address       = CInputManager::Get().GetAddress(port);
  std::string libretroMotor = LibretroTranslator::GetMotorName(effect);
  std::string featureName   = CButtonMapper::Get().GetControllerFeature(controllerId, libretroMotor);
  float       magnitude     = static_cast<float>(strength) / MAX_RUMBLE_STRENGTH;

  if (controllerId.empty() || address.empty() || featureName.empty())
    return false;

  game_input_event eventStruct;
  eventStruct.type            = GAME_INPUT_EVENT_MOTOR;
  eventStruct.controller_id   = controllerId.c_str();
  eventStruct.port_address    = address.c_str();
  eventStruct.port_type       = GAME_PORT_CONTROLLER;
  eventStruct.feature_name    = featureName.c_str();
  eventStruct.motor.magnitude = CONSTRAIN(magnitude, 0.0f, 1.0f);

  CLibretroEnvironment::Get().GetAddon()->KodiInputEvent(eventStruct);
  return true;
}

bool CFrontendBridge::SensorSetState(unsigned port, retro_sensor_action action, unsigned rate)
{
  const bool bEnabled = (action == RETRO_SENSOR_ACCELEROMETER_ENABLE);

  CInputManager::Get().EnableAnalogSensors(port, bEnabled);

  return true;
}

float CFrontendBridge::SensorGetInput(unsigned port, unsigned id)
{
  float axisState = 0.0f;

  float x, y, z;
  if (CInputManager::Get().AccelerometerState(port, x, y, z))
  {
    switch (id)
    {
    case RETRO_SENSOR_ACCELEROMETER_X:
      axisState = x;
      break;
    case RETRO_SENSOR_ACCELEROMETER_Y:
      axisState = y;
      break;
    case RETRO_SENSOR_ACCELEROMETER_Z:
      axisState = z;
      break;
    default:
      break;
    }
  }

  return axisState;
}

bool CFrontendBridge::StartCamera(void)
{
  return false; // Not implemented
}

void CFrontendBridge::StopCamera(void)
{
  // Not implemented
}

retro_time_t CFrontendBridge::PerfGetTimeUsec(void)
{
  return 0; // Not implemented
}

retro_perf_tick_t CFrontendBridge::PerfGetCounter(void)
{
  return 0; // Not implemented
}

uint64_t CFrontendBridge::PerfGetCpuFeatures(void)
{
  return 0; // Not implemented
}

void CFrontendBridge::PerfLog(void)
{
  // Not implemented
}

void CFrontendBridge::PerfRegister(retro_perf_counter *counter)
{
  // Not implemented
}

void CFrontendBridge::PerfStart(retro_perf_counter *counter)
{
  // Not implemented
}

void CFrontendBridge::PerfStop(retro_perf_counter *counter)
{
  // Not implemented
}

bool CFrontendBridge::StartLocation(void)
{
  return false; // Not implemented
}

void CFrontendBridge::StopLocation(void)
{
  // Not implemented
}

bool CFrontendBridge::GetLocation(double *lat, double *lon, double *horiz_accuracy, double *vert_accuracy)
{
  return false; // Not implemented
}

void CFrontendBridge::SetLocationInterval(unsigned interval_ms, unsigned interval_distance)
{
  // Not implemented
}

void CFrontendBridge::LocationInitialized(void)
{
  // Not implemented
}

void CFrontendBridge::LocationDeinitialized(void)
{
  // Not implemented
}

const char *CFrontendBridge::GetPath(retro_vfs_file_handle *stream)
{
  if (stream == nullptr)
    return "";

  FileHandle *fileHandle = reinterpret_cast<FileHandle*>(stream);

  return fileHandle->path.c_str();
}

retro_vfs_file_handle *CFrontendBridge::OpenFile(const char *path, unsigned mode, unsigned hints)
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
    //flags &= ADDDON_READ_TRUNCATED;

    if (hints & RETRO_VFS_FILE_ACCESS_HINT_FREQUENT_ACCESS)
      flags &= ADDON_READ_CACHED;

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

int CFrontendBridge::CloseFile(retro_vfs_file_handle *stream)
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

int64_t CFrontendBridge::FileSize(retro_vfs_file_handle *stream)
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

int64_t CFrontendBridge::GetPosition(retro_vfs_file_handle *stream)
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

int64_t CFrontendBridge::Seek(retro_vfs_file_handle *stream, int64_t offset, int seek_position)
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

  const int64_t newOffset = fileHandle->file->Seek(offset, whence);

  if (newOffset < 0)
    return -1;

  // Return the resulting offset location as measured in bytes from the
  // beginning of the file
  return newOffset;
}

int64_t CFrontendBridge::ReadFile(retro_vfs_file_handle *stream, void *s, uint64_t len)
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

int64_t CFrontendBridge::WriteFile(retro_vfs_file_handle *stream, const void *s, uint64_t len)
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

int CFrontendBridge::FlushFile(retro_vfs_file_handle *stream)
{
  // Return -1 on failure
  if (stream == nullptr)
    return -1;

  FileHandle *fileHandle = reinterpret_cast<FileHandle*>(stream);

  fileHandle->file->Flush();

  // Return 0 on success
  return 0;
}

int CFrontendBridge::RemoveFile(const char *path)
{
  // Return -1 on failure
  if (path == nullptr)
    return -1;

  if (!kodi::vfs::DeleteFile(path))
    return -1;

  // Return 0 on success
  return 0;
}

int CFrontendBridge::RenameFile(const char *old_path, const char *new_path)
{
  // Return -1 on failure
  if (old_path == nullptr || new_path == nullptr)
    return -1;

  if (!kodi::vfs::RenameFile(old_path, new_path))
    return -1;

  // Return 0 on success
  return 0;
}

int64_t CFrontendBridge::Truncate(retro_vfs_file_handle *stream, int64_t length)
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

int CFrontendBridge::Stat(const char *path, int32_t *size)
{
  int returnBitmask = 0;

  // Return mask with no flags set if the path was not valid
  if (path == nullptr)
    return returnBitmask;

  kodi::vfs::FileStatus statFile;
  if (!kodi::vfs::StatFile(path, statFile))
    return returnBitmask;

  // Set bitmask flags
  returnBitmask &= RETRO_VFS_STAT_IS_VALID;

  if (statFile.GetIsDirectory())
    returnBitmask &= RETRO_VFS_STAT_IS_DIRECTORY;

  if (statFile.GetIsCharacter())
    returnBitmask &= RETRO_VFS_STAT_IS_CHARACTER_SPECIAL;

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

int CFrontendBridge::MakeDirectory(const char *dir)
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

retro_vfs_dir_handle *CFrontendBridge::OpenDirectory(const char *dir, bool include_hidden)
{
  // Return NULL for error
  if (dir == nullptr)
    return nullptr;

  std::unique_ptr<DirectoryHandle> directoryHandle(new DirectoryHandle{ dir });

  // Return the opaque dir handle
  return reinterpret_cast<retro_vfs_dir_handle*>(directoryHandle.release());
}

bool CFrontendBridge::ReadDirectory(retro_vfs_dir_handle *dirstream)
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

const char *CFrontendBridge::GetDirectoryName(retro_vfs_dir_handle *dirstream)
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

bool CFrontendBridge::IsDirectory(retro_vfs_dir_handle *dirstream)
{
  // Return false on error
  if (dirstream == nullptr)
    return false;

  DirectoryHandle *directoryHandle = reinterpret_cast<DirectoryHandle*>(dirstream);

  if (directoryHandle->currentPosition == directoryHandle->items.end())
    return false;

  return directoryHandle->currentPosition->IsFolder();
}

int CFrontendBridge::CloseDirectory(retro_vfs_dir_handle *dirstream)
{
  // Return -1 on failure
  if (dirstream == nullptr)
    return -1;

  DirectoryHandle *directoryHandle = reinterpret_cast<DirectoryHandle*>(dirstream);

  delete directoryHandle;

  // Return 0 on success
  return 0;
}
