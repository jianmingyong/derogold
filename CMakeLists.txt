cmake_minimum_required(VERSION 3.8)

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")

project(DeroGold)

find_program(CCACHE_PROGRAM ccache)

if(CCACHE_PROGRAM)
    set(CMAKE_C_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
    message(STATUS "ccache is found and will be used to speed up compilation for DeroGold source code.")
endif()

if(DEFINED CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING "Choose the type of build, options are: Debug, Release, RelWithDebInfo")
else()
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build, options are: Debug, Release, RelWithDebInfo")
endif()

set(VERSION "0.7.2")

# # This section describes our general CMake setup options
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Enable c++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# # This section is for settings found in the slow-hash routine(s) that may benefit some systems (mostly ARM)
set(FORCE_USE_HEAP ON CACHE BOOL "Force the use of heap memory allocation")
set(NO_AES OFF CACHE BOOL "Turn off Hardware AES instructions?")
set(NO_OPTIMIZED_MULTIPLY_ON_ARM OFF CACHE BOOL "Turn off Optimized Multiplication on ARM?")

if(FORCE_USE_HEAP)
    add_definitions(-DFORCE_USE_HEAP)
    message(STATUS "FORCE_USE_HEAP: ENABLED")
else()
    message(STATUS "FORCE_USE_HEAP: DISABLED")
endif()

if(NO_AES)
    add_definitions(-DNO_AES)
    message(STATUS "HW AES: DISABLED")
else()
    message(STATUS "HW AES: ENABLED")
    
    if (NOT MSVC)
        include(CheckCCompilerFlag)
        check_c_compiler_flag(-maes HAS_AES_NI_SUPPORT)

        if(HAS_AES_NI_SUPPORT)
            set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -maes")
        else()
            add_definitions(-DNO_AES)
        endif()
    endif()
endif()

if(NO_OPTIMIZED_MULTIPLY_ON_ARM)
    add_definitions(-DNO_OPTIMIZED_MULTIPLY_ON_ARM)
    message(STATUS "OPTIMIZED_ARM_MULTIPLICATION: DISABLED")
else()
    message(STATUS "OPTIMIZED_ARM_MULTIPLICATION: ENABLED")
endif()

# # We only build static binaries -- this is left here for our dependencies
set(STATIC ON CACHE BOOL FORCE "Link libraries statically? Forced to ON")

# # This section helps us tag our builds with the git commit information
set(COMMIT_ID_IN_VERSION ON CACHE BOOL "Include commit ID in version")

file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/version")

if(NOT COMMIT_ID_IN_VERSION)
    set(VERSION "${VERSION}-unknown")
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/src/config/version.h.in" "${CMAKE_CURRENT_BINARY_DIR}/version/version.h")
    add_custom_target(version ALL)
elseif(DEFINED COMMIT)
    string(REPLACE "." "\\." VERSION_RE "${VERSION}")

    if(NOT REFS MATCHES "(\\(|, )tag: v${VERSION_RE}(\\)|, )")
        set(VERSION "${VERSION}-g${COMMIT}")
    endif()

    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/src/config/version.h.in" "${CMAKE_CURRENT_BINARY_DIR}/version/version.h")
    add_custom_target(version ALL)
else()
    find_package(Git QUIET)

    if(Git_FOUND OR GIT_FOUND)
        message(STATUS "Found Git: ${GIT_EXECUTABLE}")
        add_custom_target(version ALL "${CMAKE_COMMAND}" "-D" "VERSION=${VERSION}" "-D" "GIT=${GIT_EXECUTABLE}" "-D" "TO=${CMAKE_CURRENT_BINARY_DIR}/version/version.h" "-P" "${CMAKE_CURRENT_SOURCE_DIR}/src/version.cmake" WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
    else()
        message(STATUS "WARNING: Git was not found!")
        set(VERSION "${VERSION}-unknown")
        configure_file("${CMAKE_CURRENT_SOURCE_DIR}/src/config/version.h.in" "${CMAKE_CURRENT_BINARY_DIR}/version/version.h")
        add_custom_target(version ALL)
    endif()
endif()

include_directories(include "${CMAKE_CURRENT_BINARY_DIR}/version" src external)

# # Platform specific code base information is applied here
if(MSVC)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/platform/windows)
    add_definitions("/bigobj /MP /W3 /GS- /D_CRT_SECURE_NO_WARNINGS /wd4996 /wd4345 /D_WIN32_WINNT=0x0600 /DWIN32_LEAN_AND_MEAN")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /STACK:10485760")

    foreach(VAR CMAKE_C_FLAGS_DEBUG CMAKE_CXX_FLAGS_DEBUG CMAKE_C_FLAGS_RELWITHDEBINFO CMAKE_CXX_FLAGS_RELWITHDEBINFO CMAKE_C_FLAGS_RELEASE CMAKE_CXX_FLAGS_RELEASE)
        string(REPLACE "/MD" "/MT" ${VAR} "${${VAR}}")
    endforeach()

    include_directories(SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/src/platform/msc)
elseif(MINGW)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/platform/windows)
    include_directories(SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/src/platform/mingw)
