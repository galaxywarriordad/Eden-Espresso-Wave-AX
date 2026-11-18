#pragma once
#include <cstdint>

#if defined(__BMI__) || defined(__AVX2__) || defined(_MSC_VER)
#include <immintrin.h>
#define HAS_BMI1 1
#else
#define HAS_BMI1 0
#endif

namespace Common::BitUtils {

/**
 * @brief Safely extracts a bit field from a 64-bit integer using hardware BEXTR.
 * @param value The raw source register payload.
 * @param start The starting bit position (0-63).
 * @param length The total width of the bitfield to extract.
 */
[[nodiscard]] inline uint64_t ExtractBitField(uint64_t value, uint8_t start, uint8_t length) noexcept {
#if HAS_BMI1
    return _bextr_u64(value, start, length);
#else
    // Fallback path if compiled outside target matrix
    return (value >> start) & ((1ULL << length) - 1);
#endif
}

/**
 * @brief Resets the lowest set bit of a 64-bit integer to 0 using hardware BLSR.
 * Useful for high-speed page-table dirty bit clearing or layout iteration loops.
 */
[[nodiscard]] inline uint64_t ClearLowestSetBit(uint64_t value) noexcept {
#if HAS_BMI1
    return _blsr_u64(value);
#else
    return value & (value - 1);
#endif
}

} // namespace Common::BitUtils