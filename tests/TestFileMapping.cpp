/*
 *  Copyright (C) 2026 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#include "filesystem/FileMapping.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

#include <unistd.h>

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

void WriteFile(const std::string& path, const std::vector<uint8_t>& bytes)
{
  std::ofstream file(path, std::ios::binary);
  file.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
  Require(file.good(), "a test file is written");
}

std::vector<uint8_t> ReadFile(const std::string& path)
{
  std::ifstream file(path, std::ios::binary);
  return std::vector<uint8_t>(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
}
} // namespace

int main()
{
  char folder[] = "/tmp/game-libretro-mapping-XXXXXX";
  Require(mkdtemp(folder) != nullptr, "a scratch folder is made");
  const std::string game = std::string(folder) + "/game.bin";
  const std::string other = std::string(folder) + "/other.bin";
  const std::string empty = std::string(folder) + "/empty.bin";

  // A few pages and some, so the end of the file isn't on a page boundary
  std::vector<uint8_t> bytes(3 * 4096 + 123);
  for (size_t i = 0; i < bytes.size(); ++i)
    bytes[i] = static_cast<uint8_t>(i * 7);
  WriteFile(game, bytes);
  WriteFile(other, {1, 2, 3});
  WriteFile(empty, {});

  {
    CFileMapping mapping;
    Require(mapping.Map(game), "a local file is mapped");
    Require(mapping.Size() == bytes.size(), "the whole file is mapped");
    Require(std::memcmp(mapping.Data(), bytes.data(), bytes.size()) == 0,
            "the mapping holds the file");

    // Cores are handed const data, but some write to it anyway
    const_cast<uint8_t*>(mapping.Data())[0] ^= 0xFF;
    const_cast<uint8_t*>(mapping.Data())[bytes.size() - 1] ^= 0xFF;
    Require(ReadFile(game) == bytes, "writing to the mapping leaves the file alone");

    Require(mapping.Map(other), "another file is mapped in place of the first");
    Require(mapping.Size() == 3 && mapping.Data()[2] == 3, "the other file is what's held");

    Require(!mapping.Map(std::string(folder) + "/missing.bin"), "a missing file isn't mapped");
    Require(mapping.Data() == nullptr && mapping.Size() == 0,
            "a failed map lets go of the file held before");

    Require(!mapping.Map(empty), "an empty file isn't mapped");
    Require(!mapping.Map(folder), "a folder isn't mapped");

    Require(mapping.Map(game), "a file is mapped again");
    mapping.Unmap();
    Require(mapping.Data() == nullptr && mapping.Size() == 0, "nothing is held after unmapping");
  }

  std::remove(game.c_str());
  std::remove(other.c_str());
  std::remove(empty.c_str());
  rmdir(folder);

  std::puts("PASS");
  return EXIT_SUCCESS;
}
