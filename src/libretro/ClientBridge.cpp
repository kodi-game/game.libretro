/*
 *  Copyright (C) 2014-2021 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#include "ClientBridge.h"

#include "GameInfoLoader.h"

// Causing errors with std::numeric_limits<int>::max()
#ifdef max
#undef max
#endif

#include <limits>

using namespace LIBRETRO;

namespace
{
constexpr unsigned int MAX_PATH = 4096;
}

CClientBridge::CClientBridge()
  : m_retro_keyboard_event(nullptr),
    m_retro_hw_context_reset(nullptr),
    m_retro_hw_context_destroy(nullptr),
    m_retro_audio_set_state_callback(nullptr),
    m_retro_audio_callback(nullptr),
    m_retro_frame_time_callback(nullptr),
    m_retro_set_eject_state(nullptr),
    m_retro_get_eject_state(nullptr),
    m_retro_get_image_index(nullptr),
    m_retro_set_image_index(nullptr),
    m_retro_get_num_images(nullptr),
    m_retro_replace_image_index(nullptr),
    m_retro_add_image_index(nullptr),
    m_retro_set_initial_image(nullptr),
    m_retro_get_image_path(nullptr),
    m_retro_get_image_label(nullptr)
{
}

GAME_ERROR CClientBridge::KeyboardEvent(bool down, unsigned keycode, uint32_t character, uint16_t key_modifiers)
{
  if (!m_retro_keyboard_event)
    return GAME_ERROR_FAILED;

  m_retro_keyboard_event(down, keycode, character, key_modifiers);

  return GAME_ERROR_NO_ERROR;
}

GAME_ERROR CClientBridge::HwContextReset(void)
{
  if (!m_retro_hw_context_reset)
    return GAME_ERROR_FAILED;

  m_retro_hw_context_reset();

  return GAME_ERROR_NO_ERROR;
}

GAME_ERROR CClientBridge::HwContextDestroy(void)
{
  if (!m_retro_hw_context_destroy)
    return GAME_ERROR_FAILED;

  m_retro_hw_context_destroy();

  return GAME_ERROR_NO_ERROR;
}

GAME_ERROR CClientBridge::AudioEnable(bool enabled)
{
  if (!m_retro_audio_set_state_callback)
    return GAME_ERROR_FAILED;

  m_retro_audio_set_state_callback(enabled);

  return GAME_ERROR_NO_ERROR;
}

GAME_ERROR CClientBridge::AudioAvailable(void)
{
  if (!m_retro_audio_callback)
    return GAME_ERROR_FAILED;

  m_retro_audio_callback();

  return GAME_ERROR_NO_ERROR;
}

GAME_ERROR CClientBridge::FrameTime(int64_t usec)
{
  if (!m_retro_frame_time_callback)
    return GAME_ERROR_FAILED;

  m_retro_frame_time_callback(usec);

  return GAME_ERROR_NO_ERROR;
}

GAME_ERROR CClientBridge::SetEjectState(bool ejected)
{
  if (!m_retro_set_eject_state)
    return GAME_ERROR_FAILED;

  if (!m_retro_set_eject_state(ejected))
    return GAME_ERROR_FAILED;

  return GAME_ERROR_NO_ERROR;
}

bool CClientBridge::GetEjectState()
{
  if (!m_retro_get_eject_state)
    return false;

  return m_retro_get_eject_state();
}

unsigned int CClientBridge::GetImageIndex()
{
  if (!m_retro_get_image_index)
    return 0;

  return m_retro_get_image_index();
}

GAME_ERROR CClientBridge::SetImageIndex(unsigned int imageIndex)
{
  if (!m_retro_set_image_index)
    return GAME_ERROR_FAILED;

  if (!m_retro_set_image_index(imageIndex))
    return GAME_ERROR_FAILED;

  return GAME_ERROR_NO_ERROR;
}

unsigned int CClientBridge::GetImageCount()
{
  if (!m_retro_get_num_images)
    return 0;

  return m_retro_get_num_images();
}

GAME_ERROR CClientBridge::ReplaceImageIndex(unsigned int imageIndex, const std::string& filePath)
{
  if (!m_retro_replace_image_index)
    return GAME_ERROR_FAILED;

  retro_game_info gameInfo;

  CGameInfoLoader gameInfoLoader(filePath, false);
  gameInfoLoader.GetPathStruct(gameInfo);

  if (!m_retro_replace_image_index(imageIndex, &gameInfo))
    return GAME_ERROR_FAILED;

  return GAME_ERROR_NO_ERROR;
}

GAME_ERROR CClientBridge::RemoveImageIndex(unsigned int imageIndex)
{
  if (!m_retro_replace_image_index)
    return GAME_ERROR_FAILED;

  if (!m_retro_replace_image_index(imageIndex, nullptr))
    return GAME_ERROR_FAILED;

  return GAME_ERROR_NO_ERROR;
}

GAME_ERROR CClientBridge::AddImageIndex()
{
  if (!m_retro_add_image_index)
    return GAME_ERROR_FAILED;

  if (!m_retro_add_image_index())
    return GAME_ERROR_FAILED;

  return GAME_ERROR_NO_ERROR;
}

GAME_ERROR CClientBridge::SetInitialImage(unsigned int imageIndex, const std::string& filePath)
{
  if (!m_retro_set_initial_image)
    return GAME_ERROR_FAILED;

  if (!m_retro_set_initial_image(imageIndex, filePath.c_str()))
    return GAME_ERROR_FAILED;

  return GAME_ERROR_NO_ERROR;
}

std::string CClientBridge::GetImagePath(unsigned int imageIndex)
{
  if (!m_retro_get_image_path)
    return "";

  char imagePath[MAX_PATH] = {};
  if (!m_retro_get_image_path(imageIndex, imagePath, sizeof(imagePath)))
    return "";

  return imagePath;
}

std::string CClientBridge::GetImageLabel(unsigned int imageIndex)
{
  if (!m_retro_get_image_label)
    return "";

  char imageLabel[MAX_PATH] = {};
  if (!m_retro_get_image_label(imageIndex, imageLabel, sizeof(imageLabel)))
    return "";

  return imageLabel;
}
