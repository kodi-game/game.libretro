/*
 *  Copyright (C) 2026 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#include "LibretroMemory.h"

using namespace LIBRETRO;

void CLibretroMemory::Initialize(GetMemoryData getData, GetMemorySize getSize)
{
  Deinitialize();
  m_getData = getData;
  m_getSize = getSize;
}

void CLibretroMemory::Deinitialize()
{
  m_map.Clear();
  m_getData = nullptr;
  m_getSize = nullptr;
  m_contentActive = false;
  m_contentMap = false;
  ++m_generation;
}

void CLibretroMemory::BeginContent()
{
  m_contentActive = true;
  ++m_generation; // Standard memory can change even without SET_MEMORY_MAPS.
}

void CLibretroMemory::EndContent()
{
  // libretro.h promises ptr validity for the session, not across content unload.
  // RetroArch tears content/core down together. For our retained core, preserve
  // an unreplaced init map; discard load/runtime maps on unload or failed load.
  // Never resurrect a superseded init map: replacement does not promise that
  // its old pointers survive. See tests/MemoryModel.md for upstream evidence.
  if (m_contentMap)
    m_map.Clear();
  m_contentMap = false;
  m_contentActive = false;
  ++m_generation;
}

bool CLibretroMemory::SetMemoryMap(const retro_memory_map& map)
{
  if (!m_map.Initialize(map))
    return false;

  m_contentMap = m_contentActive;
  ++m_generation;
  return true;
}

bool CLibretroMemory::GetMemory(unsigned type, uint8_t*& data, size_t& size) const
{
  data = nullptr;
  size = 0;
  if (!m_contentActive || !m_getData || !m_getSize)
    return false;

  data = static_cast<uint8_t*>(m_getData(type));
  size = m_getSize(type);
  return data != nullptr && size != 0;
}
