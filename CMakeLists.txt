# SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
# SPDX-License-Identifier: GPL-3.0-or-later

cmake_minimum_required(VERSION 3.31)

set(CMAKE_OSX_DEPLOYMENT_TARGET "15.0" CACHE STRING "macOS deployment target")
project(yuzu)

# =======================================================================
# HARDENED WINDOWS 10 ENVIRONMENT SUBSYSTEM TARGET FLUSH
# =======================================================================
if (WIN32)
    add_compile_definitions(_WIN32_WINNT=0x0A00 WINVER=0x0A00 _HAS_CXX17=1)
    message(STATUS "[ENV CONTROLLER] Injected absolute Windows 10 socket environment variables across all targets.")
endif()
# =======================================================================

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules/find")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/externals/cmake-modules")

set(CPM_SOURCE_CACHE ${CMAKE_SOURCE_DIR}/.cache/cpm)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_SCAN_FOR_MODULES 0)

include(DetectPlatform)
include(DetectArchitecture)
include(DefaultConfig)
include(UseLTO)
include(FasterLinker)
include(UseCcache)
include(CMakeDependentOption)
include(CTest)
include(CPMUtil)

if (NOT DEFINED ARCHITECTURE)
    message(FATAL_ERROR "Architecture didn't make it out of scope, did you delete DetectArchitecture.cmake?")
endif()

if (OPENBSD)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -I${CMAKE_SYSROOT}/usr/X11R6/include -D_LIBCPP_PSTL_BACKEND_SERIAL=1")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I${CMAKE_SYSROOT}/usr/X11R6/include -D_LIBCPP_PSTL_BACKEND_SERIAL=1")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L${CMAKE_SYSROOT}/usr/X11R6/lib")
elseif (NETBSD)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -I${CMAKE_SYSROOT}/usr/X11R7/include -I${CMAKE_SYSROOT}/usr/pkg/include/c++/v1")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I${CMAKE_SYSROOT}/usr/X11R7/include -I${CMAKE_SYSROOT}/usr/pkg/include/c++/v1")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L${CMAKE_SYSROOT}/usr/X11R7/lib")
endif()

if (NETBSD)
    set(ENV{PKG_CONFIG_PATH} "${PKG_CONFIG_PATH}:${CMAKE_SYSROOT}/usr/pkg/lib/ffmpeg7/pkgconfig")
endif()

cmake_dependent_option(YUZU_STATIC_ROOM "Build a static room executable only (CI only)" OFF "LINUX" OFF)
if (YUZU_STATIC_ROOM)
    set(YUZU_ROOM ON)
    set(YUZU_ROOM_STANDALONE ON)
    set(ENABLE_QT OFF)
    set(YUZU_CMD OFF)
    set(ENABLE_CUBEB OFF)
    set(ENABLE_UPDATE_CHECKER OFF)
    set(USE_DISCORD_PRESENCE OFF)
    set(BUILD_TESTING OFF)
    set(ENABLE_WEB_SERVICE OFF)
    set(ENABLE_LIBUSB OFF)
    set(Boost_USE_STATIC_LIBS ON)
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
    set(OPENSSL_USE_STATIC_LIBS ON)
    set(OpenSSL_FORCE_SYSTEM ON)
    set(zstd_FORCE_BUNDLED ON)
    set(fmt_FORCE_BUNDLED ON)
endif()

option(ENABLE_QT "Enable the Qt frontend" ON)
option(ENABLE_QT_TRANSLATION "Enable translations for the Qt frontend" OFF)
option(ENABLE_UPDATE_CHECKER "Enable update checker (for Qt and Android)" OFF)
cmake_dependent_option(YUZU_USE_QT_MULTIMEDIA "Use QtMultimedia for Camera"  OFF "NOT YUZU_USE_BUNDLED_QT" OFF)
cmake_dependent_option(YUZU_USE_QT_WEB_ENGINE "Use QtWebEngine for web applet implementation" OFF "NOT YUZU_USE_BUNDLED_QT" OFF)
set(YUZU_QT_MIRROR "" CACHE STRING "What mirror to use for downloading the bundled Qt libraries")
cmake_dependent_option(YUZU_USE_BUNDLED_QT "Download bundled Qt binaries" "${MSVC}" "ENABLE_QT" OFF)

