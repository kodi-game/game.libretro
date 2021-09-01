# Libretro wrapper for Kodi's Game API

This add-on provides a wrapper that allows Libretro cores to be loaded as game add-ons. Libretro cores are shared libraries that use the Libretro API, so the wrapper is responsible for translating function calls between the Libretro API and the Game API.

This add-on depends on the Game API, which was added in Kodi 18 (Leia).

[![License: GPL-2.0-or-later](https://img.shields.io/badge/License-GPL%20v2+-blue.svg)](LICENSE.md)
[![Build Status](https://dev.azure.com/teamkodi/kodi-game/_apis/build/status/kodi-game.game.libretro?branchName=Matrix)](https://dev.azure.com/teamkodi/kodi-game/_build/latest?definitionId=25&branchName=Matrix)
[![Build Status](https://jenkins.kodi.tv/view/Addons/job/kodi-game/job/game.libretro/job/Matrix/badge/icon)](https://jenkins.kodi.tv/blue/organizations/jenkins/kodi-game%2Fgame.libretro/branches/)
<!--- [![Build Status](https://ci.appveyor.com/api/projects/status/github/kodi-game/game.libretro?svg=true)](https://ci.appveyor.com/project/kodi-game/game-libretro) -->

## Loading mechanism

In Kodi, a game add-on can import another game add-on in its `addon.xml` file. The imported add-on will be loaded instead of the actual add-on, and it is given the original add-on's path as a parameter. If multiple game add-ons are imported, the first will be loaded and given the list of imports, starting with itself and ending with the original game add-on.

Using this strategy, Libretro cores import game.libretro, so Kodi loads game.libretro's library in place of the libretro core and passes it the path to the core. game.libretro in turns loads the Libretro core and begins translating function calls to and from the core.

## Building

Building this add-on requires Kodi's internal CMake-based build system for binary add-ons. If you are cross-compiling or just packaging the add-on, it is recommended that you use the Makefile provided with Kodi.

The Makefile will download, build and install the add-on and its dependencies. There is no need to manually clone the add-on if Kodi's source is available.

The version fetched by Kodi's build system is defined by a text file included with Kodi at [project/cmake/addons/addons/game.libretro](https://github.com/garbear/xbmc/tree/retroplayer-15.2/project/cmake/addons/addons/game.libretro) or fetched from the [binary add-ons repo](https://github.com/xbmc/repo-binary-addons) specified in [cmake/addons/bootstrap/repositories/binary-addons.txt](https://github.com/xbmc/xbmc/blob/master/cmake/addons/bootstrap/repositories/binary-addons.txt).

### Building on Linux

First, make sure Kodi's add-on build system is installed somewhere. You can perform a system install (to `/usr/local`) or a local install (I prefer `$HOME/kodi`). Specify this when you build Kodi:

```shell
./bootstrap
./configure --prefix=$HOME/kodi
make
make install
```

Now, run the Makefile with the path to the build system:

```shell
cd tools/depends/target/binary-addons
make PREFIX=$HOME/kodi ADDONS="game.libretro"
```

You can specify multiple add-ons, and wildcards are accepted too. For example, `ADDONS="pvr.*"` will build all pvr add-ons.

On Linux this performs a cross-compile install, so to package the add-on you'll need to copy the library and add-on files manually:

```shell
mkdir game.libretro
cp -r $HOME/kodi/share/kodi/addons/game.libretro/ .
cp -r $HOME/kodi/lib/kodi/addons/game.libretro/ .
```

To rebuild the add-on or compile a different one, clean the build directory:

```shell
make clean
```

### Building on OSX

Building on OSX is similar to Linux, but all the paths are determined for you. This command will download, build and install the add-on to the `addons/` directory in your Kodi repo:

```shell
cd tools/depends/target/binary-addons
make ADDONS="game.libretro"
```

### Building on Windows

First, download and install [CMake](http://www.cmake.org/download/).

To compile on windows, open a command prompt at `tools\buildsteps\win32` and run the script:

```
make-addons.bat install game.libretro
```

## Developing

When developing, compiling from a git repo is more convenient than repeatedly pushing changes to a remote one for Kodi's Makefile.

### Developing on Linux

The add-on requires several dependencies to build properly. Like Kodi's build system, you can perform a system install or a local one.

With these dependencies in place, the add-on can be built:

```shell
git clone https://github.com/kodi-game/game.libretro.git
cd game.libretro
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_PREFIX_PATH=$HOME/kodi \
      -DCMAKE_INSTALL_PREFIX=$HOME/workspace/kodi/addons \
      -DPACKAGE_ZIP=1 \
      ..
make
make install
```

where `$HOME/workspace/kodi` symlinks to the directory you cloned Kodi into.

### Developing on Windows

This instructions here came from this helpful [forum post](http://forum.kodi.tv/showthread.php?tid=173361&pid=2097898#pid2097898).

First, open `tools\windows\prepare-binary-addons-dev.bat` and change `-DCMAKE_BUILD_TYPE=Debug ^` to `-DCMAKE_BUILD_TYPE=Release ^`.

Open a command prompt at `tools\windows` and run the script:

```shell
prepare-binary-addons-dev.bat game.libretro
```

Open `cmake\addons\build\kodi-addons.sln` and build the solution. This downloads the add-on from the version specified in its text file (see above) and creates a Visual Studio project for it. If the build fails, try running it twice.

This should package and copy the add-on to the `addons/` directory. If not, you can try opening the solution `cmake\addons\build\<addon-id>-prefix\src\<addon-id>-build\<addon-id>.sln` and building the INSTALL project or, worse case, copy by hand.
