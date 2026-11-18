#include <memory>
#include <new>
#include "common/settings.h"
#include "video_core/texture_swizzle_simd.h"

namespace VideoCore::Texture {

class TextureStagingPool {
public:
    explicit TextureStagingPool() {
        // Resolve buffer configuration layouts defined inside settings dropdown mappings
        std::size_t buffer_limit = GetStreamSizeLimit();
        
        // Allocate fixed pinned memory pool locked to 64-byte Skylake Cache Lines
        m_staging_buffer = static_cast<u8*>(_aligned_malloc(buffer_limit, 64));
    }

    ~TextureStagingPool() {
        if (m_staging_buffer) {
            _aligned_free(m_staging_buffer);
        }
    }

    void StreamTextureLinear(const u8* linear_src_ptr, std::size_t texture_bytes) noexcept {
        std::size_t allowed_size = GetStreamSizeLimit();
        std::size_t active_bytes = (std::min)(texture_bytes, allowed_size);

        // Fire AVX2 Non-Temporal processing loops to unswizzle directly into the stage
        UnswizzleCopyAVX2(m_staging_buffer, linear_src_ptr, active_bytes);
    }

private:
    std::size_t GetStreamSizeLimit() const noexcept {
        // Evaluate framework enum flags dynamically: Medium maps to 32MB bounds
        switch (Settings::values.unswizzle_stream_size.GetValue()) {
            case Settings::StreamSizeOption::Small:   return 16 * 1024 * 1024;
            case Settings::StreamSizeOption::Medium:  return 32 * 1024 * 1024;
            case Settings::StreamSizeOption::Large:   return 64 * 1024 * 1024;
            default:                                  return 32 * 1024 * 1024;
        }
    }

    u8* m_staging_buffer{nullptr};
};

} // namespace VideoCore::Texture