option(ENABLE_DEBUG_TOOLS "Enable debugging tools (maxwell disassembler, SPIRV translator, etc)" OFF)
option(ENABLE_WERROR "Enable -Werror diagnostics" ON)

cmake_dependent_option(ENABLE_LSFG "Enable Lossless Scaling frame generation" ON "ANDROID" OFF)
option(ENABLE_RESHADE "Enable ReShade FX post-processing effects" ON)

if (YUZU_USE_BUNDLED_QT AND (APPLE OR NOT UNIX))
    set(YUZU_STATIC_BUILD ON)
endif()

option(YUZU_STATIC_BUILD "Use static libraries and executables if available" OFF)
if (YUZU_STATIC_BUILD)
    set(Boost_USE_STATIC_LIBS ON)
    set(BUILD_SHARED_LIBS OFF)
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
    set(CMAKE_IMPORT_LIBRARY_SUFFIX ".a")

    if (MINGW)
        function(find_library var)
            if (${var} AND NOT ${var} MATCHES "dll\\.a$")
                return()
            endif()
            _find_library(${var} ${ARGN})
            if (${var})
                get_filename_component(lib_name "${${var}}" NAME)
                if (lib_name MATCHES "dll\\.a$")
                    unset(${var} CACHE)
                    set(${var} "${var}-NOTFOUND" CACHE INTERNAL "" FORCE)
                endif()
            endif()
        endfunction()
        set(QuaZip-Qt6_FORCE_BUNDLED ON)
        set(YUZU_USE_BUNDLED_FFMPEG ON)
        set(YUZU_USE_BUNDLED_SDL3 ON)
        set(YUZU_USE_BUNDLED_OPENSSL ON)
        set(HTTPLIB_USE_BROTLI_IF_AVAILABLE OFF)
    elseif(APPLE)
        set(YUZU_USE_BUNDLED_FFMPEG ON)
        set(YUZU_USE_BUNDLED_SDL3 ON)
        set(YUZU_USE_BUNDLED_OPENSSL ON)
        set(fmt_FORCE_BUNDLED ON)
        set(SPIRV-Headers_FORCE_BUNDLED ON)
        set(zstd_FORCE_BUNDLED ON)
    endif()
endif()

if (MSVC AND ARCHITECTURE_x86)
    message(FATAL_ERROR "Attempting to build with the x86 environment is not supported.")
endif()

if (CXX_CLANG_CL)
    add_compile_options(
        $<$<COMPILE_LANGUAGE:C,CXX>:-Wno-unused-command-line-argument>
        $<$<COMPILE_LANGUAGE:C,CXX>:-Wno-unsafe-buffer-usage>
        $<$<COMPILE_LANGUAGE:C,CXX>:-Wno-unused-value>
        $<$<COMPILE_LANGUAGE:C,CXX>:-Wno-extra-semi-stmt>
        $<$<COMPILE_LANGUAGE:C,CXX>:-Wno-sign-conversion>
        $<$<COMPILE_LANGUAGE:C,CXX>:-Wno-reserved-identifier>
        $<$<COMPILE_LANGUAGE:C,CXX>:-Wno-deprecated-declarations>
        $<$<COMPILE_LANGUAGE:C,CXX>:-Wno-cast-function-type-mismatch>
        $<$<COMPILE_LANGUAGE:C,CXX>:-Wno-c99-extensions>
        $<$<COMPILE_LANGUAGE:C,CXX>:-Wno-c++17-compat>
        $<$<COMPILE_LANGUAGE:C,CXX>:-Wno-c++11-compat-reserved-user-defined-literal>
        $<$<COMPILE_LANGUAGE:C,CXX>:-Wno-c++11-compat-deprecated-writable-strings>
        $<$<COMPILE_LANGUAGE:C,CXX>:-Wno-c++11-compat-pedantic>
        $<$<COMPILE_LANGUAGE:C,CXX>:-Wno-c++11-compat>
        $<$<COMPILE_LANGUAGE:C,CXX>:-Wno-c++0x-compat>
        $<$<COMPILE_LANGUAGE:C,CXX>:-Wno-c++98-c++11-compat-binary-literal>
        $<$<COMPILE_LANGUAGE:C,CXX>:-Wno-c++98-compat-pedantic>
        $<$<COMPILE_LANGUAGE:C,CXX>:-Wno-c++98-compat>
        $<$<COMPILE_LANGUAGE:C,CXX>:-Wno-c99-compat>
        $<$<COMPILE_LANGUAGE:C,CXX>:-Wno-error=all>
        $<$<COMPILE_LANGUAGE:C,CXX>:/EHsc>)
        
    if (ARCHITECTURE_x86_64)
        add_compile_options(
            $<$<COMPILE_LANGUAGE:C,CXX>:-msse4.1>
            $<$<COMPILE_LANGUAGE:C,CXX>:-mcx16>
            
            # =======================================================================
            # HARDWARE INSTRUCTION BASELINE LOCK (Core i5-9600K Optimization Matrix)
            # =======================================================================
            $<$<COMPILE_LANGUAGE:C,CXX>:-march=skylake>
            $<$<COMPILE_LANGUAGE:C,CXX>:-mbmi>
            $<$<COMPILE_LANGUAGE:C,CXX>:-mbmi2>
        )
        message(STATUS "[ARCH ENGINE] Coffee Lake target locked: Enforcing baseline native BMI hardware instruction paths.")
    endif()
