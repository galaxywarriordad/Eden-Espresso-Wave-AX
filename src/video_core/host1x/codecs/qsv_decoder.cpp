// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

#include <windows.h>
#include <mfapi.h>
#include <mftransform.h>
#include <mfobjects.h>
#include <mferror.h>
#include <d3d11_4.h>
#include <dxgi1_6.h>
#include <string_view>
#include <objbase.h>
#include <cstring>

#include "common/logging.h"
#include "common/thread.h"
#include "video_core/host1x/codecs/qsv_decoder.h"

namespace {
constexpr CLSID CLSID_MSH264DecoderMFT = { 0x62CE7E72, 0x4C71, 0x4d20, { 0xB1, 0x3D, 0x72, 0x78, 0xE4, 0x06, 0x1F, 0x4D } };
constexpr CLSID CLSID_MSHEVCDecoderMFT = { 0x96611016, 0x6001, 0x4b78, { 0x93, 0x43, 0xAB, 0x54, 0x72, 0x9A, 0x79, 0x01 } };
constexpr CLSID CLSID_MSVP8DecoderMFT  = { 0x451e3cb7, 0x2622, 0x4ba5, { 0x8e, 0x1d, 0x26, 0x22, 0x4b, 0xa5, 0x8e, 0x1d } };
constexpr CLSID CLSID_MSVP9DecoderMFT  = { 0xa527022d, 0x7c72, 0x4d11, { 0xb1, 0x2f, 0x07, 0xbf, 0x1d, 0x0d, 0x7d, 0x35 } };

/// Pins the Host1x QSV video feeder thread once onto Core 5 (Mask 0x20) so DXGI/Media Foundation
/// calls never invade Primary JIT Cores 2–4 (0x1C) or Vulkan Submission Core 1 (0x02).
void EnsureQsvThreadQuarantined() {
    thread_local bool quarantined = [] {
        Common::SetCurrentThreadName("QSVVideoFeeder");
        Common::SetCurrentThreadPartition(Common::CorePartition::AuxQuarantine);
        Common::SetCurrentThreadPriority(Common::ThreadPriority::Low);
        return true;
    }();
    (void)quarantined;
}
} // Anonymous namespace

namespace VideoCore::Host1x {

struct QSVHardwareDecoderRing::Impl {
    ID3D11Device* intel_d3d11_device{nullptr};
    ID3D11DeviceContext* intel_context{nullptr};
    IMFTransform* decoder_transform{nullptr};
    IMFAttributes* transform_attributes{nullptr};
    IMFDXGIDeviceManager* device_manager{nullptr};
    UINT reset_token{0};

