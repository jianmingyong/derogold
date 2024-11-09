import * as fs from 'fs';
import type {
    CmakeMinimumRequiredV10,
    Schema
} from "./schema"

type CMakeSupportedVersion = 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10;
type CMakeCompiler = "msvc" | "msvc-clang" | "gcc" | "clang";
type CMakeBuildType = "Debug" | "Release" | "RelWithDebInfo" | "MinSizeRel";
type CMakeTarget = "install" | "package";

const CMAKE_MINIMUM_REQUIRED: CmakeMinimumRequiredV10 = {
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
    generateDefaultConfigurationPreset("msvc"),
    generateDefaultConfigurationPreset("msvc-clang")
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
    const result: {
        name: string,
        hidden?: boolean,
        inherits?: string[],
        generator?: string,
        environment?: Record<string, string>,
        cacheVariables?: Record<string, string>
    } = { name: `default-${compiler}`, hidden: true };
    result.inherits = ["default"];
    result.generator = "Ninja Multi-Config";

    if (compiler === "msvc" || compiler === "msvc-clang") {
        result.generator = "Visual Studio 17 2022";
    } else {
        result.environment = {
            CC: `${compiler}`,
            CXX: `${compiler === "gcc" ? "g++" : "clang++"}`
        };
    }

    if (buildType !== undefined) {
        result.name = `${result.name}-${buildType.toLowerCase()}`;
        result.inherits[0] = `${result.inherits[0]}-${compiler}`
        result.generator = "Ninja";
        delete result.environment;
        result.cacheVariables = { CMAKE_BUILD_TYPE: buildType };
    }

    return result;
}

const CMAKE_DEFAULT_WINDOWS_CONFIGURATION_PRESET = [
    generateWindowsConfigurationPreset("msvc"),
    generateWindowsConfigurationPreset("msvc", "install"),
    generateWindowsConfigurationPreset("msvc", "package"),

    generateWindowsConfigurationPreset("msvc-clang"),
    generateWindowsConfigurationPreset("msvc-clang", "install"),
    generateWindowsConfigurationPreset("msvc-clang", "package"),

    generateWindowsConfigurationPreset("gcc"),
    generateWindowsConfigurationPreset("gcc", "install"),
    generateWindowsConfigurationPreset("gcc", "package"),

    generateWindowsConfigurationPreset("clang"),
    generateWindowsConfigurationPreset("clang", "install"),
    generateWindowsConfigurationPreset("clang", "package"),
]

function generateWindowsConfigurationPreset(compiler: CMakeCompiler, target?: CMakeTarget) {
    const result: {
        name: string,
        inherits?: string[],
        installDir?: string,
        cacheVariables?: Record<string, string>,
        architecture?: string,
        toolset?: string,
    } = { name: `windows-x64-${compiler}` };

    result.inherits = [`default-${compiler}`];
    result.installDir = "${sourceDir}/build";
    result.cacheVariables = {
        VCPKG_TARGET_TRIPLET: "x64-windows-static",
        ARCH: "native",
        SET_PACKAGE_OUTPUT_SUFFIX: "windows-x64-msvc"
    };

    if (compiler === "msvc" || compiler === "msvc-clang") {
        result.architecture = "x64";

        if (compiler === "msvc-clang") {
            result.toolset = "ClangCL,host=x64";
        } else {
            result.toolset = "host=x64";
        }
    } else {
        result.name = `windows-x64-mingw-${compiler}`;
        result.cacheVariables.VCPKG_TARGET_TRIPLET = "x64-mingw-static";
        result.cacheVariables.SET_PACKAGE_OUTPUT_SUFFIX = `windows-x64-mingw-${compiler}`;
    }

    if (target !== undefined) {
        result.name = `${result.name}-${target}`;

        if (compiler === "gcc" || compiler === "clang") {
            result.inherits[0] = `${result.inherits[0]}-release`;
        }

        result.cacheVariables.VCPKG_TARGET_TRIPLET = `${result.cacheVariables.VCPKG_TARGET_TRIPLET}-release`;
    }

    if (target === "package") {
        result.cacheVariables.ARCH = "default";
        result.cacheVariables.SET_COMMIT_ID_IN_VERSION = "OFF";
    }

    return result;
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

function generatePresetConfig(version: CMakeSupportedVersion): Schema {
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