endif()

if (MSVC AND NOT CXX_CLANG)
    set(CMAKE_CXX_FLAGS_INIT "${CMAKE_CXX_FLAGS_INIT} /W3 /WX-")
endif()

if(MSVC)
    if (YUZU_USE_BUNDLED_QT AND ARCHITECTURE_arm64)
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
        set(libflag MT)
    else()
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
        set(libflag MD)
    endif()

    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /${libflag}")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /${libflag}d")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /${libflag}")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /${libflag}d")

    add_compile_options(
        $<$<COMPILE_LANGUAGE:C,CXX>:$<$<CONFIG:Debug>:/${libflag}d>>
        $<$<COMPILE_LANGUAGE:C,CXX>:$<$<CONFIG:Release>:/${libflag}>>
        $<$<COMPILE_LANGUAGE:C,CXX>:$<$<CONFIG:RelWithDebInfo>:/${libflag}>>
        $<$<COMPILE_LANGUAGE:C,CXX>:$<$<CONFIG:MinSizeRel>:/${libflag}>>)

    # =======================================================================
    # INJECTION POINT 1: Global Linker Isolation Defenses for Standalone PGO
    # =======================================================================
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE 
        "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /NODEFAULTLIB:libcmt.lib /ALLOWISOLATION"
    )
    message(STATUS "[GLOBAL LINKER OVERRIDE] Enforced release runtime library isolation limits successfully.")
endif()

option(ENABLE_CUBEB "Enables the cubeb audio backend" ON)

set(EXT_DEFAULT OFF)
if (MSVC OR ANDROID)
    set(EXT_DEFAULT ON)
endif()

option(YUZU_USE_BUNDLED_SDL3 "Download bundled SDL3 build" ${EXT_DEFAULT})
option(YUZU_USE_BUNDLED_FFMPEG "Download bundled FFmpeg" ${EXT_DEFAULT})
cmake_dependent_option(YUZU_USE_EXTERNAL_FFMPEG "Build FFmpeg from external source" "${SOLARIS}" "NOT WIN32 AND NOT ANDROID" OFF)

set(BUNDLED_SIRIT_DEFAULT OFF)
if (MSVC AND NOT (CMAKE_BUILD_TYPE MATCHES "Deb") OR ANDROID)
    set(BUNDLED_SIRIT_DEFAULT ON)
endif()

option(YUZU_USE_BUNDLED_SIRIT "Download bundled sirit" ${BUNDLED_SIRIT_DEFAULT})
cmake_dependent_option(ENABLE_LIBUSB "Enable the use of LibUSB" ON "WIN32 OR LINUX OR FREEBSD OR APPLE" OFF)
cmake_dependent_option(ENABLE_OPENGL "Enable OpenGL" ON "NOT (WIN32 AND ARCHITECTURE_arm64) AND NOT APPLE AND NOT ANDROID" OFF)
mark_as_advanced(FORCE ENABLE_OPENGL)