elseif(APPLE)
    include_directories(SYSTEM /usr/include/malloc)
    enable_language(ASM)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/platform/osx)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/platform/posix)
else()
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/platform/linux)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/platform/posix)
endif()

if(NOT MSVC)
    if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
        # This option has no effect in glibc version less than 2.20.
        # Since glibc 2.20 _BSD_SOURCE is deprecated, this macro is recomended instead
        add_definitions("-D_DEFAULT_SOURCE -D_GNU_SOURCE")
    endif()

    # # These options generate all those nice warnings we see while building
    set(WARNINGS "-Wall -Wextra -Wpointer-arith -Wvla -Wwrite-strings  -Wno-error=extra -Wno-error=unused-function -Wno-error=sign-compare -Wno-error=strict-aliasing -Wno-error=type-limits -Wno-unused-parameter -Wno-error=unused-variable -Wno-error=undef -Wno-error=uninitialized -Wno-error=unused-result")

    if(CMAKE_C_COMPILER_ID STREQUAL "Clang")
        set(WARNINGS "${WARNINGS} -Wno-error=mismatched-tags -Wno-error=null-conversion -Wno-overloaded-shift-op-parentheses -Wno-error=shift-count-overflow -Wno-error=tautological-constant-out-of-range-compare -Wno-error=unused-private-field -Wno-error=unneeded-internal-declaration -Wno-error=unused-function -Wno-error=missing-braces -Wno-error=unused-command-line-argument")
    else()
        set(WARNINGS "${WARNINGS} -Wlogical-op -Wno-error=maybe-uninitialized -Wno-error=clobbered -Wno-error=unused-but-set-variable")
    endif()

    if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
        set(WARNINGS "${WARNINGS} -Wno-error=odr")
    endif()

    set(C_WARNINGS "-Waggregate-return -Wnested-externs -Wold-style-definition -Wstrict-prototypes -Wno-implicit-fallthrough")
    set(CXX_WARNINGS "-Wno-reorder -Wno-missing-field-initializers")

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11 ${STATICASSERTC_FLAG} ${WARNINGS} ${C_WARNINGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 ${STATICASSERTCPP_FLAG} ${WARNINGS} ${CXX_WARNINGS}")

    if(APPLE)
        if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
            # Need to build against libc++ instead of libstc++ on apple
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -nostdinc++ -I/usr/local/opt/llvm/include/c++/v1 -nodefaultlibs -lc++ -lc++abi -lm -lc -lgcc -lgcc_eh")

            # Need these flags so gcc OSX works on the cryptopp ASM - https://groups.google.com/forum/#!topic/cryptopp-users/po8yO-vYaac
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DCRYPTOPP_CLANG_INTEGRATED_ASSEMBLER=1")

        elseif(CMAKE_C_COMPILER_ID STREQUAL "Clang")
            # Need to link against the llvm libc++ library, default is too old for std::filesystem
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++ -I/usr/local/opt/llvm/include/c++/v1")
        endif()
    endif()

    if(NOT APPLE)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
    endif()

    # # Setting up DEBUG flags
    if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
        set(DEBUG_FLAGS "-g3 -Og -gdwarf-4 -fvar-tracking -fvar-tracking-assignments -fno-inline -fno-omit-frame-pointer")
    else()
        set(DEBUG_FLAGS "-g3 -O0 -fno-omit-frame-pointer")
    endif()

    # # Setting up RELEASE flags
    set(RELEASE_FLAGS "-Ofast -DNDEBUG -Wno-unused-variable")

    if(NOT APPLE)
        # There is a clang bug that does not allow to compile code that uses AES-NI intrinsics if -flto is enabled
        if(CMAKE_C_COMPILER_ID STREQUAL "GNU" AND CMAKE_SYSTEM_NAME STREQUAL "Linux" AND CMAKE_BUILD_TYPE STREQUAL "Release")
            # On linux, to build in lto mode, check that ld.gold linker is used: 'update-alternatives --install /usr/bin/ld ld /usr/bin/ld.gold HIGHEST_PRIORITY'
            set(CMAKE_AR gcc-ar)
            set(CMAKE_RANLIB gcc-ranlib)
        endif()
    endif()

    # # Set up the normal CMake flags as we've built them
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} ${DEBUG_FLAGS}")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${DEBUG_FLAGS}")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} ${RELEASE_FLAGS}")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${RELEASE_FLAGS}")

    # # Statically link our binaries
    if(NOT APPLE)
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++")
    endif()
endif()

find_package(Boost REQUIRED)
find_package(cryptopp CONFIG REQUIRED)
find_package(leveldb CONFIG REQUIRED)
find_package(miniupnpc CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(RapidJSON CONFIG REQUIRED)
find_package(RocksDB CONFIG REQUIRED)
find_package(zstd CONFIG REQUIRED)

# So we can use std::string with rapidjson
add_definitions(-DRAPIDJSON_HAS_STDSTRING)

add_subdirectory(src)