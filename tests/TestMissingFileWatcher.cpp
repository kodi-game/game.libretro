/*
 *  Copyright (C) 2026 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#include "filesystem/MissingFileWatcher.h"

#include <cstdio>
#include <cstdlib>
#include <string>
#include <thread>

using namespace LIBRETRO;

namespace
{
void Require(bool condition, const char* message)
{
  if (!condition)
  {
    std::fprintf(stderr, "FAIL: %s\n", message);
    std::exit(EXIT_FAILURE);
  }
}
} // namespace

int main()
{
  CMissingFileWatcher watcher;
  CMissingFileWatcher other;

  CMissingFileWatcher::NoteFile("/system/bios.bin", false);
  Require(watcher.MissingFile().empty(), "nothing is noted outside a scope");

  {
    CMissingFileWatcher::Scope watch(watcher, "/system/");
    CMissingFileWatcher::NoteFile("/system/optional.bin", false);
    CMissingFileWatcher::NoteFile("/system/bios.bin", false);
    Require(watcher.MissingFile() == "bios.bin", "the last miss in the folder is kept");

    CMissingFileWatcher::NoteFile("/games/game.bin", false);
    CMissingFileWatcher::NoteFile("/system", false);
    Require(watcher.MissingFile() == "bios.bin", "files outside the folder are ignored");

    // Another client loading on its own thread keeps its own watcher
    std::thread([&other] {
      CMissingFileWatcher::Scope otherWatch(other, "/system");
      CMissingFileWatcher::NoteFile("/system/other.bin", false);
    }).join();
    Require(watcher.MissingFile() == "bios.bin", "another thread's miss doesn't reach this watcher");
    Require(other.MissingFile() == "other.bin", "another thread's miss reaches its own watcher");

    CMissingFileWatcher::NoteFile("/system/sub/found.bin", true);
    Require(watcher.MissingFile().empty(), "a file found in the folder afterwards clears it");

    CMissingFileWatcher::NoteFile("/system/sub/bios2.bin", false);
  }
  Require(watcher.MissingFile() == "sub/bios2.bin", "the miss is kept after the scope");

  CMissingFileWatcher::NoteFile("/system/late.bin", false);
  Require(watcher.MissingFile() == "sub/bios2.bin", "nothing is noted once the scope ends");

  // A core that only looks for its BIOS when loading from memory, and gives up
  // on the retry by path straight away
  watcher.Clear();
  {
    CMissingFileWatcher::Scope watch(watcher, "/system");
    CMissingFileWatcher::NoteFile("/system/o2rom.bin", false);
  }
  {
    CMissingFileWatcher::Scope watch(watcher, "/system");
  }
  Require(watcher.MissingFile() == "o2rom.bin", "a miss outlives a retry that looks for nothing");

  watcher.Clear();
  {
    CMissingFileWatcher::Scope watch(watcher, "/system");
    CMissingFileWatcher::NoteFile("/systems/bios.bin", false);
    Require(watcher.MissingFile().empty(), "a folder beside it is ignored");
  }

  watcher.Clear();
  {
    CMissingFileWatcher::Scope watch(watcher, "/");
    CMissingFileWatcher::NoteFile("/bios.bin", false);
    Require(watcher.MissingFile() == "bios.bin", "a miss in a root folder is named");
    CMissingFileWatcher::NoteFile("/", false);
    Require(watcher.MissingFile() == "bios.bin", "the root itself is ignored");
  }

  std::puts("PASS");
  return EXIT_SUCCESS;
}
