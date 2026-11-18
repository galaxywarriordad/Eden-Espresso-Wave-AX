// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <functional>
#include <span>
#include <vector>
#include <xbyak/xbyak.h> // Fixed: Injected full header to resolve incomplete base class type error
#include "common/common_types.h"

namespace Tegra::Engines {
class Maxwell3D;
}

namespace VideoCore {

using MacroJITExecutionTarget = std::function<u32(Tegra::Engines::Maxwell3D&, const std::vector<u32>&)>;

// Milestone 6 Acceleration Engine: Inherits natively from fully defined CodeGenerator
class MaxwellMacroJITCompiler : public Xbyak::CodeGenerator {
public:
    explicit MaxwellMacroJITCompiler();
    ~MaxwellMacroJITCompiler();

    MaxwellMacroJITCompiler(const MaxwellMacroJITCompiler&) = delete;
    MaxwellMacroJITCompiler& operator=(const MaxwellMacroJITCompiler&) = delete;

    MaxwellMacroJITCompiler(MaxwellMacroJITCompiler&&) = delete;
    MaxwellMacroJITCompiler& operator=(MaxwellMacroJITCompiler&&) = delete;

    /**
     * @brief Translates high-frequency guest GPU macros straight into native host x86_64 machine code.
     * @return Fast-path function pointer mapping straight to our execution page sandbox blocks.
     */
    MacroJITExecutionTarget CompileMacro(u32 macro_id, std::span<const u32> code);

    void InvalidateCache();
};

} // namespace VideoCore