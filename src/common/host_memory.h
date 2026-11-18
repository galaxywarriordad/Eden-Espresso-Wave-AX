// Target Realignment Patch: src/common/host_memory.h
// Force complete micro-architectural alignment compliance during raw pointer loops

#pragma once

#include <memory>
#include <optional>
#include "common/common_funcs.h"
#include "common/common_types.h"

namespace Common {

enum class MemoryPermission : u32 {
    Read = 1 << 0,
    Write = 1 << 1,
    ReadWrite = Read | Write,
    Execute = 1 << 2,
};
DECLARE_ENUM_FLAG_OPERATORS(MemoryPermission)

class HostMemory {
public:
    explicit HostMemory(size_t backing_size_, size_t virtual_size_);
    ~HostMemory();

    HostMemory(const HostMemory& other) = delete;
    HostMemory& operator=(const HostMemory& other) = delete;

    HostMemory(HostMemory&& other) noexcept;
    HostMemory& operator=(HostMemory&& other) noexcept;

    void Map(size_t virtual_offset, size_t host_offset, size_t length, MemoryPermission perms, bool separate_heap);
    void Unmap(size_t virtual_offset, size_t length, bool separate_heap);
    void Protect(size_t virtual_offset, size_t length, MemoryPermission perms);
    void EnableDirectMappedAddress();
    void ClearBackingRegion(size_t physical_offset, size_t length, u32 fill_value);

    // TARGET REALIGNMENT REFIT: Forced 64-byte alignment masking to eliminate cache line splits
    [[nodiscard]] inline u8* BackingBasePointer() noexcept {
        return reinterpret_cast<u8*>(reinterpret_cast<uintptr_t>(backing_base) & 0xFFFFFFFFFFFFFFC0ULL);
    }
    [[nodiscard]] inline const u8* BackingBasePointer() const noexcept {
        return reinterpret_cast<const u8*>(reinterpret_cast<uintptr_t>(backing_base) & 0xFFFFFFFFFFFFFFC0ULL);
    }

    [[nodiscard]] u8* VirtualBasePointer() noexcept { return virtual_base; }
    [[nodiscard]] const u8* VirtualBasePointer() const noexcept { return virtual_base; }

    bool IsInVirtualRange(void* address) const noexcept {
        return address >= virtual_base && address < virtual_base + virtual_size;
    }

private:
    size_t backing_size{};
    size_t virtual_size{};

#if !(defined(__OPENORBIS__) || defined(__managarm__))
    class Impl;
    std::unique_ptr<Impl> impl;
#endif
    u8* backing_base{};
    u8* virtual_base{};
    size_t virtual_base_offset{};
    bool fallback_buffer{false};
};

} // namespace Common
