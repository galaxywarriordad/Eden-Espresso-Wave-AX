// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <string_view>

namespace Eden::Audio {

class FlexASIOManager {
public:
    /**
     * @brief Enforces an optimized, localized FlexASIO configuration profile within the execution directory.
     */
    static bool ProvisionLocalConfiguration();
};

} // namespace Eden::Audio
