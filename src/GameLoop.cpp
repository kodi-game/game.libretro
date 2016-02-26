/*
 *      Copyright (C) 2016 Team Kodi
 *      http://kodi.tv
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this Program; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include "GameLoop.h"
#include "LibretroDLL.h"

#include "p8-platform/util/timeutils.h"

using namespace LIBRETRO;

#define DEFAULT_FPS  60

CGameLoop::CGameLoop() :
  m_client(nullptr),
  m_fps(0.0)
{
}

CGameLoop& CGameLoop::GetInstance()
{
  static CGameLoop instance;
  return instance;
}

void CGameLoop::Start(double fps, CLibretroDLL* client)
{
  m_client = client;
  m_fps = fps;

  CreateThread();
}

void CGameLoop::Stop()
{
  StopThread(-1);
  m_sleepEvent.Signal();

  m_client = nullptr;
  m_fps = 0.0;
}

void *CGameLoop::Process(void)
{
  using namespace P8PLATFORM;

  double nextFrame = GetTimeMs();

  while (!IsStopped())
  {
    FrameEvent();

    nextFrame += FrameTimeMs();

    const int64_t now = GetTimeMs();

    double sleepTimeMs = nextFrame - now;

    if (sleepTimeMs > 1.0)
      m_sleepEvent.Wait(static_cast<uint32_t>(sleepTimeMs));
    else
      nextFrame = now; // Frame took too long, process next frame immediately
  }

  return nullptr;
}

void CGameLoop::FrameEvent(void)
{
  if (!m_client)
    return;

  m_client->retro_run();
}

double CGameLoop::FrameTimeMs() const
{
  if (m_fps != 0.0)
    return 1000.0 / m_fps;
  else
    return 1000.0 / DEFAULT_FPS;
}
