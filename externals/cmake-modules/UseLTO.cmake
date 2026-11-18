# SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
# SPDX-License-Identifier: GPL-3.0-or-later

## UseLTO (Hardened Coffee Lake / Clang-cl Edition) ##

option(ENABLE_LTO "Enable Link-Time Optimization (LTO)" ON) # Default to ON to safeguard PGO steps

if (ENABLE_LTO)
    if (MSVC AND CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        # 🏎️ Activate LLVM ThinLTO natively for high-velocity parallel link passes
        add_compile_options("-flto=thin")
        add_link_options("-flto=thin")
        
        # 🧵 Force the ThinLTO engine to utilize all 6 logical cores of the i5-9600K
        add_link_options("-Wl,/opt:lldltojobs=all")
        
        # 🧠 Allocate a dedicated 1GB disk cache boundary to speed up sub-module incremental links
        add_link_options("-Wl,/opt:lldltocachesize=1000")
        
        message(STATUS "[OPT CONTROLLER] Clang ThinLTO optimization matrix injected successfully.")
    else()
        # Fallback path for alternative compilation toolchains
        include(CheckIPOSupported)
        check_ipo_supported(RESULT COMPILER_SUPPORTS_LTO)
        if(NOT COMPILER_SUPPORTS_LTO)
            message(FATAL_ERROR
            "Your compiler does not support interprocedural optimization"
            " (IPO). Disable ENABLE_LTO and try again.")
        endif()
        set(CMAKE_POLICY_DEFAULT_CMP0069 NEW)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ${COMPILER_SUPPORTS_LTO})
        message(STATUS "[OPT CONTROLLER] Standard Monolithic IPO activated.")
    endif()
endif()