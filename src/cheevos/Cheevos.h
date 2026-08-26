/*
 *  Copyright (C) 2020-2024 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */
#pragma once

#include <atomic>
#include <functional>
#include <future>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#define RC_CLIENT_SUPPORTS_HASH
#include <rcheevos/rc_client.h>
#include <rcheevos/rc_libretro.h>

namespace kodi
{
namespace addon
{
class CInstanceGame;
}
} // namespace kodi

namespace LIBRETRO
{

/// @brief Callback to read core memory: (type) -> (data_ptr, size)
using MemoryAccessCallback = std::function<bool(unsigned int type, uint8_t*& data, size_t& size)>;

class CCheevos
{
public:
  CCheevos();
  static CCheevos& Get();

  /// @brief Initialize rc_client and begin login + game identification
  void Initialize(kodi::addon::CInstanceGame* gameInstance,
                  const std::string& gamePath,
                  MemoryAccessCallback memoryCallback);
  void Deinitialize();

  /// @brief Store credentials from Kodi (called before LoadGame)
  void SetCredentials(const std::string& username, const std::string& token);

  /// @brief Called every emulated frame from RunFrame()
  void DoFrame();

  /*!
   * @brief Send progress for every measured achievement to the frontend
   *
   * rc_client's progress-indicator events name a single achievement, but the
   * frontend renders a snapshot of the whole list, so the full set is
   * published whenever any of it changes.
   */
  void PublishAchievementProgress();

  /*!
   * @brief Size of the achievement progress blob, or 0 if there is nothing to save
   *
   * Returns 0 when no game is loaded in rc_client, so that savestates taken
   * with achievements inactive keep their original layout.
   */
  size_t ProgressSize();

  /*!
   * @brief Whether the achievement runtime exists at all
   *
   * True once rc_client has been created, which happens before it has
   * finished identifying the game. Distinguishes "achievements are in use but
   * not ready yet" from "achievements are switched off", so that savestates
   * only carry the cost of reserved progress space when it can be needed.
   */
  bool IsActive() const { return m_rcClient != nullptr; }

  /*!
   * @brief Serialize achievement progress into a bounded buffer
   *
   * \param buffer Destination
   * \param size Bytes available in \p buffer
   * \return Bytes written, or 0 if progress could not be saved
   */
  size_t SerializeProgress(uint8_t* buffer, size_t size);

  /*!
   * @brief Restore achievement progress previously written by SerializeProgress()
   *
   * \return True on success. A failure is not fatal: the emulator state is
   *         still restored, the achievement runtime just keeps its current
   *         state.
   */
  bool DeserializeProgress(const uint8_t* buffer, size_t size);

private:
  /*!
   * @brief Apply progress that arrived before the game had been identified
   *
   * Called once the game load completes. See DeserializeProgress().
   */
  void ApplyPendingProgress();

  /*!
   * @brief Start the login if the client is up and credentials are known
   *
   * Called from both Initialize() and SetCredentials(), because the frontend
   * may supply credentials either before or after the game is loaded. Does
   * nothing on the second call.
   */
  void BeginLogin();

  // rc_client C callbacks (static — use s_instance to get back to instance)
  static void RcheevosEventHandler(const rc_client_event_t* event, rc_client_t* client);
  static void RcheevosServerCall(const rc_api_request_t* request,
                                 rc_client_server_callback_t callback,
                                 void* callback_data,
                                 rc_client_t* client);
  static uint32_t RcheevosReadMemory(uint32_t address, uint8_t* buffer,
                                     uint32_t num_bytes, rc_client_t* client);
  static void RcheevosGetCoreMemoryInfo(unsigned int id,
                                         rc_libretro_core_memory_info_t* info);
  static void RcheevosLoginCallback(int result, const char* errorMessage, rc_client_t* client,
                                    void* userdata);
  static void RcheevosGameLoadCallback(int result, const char* errorMessage, rc_client_t* client,
                                       void* userdata);

  // Rich presence ping thread
  void RichPresencePingThread();

  // rc_client state
  rc_client_t* m_rcClient{nullptr};
  kodi::addon::CInstanceGame* m_gameInstance{nullptr};
  std::string m_gamePath;

  // Credentials (guarded by m_credentialsMutex)
  std::string m_userName;
  std::string m_loginToken;
  mutable std::mutex m_credentialsMutex;

  // Identifies this add-on to RetroAchievements, built once in Initialize()
  std::string m_userAgent;
  mutable std::mutex m_userAgentMutex;
  std::atomic<bool> m_loginStarted{false};

  // Memory access
  MemoryAccessCallback m_memoryCallback;
  rc_libretro_memory_regions_t m_memoryRegions{};
  /*!
   * @brief Rebuild the achievement list for progress roughly once a second
   *
   * Cores run at 50-60Hz, so this is close enough to a second on any of them
   * without needing a clock.
   */
  static constexpr unsigned int PROGRESS_PUBLISH_INTERVAL_FRAMES = 60;
  unsigned int m_framesSincePublish{0};

  /// @brief Last published progress, used to suppress unchanged updates
  std::string m_lastProgressSignature;

  /// @brief Whether the "progress doesn't fit" warning has been issued
  bool m_progressTooLargeWarned{false};

  std::atomic<bool> m_memoryInitialized{false};
  std::mutex m_memoryMutex;

  // Progress handed over before the game was identified, applied once it is.
  // Empty with the flag set means a reset was asked for.
  std::mutex m_pendingProgressMutex;
  std::vector<uint8_t> m_pendingProgress;
  bool m_hasPendingProgress{false};

  // Background threads
  std::atomic<bool> m_richPresenceRunning{false};
  std::thread m_richPresenceThread;
  /*!
   * @brief In-flight HTTP requests to RetroAchievements
   *
   * Each holds a callback_data pointer owned by rc_client, so these are waited
   * on in Deinitialize() before the client is destroyed rather than detached.
   */
  std::vector<std::future<void>> m_serverCalls;
  std::mutex m_serverCallsMutex;

  /// @brief Set during teardown so no further requests reach the network
  std::atomic<bool> m_shuttingDown{false};
};

} // namespace LIBRETRO