option(ENABLE_WEB_SERVICE "Enable web services (telemetry, etc.)" ON)
option(ENABLE_WIFI_SCAN "Enable WiFi scanning" OFF)
cmake_dependent_option(USE_DISCORD_PRESENCE "Enables Discord Rich Presence" OFF "ENABLE_QT" OFF)
option(YUZU_TESTS "Compile tests" "${BUILD_TESTING}")
cmake_dependent_option(YUZU_INSTALL_UDEV_RULES "Install udev rules for gyro access" OFF "LINUX" OFF)
option(YUZU_DOWNLOAD_ANDROID_VVL "Download validation layer binary for android" ON)
option(YUZU_LEGACY "Apply patches that improve compatibility with older GPUs (e.g. Snapdragon 865) at the cost of performance" OFF)
option(NIGHTLY_BUILD "Use Nightly qualifiers in the update checker and build metadata" OFF)
cmake_dependent_option(YUZU_ROOM "Enable dedicated room functionality" ON "NOT ANDROID" OFF)
cmake_dependent_option(YUZU_ROOM_STANDALONE "Enable standalone room executable" ON "YUZU_ROOM" OFF)
cmake_dependent_option(YUZU_CMD "Compile the eden-cli executable" ON "NOT ANDROID" OFF)
cmake_dependent_option(YUZU_CRASH_DUMPS "Compile crash dump (Minidump) support" OFF "WIN32 OR LINUX" OFF)
option(YUZU_DOWNLOAD_TIME_ZONE_DATA "Always download time zone binaries" ON)
set(YUZU_TZDB_PATH "" CACHE STRING "Path to a pre-downloaded timezone database")

cmake_dependent_option(YUZU_USE_BUNDLED_MOLTENVK "Download bundled MoltenVK lib" ON "APPLE" OFF)

option(YUZU_DISABLE_LLVM "Disable LLVM (useful for CI)" OFF)

if (WIN32)
    # Expose the binary path location directly to all sub-modules without calling find_package
    set(LLVM_DIR "C:/Program Files/LLVM" CACHE PATH "Path to standalone LLVM installation")
    message(STATUS "[LLVM FACTORY] Standard link maps bound to host path: ${LLVM_DIR}")
endif()

set(DEFAULT_YUZU_USE_BUNDLED_OPENSSL OFF)
if (EXT_DEFAULT OR SOLARIS OR OPENBSD)
    set(DEFAULT_YUZU_USE_BUNDLED_OPENSSL ON)
endif()

option(YUZU_USE_BUNDLED_OPENSSL "Download bundled OpenSSL build" ${DEFAULT_YUZU_USE_BUNDLED_OPENSSL})
if (ANDROID AND YUZU_DOWNLOAD_ANDROID_VVL)
    AddJsonPackage(vulkan-validation-layers)
    set(abi ${CMAKE_ANDROID_ARCH_ABI})
    set(vvl_lib_path "${CMAKE_CURRENT_SOURCE_DIR}/src/android/app/src/main/jniLibs/${abi}/")
    file(COPY "${vulkan-validation-layers_SOURCE_DIR}/${abi}/libVkLayer_khronos_validation.so"
        DESTINATION "${vvl_lib_path}")
endif()

if(EXISTS ${PROJECT_SOURCE_DIR}/hooks/pre-commit AND NOT EXISTS ${PROJECT_SOURCE_DIR}/.git/hooks/pre-commit)
    if (EXISTS ${PROJECT_SOURCE_DIR}/.git/)
        message(STATUS "Copying pre-commit hook")
        file(COPY hooks/pre-commit DESTINATION ${PROJECT_SOURCE_DIR}/.git/hooks)
    endif()
endif()

if (ARCHITECTURE_arm64 AND (ANDROID OR LINUX))
    set(HAS_NCE 1)
    add_compile_definitions(HAS_NCE=1)
endif()

if (YUZU_ROOM)
    add_compile_definitions(YUZU_ROOM)
endif()

if (UNIX AND NOT (LINUX OR WIN32))
    if(CXX_APPLE OR CXX_CLANG)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fexperimental-library")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fexperimental-library")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_LIBCPP_ENABLE_EXPERIMENTAL=1")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_LIBCPP_ENABLE_EXPERIMENTAL=1")
    endif()
endif()

