# Retained-instance reload regression

This test loads the built wrapper through its normal Kodi addon entry point and
uses a small libretro fixture core. It checks timing, core frame execution, and
stream closure across six content and standalone loads on the same addon instance. Video pixels and audio samples
are checked on the initial load; Piers initializes media streams only when the
addon instance is created. Kodi callbacks are supplied in-process; it does not
start Kodi, access the network, or write a user profile.

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
build. Tests are built separately under `$RUNNER_TEMP/build-tests` and are never
installed or packaged with the addon.

CTest lists the discovered cases, fails if none are found, and prints individual
failures. Each run also uploads `game-libretro-test-results`, including JUnit XML,
the discovery listing, and CTest logs, even when a regression fails.