    ID3D11Texture2D* last_decoded_surface{nullptr};
    HANDLE shared_nt_handle{nullptr};
    Tegra::Host1x::VideoCodecProfile current_profile{static_cast<Tegra::Host1x::VideoCodecProfile>(-1)};
};

QSVHardwareDecoderRing::QSVHardwareDecoderRing() : impl{std::make_unique<Impl>()} {}
QSVHardwareDecoderRing::~QSVHardwareDecoderRing() { ReleaseContext(); }

void* QSVHardwareDecoderRing::GetSharedHandle() const noexcept {
    return impl->shared_nt_handle;
}

bool QSVHardwareDecoderRing::InitializeIntelContext(u32 width, u32 height, Tegra::Host1x::VideoCodecProfile profile) {
    EnsureQsvThreadQuarantined();

    if (is_initialized && surface_width == width && surface_height == height && impl->current_profile == profile) {
        return true;
    }

    if (is_initialized) {
        if (impl->decoder_transform) { impl->decoder_transform->Release(); impl->decoder_transform = nullptr; }
        if (impl->transform_attributes) { impl->transform_attributes->Release(); impl->transform_attributes = nullptr; }
        is_initialized = false;
    }

    bool local_com_init = false;
    if (!com_initialized) {
        HRESULT hr_com = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
        if (SUCCEEDED(hr_com) || hr_com == RPC_E_CHANGED_MODE) {
            com_initialized = true;
            local_com_init = true;
        }
    }

    auto com_failure_guard = [this, local_com_init](bool success) {
        if (!success && local_com_init) {
            CoUninitialize();
            this->com_initialized = false;
        }
    };

    surface_width = width;
    surface_height = height;
    impl->current_profile = profile;

    HRESULT hr = MFStartup(MF_VERSION);
    if (FAILED(hr)) { com_failure_guard(false); return false; }

    if (!impl->intel_d3d11_device) {
        IDXGIFactory6* dxgi_factory{nullptr};
        hr = CreateDXGIFactory1(__uuidof(IDXGIFactory6), reinterpret_cast<void**>(&dxgi_factory));
        if (FAILED(hr)) { MFShutdown(); com_failure_guard(false); return false; }

        IDXGIAdapter1* target_adapter{nullptr};
        bool found_intel = false;

        for (UINT index = 0; dxgi_factory->EnumAdapters1(index, &target_adapter) != DXGI_ERROR_NOT_FOUND; ++index) {
            DXGI_ADAPTER_DESC1 desc; target_adapter->GetDesc1(&desc);
            if (desc.VendorId == 0x8086) {
                found_intel = true;
                std::string narrowed_description;
                for (wchar_t c : desc.Description) {
                    if (c == 0) break;
                    narrowed_description.push_back(static_cast<char>(c));
                }
                LOG_INFO(Frontend, "Intel QSV Interop: Successfully bound decoding paths to engine: {}", std::string_view(narrowed_description));
                break;
            }
            target_adapter->Release();
        }
        dxgi_factory->Release();
        if (!found_intel) { MFShutdown(); com_failure_guard(false); return false; }

        UINT creation_flags = D3D11_CREATE_DEVICE_VIDEO_SUPPORT | D3D11_CREATE_DEVICE_BGRA_SUPPORT;
        D3D_FEATURE_LEVEL feature_levels[] = { D3D_FEATURE_LEVEL_11_1 };
        hr = D3D11CreateDevice(target_adapter, D3D_DRIVER_TYPE_UNKNOWN, nullptr, creation_flags, feature_levels, 1, D3D11_SDK_VERSION, &impl->intel_d3d11_device, nullptr, &impl->intel_context);
        target_adapter->Release();
        if (FAILED(hr)) { MFShutdown(); com_failure_guard(false); return false; }

        hr = MFCreateDXGIDeviceManager(&impl->reset_token, &impl->device_manager);
        if (FAILED(hr)) { MFShutdown(); com_failure_guard(false); return false; }
        hr = impl->device_manager->ResetDevice(impl->intel_d3d11_device, impl->reset_token);
        if (FAILED(hr)) { MFShutdown(); com_failure_guard(false); return false; }
    }

    CLSID target_clsid = CLSID_MSH264DecoderMFT;
    GUID sub_type = MFVideoFormat_H264;

    switch (profile) {
        case Tegra::Host1x::VideoCodecProfile::H264_Main:
        case Tegra::Host1x::VideoCodecProfile::H264_High:
            target_clsid = CLSID_MSH264DecoderMFT;
            sub_type = MFVideoFormat_H264;
            break;
        case Tegra::Host1x::VideoCodecProfile::HEVC_Main:
            target_clsid = CLSID_MSHEVCDecoderMFT;
            sub_type = MFVideoFormat_HEVC;
            break;
        case Tegra::Host1x::VideoCodecProfile::VP8_Simple:
            target_clsid = CLSID_MSVP8DecoderMFT;
            sub_type = MFVideoFormat_VP80;
            break;
        case Tegra::Host1x::VideoCodecProfile::VP9_Main:
            target_clsid = CLSID_MSVP9DecoderMFT;
            sub_type = MFVideoFormat_VP90;
            break;
    }

    hr = CoCreateInstance(target_clsid, nullptr, CLSCTX_INPROC_SERVER, __uuidof(IMFTransform), reinterpret_cast<void**>(&impl->decoder_transform));
    if (FAILED(hr)) { MFShutdown(); com_failure_guard(false); return false; }

    if (SUCCEEDED(impl->decoder_transform->GetAttributes(&impl->transform_attributes))) {
        impl->transform_attributes->SetUINT32(MF_TRANSFORM_ASYNC_UNLOCK, FALSE);
        impl->transform_attributes->SetUINT32(MFT_CONNECTED_TO_HW_STREAM, TRUE);
    }

    hr = impl->decoder_transform->ProcessMessage(MFT_MESSAGE_SET_D3D_MANAGER, reinterpret_cast<ULONG_PTR>(impl->device_manager));
    if (FAILED(hr)) { MFShutdown(); com_failure_guard(false); return false; }

    IMFMediaType* input_type{nullptr};
    MFCreateMediaType(&input_type);
    input_type->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
    input_type->SetGUID(MF_MT_SUBTYPE, sub_type);
    hr = impl->decoder_transform->SetInputType(0, input_type, 0);
    input_type->Release();
    if (FAILED(hr)) { MFShutdown(); com_failure_guard(false); return false; }

    IMFMediaType* output_type{nullptr};
    MFCreateMediaType(&output_type);
    output_type->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
    output_type->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_NV12);
    MFSetAttributeSize(output_type, MF_MT_FRAME_SIZE, surface_width, surface_height);
    hr = impl->decoder_transform->SetOutputType(0, output_type, 0);
    output_type->Release();
    if (FAILED(hr)) { MFShutdown(); com_failure_guard(false); return false; }

