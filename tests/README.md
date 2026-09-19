# game.libretro tests

## Pure memory tests (no Kodi SDK or wrapper required)

The standalone `memory_map` test builds the internal memory model directly. It
covers owned metadata, non-owning RAM pointers, transactional replacement,
generation, arithmetic rejection, current RetroArch normalization, the documented
SNES maps, empty/informational maps, and retained-core lifecycle.

```sh
cmake -S tests -B /tmp/game-libretro-memory-tests \
  -DLIBRETRO_INCLUDE_DIR=/path/to/libretro-common/include
cmake --build /tmp/game-libretro-memory-tests
ctest --test-dir /tmp/game-libretro-memory-tests --output-on-failure
```

To also run `cheevos_memory`, point to the **already-built pinned v12.3.0**
dependency used by the addon. No dependency is downloaded by this test project:

```sh
cmake -S tests -B /tmp/game-libretro-memory-tests \
  -DLIBRETRO_INCLUDE_DIR=/path/to/libretro-common/include \
  -DRCHEEVOS_INCLUDE_DIR=/path/to/addon-build/build/depends/include/rcheevos \
  -DRCHEEVOS_LIBRARY=/path/to/addon-build/build/depends/lib/librcheevoslib.a
cmake --build /tmp/game-libretro-memory-tests
ctest --test-dir /tmp/game-libretro-memory-tests --output-on-failure
```

Explicit rcheevos paths take precedence over automatic discovery. CMake reports
the selected header directory and library so their source can be verified;
`CMAKE_PREFIX_PATH` can also locate these installed dependencies. The RA test uses
real `rc_libretro_memory_*` functions with controlled buffers; it needs no
`rc_client`, HTTP, credentials, account, or Kodi runtime. It verifies refresh on
memory generation/console changes, caching, pointer/topology replacement,
descriptor-backed failure caching, retryable flat-memory failure, teardown, and
scoped callback isolation. Memory generation covers descriptor changes, standard
memory buffers, and content lifetime invalidation, not just map replacement.

For Clang/GCC sanitizer runs, use a separate build directory with
`-DCMAKE_CXX_FLAGS="-fsanitize=address,undefined -fno-omit-frame-pointer"` and
`-DCMAKE_EXE_LINKER_FLAGS="-fsanitize=address,undefined"`. This instruments the test
and wrapper memory code; a prebuilt rcheevos archive retains its own build flags.
`-Wall -Wextra -Wpedantic` can be added to `CMAKE_CXX_FLAGS` for warning checks.

See [MemoryModel.md](MemoryModel.md) for the upstream source audit, exact lifetime
policy, replacement/generation semantics, and normalization test provenance.

## Retained-instance regressions

This test loads the built wrapper through its normal Kodi addon entry point and
uses a small libretro fixture core. It checks timing, core frame execution, and
stream closure across six content and standalone loads on the same addon instance. Video pixels and audio samples
are checked on the initial load; Piers initializes media streams only when the
addon instance is created. Kodi callbacks are supplied in-process; it does not
start Kodi, access the network, or write a user profile.

- `memory_map_reload` publishes an init map, then a content map, and loads
  standalone content without republishing on the retained core.
- `memory_map_runtime_replace` changes the controlled RAM pointer during a frame
  and checks valid/invalid environment callback results and flat-memory behavior.
- `memory_map_failed_load` publishes a map during a failed load and retries
  without republishing. All three memory scenarios also test destruction with
  content still loaded, empty-memory ABI success, and a second addon instance.
  Internal descriptor/generation assertions live in the pure tests, since the
  current Game API does not export a map.

Configure with the same Kodi SDK used to build the wrapper and a libretro-common
include directory.
Supplying `GAME_LIBRETRO_LIBRARY` enables these tests in addition to the pure
tests; only this mode requires the Kodi SDK:

```sh
cmake -S tests -B /tmp/game-libretro-tests \
  -DCMAKE_PREFIX_PATH=/path/to/kodi-sdk-prefix \
  -DLIBRETRO_INCLUDE_DIR=/path/to/libretro-common/include \
  -DGAME_LIBRETRO_LIBRARY=/path/to/built/game.libretro.dylib \
  -DRCHEEVOS_INCLUDE_DIR=/path/to/addon-build/build/depends/include/rcheevos \
  -DRCHEEVOS_LIBRARY=/path/to/addon-build/build/depends/lib/librcheevoslib.a
cmake --build /tmp/game-libretro-tests
ctest --test-dir /tmp/game-libretro-tests --output-on-failure
```

To test paired changes without modifying an installed SDK, pass
`-DKODI_INCLUDE_DIR=/path/to/kodi/xbmc/addons/kodi-dev-kit/include/kodi` when
configuring both the wrapper and these tests.

The test driver and fixture core use `dlopen`, so this harness supports POSIX
platforms. Their architecture must match the wrapper.

## GitHub Actions

[Regression tests](../.github/workflows/tests.yml) runs every registered CTest on
pull requests and pushes to `Piers` and `retroplayer-piers`, using one native
`ubuntu-latest` job. The upstream Kodi repository and branch are defined
in the workflow's top-level environment variables.

The job generates a fresh SDK with Kodi's `PrepareEnv.cmake`, then builds this
checkout with Kodi's `build_addon()` helper and the internal libretro-common and
rcheevos dependencies. Both the wrapper and harness use that SDK's CMake package
and the upstream checkout's headers. Ninja builds the wrapper once under
`$RUNNER_TEMP/build-addon`; the harness loads its `game.libretro.so` symlink and
uses the staged `build/depends/include/libretro-common` headers from that same
build. It also passes the staged pinned rcheevos headers/archive to enable the
RA memory test. Tests are built separately under `$RUNNER_TEMP/build-tests` and are never
installed or packaged with the addon.

CTest lists the discovered cases, fails if none are found, and prints individual
failures. Each run also uploads `game-libretro-test-results`, including JUnit XML,
the discovery listing, and CTest logs, even when a regression fails.
