<div id="top"></div>

<img src="https://i.imgur.com/4FlvRAt.png" width="200">

# DeroGold

<summary>Table of Contents</summary>
<ol>
    <li><a href="#installing">Installing</a></li>
    <li><a href="#build-instructions">Build Instructions</a></li>
    <ol>
      <li><a href="#windows-x64-only">Windows (x64 only)</a></li>
      <ol>
        <li><a href="#msvc-visual-studio-2022">MSVC (Visual Studio 2022)</a></li>
        <li><a href="#mingw64-msys2">MINGW64 (msys2)</a></li>
        <li><a href="#clang64-msys2">CLANG64 (msys2)</a></li>
      </ol>
      <li><a href="#linux-x64-aarch64">Linux (x64/aarch64)</a></li>
      <ol>
        <li><a href="#gcc">GCC</a></li>
        <li><a href="#clang">CLANG</a></li>
      </ol>
      <li><a href="#macos-x64-only">MacOS (x64 only)</a></li>
      <ol>
        <li><a href="#clang">CLANG</a></li>
      </ol>
    </ol>
</ol>

## Installing

We offer binary images of the latest releases here: https://github.com/derogold/derogold/releases

If you would like to compile yourself, read on.

<p align="right">(<a href="#top">back to top</a>)</p>

## Build Instructions

### Windows (x64 only)

#### MSVC (Visual Studio 2022)

Prerequisites:
- Visual Studio 2022 Community / Build Tools for Visual Studio 2022
  - Download [Build Tools for Visual Studio 2022](https://aka.ms/vs/17/release/vs_BuildTools.exe)
  - Open the installer and select C++ Buildtools
  - You are not required to install `Build Tools for Visual Studio 2022` if you have `Visual Studio 2022 Community` installed
- [Git](https://git-scm.com/downloads)
  - Skip this if you have installed `Git For Windows` in the Visual Studio installer
- [CMake](https://cmake.org/download/)
  - Skip this if you have installed `CMake Tools` in the Visual Studio installer

Building:
- From the start menu, open 'x64 Native Tools Command Prompt for VS 2022'
- Use `cd` to change to your desired directory to store DeroGold code
- `git clone --recursive https://github.com/derogold/derogold.git`
- `cd derogold`
- `cmake --preset windows-x64-msvc-publish`
- `cmake --build --preset windows-x64-msvc-publish`
- Enjoy your build at `build/bin`

<p align="right">(<a href="#top">back to top</a>)</p>

#### MINGW64 (msys2)

Prerequisites:
- [MSYS2](https://www.msys2.org/)
  - Open `MSYS2 MINGW64`
  - Run `pacman -Syu` twice (to ensure all packages are updated) The window will close the first time
  - Run `pacman -Sy mingw-w64-x86_64-toolchain mingw-w64-x86_64-cmake mingw-w64-x86_64-ninja mingw-w64-x86_64-ccache git`

Building:
- `git clone --recursive https://github.com/derogold/derogold.git`
- `cd derogold`
- `cmake --preset windows-x64-mingw-gcc-publish`
- `cmake --build --preset windows-x64-mingw-gcc-publish`
- Enjoy your build at `build/bin`

<p align="right">(<a href="#top">back to top</a>)</p>

#### CLANG64 (msys2)

Prerequisites:
- [MSYS2](https://www.msys2.org/)
  - Open `MSYS2 CLANG64`
  - Run `pacman -Syu` twice (to ensure all packages are updated) The window will close the first time
  - Run `pacman -Sy mingw-w64-clang-x86_64-toolchain mingw-w64-clang-x86_64-cmake mingw-w64-clang-x86_64-ninja mingw-w64-clang-x86_64-ccache git`

Building:
- `git clone --recursive https://github.com/derogold/derogold.git`
- `cd derogold`
- `cmake --preset windows-x64-mingw-clang-publish`
- `cmake --build --preset windows-x64-mingw-clang-publish`
- Enjoy your build at `build/bin`

<p align="right">(<a href="#top">back to top</a>)</p>

### Linux (x64/aarch64)

#### GCC

Prerequisites:
- For Ubuntu: `sudo apt install git cmake ninja-build build-essential curl zip unzip tar pkg-config`
- (Optional) `sudo apt install crossbuild-essential-arm64` for cross building aarch64 target

Building:
- `git clone --recursive https://github.com/derogold/derogold.git`
- `cd derogold`
- `cmake --preset linux-x64-gcc-publish`
- `sudo cmake --build --preset linux-x64-gcc-publish`
- use `sudo cmake --install build` if install permission failed
- Enjoy your build at `/usr/local/bin`

You can use `--preset linux-arm64-gcc-cross-publish` to cross compile for arm64/aarch64 raspberry pi. Output binaries would be stored at `build/bin`.

<p align="right">(<a href="#top">back to top</a>)</p>

#### CLANG

Prerequisites:
- For Ubuntu: `sudo apt install git cmake ninja-build clang curl zip unzip tar pkg-config`

Building:
- `git clone --recursive https://github.com/derogold/derogold.git`
- `cd derogold`
- `cmake --preset linux-x64-clang-publish`
- `sudo cmake --build --preset linux-x64-clang-publish`
- use `sudo cmake --install build` if install permission failed
- Enjoy your build at `/usr/local/bin`

<p align="right">(<a href="#top">back to top</a>)</p>

### MacOS (x64 only)

#### CLANG

Prerequisites:
- [HomeBrew](https://brew.sh/)
  - Install by running `/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"`
  - `brew install git cmake ninja llvm pkg-config`

Building:
- `git clone --recursive https://github.com/derogold/derogold.git`
- `cd derogold`
- `cmake --preset osx-x64-clang-publish`
- `sudo cmake --build --preset osx-x64-clang-publish`
- use `sudo cmake --install build` if install permission failed
- Enjoy your build at `/usr/local/bin`

<p align="right">(<a href="#top">back to top</a>)</p>