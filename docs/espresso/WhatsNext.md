# 🗂️ Eden Espresso Wave [AX] & Piccolo [AX] — Master Product & Engineering Backlog

**Programme Banner:** [The Silicon Preservation Society](https://ko-fi.com/thesiliconpreservationsociety)  
**Current Baseline:** Master `v1.35` | **Unified Plan:** `v1.3` | **Piccolo Spec:** `v0.6` (Sprint 0–5) | **Last Updated:** `2026-11-19`

**Workflow:** Active sprint gate tasks are tracked in `WhatsNext.md`. Completed items move to `CHANGELOG.md`. All Stage A/Stage B baselines, `v1.35–v2.00` releases, UHD 630 silicon offloads, Piccolo appliance milestones, and Linux-first multi-emulator expansion (`ax-sdk`) are organized below.

---

## 🏃 Next Sprint & Current Focus

**`v1.35` Async `5.00 GiB` VRAM Shield & `ax-sdk` Integration → First `PGO_stage0LD` Build & Stage A/B Benchmark Gate**

### Milestone Checklist (Reconciled With `WhatsNext.md`)
* `[✓]` **Milestone 1 — Environment & Core Partitioning:** `clang-cl 23.1+`, `Vulkan 1.4 SDK`, `FFmpeg 7.1.x` LTS, `0x01`/`0x02`/`0x1C`/`0x20` core masks, and IRQ steering configured.
* `[✓]` **Milestone 2 — `ax-sdk v1.35.0`, Async `5.00 GiB` VRAM Shield & PGO Pipeline Hardening:** Implemented proactive `5.00 GiB` async VRAM shield (`VkVramShield` + `VK_QUEUE_TRANSFER_BIT` + Vulkan 1.4 `vkCmdSetEvent2`/`WaitEvents2` split barriers + Core 5 reaper), dynamic P-core/SMT `2:3:1` topology resolver (`topology.cpp` with `LTP_PC_SMT` + RAII MMCSS cleanup), cache-line-isolated batch SPSC audio ring (`ax::audio` + `lockfree_audio_queue.hpp`), and `validate_profile.ps1` / `eden_engine.ini` staging in `PGO_stage*.bat`.
* `[★]` **CURRENT POSITION — Milestone 3: `PGO_stage0LD` Compile Verification & Stage A / Stage B Bare-Metal Gate:**
  * Execute `PGO_config.bat` (`[0LD]`) to verify clean `clang-cl` + `lld-link` compilation across `ax-sdk` and `Eden-Espresso-Wave-AX`.
  * Compile & log **Stage A Vanilla Control Baseline** (`Clang 23+` 3-Phase PGO + `Vulkan 1.4` + `FFmpeg 7.1.x`, zero AX code) → `Logs\baseline_pgo_metrics.json`, then verify Stage 0 runtime triage (`0x01`/`0x02`/`0x1C`/`0x20` affinity compliance, `VkVramShield` `5.00 GiB` async reaper logs, and `audio_underruns: 0`).
* `[⏳]` **Milestone 4 — Stage 1 & Stage 2 PGO Bake:** Run `3 × 15 min` sandboxed gameplay sweeps (`LLVM_PROFILE_FILE="eden_%p_%m.profraw"`), pass `Scripts\validate_profile.ps1` (`EXCELLENT_STREAM_READY`), and bake final L1i binary (`PGO_stage2.bat`) diffed against Stage A.
* `[⏳]` **Milestone 5 — Piccolo Sprint 0 Bare-Metal Gate Cleared:** Boot Linux `6.12-rt` + pinned NVIDIA `580.*` Live USB bench, confirm `rdmsr 0xc8f` Intel CAT status, verify `0` `LOC` ticks on Cores `0–4`, confirm `0` ALSA xruns (`2.00 ms` on `hw:CARD=FC3,DEV=0`), and validate UHD 630 (`VK_API_VERSION_1_3`) `NV12` `dma_buf` ingestion.

---

## 🔬 Reference Silicon & Two-Tier Release Discipline

### Stream 1 Reference Hardware & Driver Floor (`Last Supported Ceiling`)
* **CPU / ISA:** `Intel Core i5-9600K` (`6C/6T`, Zero SMT, `AVX2` + `BMI1/2`, no `AVX-512`).
* **Discrete GPU:** `NVIDIA GTX 980 Ti` (`Maxwell GM200`, `6 GB` GDDR5) — pinned to **`582.78 WHQL`** (Windows 10) and **`580.*` legacy branch** (Linux Piccolo), supporting **Vulkan `1.4.x` core** (`590+` dropped Maxwell).
* **Integrated GPU:** `Intel UHD Graphics 630` (`Coffee Lake Gen9.5 GT2`, `24 EUs`, native `2:1` FP16, `VDBOX`, `VEBOX`, `PAK`) — pinned to **`31.0.101.2135`** (Windows 10, capped at **`VK_API_VERSION_1_3`**) and Mesa `ANV` + `intel-media-driver` (`iHD`) on Linux Piccolo.
* **Multimedia Stack:** **`FFmpeg 7.1.x` LTS** + **`nv-codec-headers 12.2.72.0`**.

### Two-Tier Baseline Policy (`Stage A` vs. `Stage B`)
* **Stage A (Public / Scientific Control Baseline):** Untouched upstream emulator release (`Eden`, `Cemu`, `RPCS3`, `shadPS4`, `Xenia Canary`) compiled with **`Clang 23+` (`ThinLTO` + 3-Phase PGO)** against **`Vulkan 1.4 SDK`** and **`FFmpeg 7.1.x` LTS** with zero `AX` patches → outputs `baseline_pgo_metrics.json`.
* **Stage B (Full `[AX]` & `Piccolo [AX]` Release):** Layers our tiny **`ax-sdk`** master core and `< 300 MB` `PREEMPT_RT` Linux appliance onto Stage A, measuring exact $P_{99}$ jitter, VRAM stability, and audio latency deltas against the PGO-baked control.

---

## 🛠️ Completed Subsystem Additions (Sprint 0 Foundation)

### 1. Repository & CI Pipeline Hygiene (Resolved)
* **2.1 Custom Sub-60s Hygiene Check:** Removed inherited upstream workflows; deployed automated `.forgejo/workflows/hygiene.yml` runner pipeline (`NOMINMAX`, `#undef CreateEvent`, `VK_NO_PROTOTYPES` compliance).
* **2.2 EOL Line Endings Enforced:** Locked explicit `eol=crlf` (`*.bat`, `*.ps1`) and `eol=lf` (`*.cpp`, `*.h`, `*.md`, `*.sh`, `*.mk`, `*.in`, `*.cfg`, `piccolo`) rules via `.gitattributes`.
* **2.3 Standardised Script Paths:** Consolidated all root scripts into `Scripts\`, updating relative paths across PGO bakes.

### 2. Phase 2 Throughput, Safety Guards & Core Isolation (Resolved)
* **3.1 Page-Contiguity & Alignment Guard:** Verified physical page contiguity constraints (`(addr & YUZU_PAGEMASK) + size <= YUZU_PAGESIZE`) inside `vk_ubo_passthrough.cpp` to protect direct template uploads.
* **3.2 Maxwell `maxPushDescriptors` Ceiling:** Hardcoded the GTX 980 Ti's exact push descriptor slot limits, emitting null-descriptor placeholders on slot overflow.
* **3.3 SPIR-V Descriptor Clamping:** Clamped `VkImageSubresourceRange` mip/layer bounds inside `vk_texture_cache.cpp` to prevent Maxwell `VK_ERROR_DEVICE_LOST` faults.
* **3.6 FFmpeg `7.1.x` `execute2` & `libswresample` Hook:** Intercepted internal FFmpeg worker dispatch pipelines via `PinnedVideoFeederExecute2` onto Core 5 (`0x20`) and enforced AVX2 SIMD FIR resampling (`SWR_DITHER_TRIANGULAR_HIGHPASS`) feeding Core 0 (`0x01`).

### 3. Sub-Millisecond Jitter & Microarchitectural Edge Case Correction (Resolved)
* **Gap 1 — Multi-Core I-Cache Fences:** Integrated sequentially consistent thread fences (`std::memory_order_seq_cst`) inside `live_reopt_daemon.cpp` for pointer hot-swapping.
* **Gap 2 — Cross-GPU Fence Synchronisation:** Bound explicit `VkExternalSemaphoreKHR` opaque file descriptor fences to wait for planar `NV12` video slices without host RAM staging copies.
* **Gap 3 — VRAM Compaction Interconnect:** Refined `TextureCacheMatrix` interfaces so `vk_rasterizer.cpp` can evict stale texture layers inside quiet VBLANK windows.
* **Gap 4 — Non-Blocking VFS Index Maps:** Re-architected predictive I/O migrations into a reader-preferred `std::shared_mutex` hash map for amortized $O(1)$ lookup speeds.
* **Edge Case 2 — Telemetry Ingestion Rescue:** Restructured packet collections to use a modular circular buffer check across ring boundaries.
* **Edge Case 3 — Thread Starvation Valve:** Limited contiguous hardware pacing spin loops inside `core_timing.cpp` to a maximum of `100 µs`.

### 4. Tooling, Shader Harmonization & Pipeline Telemetry Automation (Resolved — `2026-11-16`)
* **2.4 Numeric Shader Macro Harmonizer (`Scripts\macro_update.ps1`):** Enforces explicit numeric defines (`USE_EASU`, `USE_RCAS`, `YUZU_USE_FP16`, `VERSION`) across all shader wrappers.
* **2.5 Pre-Build SPIR-V Validation Gate (`Scripts\shader_validate.ps1`):** Standardized all wrappers to `#version 460 core` with `#extension GL_GOOGLE_include_directive` header insertion and pre-build `glslangValidator` checks.
* **2.6 Async Per-Core Telemetry Daemon (`Scripts\telemetry_daemon.ps1`):** Deployed out-of-process profiling of `eden.exe` capturing per-core CPU utilisation, thread affinities, memory footprint, and audio buffer watermarks in JSON format.
* **2.7 Integrated PGO Stage Gates & Live IRQ Audit Menu:** Wired `macro_update.ps1`, `shader_validate.ps1`, and `telemetry_daemon.ps1` regression assertions into `PGO_stage0LD.bat`, `PGO_stage1.bat`, and `PGO_stage2.bat`, and added live IRQ affinity audit rendering to the root configurator menu.
* **2.8 Runtime Configuration Schema (`eden_engine.ini`) & Script Guards:** Defined `[EngineTopology]` (`2:3:1` scaler), `[HardwareFeatureFlags]` (Maxwell/QSV/VRAM shields), and `[WindowsPlatformHacks]` (MMCSS Pro Audio & priority classes).

---

## 🔄 Piccolo Sprint-to-Master Release Crosswalk (`v1.35` – `v2.00`)

| Piccolo Sprint | Piccolo Version | Master Release | Target Medium | Core Focus & Status |
| :--- | :--- | :--- | :--- | :--- |
| **Sprint 0** | Gate | `v1.34` (3.8) | Live USB Bench | `[ACTIVE GATE]` Bare-metal `6.12-rt` + NVIDIA `580.*` + invariant TSC + `NV12` `dma_buf` + HiBy FC3 gate. |
| **Sprint 1** | MVP (`v0.6`) | `v1.35` (5.5, 5.8) | Samsung FIT Plus | `[UP NEXT]` `< 300 MB` `toram` EROFS USB boot, A/B fallback, loading-screen core bursting (`0x1C`), `ax-sdk` extraction. |
| **Sprint 2** | `v0.7` | `v1.35` (5.5, 5.9) | Samsung FIT Plus | `[STAGED]` `6 × 1G` + `1024 × 2M` HugeTLB (`5.0 GB` floor), `io_uring` memory pinning, `2.00 ms` PLL ALSA, **Cemu `[AX]`** bring-up. |
| **Sprint 3** | `v0.8` | `v1.35` (5.1–5.4) | Samsung FIT Plus | `[STAGED]` `5.50 GB` / `5.35 GB` VRAM shield, `64 KB` sparse VMA pool, **UHD 630 FP16 (`24 EUs`) `ASTC → BC7`** transcode. |
| **Sprint 4** | `v0.9` | `v1.36` (6.1–6.7) | Samsung FIT Plus | `[STAGED]` WinRT WGC / `lsfg-vk 2.0.0+` bindless layers, **UHD 630 `VEBOX` (`NV12 → RGBA` + deinterlace)**, `256 MB BAR1` WC. |
| **Sprint 5** | `v1.0` → `v2.00` | `v1.37` – `v2.00` | Samsung FIT Plus | `[ROADMAP]` Closed-loop governor calibration, **RPCS3 `[AX]` → shadPS4 `[AX]` → Xenia `[AX]`** Linux rollout, live Re-LLVM JIT. |

---

## 📊 Scheduled Future Releases (`v1.35` – `v2.00`)

> **Phase 3 Dependency Gate Matrix:** Activity **5.7 (Runtime Hardware Feature Flags)** is the active prerequisite gate for **5.10, 6.8, and 6.9**. Completing `5.7` guarantees that all Maxwell `GM200`-exclusive overrides remain dynamically encapsulated behind runtime capability checks rather than hardcoded paths.

| Release | Phase / Sprint | Activity ID & Name | Target Components | Specific Engineering Activities |
| :--- | :--- | :--- | :--- | :--- |
| **`v1.35`** | Pipeline | **4.1 Unified Stage 0–2 Orchestrator** | `Scripts\build_all.ps1` | Wrap Stage A & Stage B Stage 0/1/2 builds + `validate_profile.ps1` with automatic SHA256 hashing and side-by-side telemetry diffing against `baseline_pgo_metrics.json`. |
| **`v1.35`** | Pipeline | **4.2 `sccache` & Build Manifest** | `CMakeLists.txt`, `build_manifest.json` | Enable `sccache` compiler caching for `~15s` incremental C++20 rebuilds and emit a JSON manifest. |
| **`v1.35`** | Pipeline | **4.3 Portable Release Packager** | `Scripts\package_release.ps1` | `[VERIFIED]` Bundle compiled SPIR-V bytecode, required DLLs, and configs into versioned archives. |
| **`v1.35`** | Pipeline | **4.4 Automated Frametime & PAK Capture Harness** | `Scripts\benchmark_frametimes.ps1`, Piccolo CI | `[VERIFIED]` Execute repeatable 60-second deterministic replay runs tracking 1% and 0.1% low stutters; wire UHD 630 PAK (`h264_vaapi`) hardware video capture on Piccolo for zero-overhead benchmark recording. |
| **`v1.35`** | Pipeline / Telemetry | **4.5 Telemetry & Batch Resilience Hardening** | `Scripts\telemetry_daemon.ps1`, `PGO_stage*.bat`, `.forgejo/` | Add ASIO underrun and voice-drop counters to audio telemetry; expand batch error rollback handling; extend CI hardware gate checks; wire `eden_engine.ini` flags into shader macro paths. |
| **`v1.35`** | Phase 3 / Sprint 3 | **5.1 Proactive `5.00 GiB` Async VRAM Shield** | `ax-sdk/include/ax/vram.hpp`, `vk_vram_shield.hpp`, `vk_rasterizer.cpp` | `[COMPLETED]` Proactive `5.00 GiB` (`kPruneTriggerBytes`) async prune trigger (`~4.50 GiB` hysteresis recovery) ahead of `5.50 / 5.35 GiB` ceilings; offloads frees via `VK_KHR_timeline_semaphore` to Core 5 (`Domain::Housekeeping`) with zero VBLANK stalls. |
| **`v1.35`** | Phase 3 / Sprint 3 | **5.2 `64 KB` Sparse VMA Pool** | `vk_rasterizer.cpp`, `texture_reaper.cpp` | `[STAGED]` Back dynamic render targets with a pre-allocated `64 KB` Sparse pool paired with async defragmentation. |
| **`v1.35`** | Phase 3 / Sprint 3 | **5.3 UHD 630 FP16 `ASTC → BC7/BC6H` Transcode** | `texture_cache.*`, `astc_to_bc7.comp`, `vaapi_dmabuf_bridge.hpp` | Dispatch `ASTC → BC7/BC6H` compute shaders onto Intel UHD 630's 24 native-FP16 EUs (`VK_API_VERSION_1_3` capped) via `dma_buf` / NT Shared Handle to eliminate Maxwell `GM200` `1:64` FP16 and compute time-slicing stalls (with GTX 980 Ti single-GPU fallback). |
| **`v1.35`** | Phase 3 / Sprint 1–2 | **5.5 Batch SPSC Audio Ring & Piccolo ALSA** | `ax-sdk/include/ax/audio.hpp`, `lockfree_audio_queue.hpp`, `ax_platform_shim.hpp` | `[COMPLETED — RING]` Cache-line-isolated `LockFreeStereoRing<2048, float>` (`PushBatch`/`PopBatch`/`PopPlanarBatch`) with per-callback telemetry; `[CODE READY — ALSA]` `hw:CARD=FC3,DEV=0` under `SCHED_FIFO 99` (`2.00 ms` + `±0.005%` micro-PLL). |
| **`v1.35`** | Phase 3 / Sprint 1 | **5.6 `ax-sdk` Extraction & Dynamic `2:3:1` Scaler** | `ax-sdk` repo, `src/topology.cpp`, `eden_engine.ini` | `[COMPLETED]` Standalone `ax-sdk` (`ax::core`) integrated via `FetchContent` (`WIN32`-gated ASIO/FlexASIO); implemented `GetLogicalProcessorInformationEx` (`LTP_PC_SMT` + `EfficiencyClass`) & Linux `sysfs` P-core `2:3:1` scaler with RAII MMCSS cleanup and `HostTimingPriorityClass=HIGH`. |
| **`v1.35`** | Phase 3 | **5.7 Runtime Hardware Flags** | `vk_device.cpp`, `vulkan_device.h` | `[CODE READY]` Encapsulate Maxwell capabilities queries behind clean runtime vendor checks to cut merge friction. |
| **`v1.35`** | Phase 3 | **5.10 `GM200` `FP16 → FP32` Promotion** | `emit_spirv.cpp`, `vk_device.cpp` | Detect Maxwell `GM200` at runtime to promote Switch 3D shader `FP16` ops to native `FP32`, bypassing half-precision penalties. |
| **`v1.36`** | Phase 4 / Sprint 4 | **6.2 Async Compute & `VEBOX` Media Offload** | `vk_device.h/.cpp`, `vaapi_dmabuf_bridge.hpp` | `[CODE READY]` Open secondary `VK_QUEUE_COMPUTE_BIT` queues with timeline semaphores; route cutscene `NV12 → RGBA` conversion, deinterlacing (`480i`/`1080i`), and scaling through UHD 630's fixed-function `VEBOX` pipe (`VAEntrypointVideoProc`) using zero EU shaders. |
| **`v1.36`** | Phase 4 | **6.8 Dedicated `VK_QUEUE_TRANSFER_BIT` & Split Barriers** | `vulkan_device.h/.cpp`, `vulkan_wrapper.h/.cpp`, `vk_vram_shield.hpp` | `[COMPLETED]` Discovered dedicated `VK_QUEUE_TRANSFER_BIT` hardware queue families in `Device::SetupFamilies()` and wired Vulkan 1.4 Synchronization2 split barriers (`vkCmdSetEvent2` → `vkCmdWaitEvents2`) for zero-stall async DMA & VRAM eviction. |
| **`v1.36`** | Phase 4 | **6.9 `256 MB BAR1` Write-Combined Pool** | `vk_memory_manager.cpp` | `[CODE READY]` Carve a bounded sub-pool out of `GM200`'s `HOST_VISIBLE` aperture for direct zero-copy UBO mapping. |

---

## 🌍 Linux-Only Multi-Emulator Expansion (`ax-sdk` + `Piccolo [AX]`)

**Strategy:** Windows 10 support concludes with **Eden Espresso Wave `[AX]`** (`v1.37` / `v2.00`). All subsequent emulators target Linux natively inside **Piccolo `[AX]`** via thin adapter forks linked to the tiny `ax-sdk` master repository, each validated against its own Stage A Vanilla LLVM + Vulkan 1.4 + 3-Phase PGO baseline.

| Order | Target Fork | Build & Toolchain Guardrails | Core `ax-sdk` & Hardware Stream Integration |
| :--- | :--- | :--- | :--- |
| **#1** | **Cemu-AX** (Wii U) | Native CMake + Ninja + Clang 23+ (ThinLTO + 3-Phase PGO); developed alongside Piccolo Sprint 2–3. | Pins 3-core PowerPC Espresso JIT onto `0x1C`; routes `cubeb` to Core 0 `2.00 ms` ALSA ring; reuses `vk_vram_shield.hpp` and `VEBOX` deinterlacing. |
| **#2** | **RPCS3-AX** (PS3) | Mandatory `-DBUILD_LLVM=ON -DSTATIC_LINK_LLVM=ON` (isolates SPU JIT from host Clang 23+ / LLVM 22+ regressions); `-DUSE_NATIVE_INSTRUCTIONS=OFF`; patch LTO guard for headless SDL2 appliance build. | Isolates 1 PPU + primary SPUs on `0x1C` / CCX 0; quarantines SPU LLVM workers on Core 5 (`0x20`) with `_MM_HINT_NTA`; enables `mitigations=off`. |
| **#3** | **shadPS4-AX** (PS4) | Modern C++20 + CMake + Clang 23+ (ThinLTO + 3-Phase PGO); developed on Stream 3 (11th Gen AVX-512 + `8 GB+` VRAM) before down-porting. | Backs `8 GB` unified memory via `6 × 1G` + `1024 × 2M` HugeTLB + `17.5 GB` `vmtouch -dql`; backports to Stream 1 (`9600K` + `980 Ti 6 GB`) via `5.35 GB` VRAM Shield + `64 KB` sparse VMA aliasing. |
| **#4** | **Xenia-AX** (Xbox 360) | Clang 23+ + Ninja (`xb` build); sequenced after `shadPS4-AX` to allow upstream Linux Vulkan backend maturity. | Maps Xenon worker threads onto `0x1C` (`0x3C` spillover), optimizes ROV split barriers, and streams constant buffers via `256 MB BAR1` WC pool. |

---

## 🔒 Appliance Mode & Kiosk Hardening Roadmap (Post-Maturity Backlog)

| Release | Phase / Sprint | Activity ID & Name | Target Components | Specific Engineering Activities |
| :--- | :--- | :--- | :--- | :--- |
| **`v1.37`** | Phase 5 / Kiosk | **7.1 Direct Boot & Kiosk Launcher** | `main.cpp`, `core.cpp` | `[BACKLOG]` Implement automatic ROM path checks, instant core load, and borderless exclusive full-screen windows. |
| **`v1.37`** | Phase 5 / Kiosk | **7.2 Controller Profile Freeze** | `input_subsystem.cpp` | `[BACKLOG]` Lock standard gamepad bindings in code, disable input remapping dialogs, and prevent hotkey overrides. |
| **`v1.37`** | Phase 5 / Kiosk | **7.3 Filesystem Sandbox & NAND** | `fs_util.cpp`, `path_util.cpp` | `[BACKLOG]` Redirect virtual NAND, save directories, and cache paths into a write-protected container. |
| **`v1.37`** | Phase 5 / Kiosk | **7.4 Branding Sanitization** | `main_window.cpp` | `[BACKLOG]` Purge remaining upstream references across crash handlers and window headers in favor of The Silicon Preservation Society (`[AX]`). |

---

## 🚫 Architectural Guardrails & Rejected Candidates

* **Two-Tier Baseline Discipline (Stage A vs. Stage B):** Never ship or benchmark an `[AX]` build without first logging its unmodified upstream Stage A (`Clang 23+` + `Vulkan 1.4` + `FFmpeg 7.1.x` + 3-Phase PGO) control baseline (`baseline_pgo_metrics.json`).
* **No Post-Espresso Windows Porting & No Bloated Desktop Linux Packaging:** Windows 10 support ends at **Eden Espresso Wave `[AX]`**. All subsequent emulators (`Cemu`, `RPCS3`, `shadPS4`, `Xenia Canary`) target Linux natively inside the `< 300 MB` `Piccolo [AX]` `toram` EROFS appliance via `ax-sdk`—never general-purpose PipeWire/Wayland desktop Linux packages.
* **No Dynamic Work-Stealing Thread Pools:** Do not replace static core partitioning (`0x01`/`0x02`/`0x1C`/`0x20`) with dynamic work-stealing schedulers; synchronization overhead and cache-line migration penalties undermine the zero-jitter determinism of `[AX]`.
* **Strict `< 300 MB` `toram` EROFS Budget:** Never pull PipeWire, PulseAudio, Openbox/Wayland compositors, NVIDIA CUDA/OptiX/NVENC blobs, or Mesa LLVM drivers into `batocera-ax-external/`.
* **Rejected — `PA-05` (AOT `ARM64 → x86_64` Persistent Disk Cache):** Rejected due to self-modifying code (SMC) hazards, heavy upstream Dynarmic/loader merge friction, and functional overlap with Activity `8.1`[cite: 10, 11, 12].