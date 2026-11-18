#pragma once
#include <cstdint>
#include <cstddef>
#include <immintrin.h>
#include "common/common_types.h"

namespace VideoCore::Texture {

/**
 * @brief Linearly copies an arbitrary texture memory layout via 256-bit AVX2 vector sweeps.
 * @param dest Primary pre-allocated VRAM staging boundary pointer.
 * @param src Source block-linear texture data layout stream pointer.
 * @param size Total memory pass size footprint in bytes (Must be a multiple of 32 or 64).
 */
inline void UnswizzleCopyAVX2(u8* __restrict dest, const u8* __restrict src, std::size_t size) noexcept {
    std::size_t offset = 0;

    // Process blocks via unrolled 64-byte structural intervals (Two AVX2 registers)
    for (; offset + 64 <= size; offset += 64) {
        // Load data from source block linearly into CPU vector extensions
        __m256i reg_low  = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(src + offset));
        __m256i reg_high = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(src + offset + 32));

        // Use non-temporal stream instructions to completely bypass L3 allocation rings
        _mm256_stream_si256(reinterpret_cast<__m256i*>(dest + offset), reg_low);
        _mm256_stream_si256(reinterpret_cast<__m256i*>(dest + offset + 32), reg_high);
    }

    // Catch any remaining data blocks down to 32-byte alignment thresholds
    for (; offset + 32 <= size; offset += 32) {
        __m256i reg = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(src + offset));
        _mm256_stream_si256(reinterpret_cast<__m256i*>(dest + offset), reg);
    }

    // Handle structural trailing edge components smaller than an AVX2 lane footprint
    if (offset < size) {
        std::memcpy(dest + offset, src + offset, size - offset);
    }

    // Force global SFENCE execution execution boundaries to lock memory pipeline ordering
    _mm_sfence();
}

} // namespace VideoCore::Texture