    impl->decoder_transform->ProcessMessage(MFT_MESSAGE_COMMAND_FLUSH, 0);
    impl->decoder_transform->ProcessMessage(MFT_MESSAGE_NOTIFY_BEGIN_STREAMING, 0);
    impl->decoder_transform->ProcessMessage(MFT_MESSAGE_NOTIFY_START_OF_STREAM, 0);

    is_initialized = true;
    return true;
}

bool QSVHardwareDecoderRing::DecodeFrameShared(std::span<const u8> bitstream_data) {
    EnsureQsvThreadQuarantined();

    if (!is_initialized || bitstream_data.empty()) return false;

    IMFSample* input_sample{nullptr};
    IMFMediaBuffer* input_buffer{nullptr};
    BYTE* write_ptr{nullptr};
    bool input_submitted = false;

    if (SUCCEEDED(MFCreateMemoryBuffer(static_cast<DWORD>(bitstream_data.size()), &input_buffer))) {
        if (SUCCEEDED(input_buffer->Lock(&write_ptr, nullptr, nullptr))) {
            std::memcpy(write_ptr, bitstream_data.data(), bitstream_data.size());
            input_buffer->SetCurrentLength(static_cast<DWORD>(bitstream_data.size()));
            input_buffer->Unlock();

            if (SUCCEEDED(MFCreateSample(&input_sample))) {
                input_sample->AddBuffer(input_buffer);
                HRESULT hr_input = impl->decoder_transform->ProcessInput(0, input_sample, 0);
                input_sample->Release();

                if (SUCCEEDED(hr_input)) {
                    input_submitted = true;
                } else if (hr_input != MF_E_NOTACCEPTING) {
                    input_buffer->Release();
                    return false;
                }
            }
        }
        input_buffer->Release();
    }

    MFT_OUTPUT_DATA_BUFFER output_buffer_wrapper{};
    DWORD status = 0;
    output_buffer_wrapper.dwStreamID = 0;

    HRESULT hr = impl->decoder_transform->ProcessOutput(0, 1, &output_buffer_wrapper, &status);

    if (hr == S_OK && output_buffer_wrapper.pSample) {
        IMFMediaBuffer* texture_buffer{nullptr};
        if (SUCCEEDED(output_buffer_wrapper.pSample->GetBufferByIndex(0, &texture_buffer))) {
            IMFDXGIBuffer* dxgi_buffer{nullptr};
            if (SUCCEEDED(texture_buffer->QueryInterface(__uuidof(IMFDXGIBuffer), reinterpret_cast<void**>(&dxgi_buffer)))) {
                ID3D11Texture2D* surface{nullptr};
                if (SUCCEEDED(dxgi_buffer->GetResource(__uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&surface)))) {

                    // Dynamic Allocator Ring caching template logic to bypass kernel handle allocation thrashes
                    if (surface != impl->last_decoded_surface) {
                        if (impl->shared_nt_handle) {
                            CloseHandle(impl->shared_nt_handle);
                            impl->shared_nt_handle = nullptr;
                        }
                        if (impl->last_decoded_surface) {
                            impl->last_decoded_surface->Release();
                        }

                        impl->last_decoded_surface = surface;

                        IDXGIResource1* dxgi_resource{nullptr};
                        if (SUCCEEDED(impl->last_decoded_surface->QueryInterface(__uuidof(IDXGIResource1), reinterpret_cast<void**>(&dxgi_resource)))) {
                            dxgi_resource->CreateSharedHandle(nullptr, DXGI_SHARED_RESOURCE_READ | DXGI_SHARED_RESOURCE_WRITE, nullptr, &impl->shared_nt_handle);
                            dxgi_resource->Release();
                        }
                    } else {
                        // Matching texture cached in slots. Resource reused seamlessly.
                        surface->Release();
                    }
                }
                dxgi_buffer->Release();
            }
            texture_buffer->Release();
        }
        output_buffer_wrapper.pSample->Release();
        if (output_buffer_wrapper.pEvents) output_buffer_wrapper.pEvents->Release();

    } else if (hr == MF_E_TRANSFORM_STREAM_CHANGE) {
        LOG_WARNING(Frontend, "Intel QSV: Stream geometric change detected inside transform.");
        IMFMediaType* changed_type{nullptr};
        if (SUCCEEDED(impl->decoder_transform->GetOutputAvailableType(0, 0, &changed_type))) {
            impl->decoder_transform->SetOutputType(0, changed_type, 0);
            changed_type->Release();
        }
        return input_submitted;
    } else if (hr == MF_E_TRANSFORM_NEED_MORE_INPUT) {
        return input_submitted;
    } else if (FAILED(hr)) {
        return false;
    }

    return impl->shared_nt_handle != nullptr;
}

