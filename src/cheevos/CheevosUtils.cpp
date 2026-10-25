/*
 *  Copyright (C) 2020-2024 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#include "CheevosUtils.h"

#include <cstdlib>

#define RC_CLIENT_SUPPORTS_HASH
#include <rcheevos/rc_client.h>

namespace LIBRETRO
{
GAME_RC_UNLOCK_STATE TranslateUnlockState(uint8_t unlocked)
{
  if ((unlocked & RC_CLIENT_ACHIEVEMENT_UNLOCKED_HARDCORE) != 0)
    return GAME_RC_UNLOCK_STATE_HARDCORE;
  if ((unlocked & RC_CLIENT_ACHIEVEMENT_UNLOCKED_SOFTCORE) != 0)
    return GAME_RC_UNLOCK_STATE_SOFTCORE;

  return GAME_RC_UNLOCK_STATE_LOCKED;
}

#if defined(HAVE_GAME_RC_ACHIEVEMENT_SETS)
GAME_RC_ACHIEVEMENT_TYPE TranslateAchievementType(uint8_t type)
{
  switch (type)
  {
    case RC_CLIENT_ACHIEVEMENT_TYPE_MISSABLE:
      return GAME_RC_ACHIEVEMENT_TYPE_MISSABLE;
    case RC_CLIENT_ACHIEVEMENT_TYPE_PROGRESSION:
      return GAME_RC_ACHIEVEMENT_TYPE_PROGRESSION;
    case RC_CLIENT_ACHIEVEMENT_TYPE_WIN:
      return GAME_RC_ACHIEVEMENT_TYPE_WIN;
    default:
      return GAME_RC_ACHIEVEMENT_TYPE_STANDARD;
  }
}
#endif

unsigned int ParseHttpStatus(const std::string& responseProtocol)
{
  const size_t start = responseProtocol.find(' ');
  if (start == std::string::npos)
    return HTTP_STATUS_NO_RESPONSE;

  return static_cast<unsigned int>(std::strtoul(responseProtocol.c_str() + start + 1, nullptr, 10));
}
} // namespace LIBRETRO
