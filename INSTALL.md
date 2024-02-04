<div id="top"></div>

<img src="https://i.imgur.com/4FlvRAt.png" width="200">

# DeroGold

<summary>Table of Contents</summary>
<ol>
    <li><a href="#installing">Installing</a></li>
    <li><a href="#build-instructions">Build Instructions</a></li>
</ol>

## Installing

We offer binary images of the latest releases here: https://github.com/derogold/derogold/releases

If you would like to compile yourself, read on.

<p align="right">(<a href="#top">back to top</a>)</p>

## Build Instructions

### Windows

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
- `git clone --recursive https://github.com/derogold/derogold.git`
- `cd derogold`
- `cmake --preset windows-x64-msvc-publish`
- `cmake --build --preset windows-x64-msvc-publish`
- Enjoy your build at `build/bin`

#### Mingw64 (msys2)

Prerequisites:
- [MSYS2](https://www.msys2.org/)
  - Open `MSYS2 MINGW64`
  - Run `pacman -Syu` twice (to ensure all packages are updated) The window will close the first time
  - Run `pacman -Sy mingw-w64-x86_64-toolchain pacman -S mingw-w64-x86_64-cmake mingw-w64-x86_64-ninja mingw-w64-x86_64-ccache git`

Building:
- `git clone --recursive https://github.com/derogold/derogold.git`
- `cd derogold`
- `cmake --preset windows-x64-mingw-gcc-publish`
- `cmake --build --preset windows-x64-mingw-gcc-publish`
- Enjoy your build at `build/bin`

<p align="right">(<a href="#top">back to top</a>)</p>