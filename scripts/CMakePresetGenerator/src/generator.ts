import * as fs from 'fs';
import type {
    CmakeMinimumRequiredV1,
    Schema
} from "./schema"

type CMakeCompiler = "msvc" | "gcc" | "clang";
type CMakeBuildType = "Debug" | "Release" | "RelWithDebInfo" | "MinSizeRel";
type CMakeTarget = "install" | "package";

const CMAKE_MINIMUM_REQUIRED: CmakeMinimumRequiredV1 = {
    major: 3,
    minor: 15
};

const CMAKE_DEFAULT_CONFIGURATION_PRESET = {
    name: "default",
    hidden: true,
    binaryDir: "${sourceDir}/build",
    environment: {
        VCPKG_OVERLAY_PORTS: "${sourceDir}/ports",
        VCPKG_OVERLAY_TRIPLETS: "${sourceDir}/triplets"
    }
}

const CMAKE_DEFAULT_MSVC_CONFIGURATION_PRESET = [
    generateDefaultConfigurationPreset("msvc")
]

const CMAKE_DEFAULT_GCC_CONFIGURATION_PRESET = [
    generateDefaultConfigurationPreset("gcc"),
    generateDefaultConfigurationPreset("gcc", "Release"),
]

const CMAKE_DEFAULT_CLANG_CONFIGURATION_PRESET = [
    generateDefaultConfigurationPreset("clang"),
    generateDefaultConfigurationPreset("clang", "Release"),
]

function generateDefaultConfigurationPreset(compiler: CMakeCompiler, buildType?: CMakeBuildType) {
    let name = `default-${compiler}`;
    let inherit = "default";
    let generator = "Ninja Multi-Config";
    let environment: Record<string, string> = {
        CC: `${compiler}`,
        CXX: `${compiler === "gcc" ? "g++" : "clang++"}`
    }
    let cacheVariables: Record<string, string> = {}

    if (compiler === "msvc") {
        generator = "Visual Studio 17 2022";
        environment = {};
    }

    if (buildType !== undefined) {
        name = `${name}-${buildType.toLowerCase()}`;
        inherit = `${inherit}-${compiler}`
        generator = "Ninja";
        environment = {};
        cacheVariables = { CMAKE_BUILD_TYPE: buildType }
    }

    let output = {};

    if (Object.keys(environment).length > 0) {
        output = { ...output, environment: environment };
    }

    if (Object.keys(cacheVariables).length > 0) {
        output = { ...output, cacheVariables: cacheVariables };
    }

    return {
        name: name,
        hidden: true,
        inherits: [inherit],
        generator: generator,
        ...output
    }
}

const CMAKE_DEFAULT_WINDOWS_CONFIGURATION_PRESET = [
    generateWindowsConfigurationPreset("msvc"),
    generateWindowsConfigurationPreset("msvc", "install"),
    generateWindowsConfigurationPreset("msvc", "package"),

    generateWindowsConfigurationPreset("gcc"),
    generateWindowsConfigurationPreset("gcc", "install"),
    generateWindowsConfigurationPreset("gcc", "package"),

    generateWindowsConfigurationPreset("clang"),
    generateWindowsConfigurationPreset("clang", "install"),
    generateWindowsConfigurationPreset("clang", "package"),
]

function generateWindowsConfigurationPreset(compiler: CMakeCompiler, target?: CMakeTarget) {
    let name = `windows-x64-${compiler}`;
    let inherit = `default-${compiler}`;
    const installDir = "${sourceDir}/build";
    const cacheVariables: Record<string, string> = {
        VCPKG_TARGET_TRIPLET: "x64-windows-static",
        ARCH: "native",
        SET_PACKAGE_OUTPUT_SUFFIX: "windows-x64-msvc"
    };
    let others: object = {
        architecture: {
            strategy: "set",
            value: "x64"
        },
        toolset: {
            strategy: "set",
            value: "host=x64"
        },
        vendor: {
            "microsoft.com/VisualStudioSettings/CMake/1.0": {
                hostOS: "Windows",
                intelliSenseMode: "windows-msvc-x64"
            }
        }
    };

    if (compiler !== "msvc") {
        name = `windows-x64-mingw-${compiler}`;
        cacheVariables.VCPKG_TARGET_TRIPLET = "x64-mingw-static";
        cacheVariables.SET_PACKAGE_OUTPUT_SUFFIX = `windows-x64-mingw-${compiler}`;
        others = {};
    }

    if (target !== undefined) {
        name = `${name}-${target}`;
        inherit = compiler === "msvc" ? inherit : `${inherit}-release`;
        cacheVariables.VCPKG_TARGET_TRIPLET = `${cacheVariables.VCPKG_TARGET_TRIPLET}-release`;
    }

    if (target === "package") {
        cacheVariables.ARCH = "default";
        cacheVariables.SET_COMMIT_ID_IN_VERSION = "OFF";
    }

    return {
        name: name,
        inherits: [inherit],
        installDir: installDir,
        cacheVariables: cacheVariables,
        ...others
    };
}

