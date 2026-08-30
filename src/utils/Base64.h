/*
 *  Copyright (C) 2020-2024 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#pragma once

#include <string>

namespace LIBRETRO
{
/*!
 * \brief Encode data as base64
 *
 * Kodi's "postdata" protocol option is base64-decoded on receipt, so the body
 * has to be encoded before it is handed over. The add-on SDK exposes no base64
 * helper, hence this one.
 */
std::string Base64Encode(const std::string& data);
} // namespace LIBRETRO
