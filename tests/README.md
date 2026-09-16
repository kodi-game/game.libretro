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

Kodi callbacks are supplied in-process, including `StartStream()` and a fake
framebuffer with ID 42. The hardware test exercises the real dev-kit, wrapper,
and core callbacks; it does not create a GL context or validate GPU output. The
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
