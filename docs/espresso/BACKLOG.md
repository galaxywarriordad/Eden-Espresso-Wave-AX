# 🗂️ Eden Espresso Wave [AX] & Piccolo [AX] — Master Product & Engineering Backlog

**Programme Banner:** [The Silicon Preservation Society (`://ko-fi.com`)](https://://ko-fi.com)  
**Current Baseline:** Master `v1.34` | **Unified Plan:** `v1.3` | **Piccolo Spec:** `v0.5` (Sprint 0–5) | **Last Updated:** `2026-11-18`

**Workflow:** Active sprint gate tasks are tracked in `WhatsNext.md`. Completed items move to `CHANGELOG.md`. All Stage A/Stage B baselines, `v1.35.0–v2.00` releases, UHD 630 silicon offloads, Piccolo appliance milestones, and Linux-first multi-emulator expansion (`ax-sdk`) are organized below.

---

## 🏃 Next Sprint & Current Focus

**`v1.34` Bare-Metal Gate Confirmation → `v1.35.0` (Pipeline Automation, `ax-sdk` Extraction & Runtime Capability Gates)**

### Milestone Checklist (Reconciled With `WhatsNext.md`)
* `[✓]` **Milestone 1 — Environment & Core Partitioning:** `clang-cl 23.1+`, `Vulkan 1.4 SDK`, `FFmpeg 7.1.x` LTS, `0x01`/`0x02`/`0x1C`/`0x20` core masks, and IRQ steering configured.
* `[✓]` **Milestone 2 — `PGO_stage0` Build & Pipeline Automation:** `build_stage0\bin\eden.exe` compiled and linked cleanly; `Scripts\macro_update.ps1`, `Scripts\shader_validate.ps1`, `Scripts\telemetry_daemon.ps1`, and live IRQ menu audits integrated across all PGO stage batch scripts.
* `[★]` **CURRENT POSITION — Milestone 3: Stage A Baseline & Stage 0 Bare-Metal Gate Confirmation:**
  * Compile & log **Stage A Vanilla Control Baseline** (`Clang 23.1+` 3-Phase PGO + `Vulkan 1.4` + `FFmpeg 7.1.x`, zero AX code) → `Logs\baseline_pgo_metrics.json`.
  * Complete Stage 0 runtime triage (`0x01`/`0x02`/`0x1C`/`0x20` affinity verification + `audio_underruns: 0`), wire ASIO underrun/voice-drop counters into `telemetry_daemon.ps1`, add batch rollback guards, and patch `topology.cpp` (`LTP_PC_SMT` + `ax_platform_shim.hpp` isolation).
* `[⏳]` **Milestone 4 — Stage 1 & Stage 2 PGO Bake:** Run `3 × 15 min` sandboxed gameplay sweeps (`LLVM_PROFILE_FILE="eden_%p_%m.profraw"`), pass `Scripts\validate_profile.ps1` (`EXCELLENT_STREAM_READY`), and bake final L1i binary (`PGO_stage2.bat`) diffed against Stage A.
* `[⏳]` **Milestone 5 — Piccolo Sprint 0 Bare-Metal Gate Cleared:** Confirm BIOS `IGD Multi-Monitor: Enabled`, boot Linux `6.12-rt` + pinned NVIDIA `580.*` Live USB bench, confirm `rdmsr 0xc8f` Intel CAT status, verify `0` `LOC` ticks on Cores `0–4`, confirm `0` ALSA xruns (`2.00 ms` on `hw:CARD=FC3,DEV=0`), and validate UHD 630 (`VK_API_VERSION_1_3`) `NV12` `dma_buf` ingestion.

---

## 🔬 Reference Silicon & Two-Tier Release Discipline

### Stream 1 Reference Hardware & Driver Floor (`Last Supported Ceiling`)
* **CPU / ISA:** `Intel Core i5-9600K` (`6C/6T`, Zero SMT, `AVX2` + `BMI1/2`, no `AVX-512`) — **active on-die Intel iGPU (`IGD Multi-Monitor` enabled) is mandatory**.
* **Discrete GPU:** `NVIDIA GTX 980 Ti` (`Maxwell GM200`, `6 GB` GDDR5) — pinned to **`582.78 WHQL`** (Windows 10) and **`580.*` legacy branch** (Linux Piccolo), supporting **Vulkan `1.4.x` core** (`590+` dropped Maxwell).
* **Integrated GPU:** `Intel UHD Graphics 630` (`Coffee Lake Gen9.5 GT2`, `24 EUs`, native `2:1` FP16, `VDBOX`, `VEBOX`, `PAK`) — pinned to **`31.0.101.2135`** (Windows 10, capped at **`VK_API_VERSION_1_3`**) and Mesa `ANV` + `intel-media-driver` (`iHD`) on Linux Piccolo.
* **Multimedia Stack:** **`FFmpeg 7.1.x` LTS** + **`nv-codec-headers 12.2.72.0`**.
* **Community Adaptation Boundary:** iGPU-less CPUs (`F`-series) are outside the official full-die preservation envelope; `Docs\SINGLE_GPU_ADAPTATION.md` documents how community self-builders can strip `vaapi_dmabuf_bridge.hpp` hooks from source.

### Two-Tier Baseline Policy (`Stage A` vs. `Stage B`)
* **Stage A (Public / Scientific Control Baseline):** Untouched upstream emulator release (`Eden`, `Cemu`, `RPCS3`, `shadPS4`, `Xenia Canary`) compiled with **`Clang 23.1+` (`ThinLTO` + 3-Phase PGO)** against **`Vulkan 1.4 SDK`** and **`FFmpeg 7.1.x` LTS** with zero `AX` patches → outputs `baseline_pgo_metrics.json`.
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

## 🔄 Piccolo Sprint-to-Master Release Crosswalk (`v1.35.0` – `v2.00`)

| Piccolo Sprint | Piccolo Version | Master Release | Target Medium | Core Focus & Status |
| :--- | :--- | :--- | :--- | :--- |
| **Sprint 0** | Gate | `v1.34` (3.8) | Live USB Bench | `[ACTIVE GATE]` Bare-metal `6.12-rt` + NVIDIA `580.*` + BIOS iGPU enabled + invariant TSC + `NV12` `dma_buf` + HiBy FC3 gate. |
| **Sprint 1** | MVP (`v0.6`) | `v1.35.0` (5.6, 5.7, 5.10) | Samsung FIT Plus | `[UP NEXT]` `< 300 MB` `toram` EROFS USB boot, A/B fallback, `ax-sdk` extraction, runtime capability probes, GM200 FP16→FP32 promotion. |
| **Sprint 2** | `v0.7` | `v1.35.1` (5.5) | Samsung FIT Plus | `[STAGED]` `6 × 1G` + `1024 × 2M` HugeTLB (`5.0 GB` floor), `io_uring` memory pinning, `2.00 ms` PLL ALSA, **Cemu-AX** bring-up. |
| **Sprint 3** | `v0.8` | `v1.35.1` (5.1–5.3) | Samsung FIT Plus | `[STAGED]` `5.50 GB` / `5.35 GB` VRAM shield, `64 KB` sparse VMA pool, **UHD 630 FP16 (`24 EUs`) `ASTC → BC7/BC6H`** transcode. |
| **Sprint 4** | `v0.9` | `v1.36` (6.2, 6.8, 6.9) | Samsung FIT Plus | `[STAGED]` `lsfg-vk 2.0.0+` bindless layers, **UHD 630 `VEBOX` (`NV12 → RGBA` + deinterlace)**, `VK_QUEUE_TRANSFER_BIT`, `256 MB BAR1` WC. |
| **Sprint 5** | `v1.0` → `v2.00` | `v1.37` – `v2.00` | Samsung FIT Plus | `[ROADMAP]` Kiosk hardening (`7.1–7.4`), **RPCS3-AX → shadPS4-AX → Xenia-AX** Linux rollout, **Live Re-LLVM Tiered JIT (`8.1`)**. |

---

## 📊 Scheduled Future Releases (`v1.35.0` – `v2.00`)

Phase 3 Dependency Gate Matrix: Activity 5.7 (Runtime Hardware Flags) is the active prerequisite gate for 5.3, 5.10, 6.2, 6.8, and 6.9. Completing 5.7 in v1.35.0 guarantees that Maxwell GM200 overrides and UHD 630 cross-GPU interop paths are dynamically verified before v1.35.1 memory/offload work lands.

Release	Phase / Sprint	Activity ID & Name	Target Components	Specific Engineering Activities
v1.35.0	Pipeline	4.1 Unified Stage 0–2 Orchestrator	Scripts\build_all.ps1	Wrap Stage A & Stage B Stage 0/1/2 builds + validate_profile.ps1 with automatic SHA256 hashing and side-by-side telemetry diffing against baseline_pgo_metrics.json.
v1.35.0	Pipeline	4.2 sccache & Build Manifest	CMakeLists.txt, build_manifest.json	Enable sccache compiler caching for ~15s incremental C++20 rebuilds and emit a JSON manifest.
v1.35.0	Pipeline	4.3 Portable Release Packager	Scripts\package_release.ps1	[VERIFIED] Bundle single-permutation SPIR-V bytecode, required DLLs, and configs into versioned archives.
v1.35.0	Pipeline	4.4 Automated Frametime & PAK Capture Harness	Scripts\benchmark_frametimes.ps1, Piccolo CI	[VERIFIED] Execute repeatable 60-second deterministic replay runs tracking 1% and 0.1% low stutters; wire UHD 630 PAK (h264_vaapi) hardware video capture on Piccolo for zero-overhead benchmark recording.
v1.35.0	Pipeline / Shaders	4.5 Telemetry, Rollback & VkSpecializationInfo Shaders	Scripts\telemetry_daemon.ps1, PGO_stage*.bat, shader wrappers	Add ASIO underrun and voice-drop counters; expand batch error rollback checks; expose USE_EASU and USE_RCAS via Vulkan VkSpecializationInfo (layout(constant_id = N)) so eden_engine.ini toggles take effect at pipeline creation without recompiling .spv binaries.
v1.35.0	Phase 3 / Sprint 1	5.6 ax-sdk Extraction & Config-Driven Scaler	ax-sdk repo (libax_core), topology.cpp, eden_engine.ini	Extract tiny ax-sdk master repo; fix LTP_PC_SMT flag check and <windows.h> header isolation in topology.cpp; read 2:3:1 ratios dynamically on Intel & AMD Ryzen.
v1.35.0	Phase 3 / Sprint 1	5.7 Runtime Hardware Flags & iGPU Pre-Flight Gate	vk_device.cpp, vulkan_device.h, Docs\SINGLE_GPU_ADAPTATION.md	[CODE READY] Encapsulate Maxwell GM200 queries behind runtime vendor checks; verify active on-die Intel iGPU (0x8086, VK_API_VERSION_1_3 safe query, shaderFloat16, dma_buf/NT handle export/import) at startup and emit clear BIOS IGD Multi-Monitor diagnostics if absent; publish Docs\SINGLE_GPU_ADAPTATION.md for self-builders on iGPU-less CPUs.
v1.35.0	Phase 3	5.10 GM200 FP16→FP32 Promotion	emit_spirv.cpp, vk_device.cpp	Gated by 5.7: detect Maxwell GM200 at runtime to promote Switch 3D shader FP16 ops to native FP32, bypassing half-precision penalties.
v1.35.1	Phase 3 / Sprint 3	5.1 VRAM Shield (5.50 / 5.35 GB)	vk_rasterizer.cpp, vk_vram_shield.hpp	[CODE READY] Poll DXGI and VK_EXT_memory_budget to intercept execution inside VBLANK windows for cache eviction.
v1.35.1	Phase 3 / Sprint 3	5.2 64 KB Sparse VMA Pool	vk_rasterizer.cpp, texture_reaper.cpp	[CODE READY] Back dynamic render targets with a pre-allocated 64 KB Sparse pool paired with VBLANK defragmentation.
v1.35.1	Phase 3 / Sprint 3	5.3 UHD 630 FP16 ASTC → BC7/BC6H Transcode	texture_cache.*, astc_to_bc7.comp, vaapi_dmabuf_bridge.hpp	Gated by 5.7: dispatch ASTC → BC7/BC6H compute shaders onto Intel UHD 630's 24 native-FP16 EUs (VK_API_VERSION_1_3 capped) via dma_buf / NT Shared Handle to eliminate Maxwell GM200 1:64 FP16 and compute time-slicing stalls.
v1.35.1	Phase 3 / Sprint 2	5.5 Piccolo Linux Core & ALSA	ax_platform_shim.hpp, lockfree_audio_queue.hpp	[CODE READY] Attach high-priority direct ALSA interconnects onto hw:CARD=FC3,DEV=0 under SCHED_FIFO 99 (2.00 ms + ±0.005% micro-PLL).
v1.36	Phase 4 / Sprint 4	6.2 Async Compute & VEBOX Media Offload	vk_device.h/.cpp, vaapi_dmabuf_bridge.hpp	[CODE READY] Open secondary VK_QUEUE_COMPUTE_BIT queues with timeline semaphores; route cutscene NV12 → RGBA conversion, deinterlacing (480i/1080i), and scaling through UHD 630's fixed-function VEBOX pipe (VAEntrypointVideoProc) using zero EU shaders.
v1.36	Phase 4	6.8 Dedicated Transfer Bit	vk_staging_buffer_pool.cpp	[CODE READY] Bind fine-grained Vulkan split barriers (VK_QUEUE_TRANSFER_BIT) to overlap async DMA transfers with active 3D passes.
v1.36	Phase 4	6.9 256 MB BAR1 Write-Combined Pool	vk_memory_manager.cpp	[CODE READY] Carve a bounded sub-pool out of GM200's HOST_VISIBLE aperture for direct zero-copy UBO mapping.
v2.00	Phase 6 / Sprint 5	8.1 Live Re-LLVM Tiered JIT	dynarmic, live_reopt_daemon.cpp	[ROADMAP] Runtime hot-block profiling and asynchronous background LLVM recompilation on Core 5 (0x20) with seq_cst I-cache pointer hot-swapping (superseding static AOT disk caches / PA-05).


🌍 Linux-Only Multi-Emulator Expansion (ax-sdk + Piccolo [AX])

Strategy: Windows 10 support concludes with Eden Espresso Wave [AX] (v1.37 / v2.00). All subsequent emulators target Linux natively inside Piccolo [AX] via thin adapter forks linked to the tiny ax-sdk master repository (libax_core), each validated against its own Stage A Vanilla LLVM + Vulkan 1.4 + 3-Phase PGO baseline.

Order	Target Fork	Build & Toolchain Guardrails	Core ax-sdk & Hardware Stream Integration
#1	Cemu-AX (Wii U)	Native CMake + Ninja + Clang 23.1+ (ThinLTO + 3-Phase PGO); developed alongside Piccolo Sprint 2–3.	Pins 3-core PowerPC Espresso JIT onto 0x1C; routes cubeb to Core 0 2.00 ms ALSA ring; reuses vk_vram_shield.hpp and VEBOX deinterlacing.
#2	RPCS3-AX (PS3)	Mandatory -DBUILD_LLVM=ON -DSTATIC_LINK_LLVM=ON (isolates SPU JIT from host Clang 23.1+ / LLVM 22+ regressions); -DUSE_NATIVE_INSTRUCTIONS=OFF; patch LTO guard for headless SDL2 appliance build.	Isolates 1 PPU + primary SPUs on 0x1C / CCX 0; quarantines SPU LLVM workers on Core 5 (0x20) with _MM_HINT_NTA; enables mitigations=off + VEBOX 1080i deinterlacing.
#3	shadPS4-AX (PS4)	Modern C++20 + CMake + Clang 23.1+ (ThinLTO + 3-Phase PGO); developed on Stream 3 (11th Gen AVX-512 + 8 GB+ VRAM) before down-porting.	Backs 8 GB unified memory via 6 × 1G + 1024 × 2M HugeTLB + 17.5 GB vmtouch -dql; backports to Stream 1 (9600K + 980 Ti 6 GB) via 5.35 GB VRAM Shield + 64 KB sparse VMA aliasing + UHD 630 FP16 transcode.
#4	Xenia-AX (Xbox 360)	Clang 23.1+ + Ninja (xb build); sequenced after shadPS4-AX to allow upstream Linux Vulkan backend maturity.	Maps Xenon worker threads onto 0x1C (0x3C spillover), optimizes ROV split barriers, and streams constant buffers via 256 MB BAR1 WC pool.


🔒 Appliance Mode & Kiosk Hardening Roadmap (Post-Maturity Backlog)


Release	Phase / Sprint	Activity ID & Name	Target Components	Specific Engineering Activities
v1.37	Phase 5 / Kiosk	7.1 Direct Boot & Kiosk Launcher	main.cpp, core.cpp	[BACKLOG] Implement automatic ROM path checks, instant core load, and borderless exclusive full-screen windows.
v1.37	Phase 5 / Kiosk	7.2 Controller Profile Freeze	input_subsystem.cpp	[BACKLOG] Lock standard gamepad bindings in code, disable input remapping dialogs, and prevent hotkey overrides.
v1.37	Phase 5 / Kiosk	7.3 Filesystem Sandbox & NAND	fs_util.cpp, path_util.cpp	[BACKLOG] Redirect virtual NAND, save directories, and cache paths into a write-protected container.
v1.37	Phase 5 / Kiosk	7.4 Branding Sanitization	main_window.cpp	[BACKLOG] Purge remaining upstream references across crash handlers and window headers in favor of The Silicon Preservation Society ([AX]).


🚫 Architectural Guardrails & Rejected Candidates

• Two-Tier Baseline Discipline (Stage A vs. Stage B): Never ship or benchmark an [AX] build without first logging its unmodified upstream Stage A (Clang 23.1+ + Vulkan 1.4 + FFmpeg 7.1.x + 3-Phase PGO) control baseline (baseline_pgo_metrics.json).
• Mandatory Active On-Die Intel iGPU Policy: Stream 1 and Stream 3 builds require an active on-die Intel iGPU (IGD Multi-Monitor enabled) to harvest VDBOX, 24 EUs FP16, VEBOX, and PAK. Do not maintain dual fallback compute pipelines inside official [AX] builds; maintain Docs\SINGLE_GPU_ADAPTATION.md for self-builders on F-series SKUs.
• No Post-Espresso Windows Porting & No Bloated Desktop Linux Packaging: Windows 10 support ends at Eden Espresso Wave [AX]. All subsequent emulators (Cemu, RPCS3, shadPS4, Xenia Canary) target Linux natively inside the < 300 MB Piccolo [AX] toram EROFS appliance via ax-sdk—never general-purpose PipeWire/Wayland desktop Linux packages.
• No Dynamic Work-Stealing Thread Pools: Do not replace static core partitioning (0x01/0x02/0x1C/0x20) with dynamic work-stealing schedulers; synchronization overhead and cache-line migration penalties undermine the zero-jitter determinism of [AX].
• Strict < 300 MB toram EROFS Budget: Never pull PipeWire, PulseAudio, Openbox/Wayland compositors, NVIDIA CUDA/OptiX/NVENC blobs, or Mesa LLVM drivers into batocera-ax-external/.
• Rejected — PA-05 (AOT ARM64→x86_64 Persistent Disk Cache): Rejected due to self-modifying code (SMC) hazards, heavy upstream Dynarmic/loader merge friction, and functional overlap with Activity 8.1 (Live Re-LLVM Tiered JIT).

---