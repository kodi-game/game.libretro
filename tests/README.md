# Retained-instance and hardware-stream regressions

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

Kodi callbacks are supplied in-process, including `StartStream()` and a fake
framebuffer with ID 42. The hardware test exercises the real dev-kit, wrapper,
and core callbacks; it does not create a GL context or validate GPU output. The
preference tests model Kodi's negotiation and refusal state, including clearing
it with `GAME_HW_CONTEXT_NONE` before a stream opens. Actual FBO identity and
attachment growth are covered by the paired Kodi rendering tests. The
tests do not start Kodi, access the network, or write a user profile.

Configure with the same Kodi SDK used to build the wrapper and a libretro-common
include directory. Both require the updated Game API 8.2.0 headers:

```sh
cmake -S tests -B /tmp/game-libretro-tests \
  -DCMAKE_PREFIX_PATH=/path/to/kodi-sdk-prefix \
  -DLIBRETRO_INCLUDE_DIR=/path/to/libretro-common/include \
  -DGAME_LIBRETRO_LIBRARY=/path/to/built/game.libretro.dylib
cmake --build /tmp/game-libretro-tests
ctest --test-dir /tmp/game-libretro-tests --output-on-failure
```

To test paired changes without modifying an installed SDK, pass
`-DKODI_INCLUDE_DIR=/path/to/kodi/xbmc/addons/kodi-dev-kit/include/kodi` when
configuring both the wrapper and these tests.

The test driver and fixture core use `dlopen`, so this harness supports POSIX
platforms. Their architecture must match the wrapper.
