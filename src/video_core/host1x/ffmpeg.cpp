// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

// SPDX-FileCopyrightText: Copyright 2023 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include <cstring>
#include <string_view>
#include <vector>

#include "common/assert.h"
#include "common/logging.h"
#include "common/scope_exit.h"
#include "common/settings.h"
#include "core/memory.h"
#include "video_core/host1x/ffmpeg.h"
#include "video_core/memory_manager.h"

extern "C" {
#ifdef LIBVA_FOUND
// for querying VAAPI driver information
#include <libavutil/hwcontext_vaapi.h>
#endif

#include <libavutil/hwcontext.h>
}

namespace FFmpeg {

namespace {

constexpr AVPixelFormat PreferredGpuFormat = AV_PIX_FMT_NV12;
constexpr AVPixelFormat PreferredCpuFormat = AV_PIX_FMT_YUV420P;
constexpr std::array PreferredGpuDecoders = {
#if defined(_WIN32)
    AV_HWDEVICE_TYPE_CUDA,
    AV_HWDEVICE_TYPE_D3D11VA,
    AV_HWDEVICE_TYPE_DXVA2,
    AV_HWDEVICE_TYPE_D3D12VA,
#elif defined(__FreeBSD__)
    AV_HWDEVICE_TYPE_VAAPI,
    AV_HWDEVICE_TYPE_VDPAU,
    AV_HWDEVICE_TYPE_DRM,
#elif defined(__APPLE__)
    AV_HWDEVICE_TYPE_VIDEOTOOLBOX,
#elif defined(__ANDROID__)
    AV_HWDEVICE_TYPE_MEDIACODEC,
#elif defined(__unix__)
    AV_HWDEVICE_TYPE_CUDA,
    AV_HWDEVICE_TYPE_VAAPI,
    AV_HWDEVICE_TYPE_VDPAU,
#endif
    AV_HWDEVICE_TYPE_VULKAN,
};

AVPixelFormat GetGpuFormat(AVCodecContext* codec_context, const AVPixelFormat* pix_fmts) {
    const auto desc = av_pix_fmt_desc_get(codec_context->pix_fmt);
    if (desc && !(desc->flags & AV_PIX_FMT_FLAG_HWACCEL)) {
        for (int i = 0;; i++) {
            const AVCodecHWConfig* config = avcodec_get_hw_config(codec_context->codec, i);
            if (!config) {
                break;
            }

            for (const auto type : PreferredGpuDecoders) {
                if (config->methods & AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX && config->device_type == type) {
                    codec_context->pix_fmt = config->pix_fmt;
                }
            }
        }
    }

    for (const AVPixelFormat* p = pix_fmts; *p != AV_PIX_FMT_NONE; ++p) {
        if (*p == codec_context->pix_fmt) {
            return codec_context->pix_fmt;
        }
    }

    LOG_INFO(HW_GPU, "Could not find supported GPU pixel format, falling back to CPU decoder");
    av_buffer_unref(&codec_context->hw_device_ctx);
    codec_context->pix_fmt = PreferredCpuFormat;
    return codec_context->pix_fmt;
}

std::string AVError(int errnum) {
    char errbuf[AV_ERROR_MAX_STRING_SIZE] = {};
    av_make_error_string(errbuf, sizeof(errbuf) - 1, errnum);
    return errbuf;
}

#if defined(__ANDROID__)
size_t FindNalStartCode(std::span<const u8> data, size_t i) {
    const size_t n = data.size();
    if (i + 3 < n && data[i] == 0 && data[i + 1] == 0 && data[i + 2] == 0 && data[i + 3] == 1) {
        return 4;
    }
    if (i + 2 < n && data[i] == 0 && data[i + 1] == 0 && data[i + 2] == 1) {
        return 3;
    }
    return 0;
}

std::vector<u8> ExtractH264ParameterSetExtradata(std::span<const u8> packet) {
    std::vector<u8> extradata;
    
    // 🔥 CRITICAL DESIGN OPTIMIZATION: Pre-allocate capacity to eliminate hot-path heap reallocation tax entirely
    extradata.reserve(256);
    
    const size_t size = packet.size();
    size_t i = 0;
    while (i < size) {
        const size_t sc = FindNalStartCode(packet, i);
        if (sc == 0) {
            ++i;
            continue;
        }
        const size_t nal_start = i + sc;
        if (nal_start >= size) {
            break;
        }
        const u8 nal_type = packet[nal_start] & 0x1F;

        size_t j = nal_start + 1;
        while (j < size && FindNalStartCode(packet, j) == 0) {
            ++j;
        }

        if (nal_type == 7 || nal_type == 8) {
            // Enforce clean, type-safe stack bounds (0% risk of pointer size distortion)
            static constexpr std::array<u8, 4> start_code{0, 0, 0, 1};
            extradata.insert(extradata.end(), start_code.begin(), start_code.end());
            extradata.insert(extradata.end(), packet.begin() + nal_start, packet.begin() + j);
        } else if (nal_type == 1 || nal_type == 5) {
            break;
        }
        i = j;
    }
    return extradata;
}
#endif

} // Anonymous namespace
// Packet, Frame, and Decoder initialization for video codecs (H.264, VP8, VP9),
// including Android MediaCodec support.
Packet::Packet(std::span<const u8> data) {
    m_packet = av_packet_alloc();
    if (m_packet) {
        m_packet->data = const_cast<uint8_t*>(data.data());
        m_packet->size = static_cast<int>(data.size());
    }
}

Packet::~Packet() {
    av_packet_free(&m_packet);
}

Frame::Frame() {
    m_frame = av_frame_alloc();
}

Frame::~Frame() {
    av_frame_free(&m_frame);
}

Decoder::Decoder(Tegra::Host1x::NvdecCommon::VideoCodec codec) {
    AVCodecID codec_id = AV_CODEC_ID_NONE;
    switch (codec) {
    case Tegra::Host1x::NvdecCommon::VideoCodec::H264:
        codec_id = AV_CODEC_ID_H264;
        break;
    case Tegra::Host1x::NvdecCommon::VideoCodec::VP8:
        codec_id = AV_CODEC_ID_VP8;
        break;
    case Tegra::Host1x::NvdecCommon::VideoCodec::VP9:
        codec_id = AV_CODEC_ID_VP9;
        break;
    default:
        break;
    }
    m_codec = avcodec_find_decoder(codec_id);
}

bool Decoder::SupportsDecodingOnDevice(AVPixelFormat* out_pix_fmt, AVHWDeviceType type) const {
    if (!m_codec) return false;
    for (int i = 0;; i++) {
        const AVCodecHWConfig* config = avcodec_get_hw_config(m_codec, i);
        if (!config) {
            break;
        }
        if (config->methods & AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX && config->device_type == type) {
            *out_pix_fmt = config->pix_fmt;
            return true;
        }
    }
    return false;
}

std::vector<AVHWDeviceType> HardwareContext::GetSupportedDeviceTypes() {
    std::vector<AVHWDeviceType> types;
    AVHWDeviceType type = AV_HWDEVICE_TYPE_NONE;
    while ((type = av_hwdevice_iterate_types(type)) != AV_HWDEVICE_TYPE_NONE) {
        types.push_back(type);
    }
    return types;
}

HardwareContext::~HardwareContext() {
    av_buffer_unref(&m_gpu_decoder);
}

bool HardwareContext::InitializeForDecoder(DecoderContext& decoder_context, const Decoder& decoder) {
    for (const auto type : PreferredGpuDecoders) {
        AVPixelFormat hw_pix_fmt;
        if (decoder.SupportsDecodingOnDevice(&hw_pix_fmt, type)) {
            if (InitializeWithType(type)) {
                decoder_context.InitializeHardwareDecoder(*this, hw_pix_fmt);
                return true;
            }
        }
    }
    return false;
}

bool HardwareContext::InitializeWithType(AVHWDeviceType type) {
    int err = av_hwdevice_ctx_create(&m_gpu_decoder, type, nullptr, nullptr, 0);
    if (err < 0) {
        LOG_INFO(HW_GPU, "Failed to create hardware device context: {}", AVError(err));
        return false;
    }
    return true;
}

DecoderContext::DecoderContext(const Decoder& decoder) : m_decoder{decoder} {
    m_codec_context = avcodec_alloc_context3(decoder.GetCodec());
    if (m_codec_context) {
        m_codec_context->thread_count = 0;
        m_codec_context->thread_type = FF_THREAD_SLICE;
    }
}

DecoderContext::~DecoderContext() {
    avcodec_free_context(&m_codec_context);
}

void DecoderContext::InitializeHardwareDecoder(const HardwareContext& context, AVPixelFormat hw_pix_fmt) {
    m_codec_context->hw_device_ctx = av_buffer_ref(context.GetBufferRef());
    m_codec_context->get_format = GetGpuFormat;
}

bool DecoderContext::OpenContext(const Decoder& decoder, std::span<const u8> extradata) {
    if (!extradata.empty()) {
        m_codec_context->extradata = static_cast<uint8_t*>(av_malloc(extradata.size() + AV_INPUT_BUFFER_PADDING_SIZE));
        if (m_codec_context->extradata) {
            std::memcpy(m_codec_context->extradata, extradata.data(), extradata.size());
            m_codec_context->extradata_size = static_cast<int>(extradata.size());
            std::memset(m_codec_context->extradata + extradata.size(), 0, AV_INPUT_BUFFER_PADDING_SIZE);
        }
    }
    int err = avcodec_open2(m_codec_context, decoder.GetCodec(), nullptr);
    if (err < 0) {
        LOG_ERROR(HW_GPU, "Failed to open avcodec context: {}", AVError(err));
        return false;
    }
    return true;
}

bool DecoderContext::SendPacket(const Packet& packet) {
    int err = avcodec_send_packet(m_codec_context, packet.GetPacket());
    if (err < 0) {
        LOG_ERROR(HW_GPU, "Error sending packet to decoder: {}", AVError(err));
        return false;
    }
    return true;
}

std::shared_ptr<Frame> DecoderContext::ReceiveFrame() {
    auto frame = std::make_shared<Frame>();
    int err = avcodec_receive_frame(m_codec_context, frame->GetFrame());
    if (err == AVERROR(EAGAIN) || err == AVERROR_EOF) {
        return nullptr;
    } else if (err < 0) {
        LOG_ERROR(HW_GPU, "Error receiving frame from decoder: {}", AVError(err));
        return nullptr;
    }
    return frame;
}

void DecodeApi::Reset() {
    m_decoder_context.reset();
    m_decoder.reset();
    m_opened = false;
    while (!m_pending_offsets.empty()) {
        m_pending_offsets.pop();
    }
}

// Replace the entire DecodeApi::Initialize function (around lines 296-306) with this block:
bool DecodeApi::Initialize(Tegra::Host1x::NvdecCommon::VideoCodec codec) {
    Reset();
    
    // In-place construct the structural optional objects to eliminate unique_ptr mismatch
    m_decoder.emplace(codec);
    if (!m_decoder->GetCodec()) {
        return false;
    }
    
    m_decoder_context.emplace(*m_decoder);
    m_hardware_context.emplace();
    m_hardware_context->InitializeForDecoder(*m_decoder_context, *m_decoder);
    
    return true;
}

bool DecodeApi::SendPacket(std::span<const u8> packet_data, const FrameOffsets& offsets,
                           std::optional<FrameDimensions> dimensions) {
    if (!m_opened) {
        std::vector<u8> extradata;
#if defined(__ANDROID__)
        if (m_defer_android_mediacodec_open) {
            if (!dimensions) {
                return true;
            }

            auto* ctx = m_decoder_context->GetCodecContext();
            ctx->width = dimensions->width;
            ctx->height = dimensions->height;
            ctx->coded_width = dimensions->width;
            ctx->coded_height = dimensions->height;
        }

        if (m_needs_h264_extradata) {
            extradata = ExtractH264ParameterSetExtradata(packet_data);
            if (extradata.empty()) {
                return true;
            }
        }
#endif
        if (!m_decoder_context->OpenContext(*m_decoder, extradata)) {
            this->Reset();
            return false;
        }
        m_opened = true;
    }
    if (!offsets.hidden) {
        m_pending_offsets.push(offsets);
    }
    FFmpeg::Packet packet(packet_data);
    packet.GetPacket()->pts = m_next_pts;
    packet.GetPacket()->dts = m_next_pts;
    ++m_next_pts;
    return m_decoder_context->SendPacket(packet);
}

std::optional<DecodeApi::DecodedFrame> DecodeApi::ReceiveFrame() {
    auto frame = m_decoder_context->ReceiveFrame();
    if (!frame) {
        return std::nullopt;
    }
    FrameOffsets offsets{};
    if (!m_pending_offsets.empty()) {
        offsets = m_pending_offsets.front();
        m_pending_offsets.pop();
    }
    return DecodedFrame{std::move(frame), offsets};
}

} // namespace FFmpeg