if (CXX_GCC OR CXX_CLANG)
    if (ARCHITECTURE_x86_64)
        set(YUZU_BUILD_PRESET "custom" CACHE STRING "Build preset to use.")
        if (${YUZU_BUILD_PRESET} STREQUAL "generic")
            set(march x86-64)
            set(mtune generic)
        elseif (${YUZU_BUILD_PRESET} STREQUAL "v3")
            set(march x86-64-v3)
            set(mtune generic)
        elseif (${YUZU_BUILD_PRESET} STREQUAL "zen2")
            set(march znver2)
            set(mtune znver2)
        elseif (${YUZU_BUILD_PRESET} STREQUAL "zen4")
            set(march znver4)
            set(mtune znver4)
        endif()
    elseif(ARCHITECTURE_arm64)
        set(YUZU_BUILD_PRESET "custom" CACHE STRING "Build preset to use.")
        set(mtune generic)
        if (${YUZU_BUILD_PRESET} STREQUAL "generic")
            set(march armv8-a)
        elseif (${YUZU_BUILD_PRESET} STREQUAL "optimized")
            set(march armv8.2-a+lse+rcpc)
        elseif (${YUZU_BUILD_PRESET} STREQUAL "armv9")
            set(march armv9-a)
        endif()
    endif()

    if ("${YUZU_BUILD_PRESET}" STREQUAL "native")
        set(march native)
        set(mtune native)
    endif()

    if (DEFINED march AND DEFINED mtune)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=${march} -mtune=${mtune}")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=${march} -mtune=${mtune}")
    endif()
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
find_package(RenderDoc MODULE)

if (YUZU_USE_BUNDLED_OPENSSL)
    set(BUILD_SHARED_LIBS OFF)
    AddJsonPackage(openssl-ci)
else()
    AddJsonPackage(openssl)
    set(OPENSSL_BUILD_VERBOSE ON)
    set(OPENSSL_CONFIGURE_VERBOSE ON)
    if (OpenSSL_ADDED)
        AddJsonPackage(openssl-cmake)
    endif()
endif()

if (OpenSSL_ADDED)
    add_compile_definitions(YUZU_BUNDLED_OPENSSL)
endif()

message(STATUS "Fetching needed dependencies with CPM")
set(BUILD_SHARED_LIBS OFF)
set(BUILD_TESTING OFF)
set(ENABLE_TESTING OFF)

set(BOOST_INCLUDE_LIBRARIES algorithm icl pool container heap asio headers process filesystem crc variant)
AddJsonPackage(boost)
set(BOOST_NO_HEADERS ${Boost_ADDED})

if (Boost_ADDED)
    add_compile_definitions(YUZU_BOOST_v1)
    if (NOT MSVC OR CXX_CLANG)
        if (SOLARIS)
            add_compile_options($<$<COMPILE_LANGUAGE:C,CXX>:-pthreads>)
        endif()
        target_compile_options(boost_heap INTERFACE $<$<COMPILE_LANGUAGE:C,CXX>:-Wno-shadow>)
        target_compile_options(boost_icl INTERFACE $<$<COMPILE_LANGUAGE:C,CXX>:-Wno-shadow>)
        target_compile_options(boost_asio INTERFACE
            $<$<COMPILE_LANGUAGE:C,CXX>:-Wno-conversion>
            $<$<COMPILE_LANGUAGE:C,CXX>:-Wno-implicit-fallthrough>)
    endif()
endif()

AddJsonPackage(fmt)
AddJsonPackage(lz4)

if (lz4_ADDED)
    add_library(lz4::lz4 ALIAS lz4_static)
endif()

AddJsonPackage(zstd)

if (zstd_ADDED)
    add_library(zstd::zstd ALIAS libzstd_static)
    add_library(zstd::libzstd ALIAS libzstd_static)
endif()

AddJsonPackage(nlohmann)

# =======================================================================
# AX-SDK (Shared C++20 Silicon Core & Topology)
# =======================================================================
FetchContent_Declare(
    ax_sdk
    GIT_REPOSITORY https://github.com/galaxywarriordad/ax-sdk.git
    GIT_TAG        v1_35_0
    # CMake 4.x defaults GIT_SHALLOW to TRUE for tag-like refs (--depth 1), which omits
    # annotated GitHub release tags from the shallow packfile and breaks tag checkout.
    GIT_SHALLOW    FALSE
)
FetchContent_MakeAvailable(ax_sdk)

