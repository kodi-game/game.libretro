/*
 *  Copyright (C) 2026 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#pragma once

#include <libretro-common/libretro.h>

#include <cstdint>

namespace LIBRETRO
{
  /*!
   * \brief Runs a Vulkan core on Kodi's OpenGL ES framebuffer
   */
  class IVulkanBridge
  {
  public:
    virtual ~IVulkanBridge() = default;

    /*!
     * \brief Whether Kodi draws with OpenGL ES through EGL on this thread and
     *        the GPU can share images with it
     */
    virtual bool IsUsable() = 0;

    virtual void Request() = 0;
    virtual bool IsRequested() const = 0;

    /*!
     * \brief Forget the core's request, for the next core
     */
    virtual void Reset() = 0;

    virtual void SetNegotiation(const retro_hw_render_context_negotiation_interface* negotiation) = 0;

    /*!
     * \brief Set the bridge up as Kodi makes the context, if a core asked for Vulkan
     *
     * \return false only if the bridge was needed and couldn't be set up
     */
    virtual bool ContextReset() = 0;

    /*!
     * \brief Tear the bridge down as Kodi destroys the context
     */
    virtual void ContextDestroy() = 0;

    /*!
     * \return The retro_hw_render_interface_vulkan, or nullptr while the bridge isn't running
     */
    virtual const retro_hw_render_interface* Interface() const = 0;

    /*!
     * \brief Draw the core's image into the framebuffer, if the bridge is running
     */
    virtual bool Present(unsigned int width, unsigned int height, uintptr_t framebuffer) = 0;

    /*!
     * \brief On a duplicated frame, run the core's command buffers and signal its
     *        semaphore without copying anything, if the bridge is running
     */
    virtual void Dupe() = 0;
  };

  /*!
   * \brief The bridge, from whichever implementation the build chose
   */
  IVulkanBridge& VulkanBridge();
} // namespace LIBRETRO
