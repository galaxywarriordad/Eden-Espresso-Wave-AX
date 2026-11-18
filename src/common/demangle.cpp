// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

#include <string>
#include <string_view>
#include <cstdlib>

#include "common/demangle.h"

#ifdef _WIN32
// Native Windows MSVC runtime engine demangler declaration
extern "C" char* __cdecl __unDName(char* outputString, const char* name, int maxStringLength, 
                                   void* (*pAlloc)(size_t), void (*pFree)(void*), unsigned short flags);
#endif

namespace Common {

std::string DemangleSymbol(const std::string& mangled) {
    if (mangled.empty()) {
        return mangled;
    }

#ifdef _WIN32
    // Bypasses missing LLVMDemangle.lib dependencies via native MSVC ABI parsing
    if (char* demangled = __unDName(nullptr, mangled.c_str(), 0, malloc, free, 0)) {
        std::string result(demangled);
        free(demangled);
        return result;
    }
#endif

    return mangled;
}

} // namespace Common
