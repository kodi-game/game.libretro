/*
 *  Copyright (C) 2020-2024 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#include "Base64.h"

#include <stdint.h>

namespace LIBRETRO
{
std::string Base64Encode(const std::string& data)
{
  static constexpr char alphabet[] =
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

  std::string encoded;
  encoded.reserve(((data.size() + 2) / 3) * 4);

  size_t i = 0;
  while (i + 2 < data.size())
  {
    const uint32_t triple = (static_cast<uint8_t>(data[i]) << 16) |
                            (static_cast<uint8_t>(data[i + 1]) << 8) |
                            static_cast<uint8_t>(data[i + 2]);
    encoded += alphabet[(triple >> 18) & 0x3F];
    encoded += alphabet[(triple >> 12) & 0x3F];
    encoded += alphabet[(triple >> 6) & 0x3F];
    encoded += alphabet[triple & 0x3F];
    i += 3;
  }

  if (i < data.size())
  {
    uint32_t triple = static_cast<uint8_t>(data[i]) << 16;
    const bool haveTwo = (i + 1 < data.size());
    if (haveTwo)
      triple |= static_cast<uint8_t>(data[i + 1]) << 8;

    encoded += alphabet[(triple >> 18) & 0x3F];
    encoded += alphabet[(triple >> 12) & 0x3F];
    encoded += haveTwo ? alphabet[(triple >> 6) & 0x3F] : '=';
    encoded += '=';
  }

  return encoded;
}
} // namespace LIBRETRO