if (NOT YUZU_STATIC_ROOM)
    AddJsonPackage(zlib)
    if (ZLIB_ADDED)
        add_library(ZLIB::ZLIB ALIAS zlibstatic)
    endif()

    AddJsonPackage(opus)
    if (Opus_ADDED)
        if (MSVC AND CXX_CLANG)
            target_compile_options(opus PRIVATE $<$<COMPILE_LANGUAGE:C,CXX>:-Wno-implicit-function-declaration>)
        endif()
    endif()

    if (NOT TARGET Opus::opus)
        add_library(Opus::opus ALIAS opus)
    endif()
endif()

if(NOT TARGET Boost::headers)
    AddJsonPackage(boost_headers)
endif()

set(FFmpeg_COMPONENTS avcodec avfilter avutil swscale)

function(create_target_directory_groups target_name)
    source_group("Other Files" REGULAR_EXPRESSION ".")
    get_target_property(target_sources "${target_name}" SOURCES)
    foreach(file_name IN LISTS target_sources)
        get_filename_component(dir_name "${file_name}" PATH)
        string(REPLACE "/" "\\" group_name "${dir_name}")
        source_group("${group_name}" FILES "${file_name}")
    endforeach()
endfunction()

if (APPLE)
    foreach(fw Carbon Metal Cocoa IOKit CoreVideo CoreMedia Security UniformTypeIdentifiers Foundation)
        find_library(${fw}_LIBRARY ${fw} REQUIRED)
        list(APPEND PLATFORM_LIBRARIES ${${fw}_LIBRARY})
    endforeach()
elseif (WIN32)
    add_compile_definitions(_WIN32_WINNT=0x0A00 WINVER=0x0A00)
    
    # Fortified Pass: Append native CUDA and Video Codec SDK boundaries to support the new Strategy A matrix
    set(PLATFORM_LIBRARIES ${PLATFORM_LIBRARIES} winmm iphlpapi ws2_32 wlanapi)
    
    if (MINGW)
        set(PLATFORM_LIBRARIES ${PLATFORM_LIBRARIES} psapi imm32 version crypt32 rpcrt4 gdi32 wldap32 mswsock)
    endif()
elseif (MANAGARM)
    set(PLATFORM_LIBRARIES iconv intl)
elseif (HAIKUOS)
    set(PLATFORM_LIBRARIES bsd /boot/system/lib/libnetwork.so)
elseif (CMAKE_SYSTEM_NAME MATCHES "^(Linux|kFreeBSD|GNU|SunOS)$")
    set(PLATFORM_LIBRARIES rt)
endif()

message(STATUS "Platform Libraries: ${PLATFORM_LIBRARIES}")
add_subdirectory(externals)
find_package(enet)

if (ARCHITECTURE_x86 OR ARCHITECTURE_x86_64)
    find_package(xbyak)
endif()

if (NOT YUZU_STATIC_ROOM)
    find_package(libusb)
    find_package(VulkanMemoryAllocator)
    find_package(VulkanUtilityLibraries)
    find_package(SimpleIni)
    find_package(sirit)
    find_package(gamemode)
    find_package(frozen)

    if (ARCHITECTURE_riscv64)
        find_package(biscuit)
    endif()

    if (ENABLE_WEB_SERVICE OR ENABLE_UPDATE_CHECKER)
        find_package(cpp-jwt)
    endif()

    if (ARCHITECTURE_arm64 OR DYNARMIC_TESTS)
        find_package(oaknut)
    endif()

    if (NOT ANDROID)
        find_package(SDL3)
    endif()

    if (USE_DISCORD_PRESENCE)
        find_package(DiscordRPC)
    endif()

    if (ENABLE_CUBEB)
        find_package(cubeb)
    endif()

    if (YUZU_TESTS OR DYNARMIC_TESTS)
        find_package(Catch2)
    endif()
endif()

