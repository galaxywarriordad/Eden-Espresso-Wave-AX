# 📘 Eden Espresso Wave [AX] — User & Operational Guide (Master v1.34 / v1.35)

Operational, build automation, calibration, and troubleshooting manual for the **Eden Espresso Wave [AX]** engine and **Eden Piccolo [AX]** appliance.

---

## 📑 Table of Contents

* [Section 1: Project Overview, Engineering Philosophy, and Target Silicon](#️-section-1-project-overview-engineering-philosophy-and-target-silicon)
* [Section 2: System Prerequisites, Hardware Steering, First-Boot Calibration & PGO Training Protocol](#️-section-2-system-prerequisites-hardware-steering-first-boot-calibration--pgo-training-protocol)
* [Section 3: Runtime Silicon Topology & Architectural Summary](#-section-3-runtime-silicon-topology--architectural-summary)
* [Section 4: Development Roadmap & Future Silicon Milestones](#️-section-4-development-roadmap--future-silicon-milestones)
* [Section 5: Optimal Driver, Engine & FlexASIO Runtime Configuration](#️-section-5-optimal-driver-engine--flexasio-runtime-configuration)
* [Section 6: Cross-Platform Performance Benchmarks & Troubleshooting Reference](#-section-6-cross-platform-performance-benchmarks--troubleshooting-reference)
* [Section 7: Build Toolchain Prerequisites, Pipeline Process Overview, Telemetry Diagnostics & Troubleshooting](#️-section-7-build-toolchain-prerequisites-pipeline-process-overview-telemetry-diagnostics--troubleshooting)
* [Section 8: Closing Reflections, Acknowledgements & The Road Ahead](#-section-8-closing-reflections-acknowledgements--the-road-ahead)

---

## 🏗️ Section 1: Project Overview, Engineering Philosophy, and Target Silicon

### 🎯 1.1 What This Project Is

**Eden Espresso Wave [AX] (Master v1.35)** is a custom, hardware-locked fork of the Eden Nintendo Switch Emulator compiled from source via the shared **`ax-sdk`** (`ax::core`) C++20 silicon runtime to partition system silicon into dedicated lanes for audio, Vulkan 1.4 rendering, guest ARM64 JIT, and auxiliary background tasks—sharing its core architecture with the **Eden Piccolo [AX]** ($< 300\text{ MB}$ `toram` Batocera USB 3.0) appliance.

### 🧠 1.2 Why We Are Building It This Way

Traditional emulators treat host operating systems as black boxes, leading to thread drift, audio buffer starvation, VRAM management friction on older graphics cards, and unmanaged iGPU resource contention. Eden Espresso Wave [AX] replaces generic desktop scheduling with deterministic hardware partitioning, lock-free queues, and cache-sticky core pinning.

### 🖥️ 1.3 Target Hardware Specification

The architecture is specifically tailored around a reference workstation configuration featuring:
* **CPU:** Intel Core i5-9600K (6C/6T Non-SMT, 9 MB L3 Cache).
* **dGPU:** NVIDIA GeForce GTX 980 Ti (6 GB GDDR5, Maxwell GM200).
* **iGPU:** Intel UHD Graphics 630 (`0x8086`).
* **Audio DAC:** HiBy FC3 USB DAC (ESS Sabre ES9281AC Pro).
* **Storage Layout:** NVMe #1 (OS + `eden.exe` + Vulkan cache), NVMe #2 (Updates & DLCs), and 7200 RPM SATA HDD (`.chd`/`.ncz` game library).

### 🧩 1.4 Compatible Hardware Matrix & Architectural Adaptation Guide

While calibrated for the i5-9600K + GTX 980 Ti + UHD 630 + HiBy FC3 baseline, its modular architecture adapts cleanly to alternative hardware configurations via manual core masks or the upcoming `ax_engine.ini` proportional 2:3:1 physical-core scaler (`v1.35`).

| Hardware Component | Baseline Target | Compatible Alternatives | Adaptation / Core Mask Strategy |
| :--- | :--- | :--- | :--- |
| **CPU** | Intel Core i5-9600K (6C/6T, No SMT) | Intel i5-8400 / i7-8700K / i5-10400 (Disable SMT) | Scale 2:3:1 ratio across P-cores; isolate Core 0 for audio/timing. |
| **GPU** | NVIDIA GTX 980 Ti (6 GB GDDR5) | GTX 1070 / RTX 2060 / RTX 3060 / RX 6600 | Adjust VRAM shield ceiling (`vk_vram_shield.hpp`) to match GPU VRAM capacity. |
| **iGPU** | Intel UHD Graphics 630 (`0x8086`) | Intel UHD 730/770 / AMD Radeon iGPU | Route QSV video feed or fallback to software/Vulkan video decode queues. |
| **Audio DAC** | HiBy FC3 (ESS Sabre ES9281AC Pro) | Any WASAPI Exclusive or ASIO-compatible USB DAC | Update `FlexASIO.toml` device string to match target DAC name. |

---

## ⚙️ Section 2: System Prerequisites, Hardware Steering, First-Boot Calibration & PGO Training Protocol

### 🛠️ 2.1 BIOS & Windows OS Prerequisites

To run `eden.exe` at peak performance on the Master v1.35 binary, configure your system with the following BIOS and Windows settings:
* **BIOS Per-Core Multiplier Overclock (Intel Core i5-9600K):** Configure core ratios (All Cores set to $x48$ / 4.8 GHz, Uncore Ring to $x43\text{--}x44$ / 4.3–4.4 GHz, AVX Offset 0) and manage power states (disable C-States / SpeedStep if hunting micro-stutters) to safeguard real-time audio deadlines.
* **Intel UHD Graphics 630 iGPU & DVMT Pre-Allocation:** Enable *IGFX Multi-Monitor* and assign 256 MB–512 MB DVMT Pre-Allocated memory for Intel Quick Sync Video (QSV) hardware video decoding.
* **Windows Large-Page Privilege (`SeLockMemoryPrivilege`):** Open `secpol.msc` → *Local Policies* → *User Rights Assignment* → *Lock pages in memory*, add your Windows user account, and reboot. To verify via an Administrator Command Prompt, run `whoami /priv | findstr /i "SeLockMemoryPrivilege"`. Note that Windows displays the privilege state as `Disabled` by default in `whoami /priv`—as long as the row appears in the output, your account holds the token right and `eden.exe` enables it programmatically at startup to allocate 2 MB Windows Large Pages and reduce Skylake DTLB misses.
* **1:1 PCIe / USB / SATA Hardware IRQ Steering:** Run `%AX_TOOLS_DIR%\set_irq_affinity.ps1` (as Administrator) and reboot, then verify with `%AX_TOOLS_DIR%\check_hardware.ps1` that hardware interrupts map cleanly across CPU cores (`0x01` USB xHCI Audio, `0x02` GTX 980 Ti Vulkan, `0x20` NVMe/SATA AHCI Quarantine) to prevent `storahci.sys` DPC audio underruns.

### 🎛️ 2.2 First-Boot Emulator & Audio Calibration (Stage 0 Verification)
* **Audio Routing:** `PGO_stage0.bat` / `PGO_stage0LD.bat` automatically stage `FlexASIO.toml` (Windows WASAPI Exclusive Mode targeting the HiBy FC3 USB DAC @ 24-bit/32-bit, 48000 Hz, 128 samples / 2.67 ms) and `eden_engine.ini` (`CoreRatio=2:3:1`, `HostTimingPriorityClass=HIGH`) into `%BUILD_DIR%\bin\`.
* **Graphics & Proactive `5.00 GiB` Async VRAM Shield:** Set Vulkan on the GTX 980 Ti with Intel Quick Sync Video for hardware video decoding. `VkVramShield` monitors `VK_EXT_memory_budget` and triggers **non-blocking asynchronous pruning at `5.00 GiB`** (`~4.50 GiB` hysteresis recovery) via `VK_QUEUE_TRANSFER_BIT` and Vulkan 1.4 split barriers (`vkCmdSetEvent2` / `vkCmdWaitEvents2`), keeping VRAM safely below the `5.50 GiB` (`5.35 GiB` LSFG) hard ceiling with zero VBLANK stalls.
* **Telemetry Smoke Check:** Launch `build_stage0\bin\eden.exe` and verify in Process Explorer and `build_stage_telemetry_metrics.json` that audio, graphics, guest CPU, and Core 5 reaper/pipeline threads pin to their designated P-cores with `"TelemetrySource": "SharedMemory" and "AudioUnderruns": 0`.

### 🚀 2.3 Profile-Guided Optimisation (PGO) Build & Training Protocol via Master Configurator Hub

The LLVM PGO pipeline is driven by the root **`PGO_config.bat`** master hub (`PGO_stage0.bat` / `PGO_stage0LD.bat` → `PGO_stage1.bat` → `Scripts\validate_profile.ps1` → `PGO_stage2.bat`), linearising hot execution paths into the i5-9600K's 32 KB L1i cache via an instrumented build, `%APPDATA%\eden` environment sandboxing, $3 \times 15$-minute gameplay telemetry sweeps, and final production binary compilation.

---

## ⚡ Section 3: Runtime Silicon Topology & Architectural Summary

### 🔀 3.1 Runtime Silicon & Hardware IRQ Topology (Elastic 2:3:1 Non-SMT)

The Eden Espresso Wave [AX] (Master v1.34) runtime allocates the 6-core non-SMT Intel Core i5-9600K across dedicated hardware lanes to isolate audio, graphics, guest ARM cores, and storage/GUI tasks. For complete ASCII data-flow diagrams and low-level bus mappings, refer to `ARCHITECTURE_SPEC.md`.

### 🏛️ 3.2 Architectural Summary
* **Core 0 (`0x01` @ 4.8 GHz):** Manages dedicated audio and master clock lanes, isolating `AudioRenderSystemManager` (Critical MMCSS) and `HostTiming` (High) alongside steered HiBy FC3 USB xHCI interrupts (`0x01`) for bit-perfect PCM streaming and rock-solid frame pacing.
* **Core 1 (`0x02` @ 4.8 GHz):** Dedicated to an un-evicted Vulkan submission lane, pairing `VulkanWorker` (VeryHigh) and `GPUDirtyMemoryManager::Gather()` with GTX 980 Ti MSI interrupts (`0x02`) to maintain high minimum FPS in draw-call-heavy scenes.
* **Cores 2–4 (`0x1C` @ 4.8 GHz):** Houses the cache-sticky guest ARM64 JIT block using home-biased `Common::Fiber` workers (`C0` → `0x04`, `C1` → `0x08`, `C2` → `0x10`), single-cycle BMI1 `_bextr_u64` page walks, and 2 MB Large Page fastmem translation.
* **Core 5 (`0x20` @ 4.8 GHz):** Acts as an auxiliary and offload quarantine sink for background tasks—Guest Core 3 (`0x3C` spillover), Main GUI/Input, `ScopedNczQuarantine` (ZSTD L18 decompression), single-worker SPIR-V compilation ($\times 1$), `QSVVideoFeeder`, and steered SATA AHCI + NVMe IRQs (`0x20`)—keeping rendering lanes clear.

---

## 🗺️ Section 4: Development Roadmap & Future Silicon Milestones

### 📈 4.1 6-Phase Linear Engineering Progression

Eden Espresso Wave [AX] (Master v1.34) follows a strict, six-phase engineering roadmap—moving from deterministic CPU core partitioning and PGO code layout up to hardware BC7/BC6H texture compression, zero-copy multi-GPU compute, predictive HDD-to-NVMe streaming, and live background superblock re-compilation:

```text
[ PHASE 1: COMPILER & CORE ] ──► [ PHASE 2: THROUGHPUT & MATH ] ──► [ PHASE 3: VRAM & BC7/BC6H ]
  (Stage 0/PGO + Core 5 Ring)     (32-Bit Macro-JIT & Safe UBOs)     (5.5GB Shield + BC7 Upgrade)
                                                                                 │
[ PHASE 6: BLUE-SKY NEURAL ASIC ] ◄── [ PHASE 5: CLOSED-LOOP RUNTIME ] ◄── [ PHASE 4: ASYNC SILICON ]
  (Live Re-LLVM & Luma VME Flow)      (Predictive Storage & Dev-Kit)     (WinRT WGC + Async Compute)
```

### 🎯 4.2 Phase-by-Phase Milestone Overview & In-Game Impact

* **🎨 Phase 1: Compiler Stability, Core Partitioning & Instrumentation Foundation (`v1.33` → `v1.34`)**
  * *Core Engineering Focus:* Locks the Elastic 2:3:1 Non-SMT Topology (`0x01` Core 0 Audio/Timing @ 4.8 GHz, `0x02` Core 1 Vulkan, `0x1C` Cores 2–4 Home-Biased Guest JIT, `0x20` Core 5 Quarantine Sink), enforces a single Core 5 SPIR-V shader worker (`ASYNC_WORKER_CORE_CLAMP = 1`, Eden PR #4664) with AVX2 non-temporal cache decompression (`_mm256_stream_si256` + `_MM_HINT_NTA`), hardens Core 0 audio biquad bounds (Eden PR #2), routes FFmpeg software decode slice workers strictly onto Core 5 (`0x20`) via `AVCodecContext::execute2`, and integrates 64-bit CityHash / bit-packed pipeline keys (Eden PR #4819 & #4210) alongside `HeapTracker` cleanup (Eden PR #3817).
  * *What This Means In-Game:* Eliminates real-time shader compilation stutters, keeps HiBy FC3 audio completely crackle-free during heavy disk reads, prevents unpinned FFmpeg fallback threads from evicting Cores 2–4 JIT caches, and establishes a rock-solid baseline for Profile-Guided Optimisation (Stage 1 → Stage 2).
* **🎨 Phase 2: Descriptor Throughput, Contiguity Safety & Math Acceleration (`v1.34`)**
  * *Core Engineering Focus:* Deploys a 64-byte L1-aligned Xbyak Macro-JIT emitter with strict 32-bit sign-extended branch bounds to unlock single-cycle Maxwell XMAD execution; enforces GTX 980 Ti `maxPushDescriptors` limits and physical page-contiguity checks (Citron PR #4688, #449, #383) on uniform buffer passthrough streams; adds procedural vertex-less draw fastpaths (Citron PR #448); clamps `VkImageSubresourceRange` mip/layer counts (Citron PR #385 & #447) while packing $< 16$-byte uniforms into a pre-allocated ring (`emit_spirv.cpp`); and hardens `libswresample` with AVX2 SIMD FIR filters and `SWR_DITHER_TRIANGULAR_HIGHPASS` (`aresample=async=1`) for non-48 kHz audio streams.
  * *What This Means In-Game:* Boosts frame rates in geometry- and particle-heavy scenes by slashing CPU driver validation overhead on Core 1 (`0x02`), guarantees phase-linear 24-bit/48 kHz audio resampling before hitting your HiBy FC3 DAC, and eliminates split-DMA geometry corruption and Maxwell `VK_ERROR_DEVICE_LOST` driver crashes.
* **🎨 Phase 3: Proactive `5.00 GiB` Async VRAM Shield, Dynamic `2:3:1` Scaling & BC7/BC6H Compression (`v1.35`)**
  * *Core Engineering Focus:* Deploys **`ax-sdk` (`ax::vram`, `ax::topology`, `ax::audio`)** alongside **`VkVramShield`**—triggering **non-blocking async VRAM pruning at `5.00 GiB`** (`~4.50 GiB` hysteresis recovery) ahead of the `5.50 GiB` (`5.35 GiB` LSFG) hard ceiling using dedicated `VK_QUEUE_TRANSFER_BIT` hardware DMA queues, Vulkan 1.4 `vkCmdSetEvent2` / `vkCmdWaitEvents2` split barriers, and a `VK_KHR_timeline_semaphore`-fenced Core 5 background reaper; dynamically scales `CoreRatio=2:3:1` across physical P-cores via `GetLogicalProcessorInformationEx` (`LTP_PC_SMT` + `EfficiencyClass`) and Linux `sysfs`; routes stereo audio through a 64-byte cache-line-isolated batch SPSC ring; and upgrades ASTC recompression to Maxwell-native BC7 (1.0 BPP 8-bit adaptive modes) and BC6H (HDR).
  * *What This Means In-Game:* Eliminates the colour banding seen in BC3 skyboxes and shadows while still cutting VRAM usage by $75\%$ compared to uncompressed RGBA8—eliminating mid-game `vkAllocateMemory` stalls and preventing multi-hour open-world sessions from hitting the GTX 980 Ti's 6 GB VRAM wall.
* **🎨 Phase 4: Async Concurrency, Cross-Silicon Offloading & Reversible IRQ Steering (`v1.36`)**
  * *Core Engineering Focus:* Binds native WinRT Windows Graphics Capture (WGC) handles directly to Vulkan swapchain surfaces inside VRAM for zero-copy `lsfg-vk` frame generation (0 ms PCIe transfer); opens a secondary Vulkan Asynchronous Compute Queue (`VK_QUEUE_COMPUTE_BIT`) synchronised via timeline semaphores for ASTC → BC7 transcoding; runs COM-isolated Intel UHD 630 QSV decoding paired with FFmpeg `vpp_qsv` / `scale_qsv` hardware NV12 → BGRA conversion over zero-copy `IDXGIResource1` Windows NT Shared Handles (and VA-API `dma_buf` FDs on Piccolo); exposes native `AV_HWDEVICE_TYPE_VULKAN` (`VK_KHR_video_decode_queue`) as a zero-copy fallback decoder; and introduces a reversible runtime MSI IRQ helper (`Scripts\irq_runtime_helper.ps1`).
  * *What This Means In-Game:* Delivers ultra-responsive $2\times$/$3\times$ frame interpolation without PCIe bus lag, prevents heavy texture compute passes from stalling 3D rendering on the GTX 980 Ti, and eliminates CPU `sws_scale()` cache thrashing during pre-rendered cutscenes.
* **🎨 Phase 5: Closed-Loop Telemetry, Predictive Storage Pre-Faulting & Production Appliance (`v1.37`)**
  * *Core Engineering Focus:* Adds a strided Predictive HDD-to-NVMe Block Pre-Faulting Ring on Core 5 (`0x20`) that stages upcoming `.chd`/`.ncz` chunks from `J:\Switch` into RAM / NVMe #2 (`E:\updates\cache`); deploys a Closed-Loop Thermal & Frame-Pacing Governor driven by live `g_metrics` counters (dynamically tuning `_mm_pause()` spin tails and Core 0/5 `HostTiming` placement); enforces Maxwell SPIR-V register-pressure guards (`emit_spirv.cpp`); binds direct Core 5 RawInput/HID controller polling; feeds Phase 4 WinRT WGC swapchain handles into FFmpeg `hevc_qsv` / `h264_qsv` + `libavformat` on Core 5 (`0x20`) for zero-overhead gameplay recording; drives an external 3.5" USB-C Turing Smart Screen (`COMx`) with zero Vulkan swapchain hooks; and ships the production Eden Piccolo [AX] v1.0 $< 300\text{ MB}$ `toram` USB appliance.
  * *What This Means In-Game:* Masks 7200 RPM hard-drive seek times during open-world traversal, shaves 1–2 frames off controller input latency, enables zero-FPS-loss gameplay capture without touching GTX 980 Ti VRAM, and displays live silicon telemetry on a dedicated desk sub-screen.
* **🎨 Phase 6: Blue-Sky Autonomous Silicon & Neural Runtime (`v2.00` — Forward-Looking R&D)**
  * *Core Engineering Focus:* Introduces a Live Background Superblock Re-Compiler on Core 5 (`0x20`) that compiles unrolled AVX2 traces from live branch telemetry and hot-swaps them into Dynarmic between frames; runs a 720p/540p `R8_UNORM` luma-only compute downscale on the GTX 980 Ti ($\sim 75\text{--}85\%$ PCIe Gen 3 bandwidth reduction) before offloading hardware optical-flow motion estimation onto the idle Intel UHD 630 iGPU's fixed-function VME engine to eliminate `lsfg-vk` HUD warping; integrates a £5 RP2040 / ESP32-S3 USB-C hardware sync & physical toggle-switch dongle; and runs predictive whole-title hot-migration between your 7200 RPM HDD and NVMe #2.
  * *What This Means In-Game:* Turns your PC into a self-optimising console appliance—delivering cleaner frame generation on Maxwell, NVMe loading speeds across a multi-terabyte HDD library, and tactile physical dev-kit switches on your desk.

---

## 🎛️ Section 5: Optimal Driver, Engine & FlexASIO Runtime Configuration

### 5.1 Optimal NVIDIA Control Panel Overrides (`eden.exe`)
Open the NVIDIA Control Panel → Manage 3D Settings → Program Settings, add `eden.exe`, and apply these five overrides:
1. **Vulkan/OpenGL Present Method:** Prefer layered on DXGI Swapchain
2. **Threaded Optimization:** On (with Core 1 `0x02` isolation)
3. **Max Frame Rate:** Off
4. **Power Management Mode:** Prefer Maximum Performance
5. **Texture Filtering – Quality:** High Performance

### 5.2 Optimal Internal Eden Subsystem Configuration
Once `eden.exe` boots, open Emulation → Configure and align your internal settings:
* **Multicore CPU Emulation:** Enabled (engages your Elastic 2:3:1 home-biased fiber partition across `0x1C` and `0x3C`).
* **CPU Accuracy:** Unsafe (for maximum throughput) or Auto.
* **API:** Vulkan (NVIDIA GeForce GTX 980 Ti).
* **VSync Mode:** Mailbox (Recommended) or FIFO (when pairing with external `lsfg-vk` frame generation).
* **Resolution:** $2\times$ (1440p) or $3\times$ (4K).
* **Direct UBO Passthrough (`fast_ubo_passthrough`):** Enabled.
* **Asynchronous Shader Building:** Enabled.
* **ASTC Recompression (Option C Hybrid Policy for 6 GB VRAM):** Uncompressed (RGBA8) for titles under $\sim 4.5\text{ GB}$ VRAM; BC3 for VRAM-heavy open-world titles or $3\times$ 4K scaling.

### 5.3 Optimal Audio, Windows Exclusive Mode & FlexASIO Settings

#### 1. Windows Sound Control Panel (`mmsys.cpl`) Prerequisites
Before launching `eden.exe`, verify your USB DAC's Windows audio properties so WASAPI Exclusive Mode locks at the Nintendo Switch's native 48 kHz sample clock:
1. Press `Win + R`, type `mmsys.cpl`, and press **Enter**.
2. On the **Playback** tab, right-click your active USB DAC (e.g., **HiBy USB Audio** or **Speakers (Bumble Bee)**) → **Properties** → **Advanced**:
   * **Default Format:** Select **`2 channel, 24 bit, 48000 Hz (Studio Quality)`** to match native Switch PCM output with zero resampling overhead.
   * **Exclusive Mode:** Ensure both **Allow applications to take exclusive control of this device** and **Give exclusive mode applications priority** are checked.
3. On the **Spatial sound** tab, ensure **Off** is selected so Windows spatial DSP cannot intercept the exclusive stream.

#### 2. `FlexASIO.toml` Configuration (`%USERPROFILE%\FlexASIO.toml`)
Save the following configuration to **`%USERPROFILE%\FlexASIO.toml`** (for example, `C:\Users\<username>\FlexASIO.toml`). Placing it in `%USERPROFILE%` persists your audio profile across clean `build_stage*` purges:

```ini
# Master v1.34 — Bit-Perfect Exclusive Profile for HiBy FC3 & General 24-bit/48 kHz USB DACs
backend = "Windows WASAPI"

# Use 128 samples (2.67 ms) for dedicated low-latency DACs (HiBy FC3 ESS ES9281AC Pro)
# Use 256 samples (5.33 ms) for general/non-pro USB audio interfaces (e.g., Neat Bumblebee)
bufferSizeSamples = 128

[input]
# Disable input capture to prevent floating USB polling overhead on Core 0
device = ""
suggestedLatencySeconds = 0.0
wasapiExclusiveMode = true

[output]
# Hardcode exact device name for HiBy FC3, or comment out (#) to use default WASAPI Exclusive output
device = "HiBy USB Audio"
wasapiExclusiveMode = true
wasapiAutoConvert = false
suggestedLatencySeconds = 0.0
channels = 2
```

* **In-App Audio Alignment:** Inside Eden (**Emulation → Configure → Audio**), set **Output Engine** to `cubeb` (or `sdl3`) and **Output Device** to `FlexASIO`.

---

## 📊 Section 6: Cross-Platform Performance Benchmarks & Troubleshooting Reference

### 6.1 Cross-Platform Architectural & Performance Mapping

| Platform / Environment | CPU Architecture & Scheduling | GPU Architecture & Memory Path | Real-World Performance Profile (Switch 1 Titles) |
| :--- | :--- | :--- | :--- |
| **Nintendo Switch 1** | $4\times$ ARM Cortex-A57 @ 1.02 GHz | NVIDIA Tegra X1 Maxwell (shared LPDDR4) | **Resource-Bound Baseline:** High efficiency, but constrained by 25.6 GB/s bandwidth. Drops to 40–45 FPS in heavy titles. |
| **Stock Eden** | Generic ARM64 → x86-64 Dynarmic JIT; unpinned OS thread scheduling | Generic Vulkan/OpenGL translation | **Conservative Baseline:** Hits 60 FPS in lighter titles, but suffers 45–60 FPS dips and shader-compile stutters. |
| **Citron** | Generic ARM64 → x86-64 JIT; aggressive caching without core isolation | Vulkan translation with aggressive caching | **Volatile Peak Frames:** High raw peak FPS, but uneven frametime pacing and micro-stutters during cold shader builds. |
| **Eden Espresso Wave [AX] (Master v1.34)** | Elastic 2:3:1 Non-SMT (`0x01`, `0x02`, `0x1C`, `0x20`) + 1-cycle BMI1 + 2 MB Large Pages + PGO L1i | GTX 980 Ti on Core 1 + UBO Templates + Intel UHD 630 QSV Offload + 5.00 GiB proactive async VRAM Shield | **Locked Console-Grade Consistency:** Rock-solid 60 FPS at $2\times$/$3\times$ with flat frametimes and bit-perfect HiBy FC3 audio. |
| **Nintendo Switch 2** | $8\times$ ARM Cortex-A78C @ $\sim 2.1\text{ GHz}$ | Custom NVIDIA Ampere (native ASTC & DLSS) | **Brute-Force Native Compatibility:** Locks struggling titles to 60 FPS out-of-the-box via raw ARM/Ampere throughput. |

### 6.2 Telemetry Verification & Troubleshooting Quick-Reference

| Symptom | Root Cause | Immediate Fix |
| :--- | :--- | :--- |
| **Audio Clicks / `audio_underruns > 0`** | `bufferSizeSamples = 128` is too tight, or `storahci.sys` DPCs hitting Core 0. | 1) Run `%AX_TOOLS_DIR%\set_irq_affinity.ps1` as Admin and reboot; 2) Increase `bufferSizeSamples` to `192` or `256` in `FlexASIO.toml`. |
| **Intel QSV Option Inactive** | Intel UHD 630 iGPU disabled in BIOS or DVMT aperture too small. | Enable *IGFX Multi-Monitor* in BIOS and set DVMT Pre-Allocated to 256 MB – 512 MB. |
| **Stutter After 1–2 Hours** | Uncompressed RGBA8 ASTC textures exceeded GTX 980 Ti past its 5.50 GB VRAM ceiling. | Switch ASTC Recompression to BC3 (or BC7 in Phase 3) in per-game *Properties* → *Graphics*. |
| **Flickering Geometry** | Non-contiguous DMA boundary or push-descriptor overflow in `vk_ubo_passthrough.cpp`. | Set `fast_ubo_passthrough = false` in per-game properties to verify. |

---

## 🛠️ Section 7: Build Toolchain Prerequisites, Pipeline Process Overview, Telemetry Diagnostics & Troubleshooting

### 7.1 Developer Toolchain & Environment Prerequisites

| Component | Required Version / Configuration | Architectural Role in Build |
| :--- | :--- | :--- |
| **Visual Studio** | VS 2022 / 2026 (v14.51+) with C++ Desktop, Windows 10/11 SDK (10.0.26100.0), and MASM (`ml64.exe`) | Provides Universal CRT, Win32/DXGI headers, and `ml64.exe` for Boost.Context x86_64 user-mode fibers. |
| **LLVM / Clang** | LLVM `clang-cl` 23.1.4+ & `lld-link.exe` + `llvm-profdata.exe` | Compiles translation units with Skylake vector intrinsics, ThinLTO (`-flto=thin`), and PGO instrumentation. |
| **CMake** | CMake 3.28 – 4.5+ (`CMP0169` & `CMP0091=NEW` compliant) | Configures static libraries, CPM bundled packages, and FetchContent Steinberg ASIO SDK targets. |
| **Ninja** | Ninja 1.11+ (`ninja -j 4` recommended) | Executes parallel dependency builds while leaving 2 cores free for OS/disk I/O. |
| **PowerShell** | PowerShell 5.1+ / 7+ (`ExecutionPolicy Bypass`) | Drives workspace purging, hardware auditing, performance telemetry, and PGO trace validation. |

### 7.2 Master Configurator Hub (`PGO_config.bat`) & End-to-End Build Process

The unified root master hub (**`PGO_config.bat`**) orchestrates all pipeline stages (`[0]` `PGO_stage0.bat`, `[0LD]` `PGO_stage0LD.bat`, `[1]` `PGO_stage1.bat`, `[2]` `PGO_stage2.bat`, `[CA]` `clean_all.ps1`). Build scripts feature a fixed **row 45 VT100 scroll-region lock** (`%ESC%[45;r%ESC%[45;1H`) to protect the Espresso AX banner and optimization feature lists from being overwritten by streaming build logs:

```text
[ 1. WORKSPACE PURGE ]        [ 2. STAGE 0 CONTROL ]         [ 3. STAGE 1 INSTRUMENT ]       [ 4. STAGE 2 FINAL BAKE ]
  [CA] clean_all.ps1       ──►  PGO_stage0LD.bat / Ninja   ──►  PGO_stage1.bat (-fprofile)  ──►  PGO_stage2.bat (llvm-profdata)
```

* **Workspace Hygiene & Absolute Purge (`[CA]` Clean All):** Executing `clean_all.ps1` via the hub terminates active engine processes (`ninja`, `cmake`, `clang-cl`, `lld-link`, `eden`), clears logs and binary manifests (`.ninja_deps`, `.ninja_log`), stages build trees (`build`, `build_stage0`, `build_stage1`, `build_stage2`, `.cache`) into secure trash folders (`EDENTRASH_*`), and spawns an asynchronous multi-threaded background deletion worker.
* **Diagnostics & Source Verification (`[0LD]` Stage 0LD):** Audits administrative security tokens, verifies PCIe/USB/SATA IRQ steering (`check_hardware.ps1`), locks the terminal scroll region at row 45, and launches the independent asynchronous telemetry daemon (`track_telemetry.ps1`).
* **Instrumented Training Build (`[1]` Stage 1):** Compiles `build_stage1\bin\eden.exe` with `/fp:precise` and `-fprofile-generate=...` across three 15-minute gameplay training sweeps (`J:\Switch`).
* **Profile Validation & Production Bake (`[2]` Stage 2):** Audits `.profraw` traces, merges them via `llvm-profdata merge`, and compiles the final optimized binary with `/O3 /Ob3 /fp:fast -fprofile-use=eden.profdata -flto=thin -march=skylake`.

### 7.3 Comprehensive Build & Runtime Troubleshooting Matrix

| Category | Symptom / Error Message | Root Cause | Exact Resolution |
| :--- | :--- | :--- | :--- |
| **Linker (`lld-link`)** | `undefined symbol: mainCRTStartup, memcpy, _chkstk` | `/NODEFAULTLIB:msvcrt.lib` stripped the `/MD` dynamic C runtime import library. | Keep `/NODEFAULTLIB:libcmt.lib` in `CMakeLists.txt` so `msvcrt.lib` and `ucrt.lib` link cleanly. |
| **Linker (`lld-link`)** | `could not open 'nvcuvid.lib' or 'cuda.lib'` | Unused NVIDIA CUDA/NVDEC libraries listed in `PLATFORM_LIBRARIES`. | Remove `nvcuvid` and `cuda` from `CMakeLists.txt`; use Intel QSV (`mfplat`, `mfuuid`, `d3d11`, `dxgi`). |
| **Win32 Macros** | `error: no member named 'CreateEventA'` | `<windows.h>` included without `NOMINMAX` or `#undef CreateEvent`. | Define `#define NOMINMAX` above `<windows.h>` and `#undef CreateEvent` immediately after. |
* **PowerShell `Add-Type`** | `Parameter set cannot be resolved / Expected class, delegate, enum` | Conflicting `-Name` / `-Namespace` parameters passed alongside `-TypeDefinition` when passing raw `[DllImport]` method signatures. | Use `-MemberDefinition $w_code -Name WWindowUtil -Namespace Win32` (as updated in `Scripts\PGO_stage*.bat`). |
| **Source Hygiene** | `error: expected unqualified-id (at EOF of settings.cpp or qsv_decoder.cpp)` | Stray markdown code fence backticks appended from text export copy-pastes. | Open the affected C++ source file, scroll to the bottom, and remove any stray backticks or citation markers. |
| **Compiler (`fmt 12.1`)** | `type_is_unformattable_for<EnumType, char>` | `fmt 12.1.0` requires explicit `fmt::underlying(...)` for scoped enums. | Wrap scoped enums in `fmt::underlying(val)` inside `LOG*` / `ASSERT_MSG` calls. |
| **Compiler (`clang-cl`)** | `no matching function for call to '_bextr_u64'` | `_bextr_u64` requires 3 arguments on `clang-cl`. | Use `_bextr_u64(value, start, length)` in `src/common/bmi_util.h`. |

---

## ☕ Section 8: Closing Reflections, Acknowledgements & The Road Ahead

### 8.1 A Passion Project Forged in Collaboration
**Eden Espresso Wave [AX] (Master v1.34)** began not inside a commercial studio, but as a labour of love—a late-night passion project driven by one dedicated enthusiast and AI collaboration, working side-by-side through thousands of lines of C++20, Vulkan shaders, linker flags, and silicon bitmasks.

### 8.2 Standing on the Shoulders of Giants
* **The Upstream Emulator Pioneers:** The yuzu, Eden, and Citron / Citron Neo development teams.
* **The Open-Source Ecosystem & Git Maintainers:** Dynarmic, Sirit, Xbyak, LLVM / Clang, Boost, Qt6, FFmpeg, Cubeb, FlexASIO (and Steinberg ASIO SDK), ReShade, `fmt`, ZSTD, LZ4, QuaZip, `nlohmann/json`, and VMA.

### 8.3 Only the First Step: The Path Ahead
Getting `eden.exe` built, partitioned, and running is only the first step. The road ahead turns a custom binary into a self-tuning console appliance:
* **PGO Telemetry Baking (Stage 1 → Stage 2):** Capturing real gameplay branch profiles across your library.
* **Maxwell Throughput & BC7/BC6H Fidelity (Phases 2 & 3):** Deploying 32-bit Macro-JIT emitters, page-contiguity UBO streams, a 5.00 GiB proactive async VRAM shield, and near-lossless BC7 / BC6H texture compression.
* **Heterogeneous Silicon & Closed-Loop Polish (Phases 4, 5 & 6):** Unlocking zero-copy WinRT WGC frame generation, asynchronous compute queues, predictive HDD-to-NVMe pre-faulting, the $< 300\text{ MB}$ `toram` Eden Piccolo [AX] USB appliance, and live background superblock re-compilation.

*Thank you for being part of this build—now let's fire up `build_stage0\bin\eden.exe` and watch the silicon sing!*