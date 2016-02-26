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
#pragma once

#include "p8-platform/threads/mutex.h"
#include "p8-platform/threads/threads.h"

namespace LIBRETRO
{
  class CLibretroDLL;

  class CGameLoop : protected P8PLATFORM::CThread
  {
  private:
    CGameLoop();

  public:
    static CGameLoop& GetInstance();

    virtual ~CGameLoop() { }

    void Start(double fps, CLibretroDLL* client);
    void Stop();

  protected:
    // implementation of CThread
    virtual void *Process(void) override;

  private:
    void FrameEvent(void);

    double FrameTimeMs() const;

    CLibretroDLL* m_client;
    double        m_fps;

    P8PLATFORM::CEvent m_sleepEvent;
  };
}