void QSVHardwareDecoderRing::ReleaseContext() {
    if (!is_initialized) return;
    if (impl) {
        if (impl->decoder_transform) {
            impl->decoder_transform->ProcessMessage(MFT_MESSAGE_NOTIFY_END_OF_STREAM, 0);
            impl->decoder_transform->ProcessMessage(MFT_MESSAGE_NOTIFY_END_STREAMING, 0);
        }

        if (impl->shared_nt_handle) { CloseHandle(impl->shared_nt_handle); impl->shared_nt_handle = nullptr; }
        if (impl->last_decoded_surface) { impl->last_decoded_surface->Release(); impl->last_decoded_surface = nullptr; }
        if (impl->decoder_transform) { impl->decoder_transform->Release(); impl->decoder_transform = nullptr; }
        if (impl->transform_attributes) { impl->transform_attributes->Release(); impl->transform_attributes = nullptr; }
        if (impl->device_manager) { impl->device_manager->Release(); impl->device_manager = nullptr; }
        if (impl->intel_context) { impl->intel_context->Release(); impl->intel_context = nullptr; }
        if (impl->intel_d3d11_device) { impl->intel_d3d11_device->Release(); impl->intel_d3d11_device = nullptr; }
    }
    MFShutdown();
    if (com_initialized) {
        CoUninitialize();
        com_initialized = false;
    }
    is_initialized = false;
}

} // namespace VideoCore::Host1x