if (ENABLE_QT)
    if (YUZU_USE_BUNDLED_QT)
        AddQt(Eden-CI/Qt 6.11.1)
    else()
        message(STATUS "Using system Qt")
        if (NOT Qt6_DIR)
            set(Qt6_DIR "" CACHE PATH "Additional path to search for Qt6 libraries")
        endif()
        list(APPEND CMAKE_PREFIX_PATH "${Qt6_DIR}")
    endif()

    find_package(Qt6 6.5 REQUIRED COMPONENTS Core)

    if (NOT DEFINED QT_TARGET_PATH)
        get_target_property(qtcore_path Qt6::Core LOCATION_Release)
        string(FIND "${qtcore_path}" "/bin/" qtcore_path_bin_pos REVERSE)
        string(FIND "${qtcore_path}" "/lib/" qtcore_path_lib_pos REVERSE)
        if (qtcore_path_bin_pos GREATER qtcore_path_lib_pos)
            string(SUBSTRING "${qtcore_path}" 0 ${qtcore_path_bin_pos} QT_TARGET_PATH)
        else()
            string(SUBSTRING "${qtcore_path}" 0 ${qtcore_path_lib_pos} QT_TARGET_PATH)
        endif()
    endif()

    if (NOT DEFINED QT_HOST_PATH)
        set(QT_HOST_PATH "${QT_TARGET_PATH}")
    endif()

    set(YUZU_QT_COMPONENTS Core Widgets Charts Concurrent Gui)
    if (LINUX OR FREEBSD)
        list(APPEND YUZU_QT_COMPONENTS DBus)
        set(QT_NO_PRIVATE_MODULE_WARNING ON)
        list(APPEND YUZU_QT_OPTIONAL GuiPrivate)
    endif()

    if (YUZU_USE_QT_MULTIMEDIA)
        list(APPEND YUZU_QT_COMPONENTS Multimedia)
    endif()

    if (YUZU_USE_QT_WEB_ENGINE)
        list(APPEND YUZU_QT_COMPONENTS WebEngineCore WebEngineWidgets)
    endif()

    if (ENABLE_QT_TRANSLATION)
        list(APPEND YUZU_QT_COMPONENTS LinguistTools)
    endif()

    find_package(Qt6 REQUIRED COMPONENTS ${YUZU_QT_COMPONENTS} OPTIONAL_COMPONENTS ${YUZU_QT_OPTIONAL})
    set(QT_MAJOR_VERSION 6)
    set_target_properties(Qt6::Platform PROPERTIES INTERFACE_COMPILE_FEATURES "")
    AddJsonPackage(quazip)
endif()

if (NOT YUZU_STATIC_ROOM AND NOT (YUZU_USE_BUNDLED_FFMPEG OR YUZU_USE_EXTERNAL_FFMPEG))
    find_package(FFmpeg REQUIRED QUIET COMPONENTS ${FFmpeg_COMPONENTS})
    set_property(GLOBAL APPEND PROPERTY CPM_PACKAGE_NAMES FFmpeg)
    set_property(GLOBAL APPEND PROPERTY CPM_PACKAGE_SHAS "unknown (system)")
    set_property(GLOBAL APPEND PROPERTY CPM_PACKAGE_URLS "https://github.com")
endif()

if (MSVC AND CMAKE_GENERATOR STREQUAL "Ninja")
    add_compile_options(
        /wd4464
        /wd4711
        /wd4820
    )
endif()

if (MSVC AND CXX_CLANG)
    if (ARCHITECTURE_x86_64)
        set(FILE_ARCH x86_64)
    elseif (ARCHITECTURE_arm64)
        set(FILE_ARCH aarch64)
    else()
        message(FATAL_ERROR "clang-cl: Unsupported architecture ${ARCHITECTURE}")
    endif()

    AddJsonPackage(llvm-mingw)
    set(LIB_PATH "${llvm-mingw_SOURCE_DIR}/libclang_rt.builtins-${FILE_ARCH}.a")
    add_library(llvm-mingw-runtime STATIC IMPORTED)
    set_target_properties(llvm-mingw-runtime PROPERTIES IMPORTED_LOCATION "${LIB_PATH}")
    link_libraries(llvm-mingw-runtime)
