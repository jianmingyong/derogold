<div id="top"></div>

<img src="https://i.imgur.com/4FlvRAt.png" width="200">

# DeroGold

<summary>Table of Contents</summary>
<ol>
  <li><a href="#development-resources">Development Resources</a></li>
  <li><a href="#introduction">Introduction</a></li>
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
      <li><a href="#clang-1">CLANG</a></li>
    </ol>
  </ol>
  <li><a href="#license">License</a></li>
  <li><a href="#thanks">Thanks</a></li>
</ol>

## Development Resources

* Web: https://derogold.com/
* GitHub: https://github.com/derogold/derogold
* Discord: https://discordapp.com/invite/j2aSNFn

<p align="right">(<a href="#top">back to top</a>)</p>

## Introduction

DeroGold is a digital assets project focused on preserving our life environment here on Earth.

DeroGold aspires to solve problems such as circular economy in recycling, re-use of waste materials and how we can drive positive behaviour by rewarding people with digital assets for recycling. And build habitable floating islands.

For simplicity, we say we are the digital "Nectar Card for Recycling".

However, we are much more than that. We run our own privacy digital asset that allows people and organisations to send and receive our native digital coins called DEGO.

<p align="right">(<a href="#top">back to top</a>)</p>

## Installing

We offer binary images of the latest releases here: https://github.com/derogold/derogold/releases

If you would like to compile yourself, read on.

<p align="right">(<a href="#top">back to top</a>)</p>

## Build Instructions

The CMake build system will, by default, create optimized *native* builds for your particular system type when you build the software. Using this method, the binaries created provide a better experience and all together faster performance.

However, if you wish to create *portable* binaries that can be shared between systems, specify `-DARCH=default` in your CMake arguments during the build process. Note that *portable* binaries will have a noticeable difference in performance than *native* binaries. For this reason, it is always best to build for your particular system if possible.

Note that the instructions below create a *portable* binaries due to the preset setting `-DARCH=default` for compatibility reasons.

### Windows (x64 only)

#### MSVC (Visual Studio 2022)

Prerequisites:
- [Visual Studio Community 2022](https://visualstudio.microsoft.com/downloads/#visual-studio-community-2022)
  - Note: This installs the full IDE which may take up more space. If you want a more lightweight installation, install [Build Tools for Visual Studio 2022](https://aka.ms/vs/17/release/vs_BuildTools.exe) instead.
  - In the installer, under `Workloads` tab, select `Desktop development with C++`
    - Ensure that under the optional component, the following is checked:
    - `MSVC v143 - VS 2022 C++ x64/x86 build tools (latest)`
    - `Windows 11 SDK (10.0.22621.0)` - Pick the latest one if possible.
    - `C++ CMake tools for Windows`
- [Build Tools for Visual Studio 2022](https://aka.ms/vs/17/release/vs_BuildTools.exe)
  - Note: You are not required to install this if you have [Visual Studio Community/Professional/Enterprise 2022](https://visualstudio.microsoft.com/downloads/#visual-studio-community-2022) installed.
  - In the installer, under `Workloads` tab, select `Desktop development with C++`
    - Ensure that under the optional component, the following is checked:
      - `MSVC v143 - VS 2022 C++ x64/x86 build tools (latest)`
      - `Windows 11 SDK (10.0.22621.0)` - Pick the latest one if possible.
      - `C++ CMake tools for Windows`
- [Git](https://git-scm.com/downloads)
  - Skip this if you have installed `Git For Windows` in the Visual Studio installer
  - If the development console could not find git, you can always install this to resolve the issue.
- [CMake](https://cmake.org/download/)
  - Skip this if you have installed `C++ CMake tools for Windows` in the Visual Studio installer
  - If the development console could not find cmake, you can always install this to resolve the issue.

Building:
- From the start menu, open 'x64 Native Tools Command Prompt for VS 2022'
- If you need to change the default drive C: to D: for example, just type `D:` and hit enter
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
  - Run `pacman -S mingw-w64-x86_64-toolchain mingw-w64-x86_64-cmake mingw-w64-x86_64-ninja mingw-w64-x86_64-ccache git`

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
  - Run `pacman -S mingw-w64-clang-x86_64-toolchain mingw-w64-clang-x86_64-cmake mingw-w64-clang-x86_64-ninja mingw-w64-clang-x86_64-ccache git`

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

## License

Read the [LICENSE](https://github.com/derogold/derogold/blob/master/LICENSE) file for more details.

<p align="right">(<a href="#top">back to top</a>)</p>

## Thanks

Cryptonote Developers, Bytecoin Developers, Monero Developers, Forknote Project, TurtleCoin Developers

<p align="right">(<a href="#top">back to top</a>)</p>