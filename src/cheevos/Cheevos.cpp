/*
 *  Copyright (C) 2020-2024 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#include "Cheevos.h"

#include "CheevosUtils.h"
#include "libretro/LibretroEnvironment.h"
#include "libretro/MemoryMap.h"
#include "utils/Base64.h"

#include <kodi/Filesystem.h>
#include <kodi/General.h>
#include <kodi/Network.h>
#include <kodi/addon-instance/Game.h>
#include <kodi/c-api/addon-instance/game.h>

#include <rcheevos/rc_api_runtime.h>
#include <rcheevos/rc_client.h>
#include <rcheevos/rc_libretro.h>

#include <algorithm>
#include <chrono>
#include <cstring>
#include <future>
#include <vector>

using namespace LIBRETRO;

namespace
{
// RetroAchievements identifies the client by User-Agent, so it has to name
// this add-on and its version rather than a hardcoded string, and carry
// rcheevos' own clause as that library documents.
constexpr const char* RA_CLIENT_NAME = "KodiRetroPlayer";
constexpr size_t RA_USER_AGENT_CLAUSE_SIZE = 64;
constexpr unsigned int GAME_LOAD_RETRY_MAX_DELAY_SECONDS = 120;
constexpr unsigned int LOGIN_RETRY_MAX_DELAY_SECONDS = 120;
constexpr std::chrono::minutes RP_PING_INTERVAL{2};
constexpr unsigned int RP_BUFFER_SIZE = 512;
constexpr std::chrono::seconds HTTP_REQUEST_TIMEOUT{30};
constexpr const char* HTTP_CONNECTION_TIMEOUT_SECONDS = "10";
constexpr size_t HTTP_READ_CHUNK = 4096;

} // namespace

// rc_libretro's get_core_memory_info callback takes neither a client nor a
// userdata pointer, so it alone has no way to be handed the instance. It is
// only ever called from inside rc_libretro_memory_init(), which we call
// ourselves, so the pointer is set around that call and cleared after rather
// than being left set for the life of the add-on. Every rc_client callback
// carries rc_client_t* and reads the instance back with
// rc_client_get_userdata() instead.
static CCheevos* s_memoryInfoInstance = nullptr;

CCheevos::CCheevos() = default;

CCheevos& CCheevos::Get()
{
  static CCheevos instance;
  return instance;
}

void CCheevos::Initialize(kodi::addon::CInstanceGame* gameInstance,
                          const std::string& gamePath,
                          MemoryAccessCallback memoryCallback)
{
  Deinitialize();

  // Deinitialize() latches the shutdown flag to drain in-flight requests, and
  // loading a new game runs through it first. Clearing it here is what lets
  // requests reach the network again.
  m_shuttingDown = false;

  m_gameInstance = gameInstance;
  m_gamePath = gamePath;
  m_memoryCallback = std::move(memoryCallback);

  // Create rc_client
  m_rcClient = rc_client_create(RcheevosReadMemory, RcheevosServerCall);
  if (m_rcClient == nullptr)
  {
    kodi::Log(ADDON_LOG_ERROR, "CCheevos: failed to create rc_client");
    return;
  }

  // rc_client's callbacks take no userdata parameter of their own, but they
  // all receive the client, and the client will carry one for us
  rc_client_set_userdata(m_rcClient, this);

  rc_client_set_event_handler(m_rcClient, RcheevosEventHandler);
  rc_client_set_hardcore_enabled(m_rcClient, 0);

  {
    char clause[RA_USER_AGENT_CLAUSE_SIZE]{};
    rc_client_get_user_agent_clause(m_rcClient, clause, sizeof(clause));

    // RetroAchievements identifies the integration by the leading client
    // name and version, so those stay ours. Everything after describes the
    // host, and that comes from Kodi rather than being assembled here, so it
    // stays right when Kodi's own reporting changes.
    std::string userAgent = std::string(RA_CLIENT_NAME) + "/" + kodi::addon::GetAddonInfo("version");

    const std::string kodiUserAgent = kodi::network::GetUserAgent();
    if (!kodiUserAgent.empty())
      userAgent += " (" + kodiUserAgent + ")";

    if (clause[0] != '\0')
      userAgent += " " + std::string(clause);

    std::lock_guard<std::mutex> lock(m_userAgentMutex);
    m_userAgent = std::move(userAgent);
    kodi::Log(ADDON_LOG_DEBUG, "CCheevos: user agent '%s'", m_userAgent.c_str());
  }

  // Enable verbose logging
  rc_client_enable_logging(m_rcClient, RC_CLIENT_LOG_LEVEL_VERBOSE,
    [](const char* message, const rc_client_t*) {
      kodi::Log(ADDON_LOG_DEBUG, "rc_client: %s", message);
    });

  // The frontend may not have supplied credentials yet, in which case
  // SetCredentials() starts the login when they arrive
  BeginLogin();
}

void CCheevos::BeginLogin()
{
  if (m_rcClient == nullptr || m_loginStarted)
    return;

  std::string userName;
  std::string loginToken;
  {
    std::lock_guard<std::mutex> lock(m_credentialsMutex);
    userName = m_userName;
    loginToken = m_loginToken;
  }

  if (userName.empty() || loginToken.empty())
  {
    kodi::Log(ADDON_LOG_INFO, "CCheevos: no RA credentials yet, deferring login");
    return;
  }

  m_loginStarted = true;
  m_loginRetryScheduled = false;

  // Begin async login — game load triggered on success
  kodi::Log(ADDON_LOG_INFO, "CCheevos: logging in as '%s'", userName.c_str());
  rc_client_begin_login_with_token(m_rcClient, userName.c_str(), loginToken.c_str(),
                                   RcheevosLoginCallback, this);
}

void CCheevos::BeginGameLoad()
{
  if (m_rcClient == nullptr || m_gamePath.empty())
    return;

  m_gameLoadRetryScheduled = false;

  kodi::Log(ADDON_LOG_INFO, "CCheevos: identifying game: %s", m_gamePath.c_str());

  // rc_hash_generate_from_file() cannot identify a game whose console is not
  // known up front. Let rc_client try every suitable hashing method instead.
  rc_client_begin_identify_and_load_game(m_rcClient, RC_CONSOLE_UNKNOWN, m_gamePath.c_str(),
                                         nullptr, 0, RcheevosGameLoadCallback, this);
}

void CCheevos::Deinitialize()
{
  std::vector<std::future<void>> inFlight;
  {
    std::lock_guard<std::mutex> lock(m_serverCallsMutex);
    m_shuttingDown = true;
    inFlight.swap(m_serverCalls);
  }

  for (std::future<void>& call : inFlight)
  {
    if (call.valid())
      call.wait();
  }

  // callback_data belongs to rc_client, so queued responses must be consumed
  // before the client is destroyed.
  DispatchServerResponses();

  if (m_memoryInitialized)
  {
    rc_libretro_memory_destroy(&m_memoryRegions);
    m_memoryInitialized = false;
  }

  if (m_rcClient != nullptr)
  {
    rc_client_destroy(m_rcClient);
    m_rcClient = nullptr;
  }

  {
    std::lock_guard<std::mutex> lock(m_pendingProgressMutex);
    m_pendingProgress.clear();
    m_hasPendingProgress = false;
  }

  m_loginStarted = false;
  m_loginRetryScheduled = false;
  m_loginRetryDelaySeconds = 0;
  m_gameLoadRetryScheduled = false;
  m_gameLoadRetryDelaySeconds = 0;
  m_richPresenceActive = false;
  m_lastProgressSignature.clear();
  m_gameInstance = nullptr;
}

void CCheevos::SetCredentials(const std::string& username, const std::string& token)
{
  {
    std::lock_guard<std::mutex> lock(m_credentialsMutex);
    m_userName = username;
    m_loginToken = token;
  }

  m_loginRetryScheduled = false;
  m_loginRetryDelaySeconds = 0;

  // Kodi supplies credentials after the game has been loaded, so this is
  // normally where the login actually starts
  BeginLogin();
}

void CCheevos::DoFrame()
{
  if (m_rcClient == nullptr)
    return;

  DispatchServerResponses();

  if (m_loginRetryScheduled && std::chrono::steady_clock::now() >= m_nextLoginAttempt)
    BeginLogin();

  if (m_gameLoadRetryScheduled && std::chrono::steady_clock::now() >= m_nextGameLoadAttempt)
    BeginGameLoad();

  rc_client_do_frame(m_rcClient);

  // Progress is read from the achievement list rather than driven by
  // rc_client's progress-indicator events. Those fire only for the single
  // achievement rc_client considers most relevant, and only for as long as an
  // on-screen indicator would be showing, so most measured achievements never
  // produce one and their progress would never reach the frontend.
  //
  // The list is only rebuilt once a second, and nothing is sent unless a value
  // actually changed, so this costs little per frame.
  if (++m_framesSincePublish >= PROGRESS_PUBLISH_INTERVAL_FRAMES)
  {
    m_framesSincePublish = 0;
    PublishAchievementProgress();
  }

  UpdateRichPresence();
}

void CCheevos::ResetRuntime()
{
  if (m_rcClient != nullptr)
    rc_client_reset(m_rcClient);
}

void CCheevos::PublishAchievementProgress()
{
  if (m_rcClient == nullptr || m_gameInstance == nullptr)
    return;

  // String pointers reference rc_client-owned memory, valid until the list is
  // destroyed, so the callback has to happen before that
  rc_client_achievement_list_t* achList = rc_client_create_achievement_list(
      m_rcClient, RC_CLIENT_ACHIEVEMENT_CATEGORY_CORE,
      RC_CLIENT_ACHIEVEMENT_LIST_GROUPING_LOCK_STATE);
  if (achList == nullptr)
    return;

  std::vector<game_rc_achievement_progress> progress;
  std::string signature;

  for (uint32_t b = 0; b < achList->num_buckets; ++b)
  {
    const auto& bucket = achList->buckets[b];
    for (uint32_t a = 0; a < bucket.num_achievements; ++a)
    {
      const rc_client_achievement_t* ach = bucket.achievements[a];

      // Achievements without a counting trigger report an empty string here,
      // and there is nothing to draw a bar from
      if (ach->measured_progress[0] == '\0')
        continue;

      game_rc_achievement_progress entry{};
      entry.id = ach->id;
      entry.measured_percent = ach->measured_percent;
      entry.measured_progress = ach->measured_progress;
      progress.push_back(entry);

      signature += std::to_string(ach->id);
      signature += '=';
      signature += ach->measured_progress;
      signature += ';';
    }
  }

  // Called once a second, so skip the crossing into Kodi unless something the
  // frontend would draw has actually changed
  if (signature != m_lastProgressSignature)
  {
    m_lastProgressSignature = signature;

    kodi::Log(ADDON_LOG_DEBUG, "CCheevos: publishing progress for %zu achievement(s): %s",
              progress.size(), signature.c_str());
    m_gameInstance->RCOnAchievementProgress(progress);
  }

  rc_client_destroy_achievement_list(achList);
}

void CCheevos::ApplyPendingProgress()
{
  std::vector<uint8_t> progress;

  {
    std::lock_guard<std::mutex> lock(m_pendingProgressMutex);
    if (!m_hasPendingProgress)
      return;

    progress = std::move(m_pendingProgress);
    m_pendingProgress.clear();
    m_hasPendingProgress = false;
  }

  if (m_rcClient == nullptr || !rc_client_is_game_loaded(m_rcClient))
    return;

  const int result = progress.empty()
                         ? rc_client_deserialize_progress_sized(m_rcClient, nullptr, 0)
                         : rc_client_deserialize_progress_sized(m_rcClient, progress.data(),
                                                                progress.size());

  if (result == RC_OK)
    kodi::Log(ADDON_LOG_INFO, "CCheevos: applied %zu bytes of held progress", progress.size());
  else
    kodi::Log(ADDON_LOG_ERROR, "CCheevos: held progress refused (%d)", result);
}

size_t CCheevos::ProgressSize()
{
  // rc_client_progress_size() asserts if no game is loaded, and there is
  // nothing worth saving in that case anyway
  if (m_rcClient == nullptr || !rc_client_is_game_loaded(m_rcClient))
    return 0;

  return rc_client_progress_size(m_rcClient);
}

size_t CCheevos::SerializeProgress(uint8_t* buffer, size_t size)
{
  if (buffer == nullptr || size == 0)
    return 0;

  if (m_rcClient == nullptr || !rc_client_is_game_loaded(m_rcClient))
    return 0;

  const size_t required = rc_client_progress_size(m_rcClient);
  if (required == 0)
    return 0;

  if (required > size)
  {
    // Warned once per game so the reserved size can be revisited rather than
    // silently dropping progress from every savestate
    if (!m_progressTooLargeWarned)
    {
      m_progressTooLargeWarned = true;
      kodi::Log(ADDON_LOG_WARNING,
                "CCheevos: achievement progress needs %zu bytes but only %zu are reserved in the "
                "savestate; progress will not be saved for this game",
                required, size);
    }
    return 0;
  }

  // The _sized() variant is bounded, so a runtime that grew since the size was
  // queried truncates cleanly instead of running off the end of the buffer
  if (rc_client_serialize_progress_sized(m_rcClient, buffer, size) != RC_OK)
    return 0;

  return required;
}

bool CCheevos::DeserializeProgress(const uint8_t* buffer, size_t size)
{
  if (m_rcClient == nullptr)
    return false;

  if (!rc_client_is_game_loaded(m_rcClient))
  {
    // The frontend restores a savestate as part of loading it, which is before
    // the game has been identified here: signing in and identifying are both
    // round trips to the server and are still in flight at this point. Hold
    // what we were given and apply it when the game arrives -- dropping it is
    // why restoring progress never took effect.
    std::lock_guard<std::mutex> lock(m_pendingProgressMutex);
    if (buffer != nullptr && size > 0)
      m_pendingProgress.assign(buffer, buffer + size);
    else
      m_pendingProgress.clear();
    m_hasPendingProgress = true;

    kodi::Log(ADDON_LOG_DEBUG, "CCheevos: game not identified yet, holding %zu bytes of progress",
              size);

    return true;
  }

  // A null buffer is passed through rather than refused. rcheevos treats it as
  // "no progress to restore" and resets the runtime, which is what a savestate
  // carrying none needs: emulator memory has jumped, so every delta, prior
  // value and hit count the runtime holds describes a moment that no longer
  // follows from it.
  if (buffer == nullptr || size == 0)
  {
    kodi::Log(ADDON_LOG_INFO,
              "CCheevos: savestate carries no achievement progress, runtime reset");

    return rc_client_deserialize_progress_sized(m_rcClient, nullptr, 0) == RC_OK;
  }

  return rc_client_deserialize_progress_sized(m_rcClient, buffer, size) == RC_OK;
}

// HTTP server callback — uses Kodi VFS (which wraps libcurl internally)
void CCheevos::RcheevosServerCall(const rc_api_request_t* request,
                                  rc_client_server_callback_t callback,
                                  void* callback_data,
                                  rc_client_t* client)
{
  const std::string url = (request->url != nullptr) ? request->url : "";
  const std::string postData = (request->post_data != nullptr) ? request->post_data : "";

  CCheevos* const cheevos = static_cast<CCheevos*>(rc_client_get_userdata(client));

  std::string userAgent;
  if (cheevos == nullptr)
  {
    rc_api_server_response_t resp{};
    resp.http_status_code = HTTP_STATUS_NO_RESPONSE;
    callback(&resp, callback_data);
    return;
  }

  {
    std::lock_guard<std::mutex> lock(cheevos->m_userAgentMutex);
    userAgent = cheevos->m_userAgent;
  }

  auto worker = [cheevos, url, postData, userAgent, callback, callback_data]()
  {
    std::string responseData;
    unsigned int statusCode = HTTP_STATUS_NO_RESPONSE;
    const auto deadline = std::chrono::steady_clock::now() + HTTP_REQUEST_TIMEOUT;

    try
    {
      // Options are set through the curl API rather than the "url|opt=value"
      // syntax, because the post body is form data full of '&' and '=' which
      // that syntax would split into further options
      kodi::vfs::CFile file;
      if (file.CURLCreate(url))
      {
        if (!userAgent.empty())
          file.CURLAddOption(ADDON_CURL_OPTION_PROTOCOL, "User-Agent", userAgent);

        file.CURLAddOption(ADDON_CURL_OPTION_PROTOCOL, "connection-timeout",
                           HTTP_CONNECTION_TIMEOUT_SECONDS);

        // RetroAchievements reports failures as a JSON body with a 4xx status,
        // and rc_client needs that body to explain the failure
        file.CURLAddOption(ADDON_CURL_OPTION_PROTOCOL, "failonerror", "false");

        // Kodi base64-decodes this option, so it has to be encoded here
        if (!postData.empty())
          file.CURLAddOption(ADDON_CURL_OPTION_PROTOCOL, "postdata", Base64Encode(postData));

        if (file.CURLOpen(ADDON_READ_NO_CACHE | ADDON_READ_TRUNCATED))
        {
          char buffer[HTTP_READ_CHUNK];
          ssize_t bytesRead;
          while (!cheevos->m_shuttingDown && std::chrono::steady_clock::now() < deadline &&
                 (bytesRead = file.Read(buffer, sizeof(buffer))) > 0)
            responseData.append(buffer, static_cast<size_t>(bytesRead));

          if (!cheevos->m_shuttingDown && std::chrono::steady_clock::now() < deadline)
          {
            statusCode = ParseHttpStatus(
                file.GetPropertyValue(ADDON_FILE_PROPERTY_RESPONSE_PROTOCOL, ""));
          }
          else
          {
            responseData.clear();
          }

          file.Close();
        }
      }
    }
    catch (...)
    {
      responseData.clear();
    }

    if (statusCode == HTTP_STATUS_NO_RESPONSE)
      kodi::Log(ADDON_LOG_ERROR, "CCheevos: request failed: %s", url.c_str());

    cheevos->QueueServerResponse(callback, callback_data, std::move(responseData),
                                 static_cast<int>(statusCode));
  };

  {
    std::lock_guard<std::mutex> lock(cheevos->m_serverCallsMutex);

    if (cheevos->m_shuttingDown)
    {
      cheevos->m_serverResponses.push_back(
          {callback, callback_data, {}, HTTP_STATUS_NO_RESPONSE});
      return;
    }

    std::erase_if(cheevos->m_serverCalls, [](const std::future<void>& call)
                  { return call.wait_for(std::chrono::seconds(0)) == std::future_status::ready; });

    try
    {
      cheevos->m_serverCalls.reserve(cheevos->m_serverCalls.size() + 1);
      cheevos->m_serverCalls.emplace_back(std::async(std::launch::async, std::move(worker)));
    }
    catch (...)
    {
      cheevos->m_serverResponses.push_back(
          {callback, callback_data, {}, HTTP_STATUS_NO_RESPONSE});
    }
  }
}

void CCheevos::QueueServerResponse(rc_client_server_callback_t callback,
                                   void* callbackData,
                                   std::string body,
                                   int statusCode)
{
  std::lock_guard<std::mutex> lock(m_serverCallsMutex);
  m_serverResponses.push_back({callback, callbackData, std::move(body), statusCode});
}

void CCheevos::DispatchServerResponses()
{
  for (;;)
  {
    std::vector<ServerResponse> responses;
    {
      std::lock_guard<std::mutex> lock(m_serverCallsMutex);
      responses.swap(m_serverResponses);
    }

    if (responses.empty())
      return;

    for (ServerResponse& completed : responses)
    {
      rc_api_server_response_t response{};
      response.body = completed.body.c_str();
      response.body_length = completed.body.size();
      response.http_status_code = completed.statusCode;
      completed.callback(&response, completed.callbackData);
    }
  }
}

// Memory read callback
uint32_t CCheevos::RcheevosReadMemory(uint32_t address, uint8_t* buffer,
                                       uint32_t num_bytes, rc_client_t* client)
{
  CCheevos* const cheevos = static_cast<CCheevos*>(rc_client_get_userdata(client));
  if (cheevos == nullptr)
    return 0;

  // rc_client validates every achievement's addresses as soon as the session
  // starts, which is before the game-load callback runs. If the mapping isn't
  // ready by then every achievement is disabled as out of range, so it is
  // built here on first use rather than after the game has loaded.
  if (!cheevos->m_memoryInitialized)
  {
    const rc_client_game_t* gameInfo = rc_client_get_game_info(client);
    if (gameInfo == nullptr)
      return 0;

    std::lock_guard<std::mutex> lock(cheevos->m_memoryMutex);
    if (!cheevos->m_memoryInitialized)
    {
      // rcheevos matches the regions a console is expected to have against the
      // core's memory map. Without one it can only see what the core exposes as
      // RETRO_MEMORY_SYSTEM_RAM, which is a single flat block: enough for a
      // console whose memory is one region, and nothing at all for one that is
      // split. The Saturn wants fourteen regions and matched none of them, so
      // every achievement it had was reported unsupported.
      const CMemoryMap& memoryMap = CLibretroEnvironment::Get().GetMemoryMap();

      std::vector<retro_memory_descriptor> descriptors;
      descriptors.reserve(memoryMap.Size());
      for (size_t i = 0; i < memoryMap.Size(); ++i)
        descriptors.emplace_back(memoryMap[static_cast<int>(i)].descriptor);

      retro_memory_map retroMemoryMap{};
      retroMemoryMap.descriptors = descriptors.data();
      retroMemoryMap.num_descriptors = static_cast<unsigned int>(descriptors.size());

      // A core that publishes no map is no worse off than before, and still
      // gets whatever it exposes as system RAM
      s_memoryInfoInstance = cheevos;
      rc_libretro_memory_init(&cheevos->m_memoryRegions,
                              descriptors.empty() ? nullptr : &retroMemoryMap,
                              RcheevosGetCoreMemoryInfo, gameInfo->console_id);
      s_memoryInfoInstance = nullptr;
      cheevos->m_memoryInitialized = true;

      kodi::Log(ADDON_LOG_INFO,
                "CCheevos: memory mapped for console %u from %zu descriptors, total_size=%u",
                gameInfo->console_id, descriptors.size(),
                cheevos->m_memoryRegions.total_size);
    }
  }

  return rc_libretro_memory_read(&cheevos->m_memoryRegions, address, buffer, num_bytes);
}

void CCheevos::RcheevosGetCoreMemoryInfo(unsigned int id,
                                          rc_libretro_core_memory_info_t* info)
{
  if (s_memoryInfoInstance == nullptr || !s_memoryInfoInstance->m_memoryCallback || info == nullptr)
    return;

  uint8_t* data = nullptr;
  size_t size = 0;
  s_memoryInfoInstance->m_memoryCallback(id, data, size);
  info->data = data;
  info->size = size;
}

// Event handler
void CCheevos::RcheevosEventHandler(const rc_client_event_t* event, rc_client_t* client)
{
  CCheevos* const cheevos = static_cast<CCheevos*>(rc_client_get_userdata(client));
  if (cheevos == nullptr || cheevos->m_gameInstance == nullptr)
    return;

  switch (event->type)
  {
    case RC_CLIENT_EVENT_ACHIEVEMENT_TRIGGERED:
    {
      const rc_client_achievement_t* ach = event->achievement;
      if (ach != nullptr)
      {
        game_rc_achievement_triggered data{};
        data.id = ach->id;
        data.title = ach->title;
        data.description = ach->description;
        data.badge_url = ach->badge_url;
        data.points = ach->points;
        data.hardcore = (rc_client_get_hardcore_enabled(client) != 0);

        kodi::Log(ADDON_LOG_INFO, "CCheevos: achievement triggered id=%u '%s'",
                  ach->id, ach->title != nullptr ? ach->title : "");

        cheevos->m_gameInstance->RCOnAchievementTriggered(data);
      }
      break;
    }
    case RC_CLIENT_EVENT_GAME_COMPLETED:
    {
      const rc_client_game_t* gi = rc_client_get_game_info(client);
      const char* title = (gi != nullptr && gi->title != nullptr) ? gi->title : "";
      cheevos->m_gameInstance->RCOnGameCompleted(
          title, rc_client_get_hardcore_enabled(client) != 0);
      break;
    }
    case RC_CLIENT_EVENT_ACHIEVEMENT_PROGRESS_INDICATOR_SHOW:
    case RC_CLIENT_EVENT_ACHIEVEMENT_PROGRESS_INDICATOR_UPDATE:
    {
      // Two things want this. The achievements dialog lists every measured
      // achievement, so publish the whole set; the on-screen indicator shows
      // the one the runtime picked out, so forward that as well.
      cheevos->PublishAchievementProgress();

#if defined(HAVE_GAME_RC_INDICATORS)
      const rc_client_achievement_t* achievement = event->achievement;
      if (achievement != nullptr)
      {
        game_rc_progress_indicator indicator{};
        indicator.id = achievement->id;
        indicator.title = achievement->title;
        indicator.badge_url = achievement->badge_url;
        indicator.measured_progress = achievement->measured_progress;
        indicator.measured_percent = achievement->measured_percent;

        kodi::Log(ADDON_LOG_DEBUG,
                  "CCheevos: forwarding progress indicator for achievement %u '%s' at %s",
                  achievement->id, achievement->title != nullptr ? achievement->title : "",
                  achievement->measured_progress != nullptr ? achievement->measured_progress : "");
        cheevos->m_gameInstance->RCOnProgressIndicator(&indicator);
      }
#endif
      break;
    }
    case RC_CLIENT_EVENT_ACHIEVEMENT_CHALLENGE_INDICATOR_SHOW:
    {
#if defined(HAVE_GAME_RC_INDICATORS)
      const rc_client_achievement_t* achievement = event->achievement;
      if (achievement != nullptr)
      {
        game_rc_challenge_indicator indicator{};
        indicator.id = achievement->id;
        indicator.title = achievement->title;
        indicator.badge_url = achievement->badge_url;

        cheevos->m_gameInstance->RCOnChallengeIndicator(&indicator);
      }
#endif
      break;
    }
    case RC_CLIENT_EVENT_ACHIEVEMENT_CHALLENGE_INDICATOR_HIDE:
    {
#if defined(HAVE_GAME_RC_INDICATORS)
      cheevos->m_gameInstance->RCOnChallengeIndicator(nullptr);
#endif
      break;
    }
    case RC_CLIENT_EVENT_ACHIEVEMENT_PROGRESS_INDICATOR_HIDE:
    {
#if defined(HAVE_GAME_RC_INDICATORS)
      // The runtime decides when the player has stopped working towards it
      cheevos->m_gameInstance->RCOnProgressIndicator(nullptr);
#endif
      break;
    }
    case RC_CLIENT_EVENT_SERVER_ERROR:
    {
      const rc_client_server_error_t* err = event->server_error;
      const char* message = (err != nullptr && err->error_message != nullptr) ? err->error_message
                                                                             : "Unknown error";
      const char* api = (err != nullptr && err->api != nullptr) ? err->api : "";

      kodi::Log(ADDON_LOG_WARNING, "CCheevos: server error from RetroAchievements: %s", message);
      cheevos->m_gameInstance->RCOnServerError(message, api);
      break;
    }
    case RC_CLIENT_EVENT_DISCONNECTED:
    {
      kodi::Log(ADDON_LOG_WARNING, "CCheevos: disconnected from RetroAchievements");
      cheevos->m_gameInstance->RCOnConnectionChanged(false);
      break;
    }
    case RC_CLIENT_EVENT_RECONNECTED:
    {
      kodi::Log(ADDON_LOG_INFO, "CCheevos: reconnected to RetroAchievements");
      cheevos->m_gameInstance->RCOnConnectionChanged(true);
      break;
    }
    default:
      kodi::Log(ADDON_LOG_DEBUG, "CCheevos: unhandled event %d", event->type);
      break;
  }
}

// Login callback
void CCheevos::RcheevosLoginCallback(int result, const char* errorMessage,
                                      rc_client_t* client, void* userdata)
{
  CCheevos* cheevos = static_cast<CCheevos*>(userdata);
  if (cheevos == nullptr || cheevos->m_gameInstance == nullptr)
    return;

  if (result != RC_OK)
  {
    kodi::Log(ADDON_LOG_ERROR, "CCheevos: login failed: %s",
              errorMessage != nullptr ? errorMessage : "unknown error");

    // Allow corrected credentials or a scheduled retry to start another login.
    cheevos->m_loginStarted = false;

    game_rc_login_result data{};
    data.success = false;
    data.error_message = errorMessage;

    // Kodi drops the stored token when the account is rejected, so say which
    // this was. A server that could not be reached is a reason to try again
    // later, not to sign the player out and lose what we would retry with.
    data.credentials_rejected = (result == RC_INVALID_CREDENTIALS ||
                                 result == RC_EXPIRED_TOKEN || result == RC_ACCESS_DENIED);

    if (data.credentials_rejected)
    {
      cheevos->m_loginRetryScheduled = false;
      cheevos->m_loginRetryDelaySeconds = 0;
    }
    else
    {
      if (cheevos->m_loginRetryDelaySeconds == 0)
        cheevos->m_loginRetryDelaySeconds = 1;
      else
        cheevos->m_loginRetryDelaySeconds =
            std::min(cheevos->m_loginRetryDelaySeconds * 2, LOGIN_RETRY_MAX_DELAY_SECONDS);

      cheevos->m_nextLoginAttempt = std::chrono::steady_clock::now() +
                                    std::chrono::seconds(cheevos->m_loginRetryDelaySeconds);
      cheevos->m_loginRetryScheduled = true;
    }

    cheevos->m_gameInstance->RCOnLoginResult(data);
    return;
  }

  cheevos->m_loginRetryScheduled = false;
  cheevos->m_loginRetryDelaySeconds = 0;

  const rc_client_user_t* user = rc_client_get_user_info(client);
  if (user != nullptr)
  {
    {
      std::lock_guard<std::mutex> lock(cheevos->m_credentialsMutex);
      cheevos->m_loginToken = (user->token != nullptr) ? user->token : "";
      cheevos->m_userName = (user->username != nullptr) ? user->username : cheevos->m_userName;
    }

    game_rc_login_result data{};
    data.success = true;
    data.username = user->username;
    data.display_name = user->display_name;
    data.icon_url = user->avatar_url;
    data.points = user->score;
    cheevos->m_gameInstance->RCOnLoginResult(data);

    kodi::Log(ADDON_LOG_INFO, "CCheevos: logged in as '%s' (%u points)",
              user->username != nullptr ? user->username : "", user->score);
  }

  cheevos->BeginGameLoad();
}

// Game load callback
void CCheevos::RcheevosGameLoadCallback(int result, const char* errorMessage,
                                         rc_client_t* client, void* userdata)
{
  CCheevos* cheevos = static_cast<CCheevos*>(userdata);
  if (cheevos == nullptr || cheevos->m_gameInstance == nullptr)
    return;

  if (result == RC_NO_GAME_LOADED)
  {
    cheevos->m_gameLoadRetryScheduled = false;
    cheevos->m_gameLoadRetryDelaySeconds = 0;

    game_rc_game_loaded empty{};
    cheevos->m_gameInstance->RCOnGameLoaded(empty);
    return;
  }

  if (result != RC_OK)
  {
    kodi::Log(ADDON_LOG_ERROR, "CCheevos: game load failed: %s",
              errorMessage != nullptr ? errorMessage : "unknown error");

    if (result == RC_NO_RESPONSE || result == RC_API_FAILURE || result == RC_INVALID_JSON)
    {
      if (cheevos->m_gameLoadRetryDelaySeconds == 0)
        cheevos->m_gameLoadRetryDelaySeconds = 1;
      else
        cheevos->m_gameLoadRetryDelaySeconds = std::min(
            cheevos->m_gameLoadRetryDelaySeconds * 2, GAME_LOAD_RETRY_MAX_DELAY_SECONDS);

      cheevos->m_nextGameLoadAttempt =
          std::chrono::steady_clock::now() +
          std::chrono::seconds(cheevos->m_gameLoadRetryDelaySeconds);
      cheevos->m_gameLoadRetryScheduled = true;
    }
    else if (result != RC_ABORTED)
    {
      cheevos->m_gameLoadRetryScheduled = false;
      cheevos->m_gameLoadRetryDelaySeconds = 0;

      game_rc_game_loaded empty{};
      cheevos->m_gameInstance->RCOnGameLoaded(empty);
    }

    return;
  }

  cheevos->m_gameLoadRetryScheduled = false;
  cheevos->m_gameLoadRetryDelaySeconds = 0;

  const rc_client_game_t* gameInfo = rc_client_get_game_info(client);

  // When RetroAchievements doesn't recognise a ROM's hash it still resolves it,
  // to a placeholder game titled "Unsupported Game Version (<real game>)" that
  // holds a single always-true achievement. Left alone that achievement fires
  // immediately, which reads as an unlock and then as having mastered a
  // one-achievement game. Treat the placeholder as an unsupported game instead.
  //
  // Keyed on the title because that is what the server sends; there is no flag
  // for it in the response.
  if (gameInfo != nullptr && gameInfo->title != nullptr &&
      std::strncmp(gameInfo->title, "Unsupported Game Version", 24) == 0)
  {
    kodi::Log(ADDON_LOG_INFO, "CCheevos: '%s' - this ROM version has no achievements",
              gameInfo->title);

    rc_client_unload_game(client);

    game_rc_game_loaded empty{};
    cheevos->m_gameInstance->RCOnGameLoaded(empty);
    return;
  }

  // Build achievement list
  // Note: string pointers in these structs reference rc_client-owned memory,
  // which remains valid until rc_client_destroy_achievement_list() is called below.
  std::vector<game_rc_achievement> achievements;
  rc_client_achievement_list_t* achList = rc_client_create_achievement_list(
      client, RC_CLIENT_ACHIEVEMENT_CATEGORY_CORE,
      RC_CLIENT_ACHIEVEMENT_LIST_GROUPING_LOCK_STATE);
  if (achList != nullptr)
  {
    for (uint32_t b = 0; b < achList->num_buckets; ++b)
    {
      const auto& bucket = achList->buckets[b];
      for (uint32_t a = 0; a < bucket.num_achievements; ++a)
      {
        const rc_client_achievement_t* ach = bucket.achievements[a];

        // Skip system notices such as "Warning: Unknown Emulator". They are
        // published as zero-point achievements marked unlocked, which would
        // otherwise be listed and counted as if the player had earned them.
        if (ach->title != nullptr && std::strncmp(ach->title, "Warning:", 8) == 0)
        {
          kodi::Log(ADDON_LOG_INFO, "CCheevos: skipping system notice '%s'", ach->title);
          continue;
        }

        game_rc_achievement gach{};
        gach.id = ach->id;
        gach.title = ach->title;
        gach.description = ach->description;
        gach.badge_url = ach->badge_url;
        gach.badge_locked_url = ach->badge_locked_url;
        gach.points = ach->points;
        gach.unlock_state = TranslateUnlockState(ach->unlocked);
        gach.unlock_time = static_cast<int64_t>(ach->unlock_time);
        gach.rarity = ach->rarity;
        gach.rarity_hardcore = ach->rarity_hardcore;
        achievements.push_back(gach);
      }
    }
    rc_client_destroy_achievement_list(achList);
  }

  // Build and fire the game-loaded event. Kodi counts the achievements and
  // how many are earned from the list itself, so no separate totals are sent.
  game_rc_game_loaded data{};
  data.game_id = (gameInfo != nullptr) ? gameInfo->id : 0;
  data.title = (gameInfo != nullptr) ? gameInfo->title : "";
  data.icon_url = (gameInfo != nullptr) ? gameInfo->badge_url : nullptr;
  data.achievements = achievements.data();
  data.achievement_count = static_cast<unsigned int>(achievements.size());

  kodi::Log(ADDON_LOG_INFO, "CCheevos: loaded '%s' (%u achievements)",
            data.title != nullptr ? data.title : "", data.achievement_count);

  cheevos->m_gameInstance->RCOnGameLoaded(data);

  cheevos->ApplyPendingProgress();

  cheevos->m_richPresenceActive = true;
  cheevos->m_nextRichPresencePing = std::chrono::steady_clock::now();
}

void CCheevos::UpdateRichPresence()
{
  if (!m_richPresenceActive || m_rcClient == nullptr || m_gameInstance == nullptr)
    return;

  const auto now = std::chrono::steady_clock::now();
  if (now < m_nextRichPresencePing)
    return;

  m_nextRichPresencePing = now + RP_PING_INTERVAL;

  char buffer[RP_BUFFER_SIZE]{};
  rc_client_get_rich_presence_message(m_rcClient, buffer, sizeof(buffer));
  const std::string rpMessage(buffer);

  if (!rpMessage.empty())
    m_gameInstance->RCOnRichPresenceUpdated(rpMessage.c_str());
}
