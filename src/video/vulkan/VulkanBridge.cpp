/*
 *  Copyright (C) 2026 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#include "VulkanBridge.h"

#include <algorithm>
#include <cstring>

#include <GLES3/gl3.h>
#include <dlfcn.h>
#include <kodi/General.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <unistd.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>

// after gl3.h, for GL_APIENTRY
#include <GLES2/gl2ext.h>

#ifndef EGL_DRM_RENDER_NODE_FILE_EXT
#define EGL_DRM_RENDER_NODE_FILE_EXT 0x3377
#endif

using namespace LIBRETRO;

namespace
{
constexpr uint32_t DRM_FORMAT_ABGR8888_ = 0x34324241; // 'AB24', R8G8B8A8 in memory order

const char* const MEMORY_EXTENSIONS[] = {
    VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME,
    VK_EXT_EXTERNAL_MEMORY_DMA_BUF_EXTENSION_NAME,
    VK_EXT_IMAGE_DRM_FORMAT_MODIFIER_EXTENSION_NAME,
    VK_EXT_QUEUE_FAMILY_FOREIGN_EXTENSION_NAME,
};

void Barrier(PFN_vkCmdPipelineBarrier cmdPipelineBarrier,
             VkCommandBuffer cmd,
             VkImage image,
             VkImageLayout from,
             VkImageLayout to,
             VkAccessFlags srcAccess,
             VkAccessFlags dstAccess,
             uint32_t srcQueue = VK_QUEUE_FAMILY_IGNORED,
             uint32_t dstQueue = VK_QUEUE_FAMILY_IGNORED)
{
  VkImageMemoryBarrier b{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
  b.srcAccessMask = srcAccess;
  b.dstAccessMask = dstAccess;
  b.oldLayout = from;
  b.newLayout = to;
  b.srcQueueFamilyIndex = srcQueue;
  b.dstQueueFamilyIndex = dstQueue;
  b.image = image;
  b.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
  cmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0,
                     0, nullptr, 0, nullptr, 1, &b);
}
} // namespace

struct CVulkanBridge::VulkanFunctions
{
  PFN_vkGetInstanceProcAddr GetInstanceProcAddr;
  PFN_vkCreateInstance CreateInstance;
  PFN_vkDestroyInstance DestroyInstance;
  PFN_vkEnumeratePhysicalDevices EnumeratePhysicalDevices;
  PFN_vkEnumerateDeviceExtensionProperties EnumerateDeviceExtensionProperties;
  PFN_vkGetPhysicalDeviceProperties GetPhysicalDeviceProperties;
  PFN_vkGetPhysicalDeviceProperties2 GetPhysicalDeviceProperties2;
  PFN_vkGetPhysicalDeviceQueueFamilyProperties GetPhysicalDeviceQueueFamilyProperties;
  PFN_vkGetPhysicalDeviceMemoryProperties GetPhysicalDeviceMemoryProperties;
  PFN_vkGetPhysicalDeviceFormatProperties2 GetPhysicalDeviceFormatProperties2;
  PFN_vkGetPhysicalDeviceExternalSemaphoreProperties GetPhysicalDeviceExternalSemaphoreProperties;
  PFN_vkCreateDevice CreateDevice;
  PFN_vkGetDeviceProcAddr GetDeviceProcAddr;

  PFN_vkDestroyDevice DestroyDevice;
  PFN_vkGetDeviceQueue GetDeviceQueue;
  PFN_vkDeviceWaitIdle DeviceWaitIdle;
  PFN_vkCreateCommandPool CreateCommandPool;
  PFN_vkDestroyCommandPool DestroyCommandPool;
  PFN_vkAllocateCommandBuffers AllocateCommandBuffers;
  PFN_vkBeginCommandBuffer BeginCommandBuffer;
  PFN_vkEndCommandBuffer EndCommandBuffer;
  PFN_vkCmdPipelineBarrier CmdPipelineBarrier;
  PFN_vkCmdBlitImage CmdBlitImage;
  PFN_vkQueueSubmit QueueSubmit;
  PFN_vkCreateFence CreateFence;
  PFN_vkDestroyFence DestroyFence;
  PFN_vkWaitForFences WaitForFences;
  PFN_vkResetFences ResetFences;
  PFN_vkCreateSemaphore CreateSemaphore;
  PFN_vkDestroySemaphore DestroySemaphore;
  PFN_vkGetSemaphoreFdKHR GetSemaphoreFdKHR;
  PFN_vkCreateImage CreateImage;
  PFN_vkDestroyImage DestroyImage;
  PFN_vkGetImageMemoryRequirements GetImageMemoryRequirements;
  PFN_vkAllocateMemory AllocateMemory;
  PFN_vkFreeMemory FreeMemory;
  PFN_vkBindImageMemory BindImageMemory;
  PFN_vkGetImageSubresourceLayout GetImageSubresourceLayout;
  PFN_vkGetMemoryFdKHR GetMemoryFdKHR;
  PFN_vkGetImageDrmFormatModifierPropertiesEXT GetImageDrmFormatModifierPropertiesEXT;
};

struct CVulkanBridge::GLFunctions
{
  PFNEGLGETPROCADDRESSPROC GetProcAddress;
  decltype(&eglGetCurrentDisplay) GetCurrentDisplay;
  decltype(&eglGetError) GetError;
  decltype(&eglQueryString) QueryString;
  decltype(&eglGetCurrentContext) GetCurrentContext;
  decltype(&eglQueryContext) QueryContext;
  PFNEGLCREATEIMAGEKHRPROC CreateImageKHR;
  PFNEGLDESTROYIMAGEKHRPROC DestroyImageKHR;
  PFNEGLQUERYDMABUFMODIFIERSEXTPROC QueryDmaBufModifiersEXT;
  PFNEGLQUERYDISPLAYATTRIBEXTPROC QueryDisplayAttribEXT;
  PFNEGLQUERYDEVICESTRINGEXTPROC QueryDeviceStringEXT;
  PFNEGLCREATESYNCKHRPROC CreateSyncKHR;
  PFNEGLWAITSYNCKHRPROC WaitSyncKHR;
  PFNEGLDESTROYSYNCKHRPROC DestroySyncKHR;
  PFNGLEGLIMAGETARGETTEXTURE2DOESPROC EGLImageTargetTexture2DOES;

  decltype(&glGetIntegerv) GetIntegerv;
  decltype(&glGenTextures) GenTextures;
  decltype(&glDeleteTextures) DeleteTextures;
  decltype(&glBindTexture) BindTexture;
  decltype(&glTexParameteri) TexParameteri;
  decltype(&glGenFramebuffers) GenFramebuffers;
  decltype(&glDeleteFramebuffers) DeleteFramebuffers;
  decltype(&glBindFramebuffer) BindFramebuffer;
  decltype(&glFramebufferTexture2D) FramebufferTexture2D;
  decltype(&glCheckFramebufferStatus) CheckFramebufferStatus;
  decltype(&glBlitFramebuffer) BlitFramebuffer;
  decltype(&glFenceSync) FenceSync;
  decltype(&glClientWaitSync) ClientWaitSync;
  decltype(&glDeleteSync) DeleteSync;
};

IVulkanBridge& LIBRETRO::VulkanBridge()
{
  static CVulkanBridge bridge;
  return bridge;
}

void CVulkanBridge::SetNegotiation(const retro_hw_render_context_negotiation_interface* negotiation)
{
  m_negotiation =
      reinterpret_cast<const retro_hw_render_context_negotiation_interface_vulkan*>(negotiation);
}

bool CVulkanBridge::LoadLibraries()
{
  if (m_vk)
    return true;
  if (m_loadFailed)
    return false;
  m_loadFailed = true; // until everything below is found

  m_libVulkan = dlopen("libvulkan.so.1", RTLD_NOW | RTLD_LOCAL);
  m_libEGL = dlopen("libEGL.so.1", RTLD_NOW | RTLD_LOCAL);
  if (!m_libVulkan || !m_libEGL)
  {
    kodi::Log(ADDON_LOG_ERROR, "Vulkan bridge: can't load %s",
              !m_libVulkan ? "libvulkan.so.1" : "libEGL.so.1");
    return false;
  }

  auto vk = std::make_unique<VulkanFunctions>();
  vk->GetInstanceProcAddr =
      reinterpret_cast<PFN_vkGetInstanceProcAddr>(dlsym(m_libVulkan, "vkGetInstanceProcAddr"));
  vk->CreateInstance = reinterpret_cast<PFN_vkCreateInstance>(
      vk->GetInstanceProcAddr ? vk->GetInstanceProcAddr(VK_NULL_HANDLE, "vkCreateInstance")
                              : nullptr);

  auto gl = std::make_unique<GLFunctions>();
  gl->GetProcAddress =
      reinterpret_cast<PFNEGLGETPROCADDRESSPROC>(dlsym(m_libEGL, "eglGetProcAddress"));
  if (!gl->GetProcAddress || !vk->CreateInstance)
  {
    kodi::Log(ADDON_LOG_ERROR, "Vulkan bridge: no vkCreateInstance or eglGetProcAddress");
    return false;
  }
  gl->GetCurrentDisplay =
      reinterpret_cast<decltype(gl->GetCurrentDisplay)>(dlsym(m_libEGL, "eglGetCurrentDisplay"));
  gl->GetError = reinterpret_cast<decltype(gl->GetError)>(dlsym(m_libEGL, "eglGetError"));
  gl->QueryString = reinterpret_cast<decltype(gl->QueryString)>(dlsym(m_libEGL, "eglQueryString"));
  gl->GetCurrentContext =
      reinterpret_cast<decltype(gl->GetCurrentContext)>(dlsym(m_libEGL, "eglGetCurrentContext"));
  gl->QueryContext =
      reinterpret_cast<decltype(gl->QueryContext)>(dlsym(m_libEGL, "eglQueryContext"));
#define LOAD_GL(member, name) \
  gl->member = reinterpret_cast<decltype(gl->member)>(gl->GetProcAddress(name))
  LOAD_GL(CreateImageKHR, "eglCreateImageKHR");
  LOAD_GL(DestroyImageKHR, "eglDestroyImageKHR");
  LOAD_GL(QueryDmaBufModifiersEXT, "eglQueryDmaBufModifiersEXT");
  LOAD_GL(QueryDisplayAttribEXT, "eglQueryDisplayAttribEXT");
  LOAD_GL(QueryDeviceStringEXT, "eglQueryDeviceStringEXT");
  LOAD_GL(CreateSyncKHR, "eglCreateSyncKHR");
  LOAD_GL(WaitSyncKHR, "eglWaitSyncKHR");
  LOAD_GL(DestroySyncKHR, "eglDestroySyncKHR");
  LOAD_GL(EGLImageTargetTexture2DOES, "glEGLImageTargetTexture2DOES");
  LOAD_GL(GetIntegerv, "glGetIntegerv");
  LOAD_GL(GenTextures, "glGenTextures");
  LOAD_GL(DeleteTextures, "glDeleteTextures");
  LOAD_GL(BindTexture, "glBindTexture");
  LOAD_GL(TexParameteri, "glTexParameteri");
  LOAD_GL(GenFramebuffers, "glGenFramebuffers");
  LOAD_GL(DeleteFramebuffers, "glDeleteFramebuffers");
  LOAD_GL(BindFramebuffer, "glBindFramebuffer");
  LOAD_GL(FramebufferTexture2D, "glFramebufferTexture2D");
  LOAD_GL(CheckFramebufferStatus, "glCheckFramebufferStatus");
  LOAD_GL(BlitFramebuffer, "glBlitFramebuffer");
  LOAD_GL(FenceSync, "glFenceSync");
  LOAD_GL(ClientWaitSync, "glClientWaitSync");
  LOAD_GL(DeleteSync, "glDeleteSync");
#undef LOAD_GL

  if (!gl->GetCurrentDisplay || !gl->QueryString || !gl->GetCurrentContext || !gl->QueryContext ||
      !gl->CreateImageKHR || !gl->QueryDmaBufModifiersEXT || !gl->EGLImageTargetTexture2DOES ||
      !gl->BlitFramebuffer || !gl->FenceSync)
  {
    kodi::Log(ADDON_LOG_ERROR, "Vulkan bridge: missing EGL dma-buf import or GLES 3 functions");
    return false;
  }

  m_vk = vk.release();
  m_gl = gl.release();
  m_loadFailed = false;
  return true;
}

// Kodi has to draw with OpenGL ES through EGL on this thread, and the GPU has
// to share images with it, or the core is better off falling back
bool CVulkanBridge::IsUsable()
{
  if (!LoadLibraries())
    return false;

  EGLContext context = m_gl->GetCurrentContext();
  EGLint api = 0;
  if (context == EGL_NO_CONTEXT ||
      !m_gl->QueryContext(m_gl->GetCurrentDisplay(), context, EGL_CONTEXT_CLIENT_TYPE, &api) ||
      api != EGL_OPENGL_ES_API)
    return false;

  if (m_created)
    return true;

  if (!CreateInstance(nullptr, false))
    return false;

  bool usable = false;
  uint32_t count = 0;
  m_vk->EnumeratePhysicalDevices(m_instance, &count, nullptr);
  std::vector<VkPhysicalDevice> gpus(count);
  if (count > 0 && m_vk->EnumeratePhysicalDevices(m_instance, &count, gpus.data()) >= 0)
  {
    const VkPhysicalDevice gpu = ChooseGpu(gpus.data(), count);
    usable = std::all_of(std::begin(MEMORY_EXTENSIONS), std::end(MEMORY_EXTENSIONS),
                         [&](const char* name) { return HasDeviceExtension(gpu, name); }) &&
             !SharedModifiers(gpu).empty();
  }
  m_vk->DestroyInstance(m_instance, nullptr);
  m_instance = VK_NULL_HANDLE;

  if (!usable)
    kodi::Log(ADDON_LOG_INFO, "Vulkan bridge: this GPU can't share images with GLES");
  return usable;
}

bool CVulkanBridge::CreateInstance(const VkApplicationInfo* appInfo, bool byCore)
{
  VulkanFunctions& vk = *m_vk;

  VkApplicationInfo app{};
  if (appInfo)
    app = *appInfo;
  app.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  if (app.apiVersion < VK_API_VERSION_1_2)
    app.apiVersion = VK_API_VERSION_1_2;

  if (byCore)
  {
    m_instance = m_negotiation->create_instance(vk.GetInstanceProcAddr, &app,
                                                CreateInstanceWrapper, this);
    if (!m_instance)
    {
      kodi::Log(ADDON_LOG_ERROR, "Vulkan bridge: the core failed to create its instance");
      return false;
    }
  }
  else
  {
    VkInstanceCreateInfo ici{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
    ici.pApplicationInfo = &app;
    if (vk.CreateInstance(&ici, nullptr, &m_instance) != VK_SUCCESS)
    {
      kodi::Log(ADDON_LOG_ERROR, "Vulkan bridge: vkCreateInstance failed");
      m_instance = VK_NULL_HANDLE;
      return false;
    }
  }

#define LOAD_INSTANCE(name) \
  vk.name = reinterpret_cast<PFN_vk##name>(vk.GetInstanceProcAddr(m_instance, "vk" #name))
  LOAD_INSTANCE(DestroyInstance);
  LOAD_INSTANCE(EnumeratePhysicalDevices);
  LOAD_INSTANCE(EnumerateDeviceExtensionProperties);
  LOAD_INSTANCE(GetPhysicalDeviceProperties);
  LOAD_INSTANCE(GetPhysicalDeviceProperties2);
  LOAD_INSTANCE(GetPhysicalDeviceQueueFamilyProperties);
  LOAD_INSTANCE(GetPhysicalDeviceMemoryProperties);
  LOAD_INSTANCE(GetPhysicalDeviceFormatProperties2);
  LOAD_INSTANCE(GetPhysicalDeviceExternalSemaphoreProperties);
  LOAD_INSTANCE(CreateDevice);
  LOAD_INSTANCE(GetDeviceProcAddr);
#undef LOAD_INSTANCE
  return true;
}

// Modifiers both sides can use for R8G8B8A8, single plane, as a blit target
std::vector<uint64_t> CVulkanBridge::SharedModifiers(VkPhysicalDevice gpu)
{
  EGLuint64KHR eglModifiers[64];
  EGLBoolean externalOnly[64];
  EGLint numEgl = 0;
  m_gl->QueryDmaBufModifiersEXT(m_gl->GetCurrentDisplay(), DRM_FORMAT_ABGR8888_, 64, eglModifiers,
                                externalOnly, &numEgl);

  VkDrmFormatModifierPropertiesEXT vkModifiers[64];
  VkDrmFormatModifierPropertiesListEXT modifierList{
      VK_STRUCTURE_TYPE_DRM_FORMAT_MODIFIER_PROPERTIES_LIST_EXT};
  modifierList.drmFormatModifierCount = 64;
  modifierList.pDrmFormatModifierProperties = vkModifiers;
  VkFormatProperties2 formatProps{VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2};
  formatProps.pNext = &modifierList;
  m_vk->GetPhysicalDeviceFormatProperties2(gpu, VK_FORMAT_R8G8B8A8_UNORM, &formatProps);

  std::vector<uint64_t> shared;
  for (uint32_t i = 0; i < modifierList.drmFormatModifierCount; i++)
  {
    if (vkModifiers[i].drmFormatModifierPlaneCount != 1 ||
        !(vkModifiers[i].drmFormatModifierTilingFeatures & VK_FORMAT_FEATURE_BLIT_DST_BIT))
      continue;
    for (EGLint j = 0; j < numEgl; j++)
    {
      if (eglModifiers[j] == vkModifiers[i].drmFormatModifier && !externalOnly[j])
        shared.push_back(vkModifiers[i].drmFormatModifier);
    }
  }
  return shared;
}

bool CVulkanBridge::HasDeviceExtension(VkPhysicalDevice gpu, const char* name)
{
  uint32_t count = 0;
  m_vk->EnumerateDeviceExtensionProperties(gpu, nullptr, &count, nullptr);
  std::vector<VkExtensionProperties> extensions(count);
  m_vk->EnumerateDeviceExtensionProperties(gpu, nullptr, &count, extensions.data());
  for (const auto& extension : extensions)
  {
    if (std::strcmp(extension.extensionName, name) == 0)
      return true;
  }
  return false;
}

// The GPU Kodi draws with, matched by its DRM render node
VkPhysicalDevice CVulkanBridge::ChooseGpu(const VkPhysicalDevice* gpus, uint32_t count)
{
  struct stat node{};
  bool haveNode = false;
  EGLAttrib device = 0;
  if (m_gl->QueryDisplayAttribEXT && m_gl->QueryDeviceStringEXT &&
      m_gl->QueryDisplayAttribEXT(m_gl->GetCurrentDisplay(), EGL_DEVICE_EXT, &device))
  {
    const char* file = m_gl->QueryDeviceStringEXT(reinterpret_cast<EGLDeviceEXT>(device),
                                                  EGL_DRM_RENDER_NODE_FILE_EXT);
    haveNode = file && stat(file, &node) == 0;
  }

  for (uint32_t i = 0; haveNode && i < count; i++)
  {
    if (!HasDeviceExtension(gpus[i], VK_EXT_PHYSICAL_DEVICE_DRM_EXTENSION_NAME))
      continue;
    VkPhysicalDeviceDrmPropertiesEXT drm{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRM_PROPERTIES_EXT};
    VkPhysicalDeviceProperties2 props{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2};
    props.pNext = &drm;
    m_vk->GetPhysicalDeviceProperties2(gpus[i], &props);
    if (drm.hasRender && makedev(drm.renderMajor, drm.renderMinor) == node.st_rdev)
      return gpus[i];
  }

  for (uint32_t i = 0; i < count; i++)
  {
    VkPhysicalDeviceProperties props;
    m_vk->GetPhysicalDeviceProperties(gpus[i], &props);
    if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU ||
        props.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
      return gpus[i];
  }
  return gpus[0];
}

bool CVulkanBridge::CanShareSemaphores(VkPhysicalDevice gpu)
{
  const char* eglExtensions = m_gl->QueryString(m_gl->GetCurrentDisplay(), EGL_EXTENSIONS);
  if (!eglExtensions || !std::strstr(eglExtensions, "EGL_ANDROID_native_fence_sync") ||
      !std::strstr(eglExtensions, "EGL_KHR_wait_sync") || !m_gl->CreateSyncKHR ||
      !m_gl->WaitSyncKHR || !m_gl->DestroySyncKHR ||
      !HasDeviceExtension(gpu, VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME))
    return false;

  VkPhysicalDeviceExternalSemaphoreInfo info{
      VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_SEMAPHORE_INFO};
  info.handleType = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_SYNC_FD_BIT;
  VkExternalSemaphoreProperties props{VK_STRUCTURE_TYPE_EXTERNAL_SEMAPHORE_PROPERTIES};
  m_vk->GetPhysicalDeviceExternalSemaphoreProperties(gpu, &info, &props);
  return props.externalSemaphoreFeatures & VK_EXTERNAL_SEMAPHORE_FEATURE_EXPORTABLE_BIT;
}

std::vector<const char*> CVulkanBridge::DeviceExtensions(VkPhysicalDevice gpu)
{
  std::vector<const char*> extensions(std::begin(MEMORY_EXTENSIONS), std::end(MEMORY_EXTENSIONS));
  if (CanShareSemaphores(gpu))
    extensions.push_back(VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME);
  return extensions;
}

// The core's instance, with the bridge's minimum Vulkan version
VkInstance CVulkanBridge::CreateInstanceWrapper(void* opaque, const VkInstanceCreateInfo* createInfo)
{
  CVulkanBridge* bridge = static_cast<CVulkanBridge*>(opaque);

  VkApplicationInfo app{VK_STRUCTURE_TYPE_APPLICATION_INFO};
  if (createInfo->pApplicationInfo)
    app = *createInfo->pApplicationInfo;
  if (app.apiVersion < VK_API_VERSION_1_2)
    app.apiVersion = VK_API_VERSION_1_2;

  VkInstanceCreateInfo info = *createInfo;
  info.pApplicationInfo = &app;
  VkInstance instance = VK_NULL_HANDLE;
  if (bridge->m_vk->CreateInstance(&info, nullptr, &instance) != VK_SUCCESS)
    return VK_NULL_HANDLE;
  return instance;
}

// The core's device, with the extensions the bridge needs added
VkDevice CVulkanBridge::CreateDeviceWrapper(VkPhysicalDevice gpu,
                                            void* opaque,
                                            const VkDeviceCreateInfo* createInfo)
{
  CVulkanBridge* bridge = static_cast<CVulkanBridge*>(opaque);

  std::vector<const char*> extensions(
      createInfo->ppEnabledExtensionNames,
      createInfo->ppEnabledExtensionNames + createInfo->enabledExtensionCount);
  for (const char* name : bridge->DeviceExtensions(gpu))
  {
    if (std::none_of(extensions.begin(), extensions.end(),
                     [name](const char* other) { return std::strcmp(name, other) == 0; }))
      extensions.push_back(name);
  }

  VkDeviceCreateInfo info = *createInfo;
  info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
  info.ppEnabledExtensionNames = extensions.data();
  VkDevice device = VK_NULL_HANDLE;
  if (bridge->m_vk->CreateDevice(gpu, &info, nullptr, &device) != VK_SUCCESS)
    return VK_NULL_HANDLE;
  return device;
}

bool CVulkanBridge::CreateDevice()
{
  VulkanFunctions& vk = *m_vk;

  // Fields past destroy_device only exist from version 2
  const bool v2 = m_negotiation && m_negotiation->interface_version >= 2;
  if (m_negotiation)
    kodi::Log(ADDON_LOG_DEBUG, "Vulkan bridge: negotiation version %u%s%s",
              m_negotiation->interface_version,
              v2 && m_negotiation->create_instance ? ", instance by the core" : "",
              v2 && m_negotiation->create_device2 ? ", create_device2" : "");
  if (!CreateInstance((m_negotiation && m_negotiation->get_application_info)
                          ? m_negotiation->get_application_info()
                          : nullptr,
                      v2 && m_negotiation->create_instance))
    return false;

  uint32_t count = 0;
  vk.EnumeratePhysicalDevices(m_instance, &count, nullptr);
  std::vector<VkPhysicalDevice> gpus(count);
  if (count == 0 || vk.EnumeratePhysicalDevices(m_instance, &count, gpus.data()) < 0)
  {
    kodi::Log(ADDON_LOG_ERROR, "Vulkan bridge: no Vulkan device");
    return false;
  }
  m_gpu = ChooseGpu(gpus.data(), count);

  std::vector<const char*> extensions = DeviceExtensions(m_gpu);

  retro_vulkan_context context{};
  const bool coreMakesDevice =
      m_negotiation && ((v2 && m_negotiation->create_device2) || m_negotiation->create_device);
  bool byCore = false;
  if (v2 && m_negotiation->create_device2)
  {
    byCore = m_negotiation->create_device2(&context, m_instance, m_gpu, VK_NULL_HANDLE,
                                           vk.GetInstanceProcAddr, CreateDeviceWrapper, this);
    if (!byCore)
    {
      // libretro_vulkan.h: let the core choose the GPU once
      kodi::Log(ADDON_LOG_INFO, "Vulkan bridge: the core can't use Kodi's GPU, letting it choose");
      context = {};
      byCore = m_negotiation->create_device2(&context, m_instance, VK_NULL_HANDLE, VK_NULL_HANDLE,
                                             vk.GetInstanceProcAddr, CreateDeviceWrapper, this);
    }
  }
  else if (m_negotiation && m_negotiation->create_device)
  {
    const VkPhysicalDeviceFeatures features{}; // cores read it without checking for null
    byCore = m_negotiation->create_device(
        &context, m_instance, m_gpu, VK_NULL_HANDLE, vk.GetInstanceProcAddr, extensions.data(),
        static_cast<unsigned int>(extensions.size()), nullptr, 0, &features);
  }

  if (coreMakesDevice)
  {
    if (!byCore)
    {
      kodi::Log(ADDON_LOG_ERROR, "Vulkan bridge: the core failed to create its device");
      return false;
    }
    m_gpu = context.gpu;
    m_device = context.device;
    m_queue = context.queue;
    m_queueFamily = context.queue_family_index;
    m_deviceByCore = true;
  }
  else
  {
    uint32_t numFamilies = 0;
    vk.GetPhysicalDeviceQueueFamilyProperties(m_gpu, &numFamilies, nullptr);
    std::vector<VkQueueFamilyProperties> families(numFamilies);
    vk.GetPhysicalDeviceQueueFamilyProperties(m_gpu, &numFamilies, families.data());
    for (m_queueFamily = 0; m_queueFamily < numFamilies; m_queueFamily++)
    {
      if (families[m_queueFamily].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        break;
    }
    const float priority = 1.0f;
    VkDeviceQueueCreateInfo qci{VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
    qci.queueFamilyIndex = m_queueFamily;
    qci.queueCount = 1;
    qci.pQueuePriorities = &priority;
    VkDeviceCreateInfo dci{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
    dci.queueCreateInfoCount = 1;
    dci.pQueueCreateInfos = &qci;
    dci.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    dci.ppEnabledExtensionNames = extensions.data();
    if (vk.CreateDevice(m_gpu, &dci, nullptr, &m_device) != VK_SUCCESS)
    {
      kodi::Log(ADDON_LOG_ERROR, "Vulkan bridge: vkCreateDevice failed");
      return false;
    }
  }

#define LOAD_DEVICE(name) \
  vk.name = reinterpret_cast<PFN_vk##name>(vk.GetDeviceProcAddr(m_device, "vk" #name))
  LOAD_DEVICE(DestroyDevice);
  LOAD_DEVICE(GetDeviceQueue);
  LOAD_DEVICE(DeviceWaitIdle);
  LOAD_DEVICE(CreateCommandPool);
  LOAD_DEVICE(DestroyCommandPool);
  LOAD_DEVICE(AllocateCommandBuffers);
  LOAD_DEVICE(BeginCommandBuffer);
  LOAD_DEVICE(EndCommandBuffer);
  LOAD_DEVICE(CmdPipelineBarrier);
  LOAD_DEVICE(CmdBlitImage);
  LOAD_DEVICE(QueueSubmit);
  LOAD_DEVICE(CreateFence);
  LOAD_DEVICE(DestroyFence);
  LOAD_DEVICE(WaitForFences);
  LOAD_DEVICE(ResetFences);
  LOAD_DEVICE(CreateSemaphore);
  LOAD_DEVICE(DestroySemaphore);
  LOAD_DEVICE(GetSemaphoreFdKHR);
  LOAD_DEVICE(CreateImage);
  LOAD_DEVICE(DestroyImage);
  LOAD_DEVICE(GetImageMemoryRequirements);
  LOAD_DEVICE(AllocateMemory);
  LOAD_DEVICE(FreeMemory);
  LOAD_DEVICE(BindImageMemory);
  LOAD_DEVICE(GetImageSubresourceLayout);
  LOAD_DEVICE(GetMemoryFdKHR);
  LOAD_DEVICE(GetImageDrmFormatModifierPropertiesEXT);
#undef LOAD_DEVICE

  if (!m_queue)
    vk.GetDeviceQueue(m_device, m_queueFamily, 0, &m_queue);
  m_syncFd = CanShareSemaphores(m_gpu);

  VkPhysicalDeviceProperties props;
  vk.GetPhysicalDeviceProperties(m_gpu, &props);
  kodi::Log(ADDON_LOG_INFO, "Vulkan bridge: %s, queue family %u, device made by the %s, %s",
            props.deviceName, m_queueFamily, m_deviceByCore ? "core" : "frontend",
            m_syncFd ? "GLES waits on the GPU" : "the CPU waits for each frame");
  return true;
}

bool CVulkanBridge::CreateFrames()
{
  VulkanFunctions& vk = *m_vk;

  VkCommandPoolCreateInfo cpi{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
  cpi.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  cpi.queueFamilyIndex = m_queueFamily;
  if (vk.CreateCommandPool(m_device, &cpi, nullptr, &m_pool) != VK_SUCCESS)
    return false;

  for (Frame& frame : m_frames)
  {
    VkCommandBufferAllocateInfo cai{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    cai.commandPool = m_pool;
    cai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cai.commandBufferCount = 1;
    VkFenceCreateInfo fci{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
    if (vk.AllocateCommandBuffers(m_device, &cai, &frame.cmd) != VK_SUCCESS ||
        vk.CreateFence(m_device, &fci, nullptr, &frame.fence) != VK_SUCCESS)
      return false;

    if (m_syncFd)
    {
      VkExportSemaphoreCreateInfo exportInfo{VK_STRUCTURE_TYPE_EXPORT_SEMAPHORE_CREATE_INFO};
      exportInfo.handleTypes = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_SYNC_FD_BIT;
      VkSemaphoreCreateInfo sci{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
      sci.pNext = &exportInfo;
      if (vk.CreateSemaphore(m_device, &sci, nullptr, &frame.copied) != VK_SUCCESS)
        return false;
    }
  }
  return true;
}

void CVulkanBridge::DestroyFrames()
{
  for (Frame& frame : m_frames)
  {
    DestroyExportImage(frame);
    if (frame.copied)
      m_vk->DestroySemaphore(m_device, frame.copied, nullptr);
    if (frame.fence)
      m_vk->DestroyFence(m_device, frame.fence, nullptr);
    frame = Frame{};
  }
  if (m_pool)
    m_vk->DestroyCommandPool(m_device, m_pool, nullptr);
  m_pool = VK_NULL_HANDLE;
}

bool CVulkanBridge::ContextReset()
{
  if (!m_requested || m_created)
    return true;

  if (!LoadLibraries() || !CreateDevice() || !CreateFrames())
  {
    Teardown(true);
    return false;
  }

  m_interface.interface_type = RETRO_HW_RENDER_INTERFACE_VULKAN;
  m_interface.interface_version = RETRO_HW_RENDER_INTERFACE_VULKAN_VERSION;
  m_interface.handle = this;
  m_interface.instance = m_instance;
  m_interface.gpu = m_gpu;
  m_interface.device = m_device;
  m_interface.get_device_proc_addr = m_vk->GetDeviceProcAddr;
  m_interface.get_instance_proc_addr = m_vk->GetInstanceProcAddr;
  m_interface.queue = m_queue;
  m_interface.queue_index = m_queueFamily;
  m_interface.set_image = SetImage;
  m_interface.get_sync_index = GetSyncIndex;
  m_interface.get_sync_index_mask = GetSyncIndexMask;
  m_interface.set_command_buffers = SetCommandBuffers;
  m_interface.wait_sync_index = WaitSyncIndex;
  m_interface.lock_queue = LockQueue;
  m_interface.unlock_queue = UnlockQueue;
  m_interface.set_signal_semaphore = SetSignalSemaphore;

  m_index = 0;
  m_created = true;
  return true;
}

void CVulkanBridge::ContextDestroy()
{
  if (m_requested)
    Teardown(true);
}

void CVulkanBridge::Teardown(bool coreLoaded)
{
  if (m_device)
  {
    m_vk->DeviceWaitIdle(m_device);
    DestroyFrames();

    // the core frees what it made on the device, the device is ours
    if (coreLoaded && m_deviceByCore && m_negotiation && m_negotiation->destroy_device)
      m_negotiation->destroy_device();
    m_vk->DestroyDevice(m_device, nullptr);
  }
  m_device = VK_NULL_HANDLE;
  m_queue = VK_NULL_HANDLE;
  m_deviceByCore = false;
  m_syncFd = false;

  if (m_instance)
    m_vk->DestroyInstance(m_instance, nullptr);
  m_instance = VK_NULL_HANDLE;
  m_gpu = VK_NULL_HANDLE;

  m_haveImage = false;
  m_waitSemaphores.clear();
  m_coreCommands.clear();
  m_signalSemaphore = VK_NULL_HANDLE;
  m_interface = {};
  m_created = false;
}

// Kodi can drop the context without destroying it, then the core is unloaded
void CVulkanBridge::Reset()
{
  if (m_created)
    Teardown(false);
  m_requested = false;
  m_negotiation = nullptr;
}

const retro_hw_render_interface* CVulkanBridge::Interface() const
{
  return m_created ? reinterpret_cast<const retro_hw_render_interface*>(&m_interface) : nullptr;
}

bool CVulkanBridge::CreateExportImage(Frame& frame, unsigned int width, unsigned int height)
{
  VulkanFunctions& vk = *m_vk;

  EGLDisplay dpy = m_gl->GetCurrentDisplay();
  std::vector<uint64_t> common = SharedModifiers(m_gpu);
  if (common.empty())
  {
    kodi::Log(ADDON_LOG_ERROR, "Vulkan bridge: no DRM modifier both Vulkan and EGL can use");
    return false;
  }

  VkImageDrmFormatModifierListCreateInfoEXT listInfo{
      VK_STRUCTURE_TYPE_IMAGE_DRM_FORMAT_MODIFIER_LIST_CREATE_INFO_EXT};
  listInfo.drmFormatModifierCount = static_cast<uint32_t>(common.size());
  listInfo.pDrmFormatModifiers = common.data();
  VkExternalMemoryImageCreateInfo externalInfo{VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO};
  externalInfo.pNext = &listInfo;
  externalInfo.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT;
  VkImageCreateInfo ici{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
  ici.pNext = &externalInfo;
  ici.imageType = VK_IMAGE_TYPE_2D;
  ici.format = VK_FORMAT_R8G8B8A8_UNORM;
  ici.extent = {width, height, 1};
  ici.mipLevels = 1;
  ici.arrayLayers = 1;
  ici.samples = VK_SAMPLE_COUNT_1_BIT;
  ici.tiling = VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT;
  ici.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
  ici.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  if (vk.CreateImage(m_device, &ici, nullptr, &frame.image) != VK_SUCCESS)
    return false;

  VkImageDrmFormatModifierPropertiesEXT modifierProps{
      VK_STRUCTURE_TYPE_IMAGE_DRM_FORMAT_MODIFIER_PROPERTIES_EXT};
  vk.GetImageDrmFormatModifierPropertiesEXT(m_device, frame.image, &modifierProps);
  const uint64_t modifier = modifierProps.drmFormatModifier;

  VkMemoryRequirements req;
  vk.GetImageMemoryRequirements(m_device, frame.image, &req);
  VkPhysicalDeviceMemoryProperties memProps;
  vk.GetPhysicalDeviceMemoryProperties(m_gpu, &memProps);
  uint32_t memType = 0;
  for (; memType < memProps.memoryTypeCount; memType++)
  {
    if ((req.memoryTypeBits & (1u << memType)) &&
        (memProps.memoryTypes[memType].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT))
      break;
  }
  VkMemoryDedicatedAllocateInfo dedicated{VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO};
  dedicated.image = frame.image;
  VkExportMemoryAllocateInfo exportInfo{VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO};
  exportInfo.pNext = &dedicated;
  exportInfo.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT;
  VkMemoryAllocateInfo mai{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
  mai.pNext = &exportInfo;
  mai.allocationSize = req.size;
  mai.memoryTypeIndex = memType;
  if (vk.AllocateMemory(m_device, &mai, nullptr, &frame.memory) != VK_SUCCESS ||
      vk.BindImageMemory(m_device, frame.image, frame.memory, 0) != VK_SUCCESS)
    return false;

  VkSubresourceLayout layout;
  VkImageSubresource sub{VK_IMAGE_ASPECT_MEMORY_PLANE_0_BIT_EXT, 0, 0};
  vk.GetImageSubresourceLayout(m_device, frame.image, &sub, &layout);
  int fd = -1;
  VkMemoryGetFdInfoKHR fdInfo{VK_STRUCTURE_TYPE_MEMORY_GET_FD_INFO_KHR};
  fdInfo.memory = frame.memory;
  fdInfo.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT;
  if (vk.GetMemoryFdKHR(m_device, &fdInfo, &fd) != VK_SUCCESS)
    return false;

  const EGLint attribs[] = {EGL_WIDTH,
                            static_cast<EGLint>(width),
                            EGL_HEIGHT,
                            static_cast<EGLint>(height),
                            EGL_LINUX_DRM_FOURCC_EXT,
                            static_cast<EGLint>(DRM_FORMAT_ABGR8888_),
                            EGL_DMA_BUF_PLANE0_FD_EXT,
                            fd,
                            EGL_DMA_BUF_PLANE0_OFFSET_EXT,
                            static_cast<EGLint>(layout.offset),
                            EGL_DMA_BUF_PLANE0_PITCH_EXT,
                            static_cast<EGLint>(layout.rowPitch),
                            EGL_DMA_BUF_PLANE0_MODIFIER_LO_EXT,
                            static_cast<EGLint>(modifier & 0xffffffff),
                            EGL_DMA_BUF_PLANE0_MODIFIER_HI_EXT,
                            static_cast<EGLint>(modifier >> 32),
                            EGL_NONE};
  EGLImageKHR image =
      m_gl->CreateImageKHR(dpy, EGL_NO_CONTEXT, EGL_LINUX_DMA_BUF_EXT, nullptr, attribs);
  close(fd); // the EGLImage holds its own reference
  if (image == EGL_NO_IMAGE_KHR)
  {
    kodi::Log(ADDON_LOG_ERROR, "Vulkan bridge: eglCreateImageKHR failed: 0x%x", m_gl->GetError());
    return false;
  }
  frame.eglImage = image;

  GLint oldTexture = 0;
  GLint oldRead = 0;
  m_gl->GetIntegerv(GL_TEXTURE_BINDING_2D, &oldTexture);
  m_gl->GetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &oldRead);

  m_gl->GenTextures(1, &frame.texture);
  m_gl->BindTexture(GL_TEXTURE_2D, frame.texture);
  m_gl->EGLImageTargetTexture2DOES(GL_TEXTURE_2D, image);
  m_gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  m_gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  m_gl->GenFramebuffers(1, &frame.framebuffer);
  m_gl->BindFramebuffer(GL_READ_FRAMEBUFFER, frame.framebuffer);
  m_gl->FramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                             frame.texture, 0);
  const GLenum status = m_gl->CheckFramebufferStatus(GL_READ_FRAMEBUFFER);

  m_gl->BindTexture(GL_TEXTURE_2D, static_cast<GLuint>(oldTexture));
  m_gl->BindFramebuffer(GL_READ_FRAMEBUFFER, static_cast<GLuint>(oldRead));

  if (status != GL_FRAMEBUFFER_COMPLETE)
  {
    kodi::Log(ADDON_LOG_ERROR,
              "Vulkan bridge: imported texture isn't a complete framebuffer (0x%x)", status);
    return false;
  }

  frame.width = width;
  frame.height = height;
  kodi::Log(ADDON_LOG_DEBUG, "Vulkan bridge: %ux%u dma-buf, modifier 0x%016llx, pitch %llu", width,
            height, static_cast<unsigned long long>(modifier),
            static_cast<unsigned long long>(layout.rowPitch));
  return true;
}

void CVulkanBridge::DestroyExportImage(Frame& frame)
{
  if (m_gl && m_gl->GetCurrentContext() != EGL_NO_CONTEXT)
  {
    if (frame.glFence)
      m_gl->DeleteSync(static_cast<GLsync>(frame.glFence));
    if (frame.framebuffer)
      m_gl->DeleteFramebuffers(1, &frame.framebuffer);
    if (frame.texture)
      m_gl->DeleteTextures(1, &frame.texture);
    if (frame.eglImage)
      m_gl->DestroyImageKHR(m_gl->GetCurrentDisplay(), static_cast<EGLImageKHR>(frame.eglImage));
  }
  frame.glFence = nullptr;
  frame.framebuffer = 0;
  frame.texture = 0;
  frame.eglImage = nullptr;

  if (frame.image)
    m_vk->DestroyImage(m_device, frame.image, nullptr);
  if (frame.memory)
    m_vk->FreeMemory(m_device, frame.memory, nullptr);
  frame.image = VK_NULL_HANDLE;
  frame.memory = VK_NULL_HANDLE;
  frame.width = 0;
  frame.height = 0;
}

// Both APIs must be done with a frame before it's used again
void CVulkanBridge::WaitFrame(Frame& frame)
{
  {
    std::unique_lock<std::mutex> lock(m_frameMutex);
    if (frame.submitted)
    {
      m_vk->WaitForFences(m_device, 1, &frame.fence, VK_TRUE, UINT64_MAX);
      m_vk->ResetFences(m_device, 1, &frame.fence);
      frame.submitted = false;
    }
  }
  if (frame.glFence)
  {
    m_gl->ClientWaitSync(static_cast<GLsync>(frame.glFence), GL_SYNC_FLUSH_COMMANDS_BIT,
                         1000000000);
    m_gl->DeleteSync(static_cast<GLsync>(frame.glFence));
    frame.glFence = nullptr;
  }
}

// GLES waits for the copy on the GPU, through a sync_file from its semaphore
bool CVulkanBridge::WaitOnGpu(Frame& frame)
{
  if (!m_syncFd)
    return false;

  int fd = -1;
  VkSemaphoreGetFdInfoKHR info{VK_STRUCTURE_TYPE_SEMAPHORE_GET_FD_INFO_KHR};
  info.semaphore = frame.copied;
  info.handleType = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_SYNC_FD_BIT;
  EGLSyncKHR sync = EGL_NO_SYNC_KHR;
  EGLDisplay dpy = m_gl->GetCurrentDisplay();
  if (m_vk->GetSemaphoreFdKHR(m_device, &info, &fd) == VK_SUCCESS)
  {
    if (fd < 0)
      return true; // already signalled
    const EGLint attribs[] = {EGL_SYNC_NATIVE_FENCE_FD_ANDROID, fd, EGL_NONE};
    sync = m_gl->CreateSyncKHR(dpy, EGL_SYNC_NATIVE_FENCE_ANDROID, attribs);
  }
  if (sync == EGL_NO_SYNC_KHR)
  {
    // a semaphore left signalled can't be signalled again, so stop using them
    if (fd >= 0)
      close(fd);
    kodi::Log(ADDON_LOG_WARNING,
              "Vulkan bridge: can't pass the copy's fence to GLES, waiting on the CPU");
    m_syncFd = false;
    return false;
  }
  m_gl->WaitSyncKHR(dpy, sync, 0);
  m_gl->DestroySyncKHR(dpy, sync);
  return true;
}

bool CVulkanBridge::Present(unsigned int width, unsigned int height, uintptr_t framebuffer)
{
  if (!m_created)
    return false;

  // nothing to copy, but the core's work and semaphore still go out
  if (!m_haveImage || width == 0 || height == 0 || framebuffer == 0)
  {
    SubmitCoreWork();
    return false;
  }

  VulkanFunctions& vk = *m_vk;
  Frame& frame = m_frames[m_index];
  WaitFrame(frame);

  if (width != frame.width || height != frame.height)
  {
    DestroyExportImage(frame);
    if (!CreateExportImage(frame, width, height))
    {
      DestroyExportImage(frame);
      SubmitCoreWork();
      return false;
    }
  }

  const VkImage coreImage = m_image.create_info.image;
  const VkImageLayout coreLayout = m_image.image_layout;
  // the frontend may not change the layout of an image shared in GENERAL
  const VkImageLayout readLayout = coreLayout == VK_IMAGE_LAYOUT_GENERAL
                                       ? VK_IMAGE_LAYOUT_GENERAL
                                       : VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

  // the core's image may belong to another queue family; ownership moves in its own
  // layout, matching the core's release, and our layout change is a separate barrier
  const bool transfer =
      m_srcQueueFamily != VK_QUEUE_FAMILY_IGNORED && m_srcQueueFamily != m_queueFamily;

  VkCommandBufferBeginInfo begin{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
  begin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
  vk.BeginCommandBuffer(frame.cmd, &begin);
  if (transfer)
    Barrier(vk.CmdPipelineBarrier, frame.cmd, coreImage, coreLayout, coreLayout, 0,
            VK_ACCESS_TRANSFER_READ_BIT, m_srcQueueFamily, m_queueFamily);
  if (!transfer || readLayout != coreLayout)
    Barrier(vk.CmdPipelineBarrier, frame.cmd, coreImage, coreLayout, readLayout,
            VK_ACCESS_MEMORY_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT);
  Barrier(vk.CmdPipelineBarrier, frame.cmd, frame.image, VK_IMAGE_LAYOUT_UNDEFINED,
          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 0, VK_ACCESS_TRANSFER_WRITE_BIT,
          VK_QUEUE_FAMILY_FOREIGN_EXT, m_queueFamily);
  VkImageBlit blit{};
  blit.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
  blit.srcOffsets[1] = {static_cast<int32_t>(width), static_cast<int32_t>(height), 1};
  blit.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
  blit.dstOffsets[1] = {static_cast<int32_t>(width), static_cast<int32_t>(height), 1};
  vk.CmdBlitImage(frame.cmd, coreImage, readLayout, frame.image,
                  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_NEAREST);
  Barrier(vk.CmdPipelineBarrier, frame.cmd, frame.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
          VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_TRANSFER_WRITE_BIT, 0, m_queueFamily,
          VK_QUEUE_FAMILY_FOREIGN_EXT);
  if (!transfer || readLayout != coreLayout)
    Barrier(vk.CmdPipelineBarrier, frame.cmd, coreImage, readLayout, coreLayout,
            VK_ACCESS_TRANSFER_READ_BIT, VK_ACCESS_MEMORY_READ_BIT);
  if (transfer)
    Barrier(vk.CmdPipelineBarrier, frame.cmd, coreImage, coreLayout, coreLayout,
            VK_ACCESS_TRANSFER_READ_BIT, 0, m_queueFamily, m_srcQueueFamily);
  vk.EndCommandBuffer(frame.cmd);

  // set_image's semaphores are ignored when the core hands over command buffers
  std::vector<VkSemaphore> wait;
  if (m_coreCommands.empty())
    wait = m_waitSemaphores;
  std::vector<VkCommandBuffer> commands = m_coreCommands;
  commands.push_back(frame.cmd);
  std::vector<VkPipelineStageFlags> waitStages(wait.size(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
  std::vector<VkSemaphore> signal;
  if (m_signalSemaphore)
    signal.push_back(m_signalSemaphore);
  if (m_syncFd)
    signal.push_back(frame.copied);

  VkSubmitInfo submit{VK_STRUCTURE_TYPE_SUBMIT_INFO};
  submit.waitSemaphoreCount = static_cast<uint32_t>(wait.size());
  submit.pWaitSemaphores = wait.data();
  submit.pWaitDstStageMask = waitStages.data();
  submit.commandBufferCount = static_cast<uint32_t>(commands.size());
  submit.pCommandBuffers = commands.data();
  submit.signalSemaphoreCount = static_cast<uint32_t>(signal.size());
  submit.pSignalSemaphores = signal.data();

  VkResult result;
  {
    std::unique_lock<std::mutex> lock(m_queueMutex);
    result = vk.QueueSubmit(m_queue, 1, &submit, frame.fence);
  }
  m_coreCommands.clear();
  m_waitSemaphores.clear();
  m_signalSemaphore = VK_NULL_HANDLE;
  if (result != VK_SUCCESS)
  {
    kodi::Log(ADDON_LOG_ERROR, "Vulkan bridge: vkQueueSubmit failed: %d", result);
    return false;
  }
  {
    std::unique_lock<std::mutex> lock(m_frameMutex);
    frame.submitted = true;
  }

  if (!WaitOnGpu(frame))
    vk.WaitForFences(m_device, 1, &frame.fence, VK_TRUE, UINT64_MAX);

  // Vulkan's first row is the top of the picture, the framebuffer's is the bottom
  GLint oldRead = 0;
  GLint oldDraw = 0;
  m_gl->GetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &oldRead);
  m_gl->GetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &oldDraw);
  m_gl->BindFramebuffer(GL_READ_FRAMEBUFFER, frame.framebuffer);
  m_gl->BindFramebuffer(GL_DRAW_FRAMEBUFFER, static_cast<GLuint>(framebuffer));
  m_gl->BlitFramebuffer(0, 0, width, height, 0, height, width, 0, GL_COLOR_BUFFER_BIT, GL_NEAREST);
  m_gl->BindFramebuffer(GL_READ_FRAMEBUFFER, static_cast<GLuint>(oldRead));
  m_gl->BindFramebuffer(GL_DRAW_FRAMEBUFFER, static_cast<GLuint>(oldDraw));
  frame.glFence = m_gl->FenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

  std::unique_lock<std::mutex> lock(m_frameMutex);
  m_index = (m_index + 1) % FRAMES;
  return true;
}

void CVulkanBridge::Dupe()
{
  if (m_created)
    SubmitCoreWork();
}

// Without a copy, e.g. on a dupe: no semaphore is waited on, but the core's command
// buffers still run once and its semaphore is still signalled; the frame's fence
// covers them for wait_sync_index
void CVulkanBridge::SubmitCoreWork()
{
  if (!m_coreCommands.empty() || m_signalSemaphore)
  {
    Frame& frame = m_frames[m_index];
    WaitFrame(frame);

    VkSubmitInfo submit{VK_STRUCTURE_TYPE_SUBMIT_INFO};
    submit.commandBufferCount = static_cast<uint32_t>(m_coreCommands.size());
    submit.pCommandBuffers = m_coreCommands.data();
    submit.signalSemaphoreCount = m_signalSemaphore ? 1 : 0;
    submit.pSignalSemaphores = &m_signalSemaphore;

    VkResult result;
    {
      std::unique_lock<std::mutex> lock(m_queueMutex);
      result = m_vk->QueueSubmit(m_queue, 1, &submit, frame.fence);
    }
    if (result == VK_SUCCESS)
    {
      std::unique_lock<std::mutex> lock(m_frameMutex);
      frame.submitted = true;
    }
    else
      kodi::Log(ADDON_LOG_ERROR, "Vulkan bridge: vkQueueSubmit failed: %d", result);
  }
  m_coreCommands.clear();
  m_waitSemaphores.clear();
  m_signalSemaphore = VK_NULL_HANDLE;
}

void CVulkanBridge::SetImage(void* handle,
                             const retro_vulkan_image* image,
                             uint32_t numSemaphores,
                             const VkSemaphore* semaphores,
                             uint32_t srcQueueFamily)
{
  auto* self = static_cast<CVulkanBridge*>(handle);
  // a core takes its image back with null, e.g. before it re-creates its video
  if (image == nullptr)
  {
    self->m_haveImage = false;
    self->m_waitSemaphores.clear();
    return;
  }
  self->m_image = *image;
  self->m_haveImage = true;
  self->m_srcQueueFamily = srcQueueFamily;
  self->m_waitSemaphores.assign(semaphores, semaphores + numSemaphores);
}

uint32_t CVulkanBridge::GetSyncIndex(void* handle)
{
  auto* self = static_cast<CVulkanBridge*>(handle);
  std::unique_lock<std::mutex> lock(self->m_frameMutex);
  return self->m_index;
}

uint32_t CVulkanBridge::GetSyncIndexMask(void* handle)
{
  return (1u << FRAMES) - 1;
}

void CVulkanBridge::SetCommandBuffers(void* handle, uint32_t num, const VkCommandBuffer* cmd)
{
  auto* self = static_cast<CVulkanBridge*>(handle);
  self->m_coreCommands.assign(cmd, cmd + num);
}

void CVulkanBridge::WaitSyncIndex(void* handle)
{
  auto* self = static_cast<CVulkanBridge*>(handle);
  std::unique_lock<std::mutex> lock(self->m_frameMutex);
  const Frame& frame = self->m_frames[self->m_index];
  if (frame.submitted)
    self->m_vk->WaitForFences(self->m_device, 1, &frame.fence, VK_TRUE, UINT64_MAX);
}

void CVulkanBridge::LockQueue(void* handle)
{
  static_cast<CVulkanBridge*>(handle)->m_queueMutex.lock();
}

void CVulkanBridge::UnlockQueue(void* handle)
{
  static_cast<CVulkanBridge*>(handle)->m_queueMutex.unlock();
}

void CVulkanBridge::SetSignalSemaphore(void* handle, VkSemaphore semaphore)
{
  static_cast<CVulkanBridge*>(handle)->m_signalSemaphore = semaphore;
}
