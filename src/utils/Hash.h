/*
 *  Copyright (C) 2026 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#pragma once

#include <cstddef>
#include <cstdint>

namespace LIBRETRO
{
//! The hash 64-bit FNV-1a starts from
constexpr uint64_t FNV1A_64_OFFSET_BASIS = 14695981039346656037ULL;

/*!
 * \brief Hash bytes with 64-bit FNV-1a
 *
 * \param data The bytes to hash
 * \param size How many bytes there are
 * \param hash The hash to carry on from, so several pieces hash as one
 *
 * \return The hash of the bytes
 */
uint64_t HashBytes(const void* data, size_t size, uint64_t hash = FNV1A_64_OFFSET_BASIS);
} // namespace LIBRETRO
