// SPDX-FileCopyrightText: Copyright 2025 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

// SPDX-FileCopyrightText: Copyright 2020 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include <algorithm>
#include <array>
#include <span>
#include <bc_decoder.h>

#include "common/common_types.h"
#include "common/settings.h"
#include "video_core/surface.h"
#include "video_core/texture_cache/types.h"
#include "video_core/texture_cache/decode_bc.h"

namespace VideoCommon {

namespace {

using VideoCore::Surface::PixelFormat;

constexpr std::array<u8, 7> BLOCK_SIZE_BY_FORMAT{
    8,  // BC1
    16, // BC2
    16, // BC3
    8,  // BC4
    16, // BC5
    16, // BC6
    16, // BC7
};

constexpr std::array<u8, 7> BYTES_PER_PIXEL_BY_FORMAT{
    4, // BC1 -> RGBA8
    4, // BC2 -> RGBA8
    4, // BC3 -> RGBA8
    1, // BC4 -> R8
    2, // BC5 -> RG8
    8, // BC6 -> RGBA16F
    4, // BC7 -> RGBA8
};

[[nodiscard]] size_t FormatIndex(PixelFormat format) {
    switch (format) {
    case PixelFormat::BC1_RGBA_UNORM:
    case PixelFormat::BC1_RGBA_SRGB:
        return 0;
    case PixelFormat::BC2_UNORM:
    case PixelFormat::BC2_SRGB:
        return 1;
    case PixelFormat::BC3_UNORM:
    case PixelFormat::BC3_SRGB:
        return 2;
    case PixelFormat::BC4_UNORM:
    case PixelFormat::BC4_SNORM:
        return 3;
    case PixelFormat::BC5_UNORM:
    case PixelFormat::BC5_SNORM:
        return 4;
    case PixelFormat::BC6H_SFLOAT:
    case PixelFormat::BC6H_UFLOAT:
        return 5;
    case PixelFormat::BC7_UNORM:
    case PixelFormat::BC7_SRGB:
        return 6;
    default:
        return 0;
    }
}

template <auto decompress, bool is_signed = false>
void DecompressBlocks(std::span<const u8> input, std::span<u8> output, BufferImageCopy& copy,
                      PixelFormat pixel_format) {
    const size_t idx = FormatIndex(pixel_format);
    const u32 block_size = BLOCK_SIZE_BY_FORMAT[idx];
    const u32 bytes_per_pixel = BYTES_PER_PIXEL_BY_FORMAT[idx];

    const u32 width = copy.image_extent.width;
    const u32 height = copy.image_extent.height;
    const u32 depth = copy.image_extent.depth;

    const u32 blocks_x = (width + 3U) / 4U;
    const u32 blocks_y = (height + 3U) / 4U;

    const size_t src_stride = static_cast<size_t>(blocks_x) * block_size;
    const size_t src_slice_stride = src_stride * blocks_y;
    const size_t dst_stride = static_cast<size_t>(width) * bytes_per_pixel;
    const size_t dst_slice_stride = dst_stride * height;

    for (u32 z = 0; z < depth; ++z) {
        const u8* src_slice = input.data() + z * src_slice_stride;
        u8* dst_slice = output.data() + z * dst_slice_stride;
        for (u32 by = 0; by < blocks_y; ++by) {
            for (u32 bx = 0; bx < blocks_x; ++bx) {
                const u8* src_block = src_slice + by * src_stride + bx * block_size;
                u8* dst_block = dst_slice + (by * 4U) * dst_stride + (bx * 4U) * bytes_per_pixel;
                if constexpr (std::is_invocable_v<decltype(decompress), const u8*, u8*, size_t,
                                                  size_t, size_t, size_t, bool>) {
                    decompress(src_block, dst_block, 0, 0,
                               std::min(4U, width - bx * 4U), std::min(4U, height - by * 4U),
                               is_signed);
                } else {
                    decompress(src_block, dst_block, 0, 0,
                               std::min(4U, width - bx * 4U), std::min(4U, height - by * 4U));
                }
            }
        }
    }
}

} // Anonymous namespace

u32 ConvertedBytesPerBlock(PixelFormat pixel_format) {
    return BYTES_PER_PIXEL_BY_FORMAT[FormatIndex(pixel_format)] * 16U;
}

void DecompressBCn(std::span<const u8> input, std::span<u8> output, BufferImageCopy& copy,
                   PixelFormat pixel_format) {
    switch (pixel_format) {
    case PixelFormat::BC1_RGBA_UNORM:
    case PixelFormat::BC1_RGBA_SRGB:
        DecompressBlocks<bcn::DecodeBc1>(input, output, copy, pixel_format);
        break;
    case PixelFormat::BC2_UNORM:
    case PixelFormat::BC2_SRGB:
        DecompressBlocks<bcn::DecodeBc2>(input, output, copy, pixel_format);
        break;
    case PixelFormat::BC3_UNORM:
    case PixelFormat::BC3_SRGB:
        DecompressBlocks<bcn::DecodeBc3>(input, output, copy, pixel_format);
        break;
    case PixelFormat::BC4_UNORM:
        DecompressBlocks<bcn::DecodeBc4, false>(input, output, copy, pixel_format);
        break;
    case PixelFormat::BC4_SNORM:
        DecompressBlocks<bcn::DecodeBc4, true>(input, output, copy, pixel_format);
        break;
    case PixelFormat::BC5_UNORM:
        DecompressBlocks<bcn::DecodeBc5, false>(input, output, copy, pixel_format);
        break;
    case PixelFormat::BC5_SNORM:
        DecompressBlocks<bcn::DecodeBc5, true>(input, output, copy, pixel_format);
        break;
    case PixelFormat::BC6H_UFLOAT:
        DecompressBlocks<bcn::DecodeBc6, false>(input, output, copy, pixel_format);
        break;
    case PixelFormat::BC6H_SFLOAT:
        DecompressBlocks<bcn::DecodeBc6, true>(input, output, copy, pixel_format);
        break;
    case PixelFormat::BC7_UNORM:
    case PixelFormat::BC7_SRGB:
        DecompressBlocks<bcn::DecodeBc7>(input, output, copy, pixel_format);
        break;
    default:
        break;
    }
}

} // namespace VideoCommon
