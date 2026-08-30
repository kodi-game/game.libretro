/*
 *  Copyright (C) 2020-2024 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#pragma once

#include <stdint.h>
#include <string>

#include <kodi/addon-instance/Game.h>

namespace LIBRETRO
{
/*!
 * \brief The status code reported when no HTTP response was received at all
 */
constexpr unsigned int HTTP_STATUS_NO_RESPONSE = 0;

/*!
 * \brief Translate rc_client's unlock bitmask into the Game API's enum
 *
 * rc_client reports softcore and hardcore as independent bits. Kodi only needs
 * to know the strongest state the achievement has been earned in.
 */
GAME_RC_UNLOCK_STATE TranslateUnlockState(uint8_t unlocked);

/*!
 * \brief Extract the status code from an HTTP response line
 *
 * \param responseProtocol A line such as "HTTP/1.1 404 Not Found"
 *
 * \return The status code, or HTTP_STATUS_NO_RESPONSE if it can't be read
 */
unsigned int ParseHttpStatus(const std::string& responseProtocol);
} // namespace LIBRETRO