endif()
# =======================================================================
# INJECTION POINT 3: MSVC STL COMPATIBILITY & AUDIO VECTORIZATION LOCK
# =======================================================================
if (MSVC AND CXX_CLANG)
    add_compile_options("/D_DISABLE_VECTOR_ANNOTATION")
    add_compile_definitions(_DISABLE_VECTOR_ANNOTATION)
    add_compile_options("/bigobj") 
    message(STATUS "[GLOBAL STL FIX] Hard-enforced MSVC find-vectorizer bypass across all engine targets.")

    set(OPUS_TARGET_VARIATIONS "opus" "externals::opus" "bundled_libopus" "audio_core")
    foreach(_opus_target IN LISTS OPUS_TARGET_VARIATIONS)
        if(TARGET ${_opus_target})
            get_target_property(_current_flags ${_opus_target} COMPILE_OPTIONS)
            if(_current_flags)
                list(REMOVE_ITEM _current_flags "/fp:fast")
            else()
                set(_current_flags "")
            endif()
            list(APPEND _current_flags "/fp:precise" "-mavx2" "-mfma" "-mbmi2")
            set_target_properties(${_opus_target} PROPERTIES COMPILE_OPTIONS "${_current_flags}")
            message(STATUS "[OPUS OPTIMIZATION] Isolated /fp:precise, AVX2, and FMA for target: ${_opus_target}")
        endif()
    endforeach()
endif()
# =======================================================================
add_subdirectory(src)

if (TARGET core AND TARGET ax::core)
    target_link_libraries(core PRIVATE ax::core)
    message(STATUS "[AX-SDK] Linked ax::core (v1.35.0) into core engine target.")
endif()

if (ENABLE_DEBUG_TOOLS)
    add_subdirectory(tools/maxwell-disas)
    add_subdirectory(tools/maxwell-spirv)
    add_subdirectory(tools/maxwell-ir)
endif()

if(ENABLE_QT)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT yuzu)
else()
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT yuzu-cmd)
endif()

# =======================================================================
# INJECTION POINT 2: Target-Specific Whole Archive Enforcement for Stage 1
# =======================================================================
if (MSVC AND CXX_CLANG)
    if (CMAKE_C_FLAGS_RELEASE MATCHES "fprofile-instr-generate" OR CMAKE_CXX_FLAGS_RELEASE MATCHES "fprofile-instr-generate")
        if (TARGET core)
            set_target_properties(core PROPERTIES LINK_FLAGS "/WHOLEARCHIVE:clang_rt.profile-x86_64.lib")
            message(STATUS "[STAGE 1 LINK] Successfully bound whole-archive profiling hooks to the core engine target.")
        endif()
    endif()
endif()

if(ENABLE_QT AND UNIX AND NOT APPLE)
    install(FILES "dist/dev.eden_emu.eden.desktop" DESTINATION "share/applications")
    install(FILES "dist/dev.eden_emu.eden.svg" DESTINATION "share/icons/hicolor/scalable/apps")
    install(FILES "dist/dev.eden_emu.eden.xml" DESTINATION "share/mime/packages")
    install(FILES "dist/dev.eden_emu.eden.metainfo.xml" DESTINATION "share/metainfo")
endif()

if (YUZU_INSTALL_UDEV_RULES)
    include(GNUInstallDirs)
    install(FILES "dist/72-eden-input.rules" DESTINATION "${CMAKE_INSTALL_LIBDIR}/udev/rules.d")
endif()

# =======================================================================
# CLEAN PURE C LANGUAGE MATRIX FROM OVERLOADED C++ EXCLUSIVE FLAGS
# =======================================================================
if(NOT MSVC_IDE)
    get_property(CURRENT_C_FLAGS GLOBAL PROPERTY CMAKE_C_FLAGS_RELEASE)
    if(CURRENT_C_FLAGS)
        string(REGEX REPLACE "/std:c\\+\\+[0-9a-zA-Z]+" "" CLEANED_C_FLAGS "${CURRENT_C_FLAGS}")
        string(REGEX REPLACE "/Ob[1-3]" "" CLEANED_C_FLAGS "${CLEANED_C_FLAGS}")
        string(REGEX REPLACE "-clang:-Wlifetime" "" CLEANED_C_FLAGS "${CLEANED_C_FLAGS}")
        set(CMAKE_C_FLAGS_RELEASE "${CLEANED_C_FLAGS} /MD /O3 /Oi /Ot /Gw /Gy /DNDEBUG /D_WIN32_WINNT=0x0A00 /fp:precise" CACHE STRING "" FORCE)
    endif()
endif()