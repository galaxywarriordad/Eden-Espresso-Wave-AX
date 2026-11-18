## ☕ Eden Espresso Wave [AX] — Master v1.34 / v1.35 Ready

A deterministic, hardware-locked fork of the Eden Nintendo Switch Emulator engineered for Intel Coffee Lake Refresh, NVIDIA Maxwell/Pascal GPUs, Intel Quick Sync Video, and bit-perfect exclusive USB DAC audio.
Eden Espresso Wave [AX] — Elevating Nintendo Switch Emulation to Professional Hardware Parity
Overview:
Eden Espresso Wave [AX] is a high-fidelity, performance-optimized Nintendo Switch emulator engine designed from the ground up as a deterministic, hardware-conscious appliance. Rather than treating emulation as a generic desktop process, it transforms your PC into a focused, finely tuned gaming platform carefully aligned with your system's hardware topology.

# Technical Highlights:

Hardware-locked execution: Utilizes a 2:3:1 core partitioning strategy on Intel Coffee Lake CPUs, pinning critical threads like audio processing, Vulkan command submission, and background shader compilation to dedicated cores for minimal jitter and predictable timing.
Optimized Vulkan pipelines: Implements vendor-aware shader compilation guards, VRAM budgeting and protection, and advanced texture compression strategies tailored to Maxwell GPUs such as the GTX 980 Ti.
Real-time telemetry & profiling: Integrates native Vulkan telemetry overlays and asynchronous hardware performance queries, enabling on-the-fly detection and tuning of performance bottlenecks, resource usage, and runtime jitters.
Advanced audio integration: Employs FlexASIO and Steinberg ASIO drivers with lock-free ring buffers and micro-pitch PLL via low-latency exclusive hardware access, eliminating audio underruns and crackles.
Layered build pipeline: Features a rigorous multi-stage Profile-Guided Optimization (PGO) process coupled with extensive CI/CD hygiene gates and hardware validation suites, ensuring binary integrity and deterministic performance across PC configurations.
Cross-silicon offloading: Seamlessly offloads video decoding to Intel UHD graphics and plans sophisticated GPU compute for ASTC texture decompression and frame generation acceleration.

# User Focus:
Eden Espresso Wave [AX] is more than just code. It prioritizes a smooth, stutter-free gaming experience with simple, guided setup instructions. It enforces legal use of features like frame interpolation through Steam’s Lossless Scaling, ensuring a clean and ethical delivery.

