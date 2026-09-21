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

## Retained-instance and hardware-stream regressions

These tests load the built wrapper through its normal Kodi addon entry point and
use a small libretro fixture core on one retained addon instance:

- `retained_instance_reload` checks timing, video pixels, audio samples, and stream
  closure across six software content and standalone loads.
- `hardware_stream_lifecycle` checks one context reset after the dev-kit installs
  its stream handle, framebuffer acquisition inside reset, preframe serialization
  sizing and restoration, and one context destroy before the core unloads. A
  320x240 frame carries nominal 16:9 DAR and current DAR of 1.5, followed by zero
  for square pixels, with all four counterclockwise frame rotations. Nine loads
  interleave successes with open, start, framebuffer-lookup, and framebuffer-zero
  failures to verify closure and recovery without reusing stale buffers.
- `preferred_gl`, `preferred_gles`, and `preferred_none` check frontend-derived
  preferences, an unavailable frontend followed by a retry, repeated probes,
  software fallback after a refused context, and later hardware negotiation.
  Queries after negotiation must preserve its properties and context.
- `retained_hardware_software` checks that unloading hardware content clears the
  wrapper's stream mode before software content loads on the same instance.
- `hardware_geometry_growth` raises and shrinks maxima during `retro_run()`,
  repeats growth on both axes, and verifies that `SET_GEOMETRY` ignores maxima.
  The core alternates framebuffer acquisition with using its cached framebuffer
  ID; the stream stays open without another context reset.
- `failed_load_content` and `failed_load_standalone` repeat failed hardware
  negotiations, preferred-render probing, frontend refusal, and a failure that
  opens software video/audio streams. Subsequent software and hardware loads
  must not inherit callbacks, stream mode, or frontend negotiation. They also
  verify that context destruction cannot notify a core before context reset.
- `failed_load_memory_retry` fails hardware loading from memory, then succeeds
  with software through the same `LoadGame()` call's path fallback.
- `memory_map_reload` publishes an init map, then a content map, and loads
  standalone content without republishing on the retained core.
- `memory_map_runtime_replace` changes the controlled RAM pointer during a frame
  and checks valid/invalid environment callback results and flat-memory behavior.
- `memory_map_failed_load` publishes a map during a failed load and retries
  without republishing. All three memory scenarios also test destruction with
  content still loaded, empty-memory ABI success, and a second addon instance.
  Internal descriptor/generation assertions live in the pure tests, since the
  current Game API does not export a map.

Kodi callbacks are supplied in-process, including `StartStream()` and a fake
framebuffer with ID 42. The hardware test exercises the real dev-kit, wrapper,
and core callbacks; it does not create a GL context or validate GPU output. The
preference tests model Kodi's negotiation and refusal state, including clearing
it with `GAME_HW_CONTEXT_NONE` before a stream opens. Actual FBO identity and
attachment growth are covered by the paired Kodi rendering tests. The
tests do not start Kodi, access the network, or write a user profile.

Configure with the same Kodi SDK used to build the wrapper and a libretro-common
include directory. The retained-wrapper tests require the existing Game API
8.2.0 headers.
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
`ubuntu-latest` job. The temporary paired Kodi repository and branch are defined
in the workflow's top-level environment variables.

The job generates a fresh SDK with Kodi's `PrepareEnv.cmake`, then builds this
checkout with Kodi's `build_addon()` helper and the internal libretro-common and
rcheevos dependencies. Both the wrapper and harness use that SDK's CMake package
and the paired checkout's headers. Ninja builds the wrapper once under
`$RUNNER_TEMP/build-addon`; the harness loads its `game.libretro.so` symlink and
uses the staged `build/depends/include/libretro-common` headers from that same
build. It also passes the staged pinned rcheevos headers/archive to enable the
RA memory test. Tests are built separately under `$RUNNER_TEMP/build-tests` and are never
installed or packaged with the addon.

CTest lists the discovered cases, fails if none are found, and prints individual
failures. Each run also uploads `game-libretro-test-results`, including JUnit XML,
the discovery listing, and CTest logs, even when a regression fails.
