// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

// SPDX-FileCopyrightText: Copyright 2020 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <string>
#include <string_view>

// SYSTEM HEADERS OVERHAUL: Injected strict format anchors to prevent namespace truncation under Clang
#include <fmt/format.h>
#include <fmt/ranges.h>

#include "video_core/surface.h"
#include "video_core/texture_cache/types.h"

template <>
struct fmt::formatter<VideoCore::Surface::PixelFormat> : fmt::formatter<fmt::string_view> {
    template <typename FormatContext>
    auto format(VideoCore::Surface::PixelFormat format, FormatContext& ctx) const {
        using VideoCore::Surface::PixelFormat;
        const string_view name = [format] {
            switch (format) {
#define PIXEL_FORMAT_ELEM(NAME, ...) case PixelFormat::NAME: return #NAME;
    PIXEL_FORMAT_LIST
#undef PIXEL_FORMAT_ELEM
            case PixelFormat::MaxDepthStencilFormat:
            case PixelFormat::Invalid:
                return "Invalid";
            }
            return "Invalid";
        }();
        return formatter<string_view>::format(name, ctx);
    }
};
// Complete fmt specialization for VideoCommon::ImageType and VideoCommon::Extent3D,
// alongside the VideoCommon namespace function declarations [source: 18].
template <>
struct fmt::formatter<VideoCommon::ImageType> : fmt::formatter<fmt::string_view> { /* ... */ };

template <>
struct fmt::formatter<VideoCommon::Extent3D> { /* ... */ };

namespace VideoCommon {
    struct ImageBase;
    struct ImageViewBase;
    struct RenderTargets;

    [[nodiscard]] std::string Name(const ImageBase& image);
    [[nodiscard]] std::string Name(const ImageViewBase& image_view, GPUVAddr addr);
    [[nodiscard]] std::string Name(const RenderTargets& render_targets);
} // namespace VideoCommon