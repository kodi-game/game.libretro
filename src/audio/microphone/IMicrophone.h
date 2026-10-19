/*
 *  Copyright (C) 2026 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>

namespace LIBRETRO
{
  /*!
   * \brief A microphone on the platform Kodi runs on
   */
  class IMicrophone
  {
  public:
    virtual ~IMicrophone() = default;

    /*!
     * \brief Start capturing mono 16-bit samples at the given rate
     */
    virtual bool Start(unsigned int rate) = 0;

    virtual void Stop() = 0;

    /*!
     * \brief Read up to count samples
     *
     * \return The number read, 0 when none are waiting, or -1 on error
     */
    virtual int Read(int16_t* samples, size_t count) = 0;
  };

  /*!
   * \brief Create a microphone, from whichever implementation the build chose
   *
   * \return nullptr on a platform without microphone support
   */
  std::unique_ptr<IMicrophone> CreateMicrophone();

  /*!
   * \brief Whether a microphone could capture now
   *
   * A core offered an interface that can't capture loses its own fallback, such
   * as a USB microphone it would open itself.
   */
  bool CanCapture();
} // namespace LIBRETRO
