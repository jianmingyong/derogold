vcpkg_check_linkage(ONLY_STATIC_LIBRARY)
string(REPLACE "." "_" CRYPTOPP_VERSION "${VERSION}")

vcpkg_from_github(
  OUT_SOURCE_PATH CMAKE_SOURCE_PATH
  REPO abdes/cryptopp-cmake
  REF d2b072ab65c036f3dca67f4204ad57d66728bf99
  SHA512 149cb96ad5eefbdd0d9a3a3c26a0161498c39cbc1ab2413cd472d26185bc43c6a046c962a87fab2d3f2ff6bd3d2a5bb890c256273270447947cd36b3a941dfbb
  HEAD_REF master
)

vcpkg_from_github(
  OUT_SOURCE_PATH SOURCE_PATH
  REPO weidai11/cryptopp
  REF 60f81a77e0c9a0e7ffc1ca1bc438ddfa2e43b78e
  SHA512 0645de8710057722ce6543a7dbf93b2ef798fbf7966955c84708ba0d960826911e8f6b1080807a96d77debe527c063b84979d521c6b1e5ce3f5177bd1126b989
  HEAD_REF master
  PATCHES
      patch.patch
      cryptopp.patch
)

file(COPY "${CMAKE_SOURCE_PATH}/cryptopp" DESTINATION "${SOURCE_PATH}")
file(COPY "${CMAKE_SOURCE_PATH}/cmake" DESTINATION "${SOURCE_PATH}")
file(COPY "${CMAKE_SOURCE_PATH}/test" DESTINATION "${SOURCE_PATH}")
file(COPY "${CMAKE_SOURCE_PATH}/cryptopp/cryptoppConfig.cmake" DESTINATION "${SOURCE_PATH}")
file(COPY "${CMAKE_SOURCE_PATH}/CMakeLists.txt" DESTINATION "${SOURCE_PATH}")

if("pem-pack" IN_LIST FEATURES)
    vcpkg_from_github(
        OUT_SOURCE_PATH PEM_PACK_SOURCE_PATH
        REPO noloader/cryptopp-pem
        REF 095f08ff2ef9bca7b81036a59f2395e4f08ce2e8
        SHA512 49912758a635faca1f49665ac9552b20576b46e0283aaabc19bb012bdc80586106452018e5088b9b46967717982ca6022ca968edc4cac96a7506d2b1a3e4bf13
        HEAD_REF master
    )

    file(GLOB PEM_PACK_FILES
        ${PEM_PACK_SOURCE_PATH}/*.h
        ${PEM_PACK_SOURCE_PATH}/*.cpp
    )
    file(INSTALL ${PEM_PACK_FILES} DESTINATION "${CURRENT_PACKAGES_DIR}/include/${PORT}")
endif()

# disable assembly on ARM Windows to fix broken build
if (VCPKG_TARGET_IS_WINDOWS AND VCPKG_TARGET_ARCHITECTURE MATCHES "^arm")
    set(CRYPTOPP_DISABLE_ASM "ON")
elseif(NOT DEFINED CRYPTOPP_DISABLE_ASM) # Allow disabling using a triplet file
    set(CRYPTOPP_DISABLE_ASM "OFF")
endif()

# Dynamic linking should be avoided for Crypto++ to reduce the attack surface,
# so generate a static lib for both dynamic and static vcpkg targets.
# See also:
#   https://www.cryptopp.com/wiki/Visual_Studio#Dynamic_Runtime_Linking
#   https://www.cryptopp.com/wiki/Visual_Studio#The_DLL

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        -DCRYPTOPP_SOURCES=${SOURCE_PATH}
        -DCRYPTOPP_BUILD_SHARED=OFF
        -DBUILD_STATIC=ON
        -DCRYPTOPP_BUILD_TESTING=OFF
        -DCRYPTOPP_BUILD_DOCUMENTATION=OFF
        -DDISABLE_ASM=${CRYPTOPP_DISABLE_ASM}
        -DUSE_INTERMEDIATE_OBJECTS_TARGET=OFF # Not required when we build static only
        -DCMAKE_POLICY_DEFAULT_CMP0063=NEW # Honor "<LANG>_VISIBILITY_PRESET" properties
    MAYBE_UNUSED_VARIABLES
        BUILD_STATIC
        USE_INTERMEDIATE_OBJECTS_TARGET
        CMAKE_POLICY_DEFAULT_CMP0063
)

vcpkg_cmake_install()
vcpkg_cmake_config_fixup(CONFIG_PATH share/cmake/cryptopp)

if(NOT VCPKG_BUILD_TYPE)
    file(RENAME "${CURRENT_PACKAGES_DIR}/debug/share/pkgconfig" "${CURRENT_PACKAGES_DIR}/debug/lib/pkgconfig")
endif()
file(RENAME "${CURRENT_PACKAGES_DIR}/share/pkgconfig" "${CURRENT_PACKAGES_DIR}/lib/pkgconfig")
vcpkg_fixup_pkgconfig()

# There is no way to suppress installation of the headers and resource files in debug build.
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/share")


if(VCPKG_LIBRARY_LINKAGE STREQUAL "static")
    file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/bin" "${CURRENT_PACKAGES_DIR}/debug/bin")
endif()

# Handle copyright
file(COPY "${SOURCE_PATH}/License.txt" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}")
file(RENAME "${CURRENT_PACKAGES_DIR}/share/${PORT}/License.txt" "${CURRENT_PACKAGES_DIR}/share/${PORT}/copyright")
