#include "video_core/host1x/host1x.h"
#include "common/settings.h"

namespace Tegra::Host1x {

std::unique_ptr<VideoDecoderInterface> CreateDecoder(VideoCodecProfile profile, Core::System& system) {
    if (profile == VideoCodecProfile::VP9_Profile0 || profile == VideoCodecProfile::VP9_Profile2) {
        // Exploit the Intel UHD 630 fixed-function hardware blocks to handle VP9 natively
        LOG_INFO(Render_Hardware, "Routing VP9 bitstream to Intel QSV Decoder Ring.");
        return std::make_unique<QSVDecoder>(system);
    } else {
        // Keep H.264/AVC processing native to the primary GTX 980 Ti NVDEC engine
        LOG_INFO(Render_Hardware, "Routing standard bitstream natively to NVIDIA NVDEC.");
        return std::make_unique<NvidiaNVDECDecoder>(system);
    }
}

} // namespace Tegra::Host1x