/*
 *  Copyright (C) 2026 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#pragma once

#include "IVulkanBridge.h"

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

#include <libretro_vulkan.h>

#include <array>
#include <cstdint>
#include <memory>
#include <mutex>
#include <vector>

namespace LIBRETRO
{
  // Each frame the core's image is copied into a dma-buf image, which GLES
  // blits into the framebuffer
  class CVulkanBridge : public IVulkanBridge
  {
  public:
    bool IsUsable() override;
    void Request() override { m_requested = true; }
    bool IsRequested() const override { return m_requested; }
    void Reset() override;
    void SetNegotiation(const retro_hw_render_context_negotiation_interface* negotiation) override;

    bool ContextReset() override;
    void ContextDestroy() override;
    const retro_hw_render_interface* Interface() const override;

    bool Present(unsigned int width, unsigned int height, uintptr_t framebuffer) override;
    void Dupe() override;

  private:
    struct VulkanFunctions;
    struct GLFunctions;

    static constexpr uint32_t FRAMES = 2;

    struct Frame
    {
      VkCommandBuffer cmd = VK_NULL_HANDLE;
      VkFence fence = VK_NULL_HANDLE;
      bool submitted = false;
      VkSemaphore copied = VK_NULL_HANDLE;
      VkImage image = VK_NULL_HANDLE;
      VkDeviceMemory memory = VK_NULL_HANDLE;
      unsigned int width = 0;
      unsigned int height = 0;
      void* eglImage = nullptr;
      unsigned int texture = 0;
      unsigned int framebuffer = 0;
      void* glFence = nullptr;
    };

    void Teardown(bool coreLoaded);
    bool LoadLibraries();
    bool CreateInstance(const VkApplicationInfo* appInfo, bool byCore);
    std::vector<uint64_t> SharedModifiers(VkPhysicalDevice gpu);
    VkPhysicalDevice ChooseGpu(const VkPhysicalDevice* gpus, uint32_t count);
    bool HasDeviceExtension(VkPhysicalDevice gpu, const char* name);
    bool CanShareSemaphores(VkPhysicalDevice gpu);
    std::vector<const char*> DeviceExtensions(VkPhysicalDevice gpu);
    bool CreateDevice();
    bool CreateFrames();
    void DestroyFrames();
    bool CreateExportImage(Frame& frame, unsigned int width, unsigned int height);
    void DestroyExportImage(Frame& frame);
    void WaitFrame(Frame& frame);
    void SubmitCoreWork();
    bool WaitOnGpu(Frame& frame);

    static VkInstance CreateInstanceWrapper(void* opaque, const VkInstanceCreateInfo* createInfo);
    static VkDevice CreateDeviceWrapper(VkPhysicalDevice gpu,
                                        void* opaque,
                                        const VkDeviceCreateInfo* createInfo);

    static void SetImage(void* handle,
                         const retro_vulkan_image* image,
                         uint32_t numSemaphores,
                         const VkSemaphore* semaphores,
                         uint32_t srcQueueFamily);
    static uint32_t GetSyncIndex(void* handle);
    static uint32_t GetSyncIndexMask(void* handle);
    static void SetCommandBuffers(void* handle, uint32_t num, const VkCommandBuffer* cmd);
    static void WaitSyncIndex(void* handle);
    static void LockQueue(void* handle);
    static void UnlockQueue(void* handle);
    static void SetSignalSemaphore(void* handle, VkSemaphore semaphore);

    bool m_requested = false;
    bool m_loadFailed = false;
    bool m_created = false;
    const retro_hw_render_context_negotiation_interface_vulkan* m_negotiation = nullptr;
    bool m_deviceByCore = false;

    void* m_libVulkan = nullptr;
    void* m_libEGL = nullptr;
    VulkanFunctions* m_vk = nullptr;
    GLFunctions* m_gl = nullptr;

    VkInstance m_instance = VK_NULL_HANDLE;
    VkPhysicalDevice m_gpu = VK_NULL_HANDLE;
    VkDevice m_device = VK_NULL_HANDLE;
    VkQueue m_queue = VK_NULL_HANDLE;
    uint32_t m_queueFamily = 0;
    VkCommandPool m_pool = VK_NULL_HANDLE;
    retro_hw_render_interface_vulkan m_interface{};
    std::mutex m_queueMutex;
    bool m_syncFd = false; // GLES waits for the copy on the GPU

    std::array<Frame, FRAMES> m_frames{};
    uint32_t m_index = 0;
    std::mutex m_frameMutex; // the core may wait for a frame on another thread

    // per frame, from the core
    retro_vulkan_image m_image{};
    bool m_haveImage = false;
    uint32_t m_srcQueueFamily = VK_QUEUE_FAMILY_IGNORED;
    std::vector<VkSemaphore> m_waitSemaphores;
    std::vector<VkCommandBuffer> m_coreCommands;
    VkSemaphore m_signalSemaphore = VK_NULL_HANDLE;
  };
} // namespace LIBRETRO
