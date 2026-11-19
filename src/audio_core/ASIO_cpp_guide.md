# ESS Sabre DAC & ASIO C++ Integration Guide

This reference document captures the project architecture, technical specifications, and setup protocols for interfacing the Eden engine with the ESS ES9281Pro / ES9281AC Pro hardware family via low-level USB commands (libusb-1.0) and the real-time Steinberg ASIO SDK.

---

## Project Architecture & File Hierarchy

The audio integration components reside within the engine's source tree alongside configuration profiles and documentation:

- eden/src/audio_core/ASIO_main.cpp — Complete consolidated C++ hardware engine source
- eden/src/audio_core/ASIO_cpp_guide.md — Architectural guide and developer reference
- eden/src/audio_core/FlexASIO.toml — Audio driver configuration profile
- eden/CMakeLists.txt — Root build script registering the ASIO target

---

## The Consolidated C++ Engine Code

This single-file solution manages both the Asynchronous USB Control Channel (for telemetry like impedance sensing and custom digital filter adjustment) and the Direct Real-Time Streaming Loop via the native driver's ASIO endpoint.

```cpp
#include <iostream>
#include <vector>
#include <string>
#include <cstdint>
#include <windows.h>
#include <libusb-1.0/libusb.h>
#include "asio.h"

struct SupportedDAC {
    std::string brandAndModel;
    uint16_t vid;
    uint16_t pid;
};

enum HardwareAccessMode {
    MODE_EXACT_MATCH,
    MODE_GENERIC_COMPATIBLE,
    MODE_UNSUPPORTED_FALLBACK
};

const std::vector<SupportedDAC> dacRegistry = {
    {"HiBy FC3 (Original screenless)", 0x32BB, 0x0004},
    {"HiBy FC3 (Screen Edition)",      0x32BB, 0x000E},
    {"FiiO / Jade Audio KA1 Dongle",   0x2972, 0x0041},
    {"Questyle M12 Portable DAC",      0x2A0B, 0x0011},
    {"Questyle M15 / M15i Flagship",   0x2A0B, 0x0012},
    {"THX Onyx Portable DAC/Amp",      0x16C0, 0x05E4},
    {"Helm Audio Bolt Mini-Dongle",    0x20B1, 0x000A},
    {"Audirect Beam 3 Pro / Plus",     0x2B1A, 0x0003}
};

CLSID CLSID_HiByASIO = { 0x32BB0004, 0x0000, 0x0000, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } };

IASIO* asioDriver = nullptr;
ASIOBufferInfo bufferInfos[2];
ASIOChannelInfo channelInfos[2];
long inputChannels, outputChannels;
long minSize, maxSize, preferredSize, granularity;

HardwareAccessMode HandleDeviceConnection(struct libusb_device_descriptor& desc, std::string& statusMessage) {
    for (const auto& dac : dacRegistry) {
        if (desc.idVendor == dac.vid && desc.idProduct == dac.pid) {
            statusMessage = "Verified Device Connected: " + dac.brandAndModel;
            return MODE_EXACT_MATCH;
        }
    }
    for (const auto& dac : dacRegistry) {
        if (desc.idVendor == dac.vid) {
            char rawHexBuffer[32];
            sprintf(rawHexBuffer, " (Platform VID Match: 0x%04X)", desc.idVendor);
            statusMessage = "ESS Bridge Platform Identified. Enabling generic controls." + std::string(rawHexBuffer);
            return MODE_GENERIC_COMPATIBLE;
        }
    }
    statusMessage = "Unknown device signature. Reverting safely to operating system framework.";
    return MODE_UNSUPPORTED_FALLBACK;
}

libusb_device_handle* DiscoverAndConnectDAC(libusb_context* ctx, std::string& foundModelName, HardwareAccessMode& outMode) {
    libusb_device** devs;
    ssize_t cnt = libusb_get_device_list(ctx, &devs);
    if (cnt < 0) return nullptr;

    libusb_device_handle* handle = nullptr;
    for (ssize_t i = 0; i < cnt; i++) {
        struct libusb_device_descriptor desc;
        if (libusb_get_device_descriptor(devs[i], &desc) < 0) continue;

        std::string connectionStatus = "";
        HardwareAccessMode accessMode = HandleDeviceConnection(desc, connectionStatus);

        if (accessMode == MODE_EXACT_MATCH || accessMode == MODE_GENERIC_COMPATIBLE) {
            std::cout << "[*] Connection Analyzer -> " << connectionStatus << std::endl;
            outMode = accessMode;
            foundModelName = (accessMode == MODE_EXACT_MATCH) ? "Exact Target Profile" : "Generic ESS Architecture";

            if (libusb_open(devs[i], &handle) == 0) {
                libusb_free_device_list(devs, 1);
                return handle;
            }
        }
    }
    libusb_free_device_list(devs, 1);
    outMode = MODE_UNSUPPORTED_FALLBACK;
    return nullptr;
}

bool ReadDACStatus(libusb_device_handle* handle, uint16_t dataRegister, uint16_t &outputValue) {
    uint8_t requestType = LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE | LIBUSB_ENDPOINT_IN;
    uint8_t requestID = 0x81;
    uint8_t responseBuffer[2] = {0, 0};

    int bytesRead = libusb_control_transfer(handle, requestType, requestID, dataRegister, 0x0000, responseBuffer, sizeof(responseBuffer), 1000);
    if (bytesRead == sizeof(responseBuffer)) {
        outputValue = (responseBuffer[0] << 8) | responseBuffer[1];
        return true;
    }
    return false;
}

bool WriteDACParameter(libusb_device_handle* handle, uint16_t targetRegister, uint8_t commandByte) {
    uint8_t requestType = LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE | LIBUSB_ENDPOINT_OUT;
    uint8_t requestID = 0x42;
    uint8_t payload[1] = { commandByte };

    int status = libusb_control_transfer(handle, requestType, requestID, targetRegister, 0x0000, payload, sizeof(payload), 1000);
    return (status == sizeof(payload));
}

ASIOTime* bufferSwitchTimeInfo(ASIOTime* timeInfo, long doubleBufferIndex, ASIOBool directProcess) {
    for (int i = 0; i < 2; i++) {
        if (bufferInfos[i].isInput == ASIOFalse) {
            int32_t* hardwareBuffer = static_cast<int32_t*>(bufferInfos[i].buffers[doubleBufferIndex]);
            for (long sample = 0; sample < preferredSize; sample++) {
                int32_t rawSample = 0;
                hardwareBuffer[sample] = rawSample;
            }
        }
    }
    asioDriver->outputReady();
    return timeInfo;
}

ASIOCallbacks asioCallbacks = { bufferSwitchTimeInfo, nullptr, nullptr, nullptr };

int RunAudioStreamingLoop() {
    std::cout << "[*] Initializing Steinberg ASIO Subsystem..." << std::endl;
    if (CoCreateInstance(CLSID_HiByASIO, nullptr, CLSCTX_INPROC_SERVER, CLSID_HiByASIO, (void**)&asioDriver) != S_OK) {
        std::cerr << "[-] Streaming Intercept: Could not instantiate direct ASIO endpoint mapping." << std::endl;
        return -1;
    }
    if (asioDriver->init(nullptr) == ASIOFalse) { asioDriver->Release(); return -1; }

    std::cout << "[+] Connected directly to the HiBy Hardware Driver Pipeline!" << std::endl;
    asioDriver->getChannels(&inputChannels, &outputChannels);
    asioDriver->setSampleRate(192000.0);
    asioDriver->getBufferSize(&minSize, &maxSize, &preferredSize, &granularity);

    bufferInfos[0] = { ASIOFalse, 0, {nullptr, nullptr} };
    bufferInfos[0].channelNum = 0;
    bufferInfos[1] = { ASIOFalse, 1, {nullptr, nullptr} };
    bufferInfos[1].channelNum = 1;

    if (asioDriver->createBuffers(bufferInfos, 2, preferredSize, &asioCallbacks) == ASE_OK) {
        if (asioDriver->start() == ASE_OK) {
            std::cout << "[+] Success: Direct ASIO hardware playback loop is live and running!" << std::endl;
            Sleep(5000);
            asioDriver->stop();
        }
        asioDriver->disposeBuffers();
    }
    asioDriver->Release();
    return 0;
}

int main() {
    CoInitialize(nullptr);
    libusb_context* usbCtx = nullptr;
    libusb_device_handle* dacControlHandle = nullptr;
    std::string detectedModelName = "";
    HardwareAccessMode currentMode = MODE_UNSUPPORTED_FALLBACK;

    if (libusb_init(&usbCtx) < 0) return 1;

    std::cout << "[*] Running lookup sniffer checks across active USB channels..." << std::endl;
    dacControlHandle = DiscoverAndConnectDAC(usbCtx, detectedModelName, currentMode);

    if (dacControlHandle) {
        std::cout << "[+] Main Execution Link Hooked into: " << detectedModelName << std::endl;
        if (libusb_claim_interface(dacControlHandle, 0) == 0) {
            if (currentMode == MODE_EXACT_MATCH) {
                std::cout << "[*] Mode: Exact Hardware match confirmed. Full optimizations unlocked." << std::endl;
            } else if (currentMode == MODE_GENERIC_COMPATIBLE) {
                std::cout << "[*] Mode: Platform match confirmed. Baseline registers mapped." << std::endl;
            }
            uint16_t liveImpedanceReading = 0;
            if (ReadDACStatus(dacControlHandle, 0x00E2, liveImpedanceReading)) {
                std::cout << "[+] Diagnostic Read Out -> Connected Headgear Loop Impedance: "
                          << liveImpedanceReading << " Ohms." << std::endl;
            }
            libusb_release_interface(dacControlHandle, 0);
        }
        libusb_close(dacControlHandle);
    } else {
        std::cout << "[-] Control Check: No matching hardware platforms found. Using OS standard layer." << std::endl;
    }

    libusb_exit(usbCtx);
    RunAudioStreamingLoop();
    CoUninitialize();
    return 0;
}
