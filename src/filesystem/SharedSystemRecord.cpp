/*
 *  Copyright (C) 2026 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#include "SharedSystemRecord.h"

#include <cstdio>
#include <sstream>
#include <utility>

using namespace LIBRETRO;

namespace
{
const std::string SHARED_KEY = "shared ";
const std::string FINGERPRINT_KEY = "fingerprint ";
constexpr size_t HASH_DIGITS = 16;

bool StartsWith(const std::string& text, const std::string& prefix)
{
  return text.compare(0, prefix.size(), prefix) == 0;
}

bool ParseHash(const std::string& text, uint64_t& value)
{
  if (text.size() != HASH_DIGITS)
    return false;

  value = 0;
  for (const char digit : text)
  {
    value <<= 4;
    if (digit >= '0' && digit <= '9')
      value |= static_cast<uint64_t>(digit - '0');
    else if (digit >= 'a' && digit <= 'f')
      value |= static_cast<uint64_t>(digit - 'a' + 10);
    else
      return false;
  }
  return true;
}

std::string FormatHash(uint64_t value)
{
  char text[HASH_DIGITS + 1];
  std::snprintf(text, sizeof(text), "%016llx", static_cast<unsigned long long>(value));
  return text;
}
} // namespace

bool CSharedSystemRecord::Parse(const std::string& text)
{
  *this = CSharedSystemRecord();

  // The shared folder's line, the fingerprint's, then "<hash> <path>" for each copy
  CSharedSystemRecord record;
  std::istringstream lines(text);
  std::string line;

  if (!std::getline(lines, line) || !StartsWith(line, SHARED_KEY))
    return false;
  record.sharedDirectory = line.substr(SHARED_KEY.size());

  if (!std::getline(lines, line) || !StartsWith(line, FINGERPRINT_KEY) ||
      !ParseHash(line.substr(FINGERPRINT_KEY.size()), record.fingerprint))
    return false;

  while (std::getline(lines, line))
  {
    uint64_t hash = 0;
    if (line.size() <= HASH_DIGITS + 1 || line[HASH_DIGITS] != ' ' ||
        !ParseHash(line.substr(0, HASH_DIGITS), hash))
      return false;
    record.copies[line.substr(HASH_DIGITS + 1)] = hash;
  }

  *this = std::move(record);
  return true;
}

std::string CSharedSystemRecord::Format() const
{
  std::string text = SHARED_KEY + sharedDirectory + "\n";
  text += FINGERPRINT_KEY + FormatHash(fingerprint) + "\n";
  for (const auto& copy : copies)
    text += FormatHash(copy.second) + " " + copy.first + "\n";
  return text;
}

SharedFileAction LIBRETRO::DecideSharedFile(const uint64_t* target,
                                            uint64_t source,
                                            const uint64_t* copied)
{
  if (target == nullptr)
    return SharedFileAction::COPY;
  if (*target == source)
    return SharedFileAction::NONE;
  if (copied != nullptr && *target == *copied)
    return SharedFileAction::REPLACE;
  return SharedFileAction::KEEP;
}

uint64_t LIBRETRO::HashBytes(const void* data, size_t size, uint64_t hash)
{
  const auto* bytes = static_cast<const uint8_t*>(data);
  for (size_t i = 0; i < size; ++i)
  {
    hash ^= bytes[i];
    hash *= 1099511628211ULL;
  }
  return hash;
}
