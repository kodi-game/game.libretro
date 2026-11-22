/*
 *  Copyright (C) 2026 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#include "IVulkanBridge.h"

using namespace LIBRETRO;

namespace
{
class CNullVulkanBridge : public IVulkanBridge
{
public:
  bool IsUsable() override { return false; }
  void Request() override {}
  bool IsRequested() const override { return false; }
  void Reset() override {}
  void SetNegotiation(const retro_hw_render_context_negotiation_interface*) override {}
  bool ContextReset() override { return true; }
  void ContextDestroy() override {}
  const retro_hw_render_interface* Interface() const override { return nullptr; }
  bool Present(unsigned int, unsigned int, uintptr_t) override { return false; }
  void Dupe() override {}
};
} // namespace

IVulkanBridge& LIBRETRO::VulkanBridge()
{
  static CNullVulkanBridge bridge;
  return bridge;
}
