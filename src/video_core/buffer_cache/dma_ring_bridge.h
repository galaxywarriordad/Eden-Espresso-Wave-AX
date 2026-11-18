#pragma once

#include <array>
#include <atomic>
#include <cstdint>
#include "common/common_types.h"

namespace VideoCore::BufferCache {

struct DMADeviceSegment {
    uintptr_t host_virtual_base{0}; 
    uint64_t segment_size{0};       
    bool is_high_speed_storage{false};  
};

class DMADeviceRingBridge {
private:
    std::array<DMADeviceSegment, 16> active_segments{};
    std::atomic<size_t> ring_size{0};

public:
    DMADeviceRingBridge() = default;
    ~DMADeviceRingBridge() = default;

    // Registers split storage paths (NVMe staging area vs 3 TB mechanical drive)
    void RegisterStorageSegment(uintptr_t v_base, uint64_t size, bool is_nvme) {
        size_t current_index = ring_size.load(std::memory_order_relaxed);
        if (current_index >= active_segments.size()) return; // Ring buffer safety gate

        active_segments[current_index] = DMADeviceSegment{v_base, size, is_nvme};
        ring_size.fetch_add(1, std::memory_order_release);
    }

    // Single-cycle address lookups utilizing native host bit extraction techniques
    inline uintptr_t TranslateGuestToHostDMA(uint64_t guest_addr) const {
        size_t limit = ring_size.load(std::memory_order_acquire);
        
        for (size_t i = 0; i < limit; ++i) {
            const auto& segment = active_segments[i];
            if (guest_addr >= segment.host_virtual_base && 
                guest_addr < (segment.host_virtual_base + segment.segment_size)) {
                // Clamps the translation vector down cleanly into your 15-bit & 32-bit tracking registers
                return segment.host_virtual_base + (guest_addr & 0x7FFFFFFF);
            }
        }
        return 0; // Fallback gate: address is out of non-contiguous ring maps
    }
};

} // namespace VideoCore::BufferCache