const CMAKE_DEFAULT_LINUX_CONFIGURATION_PRESET = [
    generateLinuxConfigurationPreset("gcc", "x64"),
    generateLinuxConfigurationPreset("gcc", "x64", "install"),
    generateLinuxConfigurationPreset("gcc", "x64", "package"),

    generateLinuxConfigurationPreset("gcc", "amd64"),
    generateLinuxConfigurationPreset("gcc", "amd64", "install"),
    generateLinuxConfigurationPreset("gcc", "amd64", "package"),

    generateLinuxConfigurationPreset("gcc", "arm64"),
    generateLinuxConfigurationPreset("gcc", "arm64", "install"),
    generateLinuxConfigurationPreset("gcc", "arm64", "package"),

    generateLinuxConfigurationPreset("clang", "x64"),
    generateLinuxConfigurationPreset("clang", "x64", "install"),
    generateLinuxConfigurationPreset("clang", "x64", "package"),

    generateLinuxConfigurationPreset("clang", "amd64"),
    generateLinuxConfigurationPreset("clang", "amd64", "install"),
    generateLinuxConfigurationPreset("clang", "amd64", "package"),

    generateLinuxConfigurationPreset("clang", "arm64"),
    generateLinuxConfigurationPreset("clang", "arm64", "install"),
    generateLinuxConfigurationPreset("clang", "arm64", "package"),
];

function generateLinuxConfigurationPreset(compiler: CMakeCompiler, arch: "x64" | "amd64" | "arm64", target?: CMakeTarget) {
    let name = `linux-${arch}-${compiler}`;
    let inherit = `default-${compiler}`;
    const cacheVariables: Record<string, string> = {
        VCPKG_TARGET_TRIPLET: `${arch}-linux`,
        ARCH: "native",
        SET_PACKAGE_OUTPUT_SUFFIX: "linux-x64-gcc"
    };

    if (arch === "amd64") {
        cacheVariables.VCPKG_TARGET_TRIPLET = `x64-linux`;
    }

    if (target !== undefined) {
        name = `${name}-${target}`;
        inherit = `${inherit}-release`;
        cacheVariables.VCPKG_TARGET_TRIPLET = `${cacheVariables.VCPKG_TARGET_TRIPLET}-release`;
    }

    if (target === "package") {
        cacheVariables.ARCH = "default";
        cacheVariables.SET_COMMIT_ID_IN_VERSION = "OFF";
    }

    return {
        name: name,
        inherits: [inherit],
        cacheVariables: cacheVariables,
    };
}

function generatePresetConfig(version: 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10): Schema {
    return {
        version: version,
        cmakeMinimumRequired: CMAKE_MINIMUM_REQUIRED,
        configurePresets: [
            CMAKE_DEFAULT_CONFIGURATION_PRESET,
            ...CMAKE_DEFAULT_MSVC_CONFIGURATION_PRESET,
            ...CMAKE_DEFAULT_GCC_CONFIGURATION_PRESET,
            ...CMAKE_DEFAULT_CLANG_CONFIGURATION_PRESET,
            ...CMAKE_DEFAULT_WINDOWS_CONFIGURATION_PRESET,
            ...CMAKE_DEFAULT_LINUX_CONFIGURATION_PRESET,
        ]
    }
}

fs.writeFileSync("CMakePresets.json", JSON.stringify(generatePresetConfig(3), undefined, 4));
