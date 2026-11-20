// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

#include "flex_asio_manager.hpp"
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>

#if __has_include(<ax/config.hpp>) && __has_include(<ax/telemetry.hpp>)
#include <ax/config.hpp>
#include <ax/telemetry.hpp>
#define EDEN_HAVE_AX_SDK_CONFIG 1
#endif

namespace Eden::Audio {

namespace {
std::string BuildFlexAsioProfile(std::uint32_t buffer_samples, std::string_view device_name) {
    const std::string resolved_device =
        device_name.empty() ? std::string("HiBy USB Audio") : std::string(device_name);

    return "# ================================================================================\n"
           "# EDEN EMULATOR HI-FI SINK: HARDWARE-LINKED TO HIBY FC3 ESS SABRE SILICON\n"
           "# ================================================================================\n\n"
           "backend = \"Windows WASAPI\"\n"
           "bufferSizeSamples = " + std::to_string(buffer_samples) + "\n\n"
           "[input]\n"
           "device = \"\"\n"
           "suggestedLatencySeconds = 0.0\n"
           "wasapiExclusiveMode = true\n\n"
           "[output]\n"
           "device = \"" + resolved_device + "\"\n"
           "wasapiExclusiveMode = true\n"
           "wasapiAutoConvert = false\n"
           "suggestedLatencySeconds = 0.0\n"
           "channels = 2\n";
}

bool WriteProfileToPath(const std::filesystem::path& target_path, std::string_view content) {
    std::ofstream config_file(target_path, std::ios::binary | std::ios::trunc);
    if (!config_file.is_open()) {
        return false;
    }
    config_file.write(content.data(), static_cast<std::streamsize>(content.size()));
    return config_file.good();
}
} // Anonymous namespace

bool FlexASIOManager::ProvisionLocalConfiguration() {
    std::uint32_t buffer_samples = 128;
    std::string device_name = "HiBy USB Audio";

#if defined(EDEN_HAVE_AX_SDK_CONFIG)
    ax::EngineConfig ax_cfg{};
    if (!ax::LoadEngineConfig("ax_engine.ini", ax_cfg)) {
        (void)ax::WriteDefaultEngineConfig("ax_engine.ini");
    }
    if (ax_cfg.asio_buffer_samples > 0) {
        buffer_samples = ax_cfg.asio_buffer_samples;
    }
    if (!ax_cfg.exclusive_device_name.empty()) {
        device_name = ax_cfg.exclusive_device_name;
    }
    if (ax_cfg.enable_shared_memory_export) {
        (void)ax::InitTelemetryShm();
    }
#endif

    const std::string profile_content = BuildFlexAsioProfile(buffer_samples, device_name);

    // 1. Provision directly inside the active runtime execution directory
    const std::filesystem::path local_path = std::filesystem::current_path() / "FlexASIO.toml";
    if (!WriteProfileToPath(local_path, profile_content)) {
        std::cerr << "[EDEN AUDIO CRITICAL] Could not write FlexASIO profile to: "
                  << local_path.string() << '\n';
        return false;
    }

    // 2. Mirror to %USERPROFILE%\FlexASIO.toml so FlexASIO never loads a stale global fallback
    if (const char* user_profile = std::getenv("USERPROFILE"); user_profile != nullptr) {
        const std::filesystem::path global_path =
            std::filesystem::path(user_profile) / "FlexASIO.toml";
        (void)WriteProfileToPath(global_path, profile_content);
    }

    std::clog << "[EDEN AUDIO INFO] " << device_name << " (" << buffer_samples
              << "-sample) FlexASIO.toml synchronized.\n";
    return true;
}

} // namespace Eden::Audio
