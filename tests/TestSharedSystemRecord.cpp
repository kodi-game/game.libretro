/*
 *  Copyright (C) 2026 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#include "filesystem/SharedSystemRecord.h"

#include <cstdio>
#include <cstdlib>
#include <string>

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
  // FNV-1a's published 64-bit values
  Require(HashBytes("", 0) == 0xcbf29ce484222325ULL, "the hash of nothing is the offset basis");
  Require(HashBytes("a", 1) == 0xaf63dc4c8601ec8cULL, "the hash of \"a\" is FNV-1a's");
  Require(HashBytes("b", 1, HashBytes("a", 1)) == HashBytes("ab", 2), "a hash continues");

  CSharedSystemRecord record;
  record.sharedDirectory = "/storage/games/bios/system";
  record.fingerprint = 0x0123456789abcdefULL;
  record.copies["dolphin-emu/Sys/GameSettings/GZLE01.ini"] = 1;
  record.copies["PPSSPP/flash0/font/jpn0.pgf"] = 0xffffffffffffffffULL;
  record.copies["name with spaces.dat"] = 42;

  CSharedSystemRecord read;
  Require(read.Parse(record.Format()), "a record reads back");
  Require(read.sharedDirectory == record.sharedDirectory &&
              read.fingerprint == record.fingerprint && read.copies == record.copies,
          "a record reads back unchanged");

  CSharedSystemRecord empty;
  empty.sharedDirectory = "D:\\Games\\System\\";
  Require(read.Parse(empty.Format()) && read.copies.empty() &&
              read.sharedDirectory == empty.sharedDirectory,
          "a record of no copies reads back");

  const char* broken[] = {
      "",
      "shared /system\n",
      "shared /system\nfingerprint 123\n",
      "shared /system\nfingerprint 0123456789ABCDEF\n",
      "shared /system\nfingerprint 0123456789abcdef\n0123456789abcdef\n",
      "shared /system\nfingerprint 0123456789abcdef\nnot-a-hash-at-al file\n",
      "fingerprint 0123456789abcdef\nshared /system\n",
  };
  for (const char* text : broken)
  {
    read.copies["left over"] = 1;
    Require(!read.Parse(text) && read.copies.empty() && read.fingerprint == 0,
            "a broken record is refused and leaves nothing behind");
  }

  const uint64_t source = 1;
  const uint64_t changed = 2;
  const uint64_t earlier = 3;
  Require(DecideSharedFile(nullptr, source, nullptr) == SharedFileAction::COPY,
          "a file the shared folder lacks is copied");
  Require(DecideSharedFile(nullptr, source, &earlier) == SharedFileAction::COPY,
          "a copy that was deleted is made again");
  Require(DecideSharedFile(&source, source, nullptr) == SharedFileAction::NONE,
          "the same file is left as it is");
  Require(DecideSharedFile(&earlier, source, &earlier) == SharedFileAction::REPLACE,
          "an earlier copy is replaced when the emulator's file changes");
  Require(DecideSharedFile(&changed, source, &earlier) == SharedFileAction::KEEP,
          "a copy changed after copying is kept");
  Require(DecideSharedFile(&changed, source, nullptr) == SharedFileAction::KEEP,
          "a file put there another way is kept");

  std::puts("PASS");
  return EXIT_SUCCESS;
}