# Vision:
This project pioneers the path toward a dedicated, bare-metal gaming appliance, exemplified by Eden Piccolo [AX], a fully optimized Linux USB boot environment delivering near-zero latency and consistent sub-millisecond frame pacing.
## 🔗 Official Repository Network
* **Main Development Repository:** [Eden Forgejo Server](https://git.eden-emu.dev/galaxywarriordad/Eden-Espresso-Wave-AX.git)
* **Sister Mirror Repository:** [GitHub Mirror Portal](https://github.com/galaxywarriordad/Eden-Espresso-Wave-AX.git)
* **Upstream Ecosystem Hub:** [Eden Community Website](https://git.eden-emu.dev/eden-emu/eden/)

## 📚 Documentation & Architecture Specifications
* [Unified Dual-Target Master Plan (v1.2)](UNIFIED_MASTER_PLAN.md)[cite: 2]
* [Windows Architecture Specification (v1.34)](ARCHITECTURE_SPEC.md)[cite: 2]
* [Linux Appliance Specification (v0.5)](EDEN_PICCOLO_SPEC.md)[cite: 2]
* [User & Architecture Guide (v1.34)](USER_GUIDE.md)[cite: 2]
* [Additional Technical Details (v1.34)](ADDITIONAL_DETAILS.md)[cite: 2]
* [What's Next (v1.34 / v1.35)](WhatsNext.md)[cite: 2]
* [Benchmark & Architectural Assessment](Benchmark_Assessment.md) — [View on Forgejo Server](https://git.eden-emu.dev/galaxywarriordad/Eden-Espresso-Wave-AX/src/branch/master/Benchmark_Assessment.md)

---

## 🗂️ Logical Documentation Hierarchy & Architecture Stack

The documentation suite follows a progressive disclosure model—moving from high-level project identity down to low-level silicon specifications and future engineering roadmaps:

### 📌 Tier 1: Core Navigation & Overview (`README.md`)
* **Purpose:** The immediate entry point for developers, contributors, and users.
* **Contents:** Project vision, reference silicon specifications (Intel i5-9600K + GTX 980 Ti), elastic core topology summary (2:3:1), live atomic telemetry metrics, and one-line build quickstarts.[cite: 2]

### 🚀 Tier 2: User Operations & Deployment (`USER_GUIDE.md`)
* **Purpose:** The definitive operational manual for setting up the environment, running builds, and calibrating hardware.
* **Contents:** 
  * APPDATA sandboxing and test title staging (`Desktop\PGO Train`).[cite: 2]
  * Automated build orchestrator usage (`Scripts\build_all.ps1`).[cite: 2]
  * Manual PGO training workflow (`PGO_stage1.bat` & `PGO_stage2.bat`).[cite: 2]
  * Hardware IRQ steering (`set_irq_affinity.ps1`) and FlexASIO exclusive audio calibration.[cite: 2]

### 🗺️ Tier 3: Low-Level Architecture & Silicon Specification (`ARCHITECTURE_SPEC.md`)
* **Purpose:** The comprehensive engineering reference detailing subsystem mechanics, data-flow, and hardware constraints.[cite: 2]
* **Contents:**
  * Macro data-flow topology diagrams.[cite: 2]
  * Host memory virtualization (`SeLockMemoryPrivilege`, 2 MB Large Pages, BMI1 `_bextr_u64`).[cite: 2]
  * Asynchronous CPU execution and user-mode fiber scheduling.[cite: 2]
  * Real-time audio routing (Core 0 MMCSS + FlexASIO 128-sample atomic ring buffer).[cite: 2]
  * Video, Telemetry OSD, SPIR-V font pipeline, and Maxwell GM200 safety guards.[cite: 2]
  * Consolidated upstream PR triage and 6-phase linear roadmap.[cite: 2]

### ⚙️ Tier 4: Configuration & Technical Reference (`ADDITIONAL_DETAILS.md` & `eden_engine.ini`)
* **Purpose:** Granular configuration schemas, runtime flags, and cross-compiler ABI guards.[cite: 2]
* **Contents:**
  * `eden_engine.ini` property definitions (core ratios, hardware feature flags, Windows scheduling classes).[cite: 2]
  * LLVM/Clang-cl 23 & MSVC ABI guard notes.[cite: 2]

### 🎯 Tier 5: Milestone Roadmap & Sprint Backlog (`WhatsNext.md`)
* **Purpose:** Transparent tracking of current sprint objectives and upcoming hardware gates.[cite: 2]
* **Contents:**
  * v1.34 stabilization and PGO Stage 1 execution sweeps.[cite: 2]
  * Milestone 5 Linux bare-metal / RT-kernel gate (6.12-rt + NVIDIA 580.* drivers).[cite: 2]
  * Phase 3–6 roadmap targets (BC7/BC6H texture compression, WinRT WGC capture, and predictive storage pre-faulting).[cite: 2]

### 📊 Tier 6: Comparative Engineering Assessment (`Benchmark_Assessment.md`)
* **Purpose:** Rigorous architectural benchmarking comparing Eden Espresso Wave [AX] against best-in-class emulators and professional console development kits, including our generational roadmap (`2026`–`2028+`).

---

## 🔄 Recommended Reading & Workflow Sequence

* **New Users / Gamers:** Start with `README.md` $\rightarrow$ proceed directly to `USER_GUIDE.md` for setup and FlexASIO configuration.[cite: 2]
* **Systems Engineers / Contributors:** Start with `README.md` $\rightarrow$ study `ARCHITECTURE_SPEC.md` for core topology and threading models $\rightarrow$ reference `ADDITIONAL_DETAILS.md` and `eden_engine.ini` for runtime tuning.[cite: 2]
* **Release Engineers:** Execute `Scripts\build_all.ps1` as detailed in `USER_GUIDE.md` and track progress against `WhatsNext.md`.[cite: 2]

---

## 🎯 Why This Build Exists

Compiled via LLVM `clang-cl` (`-march=skylake`, ThinLTO, and Profile-Guided Optimisation), Eden Espresso Wave [AX] replaces generic desktop thread scheduling with deterministic hardware partitioning:[cite: 2]
* **For Gamers & the Emulation Community:** Delivers zero-crackle audio, flat frametimes via isolated physical cores, dual-GPU silicon offloading on Intel UHD 630 iGPUs, gameplay-trained PGO profiles, and a native Vulkan Telemetry OSD tracking live working sets and GPU pipeline statistics.[cite: 2]
* **For Systems Engineers & Developers:** Enforces deterministic core/IRQ affinity masks, 2 MB Windows Large Pages (`SeLockMemoryPrivilege`), single-cycle Coffee Lake BMI1 page walks, 32-byte aligned AVX2 non-temporal streaming, zero-copy multi-GPU/audio interop, and strict repo hygiene workflows.[cite: 2]

---

## 🏗️ Reference Silicon & Runtime Topology (Elastic 2:3:1 Non-SMT)

Calibrated for an Intel Core i5-9600K (6C/6T Non-SMT), NVIDIA GeForce GTX 980 Ti, Intel UHD Graphics 630, and HiBy FC3 USB DAC[cite: 2]:

| Host Core(s) | Affinity Mask | Clock / Priority | Assigned Subsystems & Steered Hardware IRQs |
| :--- | :--- | :--- | :--- |
| **Core 0** | `0x01` | 4.9 GHz (`x49`) / Critical MMCSS | `AudioRenderSystemManager` (FlexASIO 128-sample / 2.67 ms ring + Cubeb failover), `HostTiming` (High), HiBy FC3 USB xHCI IRQ (`0x01`)[cite: 2] |
| **Core 1** | `0x02` | 4.8 GHz (`x48`) / VeryHigh | `VulkanWorker` command submission ring, `GPUDirtyMemoryManager::Gather()`, GTX 980 Ti MSI IRQ (`0x02`), Native Pipeline Statistics Query Pool[cite: 2] |
| **Cores 2–4** | `0x1C` | 4.8 GHz (`x48`) / High | Home-biased Guest ARM64 JIT fibers (C0→`0x04`, C1→`0x08`, C2→`0x10`), 1-cycle BMI1 `_bextr_u64` page walks, 2 MB Large Pages[cite: 2] |
| **Core 5** | `0x20` | 4.8 GHz (`x48`) / Normal–Low | Guest Core 3 (`0x3C` spill), Main GUI/Input, `ScopedNczQuarantine` (ZSTD L18), `PipelineWorkerPoolRing` (x1), `QSVVideoFeeder`, FFmpeg `execute2` decode slices, SATA AHCI + 2× NVMe IRQs (`0x20`)[cite: 2] |

---

## ⚡ Core Architecture & Silicon Offloading Capabilities

* **Native Vulkan Telemetry OSD & SPIR-V Shaders:** Real-time RAM working set and commit memory monitoring via Win32 `psapi` paired with native SPIR-V font rendering (`font.vert.spv`, `font.frag.spv`) through custom PSOs and dynamic vertex quad generation.[cite: 2]
* **Hardware-Accelerated Virtual Memory:** Single-cycle `_bextr_u64` page translation (`bmi_util.h`), Windows Large-Page fastmem (`SeLockMemoryPrivilege`), and lock-free weak-CAS dirty tracking (`GPUDirtyMemoryManager`).[cite: 2]
* **Maxwell GM200 / Pascal Protection:** Speculative 5.50 GB DXGI VRAM Shield (`vk_rasterizer.cpp`), zero-allocation descriptor passthrough (`vk_ubo_passthrough.cpp`), `maxPushDescriptors` ceiling guards, subresource range clamping, and calibrated 3D compute unswizzle tiers.[cite: 2]
* **Intel Quick Sync Video (QSV) & FFmpeg Isolation:** Offloads H.264, HEVC, VP8, and VP9 decoding onto Intel UHD Graphics 630 via Media Foundation handles (`QSVVideoFeeder` on Core 5 `0x20`), with FFmpeg software decode slices hooked into Core 5's worker pool.[cite: 2]
* **ZSTD Level-18 Block Decompression Quarantine:** Migrates ZSTD worker threads onto Core 5 (`0x20`) at `THREAD_PRIORITY_BELOW_NORMAL` alongside steered SATA AHCI (`storahci.sys`) and NVMe controller IRQs (`0x20`).[cite: 2]
* **Bit-Perfect ASIO Audio Pipeline:** Routes raw PCM blocks from Guest JIT cores to Core 0 via a cache-line-aligned atomic ring buffer (`lockfree_audio_queue.hpp`) directly into the Steinberg ASIO SDK (`asio::asio`) targeting `Speakers (HiBy FC3)`.[cite: 2]

---

## 📊 Live Atomic Telemetry Metrics (`build_stage_telemetry_metrics.json`)

Monitored in real-time by `Scripts\track_telemetry.ps1`[cite: 2]:
* `"audio_underruns": 0` — Confirms Core 0 (`0x01`) Critical MMCSS + FlexASIO 128-sample ring has zero buffer starvation.[cite: 2]
* `"voices_dropped": 0` — Confirms the ADSP microkernel never had to shed active voice channels under CPU scheduling pressure.[cite: 2]
* `"audio_processing_time_us"` — Tracks microsecond execution duration of the active audio render frame header.[cite: 2]
* `"qsv_frames_decoded": > 0` — Confirms `QSVVideoFeeder` on Core 5 (`0x20`) is actively decoding via Intel UHD 630 NT Shared Handles.[cite: 2]
* Working Set & Commit Memory Tracking — Real-time process memory telemetry displayed directly in the OSD and logging layers.[cite: 2]

---

## 🚀 Quick Build, PGO & Automated Pipeline Architecture

```cmd
:: 1. Unified Automated Build Orchestrator (Stages 0-2 & Packaging)
powershell -NoProfile -ExecutionPolicy Bypass -File "Scripts\build_all.ps1"

:: 2. Manual Stage-by-Stage Control (Alternative Workflow)
powershell -NoProfile -ExecutionPolicy Bypass -File "Scripts\clean_all.ps1" && PGO_stage0LD.bat
powershell -NoProfile -ExecutionPolicy Bypass -File "Scripts\clean_all.ps1" && PGO_stage1.bat
powershell -NoProfile -ExecutionPolicy Bypass -File "Scripts\validate_profile.ps1"
PGO_stage2.bat
```[cite: 2]
<div align="center">

## 🏛️ About The Silicon Preservation Society

The Silicon Preservation Society focuses on hardware preservation and extending the lifespan of aging desktop hardware through specialised software and toolsets, including Espresso Wave, Piccolo, and ax-sdk.
If this mission is important to you and you would like to help, please open a GitHub Discussion or Issue on our repository—we would love to hear from you!
**We refuse to throw away good hardware.**
---

## 🤝 Acknowledgements & Legal Compliance
* **Upstream Credits:** Built on the Eden Emulator alongside projects like yuzu, Citron, Dynarmic, Sirit, Xbyak, LLVM/Clang, Boost, Qt6, FFmpeg, Cubeb, FlexASIO/Steinberg ASIO SDK, ReShade, and Vulkan Memory Allocator (VMA).[cite: 2]
* **License & Notice:** Licensed under GPL-3.0-or-later (`LICENSE.txt`). Contains no proprietary Nintendo firmware, keys, games, or commercial binaries.[cite: 2]