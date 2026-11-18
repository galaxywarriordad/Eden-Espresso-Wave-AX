# ☕ Eden Espresso Wave [AX] — Master v1.34 / v1.35 Ready

A deterministic, hardware-locked fork of the Eden Nintendo Switch Emulator engineered for Intel Coffee Lake Refresh, NVIDIA Maxwell/Pascal GPUs, Intel Quick Sync Video, and bit-perfect exclusive USB DAC audio.

**Elevating Nintendo Switch Emulation to Professional Hardware Parity:** Eden Espresso Wave [AX] is a high-fidelity, performance-optimized Nintendo Switch emulator engine designed from the ground up as a deterministic, hardware-conscious appliance. Rather than treating emulation as a generic desktop process, it transforms your PC into a focused, finely tuned gaming platform carefully aligned with your system's hardware topology[cite: 29].

---

## ✨ Technical Highlights

* **Hardware-locked execution:** Utilizes a `2:3:1` core partitioning strategy on Intel Coffee Lake CPUs, pinning critical threads like audio processing, Vulkan command submission, and background shader compilation to dedicated cores for minimal jitter and predictable timing.
* **Optimized Vulkan pipelines:** Implements vendor-aware shader compilation guards, VRAM budgeting and protection, and advanced texture compression strategies tailored to Maxwell GPUs such as the GTX 980 Ti.
* **Real-time telemetry & profiling:** Integrates native Vulkan telemetry overlays and asynchronous hardware performance queries, enabling on-the-fly detection and tuning of performance bottlenecks, resource usage, and runtime jitters.
* **Advanced audio integration:** Employs FlexASIO and Steinberg ASIO drivers with lock-free ring buffers and micro-pitch PLL via low-latency exclusive hardware access, eliminating audio underruns and crackles.
* **Layered build pipeline:** Features a rigorous multi-stage Profile-Guided Optimization (PGO) process coupled with extensive CI/CD hygiene gates and hardware validation suites, ensuring binary integrity and deterministic performance across PC configurations.
* **Cross-silicon offloading:** Seamlessly offloads video decoding to Intel UHD graphics and schedules GPU compute for ASTC texture decompression and frame generation acceleration[cite: 29].

### 🎯 User Focus & Vision
* **User Focus:** Prioritizes a smooth, stutter-free gaming experience with simple, guided setup instructions, enforcing legal use of features like frame interpolation through Steam’s Lossless Scaling for a clean and ethical delivery.
* **Vision:** Pioneers the path toward a dedicated, bare-metal gaming appliance, exemplified by **Eden Piccolo [AX]**, a fully optimized `< 300 MB` `toram` Linux USB boot environment delivering near-zero latency and consistent sub-millisecond frame pacing[cite: 16, 29].

---

## 🌐 Official 4-Repository Ecosystem Topology

The **AX Project** separates shared low-level C++20 silicon abstractions from platform- and emulator-specific forks so upstream rebases stay clean while hardware isolation logic remains unified across Windows 10/11 and bare-metal Linux:

```text
                    +-------------------------------------------------------+
                    |                        ax-sdk                         |
                    |        https://github.com/galaxywarriordad/ax-sdk     |
                    |  Shared C++20 Silicon Core, Dynamic SMT/CCX Topology, |
                    |  Platform Shims, Telemetry Schemas & Master Specs     |
                    +-------------------------------------------------------+
                       /                        |                        \
       CMake FetchContent       Buildroot + CMake FetchContent       CMake FetchContent
                     /                          |                          \
                    v                           v                           v
+----------------------------------+  +----------------------------------+  +----------------------------------+
|     Eden-Espresso-Wave-AX        |  |           Piccolo-AX             |  |          Cemu-Latte-AX           |
|   (Forgejo — Upstream Eden Fork) |  |  (GitHub — Batocera USB 3.0 OS)  |  |    (GitHub — Wii U AX Fork)    |
|  Windows 10/11 Desktop Engine,   |  |  < 300 MB toram EROFS Appliance, |  |  Wii U / Latte GPU Desktop Fork  |
|  LLVM 23 PGO, FlexASIO & QSV     |  |  6.12-rt Kernel, ALSA & VA-API   |  |  Powered by ax::sdk Silicon Core |
+----------------------------------+  +----------------------------------+  +----------------------------------+
```

