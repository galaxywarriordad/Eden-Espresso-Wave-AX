# 🚀 Eden Espresso Wave [AX] & Piccolo [AX] — Active Sprint Gate (`WhatsNext.md`)

**Programme Banner:** [The Silicon Preservation Society](https://ko-fi.com)  
**Active Baseline:** `Eden-Espresso-Wave-AX` `v1.35.1` + `ax-sdk` `v1_36_1` (`v1.36.1`) → `v1.37.0`  
**Target SDK:** `galaxywarriordad/ax-sdk` (`ax::core` `v1_36_1`)  
**Last Updated:** `2026-11-20`

---

## 🎯 Immediate Gate: `v1.35.1` + `ax-sdk v1_36_1` Smoke Test & Git Release (`POST-REBOOT`)

*Objective: Push the local `Eden-Espresso-Wave-AX` `v1_36_1` integration, verify a clean `Stage 0LD` build with live Shared-Memory Telemetry (`Local\AX_Telemetry`), and tag `v1.35.1` before moving on to `v1.37` features.*

### 1. Completed Code, SDK & Tooling Deliverables
* `[✓]` **Maxwell `GM200` Shader Recompiler & Log Class Fix (`src/shader_recompiler/frontend/maxwell/decode.cpp`):** Fixed `LOG_DEBUG(Shader, ...)` log class compilation and resolved shader decoding stalls (`Loading Shaders 0 / 4`).
* `[✓]` **Memory Manager & Staging Pool Hardening (`memory_manager.cpp`, `vulkan_device.cpp`, `vk_staging_buffer_pool.cpp`):** Enforced Maxwell `Arch_Maxwell` (`FP16 → FP32` promotion) and bounded the `HOST_VISIBLE` BAR1 staging pool to **`128 MiB`** (`128_MiB BAR1`) to prevent PCIe aperture exhaustion on 6 GB cards.
* `[✓]` **Fresh Config `LibraryApplet` HLE Defaults (`src/common/settings.h`):** Switched `player_select_applet_mode` and `mii_edit_applet_mode` default values from `AppletMode::LLE` to **`AppletMode::HLE`** (`Custom frontend`) so clean installs boot *Mario Kart 8 Deluxe* profile/Mii selection without silent firmware hangs.
* `[✓]` **Configurator Hub IRQ Status Parser (`PGO_config.bat`):** Replaced parenthesised `echo %%A | findstr` pipes with direct `findstr /C:"Audio/Timing"`, `"Vulkan Submit"`, and `"Aux Quarantine"` file lookups against `%HW_CHECK_LOG%`.
* `[✓]` **Live Runtime Triage Filter (`Scripts\tail_eden_log.ps1`):** Created `[TL]` color-coded log tailer filtering `Arch_Maxwell`, `BAR1`, `VramShield`, `ax::`, `topology`, `underrun`, `Loading Shaders`, `Applet`, and `<Error>/<Critical>` events in real time.
* `[✓]` **Zero-Pollution Shader Validation (`Scripts\shader_validate.ps1`):** Routed `glslangValidator` output to `$env:TEMP\eden_validate.spv` with post-loop cleanup so `frag.spv` is never dropped in the repository root.
* `[✓]` **Script Workspace Hygiene (`Scripts\dev_tools\` & `ax-sdk/tools/`):** Isolated legacy helper scripts into `Scripts\dev_tools\`, delegated shared host scripts (`Get-AxProfile.ps1`, `check_hardware.ps1`, `set_irq_affinity.ps1 -Revert`, `track_telemetry.ps1`, `check_regression.ps1`, `clean_all.ps1 -Wait`, and Linux `ax_doctor.sh`) to `ax-sdk/tools/` via `Scripts\resolve_ax_tools.bat`, and fixed `Add-Type -MemberDefinition` window positioning across `PGO_stage*.bat`.
* `[✓]` **`ax-sdk` `v1_36_1` Release & Consumer Wiring (`AX-1.1` – `AX-1.3`):** Published `ax-sdk` `v1_36_1` with `ax::config` (`ax_engine.ini`), target-scoped `CMakeLists.txt` (`cxx_std_20`), cross-process Shared-Memory Telemetry (`Local\AX_Telemetry` / `/dev/shm/ax_telemetry`), and lock-free microsecond audio jitter histograms (`ax::RecordAudioCallbackMetrics`); wired into `Eden` (`sink_stream.cpp`, `flex_asio_manager.cpp`) and `FLAC_AX` (`src/main.cpp`), and aligned all 5 repositories (`ax-sdk`, `Eden-Espresso-Wave-AX`, `FLAC_AX`, `Cemu-Latte-AX`, `Piccolo-AX`) to `v1_36_1`.

### 2. Remaining Release Gate (`POST-REBOOT`)
* `[ ]` **Step A — Finalize & Push Local `Eden-Espresso-Wave-AX` (`C:\Users\nick_\Desktop\eden`):** Remove temporary `build_stage0_triage.log` and duplicate `Scripts\set_irq_affinity.ps1`, verify `Scripts\resolve_ax_tools.bat`, and run `git add -A && git commit && git push`.
* `[ ]` **Step B — Clean `[0LD]` Build (`build_stage0\bin\eden.exe`):** Run `PGO_config.bat` → `0LD` to pull `ax-sdk` (`v1_36_1`) via `FetchContent` and compile `build_stage0\bin\eden.exe` cleanly.
* `[ ]` **Step C — Single-Window `[TL]` + Shared-Memory Telemetry Smoke Test (*Mario Kart 8 Deluxe* + `HiBy FC3` DAC):** Select `TL` in `PGO_config.bat` (auto-running `reset_eden_config.ps1`), verify `Arch_Maxwell` (`FP16 → FP32`), `128_MiB BAR1`, HLE Applets, `"TelemetrySource": "SharedMemory"` in `build_stage_telemetry_metrics.json`, `0` errors, and `0` audio underruns, then tag `v1.35.1`.

---

## ⚙️ Sprint `v1.36` — Remaining `ax-sdk` Polish & Engine VRAM/BAR1/QSV Features

*Objective: Complete remaining `ax-sdk` single-header amalgamation (`AX-1.4`), wire live GPU VRAM/QSV counters into `ax::GetTelemetry()`, and re-integrate the proactive `5.00 GiB` VRAM shield, frame-fenced BAR1 recycling, and cross-GPU QSV/VA-API video offload.*

### Part 1: `ax-sdk` Repository (`galaxywarriordad/ax-sdk`)
* `[✓]` **1. `AX-1.1` Unified Settings & Platform Macros (`include/ax/config.hpp`, `src/config.cpp`):** Implemented `ax::LoadEngineConfig` / `ax::WriteDefaultEngineConfig` (`ax_engine.ini`) and standardized `AX_PLATFORM_*` / `AX_HAS_*` macros.
* `[✓]` **2. `AX-1.2` CMake Target-Scope & Hygiene Refactor (`CMakeLists.txt`):** Replaced global C++ standard mutation with `target_compile_features(ax_core ... cxx_std_20)`, `Threads::Threads`, private Win32 macro scoping, and opt-in `AX_SDK_ENABLE_WARNINGS`.
* `[✓]` **3. `AX-1.3` Shared-Memory Atomic Telemetry & Audio Jitter Profiler (`include/ax/telemetry.hpp`, `src/telemetry.cpp`):** Implemented 64B-padded `TelemetryCounters` (`Local\AX_Telemetry` / `/dev/shm/ax_telemetry`) and lock-free `min / P99 / max` microsecond audio callback histograms (`ax::RecordAudioCallbackMetrics`).
* `[ ]` **4. `AX-1.4` Single-Header Amalgamation Generator (`tools/amalgamate.ps1`, `.github/workflows/release.yml`):** Create a deterministic script bundling `include/ax/*.hpp` in topological dependency order (`config.hpp` → `telemetry.hpp` → `topology.hpp` → `memory.hpp` → `vram.hpp` / `igpu.hpp` / `audio.hpp`) and attach `ax_sdk.hpp` to tagged GitHub releases.

### Part 2: `Eden-Espresso-Wave-AX` Engine & Tooling
* `[ ]` **5. Wire GPU VRAM, BAR1 & QSV Counters into `ax::GetTelemetry()` (`vk_rasterizer.cpp`, `vk_staging_buffer_pool.cpp`, `qsv_decoder.cpp`):** Populate `vram_used_bytes`, `vram_reaps`, `bar1_stalls`, `igpu_offloads`, and `qsv_frames_decoded` so `track_telemetry.ps1` streams live GPU/iGPU metrics from `Local\AX_Telemetry`.
* `[ ]` **6. L3 Cache ID (CCX) Grouping (`ax-sdk/src/topology.cpp`):** Group logical processors by Windows `RelationCache` (`Level == 3`) and Linux `cache/index3/shared_cpu_list` so `AudioTiming`, `VulkanSubmission`, and `GuestJit` stay pinned within a single L3 slice on multi-CCX Ryzen CPUs.
* `[ ]` **7. Re-Integrate Proactive `5.00 GiB` Async `VkVramShield` + Rolling VBLANK Slack (`vk_vram_shield.hpp`, `vk_rasterizer.cpp`):** Re-enable the `5.00 GiB` (`kPruneTriggerBytes`) async `VK_QUEUE_TRANSFER_BIT` reaper (`~4.50 GiB` hysteresis target) ahead of the `5.50 GiB` (`5.35 GiB` LSFG) ceiling with `RecordFrameCompletion(uint64_t gpu_time_ns)` and runtime `synchronization2` capability gating.
* `[ ]` **8. `256-Byte` Aligned Persistent `BAR1` Ring & Size-Class Texture Sub-Allocator (`vk_staging_buffer_pool.cpp`, `vulkan_memory_allocator.cpp`):** Enforce Maxwell `GM200`'s `256-byte` `minUniformBufferOffsetAlignment` bump-allocation inside the `128 MiB BAR1` ring and group textures into power-of-two size-class slabs (`64 KiB`–`16 MiB`).
* `[ ]` **9. Cross-GPU QSV / VA-API Video Offload (`6.2`) & `64 KB` Sparse VMA Pool (`5.2`):** Wire `QSVHardwareDecoderRing` (`VK_KHR_external_memory_win32` + `VK_API_VERSION_1_3` UHD 630 check) and `vaapi_dmabuf_bridge.hpp`, and back dynamic render targets with a `64 KB` Sparse VMA pool.
* `[ ]` **10. Stage A vs. Stage B PGO Bake & `i5-8600K` Validation:** Run `3 × 15 min` gameplay sweeps (`PGO_stage1.bat`), pass `validate_profile.ps1`, bake `PGO_stage2.bat`, and log diff against Stage A `baseline_pgo_metrics.json`.

---

## 🔥 Sprint `v1.37` — UHD 630 FP16 `ASTC → BC7` Transcode, `ax-sdk` Lifecycle/CI & `Piccolo-AX`

*Objective: Deploy hardware-accelerated `ASTC → BC7` 4:1 texture transcoding on Intel UHD 630, finalize `ax-sdk` `v1.37.0` lifecycle/unit-test infrastructure (`AX-1.5` → `AX-1.6`), and bring up the bare-metal `Piccolo-AX` Linux appliance.*

### Part 1: `ax-sdk` Repository (`galaxywarriordad/ax-sdk` → `v1.37.0`)
* `[ ]` **1. `AX-1.5` Explicit `ax::Context` & Lifecycle API (`include/ax/ax.hpp`, `src/ax.cpp`):** Replace implicit static initialization with deterministic `ax::Status ax::Init(const ax::Config&)` and `void ax::Shutdown()`.
* `[ ]` **2. `AX-1.6` Hardware-Mock Unit & Concurrency CI Suite (`tests/`, `.github/workflows/ci.yml`):** Abstract CPU/GPU topology queries behind injectable mock descriptors and build CMake `CTest` suites (`test_audio_ring.cpp`, `test_topology.cpp`, `test_vram_shield.cpp`) across Windows (`clang-cl`) and Linux (`Clang`/`GCC`) GitHub Actions runners.

### Part 2: Engine, Transcoding & `Piccolo-AX` Appliance (`v1.37.0`)
* `[ ]` **3. Zero-Copy `VK_KHR_external_memory_win32` / `dma_buf` UHD 630 FP16 (`24 EUs`) `ASTC → BC7 / BC6H` Compute Transcoder (`5.3`):** Dispatch async `ASTC → BC7` block encoding onto Intel UHD 630's 24 native `2:1` FP16 EUs and transfer directly into `GTX 980 Ti` VRAM (`<= 3.50 GB/s` ring-bus cap) without a system-RAM bounce.
* `[ ]` **4. Batched Guest SMC Page-Protection Invalidations (`ax::memory`):** Batch `VirtualProtect` / `mprotect` updates to eliminate 4 KB page-fault storms during self-modifying code flushes.
* `[ ]` **5. `Piccolo-AX` Sprint 0–2 Bare-Metal Bring-Up & Cross-App USB DAC Arbitration (`/run/lock/ax-dac.lock`):** Boot Linux `6.12-rt` + pinned NVIDIA `580.*` driver inside `< 300 MB` `lz4hc,9` `toram` EROFS image, verify via `ax-sdk/tools/ax_doctor.sh`, coordinate `flock()` ownership across `Eden-AX`, `Cemu-AX`, and `FLAC_AX` on `hw:CARD=FC3,DEV=0`, and bring up `Cemu-Latte-AX`.
* `[ ]` **6. Kiosk & Appliance Hardening (`7.1` – `7.4`):** Wire direct-boot ROM path checks, borderless exclusive full-screen startup, controller profile freeze, write-protected virtual NAND sandboxing, and `[AX]` branding sanitization.
