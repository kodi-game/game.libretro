# Retained-instance reload regression

This test loads the built wrapper through its normal Kodi addon entry point and
uses a small libretro fixture core. It checks timing, video pixels, audio samples,
and stream closure across six content and standalone loads on the same addon
instance. Kodi callbacks are supplied in-process; it does not start Kodi, access
the network, or write a user profile.

Configure with the same Kodi SDK used to build the wrapper and a libretro-common
include directory:

```sh
cmake -S tests -B /tmp/game-libretro-tests \
  -DCMAKE_PREFIX_PATH=/path/to/kodi-sdk-prefix \
  -DLIBRETRO_INCLUDE_DIR=/path/to/libretro-common/include \
  -DGAME_LIBRETRO_LIBRARY=/path/to/built/game.libretro.dylib
cmake --build /tmp/game-libretro-tests
ctest --test-dir /tmp/game-libretro-tests --output-on-failure
```

The test driver and fixture core use `dlopen`, so this harness supports POSIX
platforms. Their architecture must match the wrapper.
