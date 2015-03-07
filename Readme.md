# game.libretro

Libretro compatibility layer for the Kodi Game API

# Building add-on

## Linux

This assumes you start in the working directory where you've cloned the repos

```shell
git clone https://github.com/garbear/xbmc.git
git clone https://github.com/kodi-game/game.libretro.git
```

Create a build directory

```shell
cd game.libretro
mkdir build
cd build
```

Generate a build environment with config for debugging

```shell
XBMC_SRC=`pwd`/../../xbmc

cmake -DADDONS_TO_BUILD=game.libretro \
      -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_INSTALL_PREFIX=$XBMC_SRC/addons \
      -DPACKAGE_ZIP=1 \
      $XBMC_SRC/project/cmake/addons
```

If you are developing in Eclipse, you can create a "makefile project with existing code" using `game.libretro/` as the existing code location. To build, enter Properties -> "C/C++ Build" and change the build command to `make -C build`.

It is also possible to generate Eclipse project files with cmake

```shell
cmake -G"Eclipse CDT4 - Unix Makefiles" \
      -D_ECLIPSE_VERSION=4.4 \
      -DADDONS_TO_BUILD=game.libretro \
      -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_INSTALL_PREFIX=$XBMC_SRC/addons \
      -DPACKAGE_ZIP=1 \
      $XBMC_SRC/project/cmake/addons
```

## Windows

To build on windows, change to the addons folder:

```batch
cd D:\Projects\xbmx\xbmc\project\cmake\addons
```

Generate Visual Studio 2013 solution

```batch
cmake -DADDONS_TO_BUILD=game.libretro -DADDON_SRC_PREFIX="D:\Projects\demo" -DCMAKE_BUILD_TYPE=Debug -G "Visual Studio 12"  -DCMAKE_USER_MAKE_RULES_OVERRIDE="D:\Projects\xbmx\xbmc\project\cmake\scripts\windows\c-flag-overrides.cmake" -DCMAKE_USER_MAKE_RULES_OVERRIDE_CXX="D:\Projects\xbmx\xbmc\project\cmake\scripts\windows\cxx-flag-overrides.cmake" -DCMAKE_INSTALL_PREFIX="D:\Projects\xbmx\xbmc\adons" -DPACKAGE_ZIP=1
```

Open Visual Studio, load and build this solution:

```
D:\Projects\xbmx\xbmc\project\cmake\addons\kodi-addons.sln
```

Altarnatively, copy [prepare-addons-dev.bat](https://gist.github.com/Montellese/149ecbd5ca20941d2be4) into tools/buildsteps/win32 and execute it from there. If you want to execute it from somewhere else you need to adjust the default value of WORKDIR in the batch file.

Available options are:
* **clean** to simply clean the whole generated buildsystem
* **&lt;addon-id>** to only generate the buildsystem for that addon