* **Main Switch Engine Repository (Forgejo):** [`galaxywarriordad/Eden-Espresso-Wave-AX`](https://git.eden-emu.dev/galaxywarriordad/Eden-Espresso-Wave-AX.git) — Direct upstream Eden fork for frictionless PR cherry-picks.
* **Shared Silicon SDK (GitHub):** [`galaxywarriordad/ax-sdk`](https://github.com/galaxywarriordad/ax-sdk.git) — Shared C++20 silicon core (`ax::sdk`), dynamic Intel SMT / AMD CCX resolver, platform shims, and master architecture specs.
* **Linux USB Appliance (GitHub):** [`galaxywarriordad/Piccolo-AX`](https://github.com/galaxywarriordad/Piccolo-AX.git) — Standalone `< 300 MB` `toram` EROFS Batocera Linux USB 3.0 appliance (`6.12-rt` kernel, direct ALSA `SCHED_FIFO`, and `iHD` VA-API `dma_buf`).
* **Wii U Sister Fork (GitHub):** [`galaxywarriordad/Cemu-Latte-AX`](https://github.com/galaxywarriordad/Cemu-Latte-AX.git) — Nintendo Wii U (Latte GPU) AX fork powered by `ax::sdk`.
* **Upstream Ecosystem Hub:** [`eden-emu/eden`](https://git.eden-emu.dev/eden-emu/eden/)

---

## 📚 Documentation & Architecture Specifications

### Local Engine Documentation (`Eden-Espresso-Wave-AX`)
* [User & Operational Guide (`v1.34` / `v1.35`)](USER_GUIDE.md)
* [Windows Architecture Specification (`v1.34`)](ARCHITECTURE_SPEC.md)
* [Additional Technical Details & INI Schema (`v1.34`)](ADDITIONAL_DETAILS.md)
* [Telemetry & Hardware Instrumentation (`v1.34`)](Telemetry.md)
* [Changelog (`v1.34`)](CHANGELOG.md)
* [Sprint Reports](docs/reports/)

### Shared Cross-Repo Specifications (`ax-sdk` & `Piccolo-AX`)
* [Unified Dual-Target Master Plan (`v1.3`)](https://github.com/galaxywarriordad/ax-sdk/blob/main/docs/UNIFIED_MASTER_PLAN.md)
* [Unified Multi-Architecture Silicon Topology Plan](https://github.com/galaxywarriordad/ax-sdk/blob/main/docs/SILICON_TOPOLOGY_PLAN.md)
* [Unified Subsystem Integration Plan (`v1.1`)](https://github.com/galaxywarriordad/ax-sdk/blob/main/docs/Unified_Subsystem_Integration_1.1.md)
* [Industry Benchmark & Architectural Assessment](https://github.com/galaxywarriordad/ax-sdk/blob/main/docs/Benchmark_Assessment.md)
* [Linux Appliance Specification (`Piccolo-AX`)](https://github.com/galaxywarriordad/Piccolo-AX/blob/main/docs/PICOLLO_AX_SPEC.md)

---

## 🗂️ Logical Documentation Hierarchy & Architecture Stack

The documentation suite follows a progressive disclosure model—moving from high-level project identity down to low-level silicon specifications and future engineering roadmaps:

* **Tier 1 — Core Navigation & Overview (`README.md`):** Entry point for developers, contributors, and users covering project vision, reference silicon specifications, elastic `2:3:1` topology, live atomic telemetry metrics, and build quickstarts.
* **Tier 2 — User Operations & Deployment (`USER_GUIDE.md`):** Operational manual for environment setup, `FlexASIO.toml` bit-perfect audio calibration, NVIDIA Control Panel overrides, `Scripts\set_irq_affinity.ps1` IRQ steering, and the 3-stage LLVM PGO workflow (`PGO_configurator.bat`).
* **Tier 3 — Low-Level Architecture & Silicon Specification (`ARCHITECTURE_SPEC.md`):** Engineering reference detailing macro data-flow topology diagrams, `SeLockMemoryPrivilege` 2 MB Large Pages, BMI1 `_bextr_u64` page walks, user-mode fiber scheduling, Core 0 MMCSS audio routing, and Maxwell GM200 safety guards.
* **Tier 4 — Configuration & Technical Reference (`ADDITIONAL_DETAILS.md` & `eden_engine.ini`):** Granular configuration schemas, proportional `2:3:1` scaler parameters, shader macro harmonization (`macro_update.ps1` / `shader_validate.ps1`), and LLVM `clang-cl` 23+ ABI guards.
* **Tier 5 — Shared Silicon & Cross-Platform Specs (`ax-sdk` & `Piccolo-AX`):** Cross-emulator C++20 shims, dynamic Intel SMT / AMD CCX core topology (`topology.cpp`), `< 300 MB` `toram` Batocera Buildroot overlay, and comparative industry benchmarks[cite: 13, 14, 15, 16, 21, 29].

---

## 🔄 Recommended Reading & Workflow Sequence

* **New Users / Gamers:** Start with `README.md` → proceed directly to `USER_GUIDE.md` for Windows audio (`mmsys.cpl` + `FlexASIO.toml`), BIOS, and driver setup[cite: 13, 29].
* **Systems Engineers / Contributors:** Start with `README.md` → study `ARCHITECTURE_SPEC.md` and `ax-sdk` (`UNIFIED_MASTER_PLAN.md` & `SILICON_TOPOLOGY_PLAN.md`) for core topology and threading models → reference `ADDITIONAL_DETAILS.md` for shader and compiler guards[cite: 14, 15, 16, 21, 29].
* **Release Engineers:** Execute `PGO_configurator.bat` (or `PGO_stage0LD.bat` → `PGO_stage1.bat` → `PGO_stage2.bat`) as detailed in `USER_GUIDE.md` and verify regression gates via `Scripts\telemetry_daemon.ps1`[cite: 13, 15, 29].

---

## 🏗️ Reference Silicon & Runtime Topology (Elastic `2:3:1` Non-SMT)

Calibrated for an Intel Core i5-9600K (`6C/6T` Non-SMT, locked symmetrically at `4.8 GHz` / `x48`), NVIDIA GeForce GTX 980 Ti, Intel UHD Graphics 630, and HiBy FC3 USB DAC:

| Host Core(s) | Affinity Mask | Clock / Priority | Assigned Subsystems & Steered Hardware IRQs |
| :--- | :--- | :--- | :--- |
| **Core 0** | `0x01` | `4.8 GHz` (`x48`) / Critical MMCSS | `AudioRenderSystemManager` (FlexASIO 128-sample / `2.67 ms` ring + Cubeb failover), `HostTiming` (High), HiBy FC3 USB xHCI IRQ (`0x01`) |
| **Core 1** | `0x02` | `4.8 GHz` (`x48`) / VeryHigh | `VulkanWorker` command submission ring, `GPUDirtyMemoryManager::Gather()`, GTX 980 Ti MSI IRQ (`0x02`), Native Pipeline Statistics Query Pool |
| **Cores 2–4** | `0x1C` | `4.8 GHz` (`x48`) / High | Home-biased Guest ARM64 JIT fibers (`C0`→`0x04`, `C1`→`0x08`, `C2`→`0x10`), 1-cycle BMI1 `_bextr_u64` page walks, 2 MB Large Pages |
| **Core 5** | `0x20` | `4.8 GHz` (`x48`) / Normal–Low | Guest Core 3 (`0x3C` spill), Main GUI/Input, `ScopedNczQuarantine` (ZSTD L18), `PipelineWorkerPoolRing` ($\times 1$), `QSVVideoFeeder`, FFmpeg `execute2` decode slices, SATA AHCI + 2× NVMe IRQs (`0x20`)[cite: 16, 29] |

---

## ⚡ Core Architecture & Silicon Offloading Capabilities

* **Native Vulkan Telemetry OSD & SPIR-V Shaders:** Real-time RAM working set and commit memory monitoring via Win32 `psapi` paired with native SPIR-V font rendering (`font.vert.spv`, `font.frag.spv`) through custom PSOs and dynamic vertex quad generation.
* **Hardware-Accelerated Virtual Memory:** Single-cycle `_bextr_u64` page translation (`bmi_util.h`), Windows Large-Page fastmem (`SeLockMemoryPrivilege`), and lock-free weak-CAS dirty tracking (`GPUDirtyMemoryManager`).
* **Maxwell GM200 / Pascal Protection:** Speculative `5.50 GB` DXGI VRAM Shield (`vk_rasterizer.cpp`), zero-allocation descriptor passthrough (`vk_ubo_passthrough.cpp`), `maxPushDescriptors` ceiling guards, subresource range clamping, and calibrated 3D compute unswizzle tiers.
* **Intel Quick Sync Video (QSV) & FFmpeg Isolation:** Offloads H.264, HEVC, VP8, and VP9 decoding onto Intel UHD Graphics 630 via Media Foundation handles (`QSVVideoFeeder` on Core 5 `0x20`), with FFmpeg software decode slices hooked into Core 5's worker pool.
* **ZSTD Level-18 Block Decompression Quarantine:** Migrates ZSTD worker threads onto Core 5 (`0x20`) at `THREAD_PRIORITY_BELOW_NORMAL` alongside steered SATA AHCI (`storahci.sys`) and NVMe controller IRQs (`0x20`).
* **Bit-Perfect ASIO Audio Pipeline:** Routes raw PCM blocks from Guest JIT cores to Core 0 via a cache-line-aligned atomic ring buffer (`lockfree_audio_queue.hpp`) directly into the Steinberg ASIO SDK (`asio::asio`) targeting `HiBy USB Audio` (`Speakers (HiBy FC3)`)[cite: 13, 17, 29].

---

## 📊 Live Atomic Telemetry Metrics (`build_stage_telemetry_metrics.json`)

Monitored in real-time by `Scripts\telemetry_daemon.ps1`:

* `"audio_underruns": 0` — Confirms Core 0 (`0x01`) Critical MMCSS + FlexASIO 128-sample ring has zero buffer starvation.
* `"voices_dropped": 0` — Confirms the ADSP microkernel never had to shed active voice channels under CPU scheduling pressure.
* `"audio_processing_time_us"` — Tracks microsecond execution duration of the active audio render frame header.
* `"qsv_frames_decoded": > 0` — Confirms `QSVVideoFeeder` on Core 5 (`0x20`) is actively decoding via Intel UHD 630 NT Shared Handles.
* **Working Set & Commit Memory Tracking** — Real-time process memory telemetry displayed directly in the OSD and logging layers[cite: 15, 29].

---

## 🚀 Quick Build, PGO & Automated Pipeline Architecture

```batch
:: 1. Interactive Master Configurator Hub (Recommended)
PGO_configurator.bat

:: 2. Manual Stage-by-Stage Control (Alternative Workflow)
powershell -NoProfile -ExecutionPolicy Bypass -File "Scripts\clean_all.ps1" && PGO_stage0LD.bat
powershell -NoProfile -ExecutionPolicy Bypass -File "Scripts\clean_all.ps1" && PGO_stage1.bat
powershell -NoProfile -ExecutionPolicy Bypass -File "Scripts\validate_profile.ps1"
PGO_stage2.bat
```