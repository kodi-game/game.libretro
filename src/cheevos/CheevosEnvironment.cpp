/*
 *  Copyright (C) 2020 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#include "CheevosEnvironment.h"
#include "CheevosFrontendBridge.h"

#include <rcheevos/rhash.h>

using namespace LIBRETRO;

CCheevosEnvironment& CCheevosEnvironment::Get(void)
{
  static CCheevosEnvironment _instance;
  return _instance;
}

void CCheevosEnvironment::Initialize()
{
  static rc_hash_filereader fileReader = {
    CCheevosFrontendBridge::OpenFile,
    CCheevosFrontendBridge::Seek,
    CCheevosFrontendBridge::GetPosition,
    CCheevosFrontendBridge::ReadFile,
    CCheevosFrontendBridge::CloseFile,
  };

  rc_hash_init_custom_filereader(&fileReader);
}

void CCheevosEnvironment::Deinitialize()
{
}
