// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

#include "flex_asio_manager.hpp"
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string_view>

namespace Eden::Audio {

namespace {
constexpr std::string_view kHiByFlexAsioProfile =
    "# ================================================================================\n"
    "# EDEN EMULATOR HI-FI SINK: HARDWARE-LINKED TO HIBY FC3 ESS SABRE SILICON\n"
    "# ================================================================================\n\n"
    "backend = \"Windows WASAPI\"\n"
    "bufferSizeSamples = 128\n\n"
    "[input]\n"
    "device = \"\"\n"
    "suggestedLatencySeconds = 0.0\n"
    "wasapiExclusiveMode = true\n\n"
    "[output]\n"
    "device = \"HiBy USB Audio\"\n"
    "wasapiExclusiveMode = true\n"
    "wasapiAutoConvert = false\n"
    "suggestedLatencySeconds = 0.0\n"
    "channels = 2\n";

bool WriteProfileToPath(const std::filesystem::path& target_path) {
    std::ofstream config_file(target_path, std::ios::binary | std::ios::trunc);
    if (!config_file.is_open()) {
        return false;
    }
    config_file.write(kHiByFlexAsioProfile.data(),
                      static_cast<std::streamsize>(kHiByFlexAsioProfile.size()));
    return config_file.good();
}
} // Anonymous namespace

bool FlexASIOManager::ProvisionLocalConfiguration() {
    // 1. Provision directly inside the active runtime execution directory
    const std::filesystem::path local_path = std::filesystem::current_path() / "FlexASIO.toml";
    if (!WriteProfileToPath(local_path)) {
        std::cerr << "[EDEN AUDIO CRITICAL] Could not write FlexASIO profile to: "
                  << local_path.string() << '\n';
        return false;
    }

    // 2. Mirror to %USERPROFILE%\FlexASIO.toml so FlexASIO never loads a stale global fallback
    if (const char* user_profile = std::getenv("USERPROFILE"); user_profile != nullptr) {
        const std::filesystem::path global_path =
            std::filesystem::path(user_profile) / "FlexASIO.toml";
        (void)WriteProfileToPath(global_path);
    }

    std::clog << "[EDEN AUDIO INFO] HiBy FC3 (128-sample / 2.67ms) FlexASIO.toml synchronized.\n";
    return true;
}

} // namespace Eden::Audio
