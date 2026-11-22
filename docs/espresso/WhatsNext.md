# 🧠 AI Context Handover & Sprint Gate — The Silicon Preservation Society (`[AX]`)

**Programme Banner:** [The Silicon Preservation Society](https://ko-fi.com)[cite: 1]  
**Active Baseline:** `Eden-Espresso-Wave-AX` `v1.35.1` + `ax-sdk` `v1_36_1` (`v1.36.1`) → `v1.37.0`[cite: 1]  
**Target SDK:** `galaxywarriordad/ax-sdk` (`ax::core` `v1_36_1`)[cite: 1]  
**Last Updated:** `2026-11-21` (Handover baseline; `WhatsNext.md` updated `2026-11-20`)[cite: 1]

---

## 1. Mission & Core Engineering Invariant
- **Mission:** Keep proven `x86_64` PC hardware out of landfills by building a deterministic, zero-bloat bare-metal gaming & audio/visual appliance (`Piccolo [AX]`) + Windows 10/11 desktop reference engine (`Eden Espresso Wave [AX]`).
- **Foundational Principle:** *"Make one variable a constant, then you have a base to work upon."* Hardware topology, IRQ routing, clock domains, and VRAM ceilings are fixed constants so PGO profiles and telemetry deltas reflect true code performance.

---

## 2. Reference Silicon & Bus Topology (Stream 1)
- **CPU:** Intel Core `i5-9600K` (6C/6T, Zero SMT, locked `x48` / `4.8 GHz`, `AVX2` + `BMI1/2`, no `AVX-512`), 32 GB DDR4-3600.
- **2:3:1 Physical Core Partition (`ax::topology`):**
  - **Core 0 (`0x01`):** Fused Audio & Timing (`SCHED_FIFO 80–99` / Win32 `MMCSS Pro Audio`, USB xHCI IRQ).
  - **Core 1 (`0x02`):** Primary Vulkan / OpenGL Command Submission (`GTX 980 Ti` MSI IRQ).
  - **Cores 2–4 (`0x1C`):** Tickless Guest CPU JIT (`nohz_full`, `isolcpus`) & AVX2 Polyphonic DSP.
  - **Core 5 (`0x20`):** Housekeeping Sink (`kworker`s, `rcu_nocbs`, SATA/NVMe IRQs, ZSTD L18, QSV/VA-API, RAM FLAC decoding, Akai MPK Mini MK3 RawMIDI).
- **Dual-GPU Die:**
  - **dGPU:** NVIDIA `GTX 980 Ti` (`GM200` Maxwell, `6 GB` GDDR5, Driver `582.78 WHQL` Win10 / `580.*` Linux, Vulkan `1.4` core). Uses `5.00 GiB` async VRAM pruning (`VkVramShield` + `VK_QUEUE_TRANSFER_BIT` + Vulkan `1.4` split barriers) ahead of `5.50 GiB` (`5.35 GiB` `lsfg-vk`) ceiling, `128 MiB` BAR1 staging cap, and `FP16 -> FP32` shader promotion.
  - **iGPU:** Intel `UHD Graphics 630` (`24 EUs`, native `2:1` FP16, `VDBOX`, `VEBOX`, `VK_API_VERSION_1_3` capped). Handles video decode (`NV12` `dma_buf` / NT Shared Handle, `<= 3.50 GB/s` ring-bus cap) and `ASTC -> BC7/BC6H` transcode.
- **Split-USB Audio & MIDI:**
  - **HiBy FC3 USB DAC** (`ESS Sabre ES9281AC Pro`, `hw:CARD=FC3,DEV=0`, `S32_LE`, `96`-frame / `2.00 ms` ALSA period) on rear Z390 PCH root hub (`Core 0` IRQ).
  - **Akai MPK Mini MK3** (`hw:CARD=MPKmini3,DEV=0`, 25 keys Ch 1, 8 MPC pads Ch 10, `K1–K8` `CC 70–77`, thumbstick, sustain `CC 64`) on secondary/front port (`Core 5` RawMIDI).

---

## 3. Seven-Repository Network (`galaxywarriordad/*`)
1. **`ax-sdk`** (`MIT`, `v1_36_1`): Zero-dependency C++20 core (`config`, `telemetry` `Local\AX_Telemetry` / `/dev/shm/ax_telemetry`, `topology`, `memory`, `vram`, `igpu`, `audio` `LockFreeStereoRing`) + `hardware_profiles.json` (`v2.1`) + shared `tools/`.
2. **`Eden-Espresso-Wave-AX`** (Forgejo primary + GitHub mirror, `GPL-3.0`, `v1.35.1`): Switch engine master, 3-stage Clang 23.1+ ThinLTO + PGO (`PGO_config.bat`, `PGO_stage0/0I/0LD/1/2.bat`), `telemetry_osd.cpp` (`VK_QUERY_TYPE_PIPELINE_STATISTICS`).
3. **`Piccolo-AX`** (`GPL-3.0`): `< 512 MB` `toram` EROFS (`lz4hc,9`) `Linux 6.12-rt` Buildroot `BR2_EXTERNAL` (`batocera-ax-external/`) + `axGenerator.py` (`SIGSTOP` ES, `vmtouch`, `/run/lock/ax-dac.lock` + `SIGUSR1` DAC handoff).
4. **`Generation-X-AX`** (`GPL-3.0`): Unified C++20 `ax_retro_bridge` (`EngineId 0..6`) for 7 classic engines: `RMG` (N64), `BlastEm` (Mega Drive), `Cannonball` (OutRun), `EDuke32`, `Frontier: Elite II`, `Flycast` (Dreamcast/NAOMI/Atomiswave), and `Hatari` (Atari ST/STE/TT/Falcon).
5. **`Cemu-Latte-AX`** (`MPL-2.0`): Nintendo Wii U fork wired to `ax::core` (`v1_36_1`).
6. **`FLAC_AX`** (`MIT`): Bit-perfect ESS Sabre player (`flac_ax`, `WholeAlbum`/`Window3Files`/`SingleFile` RAM preloader) + live A/V Studio (`ax_av_studio`: `Core 5` MPK Mini MK3 MIDI -> `Cores 2–4` 16-voice saw+sub synth, SVF filter & `1024`-pt Hann FFT -> `Core 0` ALSA + `Core 1` VBlank GTX 980 Ti OpenGL HUD).
7. **`piccolo-ax-desktop`** (`GPL-3.0`): Opt-in (`default n`) `< 30 MB` C/C++ desktop & Amiga-inspired pixel-art/diagnostics suite (`LXQt`/`XFCE`, `Falkon`, `AbiWord`, `Gnumeric`, `Foliate`, `mtPaint`, `GrafX2`, `AzPainter`, `LibreSprite`, `GHex`, `feh`, `scrot`, `CPU-X`, `nvtop`) with `SHARE` (`/userdata/desktop`) XDG persistence.

---

## 4. Immediate Gate: `v1.35.1` + `ax-sdk v1_36_1` Smoke Test & Git Release (`POST-REBOOT`)

*Objective: Push the local `Eden-Espresso-Wave-AX` `v1_36_1` integration, verify a clean `Stage 0LD` build with live Shared-Memory Telemetry (`Local\AX_Telemetry`), and tag `v1.35.1` before moving on to `v1.37` features.*[cite: 1]

### 4.1 Completed Code, SDK & Tooling Deliverables
* `[✓]` **Maxwell `GM200` Shader Recompiler & Log Class Fix (`src/shader_recompiler/frontend/maxwell/decode.cpp`):** Fixed `LOG_DEBUG(Shader, ...)` log class compilation and resolved shader decoding stalls (`Loading Shaders 0 / 4`)[cite: 1].
* `[✓]` **Memory Manager & Staging Pool Hardening (`memory_manager.cpp`, `vulkan_device.cpp`, `vk_staging_buffer_pool.cpp`):** Enforced Maxwell `Arch_Maxwell` (`FP16 → FP32` promotion) and bounded the `HOST_VISIBLE` BAR1 staging pool to **`128 MiB`** (`128_MiB BAR1`) to prevent PCIe aperture exhaustion on 6 GB cards[cite: 1].
* `[✓]` **Fresh Config `LibraryApplet` HLE Defaults (`src/common/settings.h`):** Switched `player_select_applet_mode` and `mii_edit_applet_mode` default values from `AppletMode::LLE` to **`AppletMode::HLE`** (`Custom frontend`) so clean installs boot *Mario Kart 8 Deluxe* profile/Mii selection without silent firmware hangs[cite: 1].
* `[✓]` **Configurator Hub IRQ Status Parser (`PGO_config.bat`):** Replaced parenthesised `echo %%A | findstr` pipes with direct `findstr /C:"Audio/Timing"`, `"Vulkan Submit"`, and `"Aux Quarantine"` file lookups against `%HW_CHECK_LOG%`[cite: 1].
* `[✓]` **Live Runtime Triage Filter (`Scripts\tail_eden_log.ps1`):** Created `[TL]` color-coded log tailer filtering `Arch_Maxwell`, `BAR1`, `VramShield`, `ax::`, `topology`, `underrun`, `Loading Shaders`, `Applet`, and `<Error>/<Critical>` events in real time[cite: 1].
* `[✓]` **Zero-Pollution Shader Validation (`Scripts\shader_validate.ps1`):** Routed `glslangValidator` output to `$env:TEMP\eden_validate.spv` with post-loop cleanup so `frag.spv` is never dropped in the repository root[cite: 1].
* `[✓]` **Script Workspace Hygiene (`Scripts\dev_tools\` & `ax-sdk/tools/`):** Isolated legacy helper scripts into `Scripts\dev_tools\`, delegated shared host scripts (`Get-AxProfile.ps1`, `check_hardware.ps1`, `set_irq_affinity.ps1 -Revert`, `track_telemetry.ps1`, `check_regression.ps1`, `clean_all.ps1 -Wait`, and Linux `ax_doctor.sh`) to `ax-sdk/tools/` via `Scripts\resolve_ax_tools.bat`, and fixed `Add-Type -MemberDefinition` window positioning across `PGO_stage*.bat`[cite: 1].
* `[✓]` **`ax-sdk` `v1_36_1` Release & Consumer Wiring (`AX-1.1` – `AX-1.3`):** Published `ax-sdk` `v1_36_1` with `ax::config` (`ax_engine.ini`), target-scoped `CMakeLists.txt` (`cxx_std_20`), cross-process Shared-Memory Telemetry (`Local\AX_Telemetry` / `/dev/shm/ax_telemetry`), and lock-free microsecond audio jitter histograms (`ax::RecordAudioCallbackMetrics`); wired into `Eden` (`sink_stream.cpp`, `flex_asio_manager.cpp`) and `FLAC_AX` (`src/main.cpp`), and aligned all 5 repositories (`ax-sdk`, `Eden-Espresso-Wave-AX`, `FLAC_AX`, `Cemu-Latte-AX`, `Piccolo-AX`) to `v1_36_1`[cite: 1].

### 4.2 Remaining Release Gate (`POST-REBOOT`)
* `[ ]` **Step A — Finalize & Push Local `Eden-Espresso-Wave-AX` (`C:\Users\nick_\Desktop\eden`):** Remove temporary `build_stage0_triage.log` and duplicate `Scripts\set_irq_affinity.ps1`, verify `Scripts\resolve_ax_tools.bat`, and run `git add -A && git commit && git push`[cite: 1].
* `[ ]` **Step B — Clean `[0LD]` Build (`build_stage0\bin\eden.exe`):** Run `PGO_config.bat` → `0LD` to pull `ax-sdk` (`v1_36_1`) via `FetchContent` and compile `build_stage0\bin\eden.exe` cleanly[cite: 1].
* `[ ]` **Step C — Single-Window `[TL]` + Shared-Memory Telemetry Smoke Test (*Mario Kart 8 Deluxe* + `HiBy FC3` DAC):** Select `TL` in `PGO_config.bat` (auto-running `reset_eden_config.ps1`), verify `Arch_Maxwell` (`FP16 → FP32`), `128_MiB BAR1`, HLE Applets, `"TelemetrySource": "SharedMemory"` in `build_stage_telemetry_metrics.json`, `0` errors, and `0` audio underruns, then tag `v1.35.1`[cite: 1].

---

## 5. Sprint `v1.36` — Remaining `ax-sdk` Polish, Engine Memory/QSV & Telemetry-Guided PGO

*Objective: Complete remaining `ax-sdk` single-header amalgamation (`AX-1.4`), wire live GPU VRAM/QSV counters into `ax::GetTelemetry()`, re-integrate the proactive `5.00 GiB` VRAM shield, frame-fenced BAR1 recycling, and cross-GPU QSV/VA-API video offload, and gate PGO sweeps on telemetry.*[cite: 1]

### 5.1 `ax-sdk` Repository (`galaxywarriordad/ax-sdk`)
* `[✓]` **1. `AX-1.1` Unified Settings & Platform Macros (`include/ax/config.hpp`, `src/config.cpp`):** Implemented `ax::LoadEngineConfig` / `ax::WriteDefaultEngineConfig` (`ax_engine.ini`) and standardized `AX_PLATFORM_*` / `AX_HAS_*` macros[cite: 1].
* `[✓]` **2. `AX-1.2` CMake Target-Scope & Hygiene Refactor (`CMakeLists.txt`):** Replaced global C++ standard mutation with `target_compile_features(ax_core ... cxx_std_20)`, `Threads::Threads`, private Win32 macro scoping, and opt-in `AX_SDK_ENABLE_WARNINGS`[cite: 1].
* `[✓]` **3. `AX-1.3` Shared-Memory Atomic Telemetry & Audio Jitter Profiler (`include/ax/telemetry.hpp`, `src/telemetry.cpp`):** Implemented 64B-padded `TelemetryCounters` (`Local\AX_Telemetry` / `/dev/shm/ax_telemetry`) and lock-free `min / P99 / max` microsecond audio callback histograms (`ax::RecordAudioCallbackMetrics`)[cite: 1].
* `[ ]` **4. `AX-1.4` Single-Header Amalgamation Generator (`tools/amalgamate.ps1`, `.github/workflows/release.yml`):** Create a deterministic script bundling `include/ax/*.hpp` in topological dependency order (`config.hpp` → `telemetry.hpp` → `topology.hpp` → `memory.hpp` → `vram.hpp` / `igpu.hpp` / `audio.hpp`) and attach `ax_sdk.hpp` to tagged GitHub releases[cite: 1].

### 5.2 `Eden-Espresso-Wave-AX` Engine, PGO & Tooling
* `[ ]` **5. Wire GPU VRAM, BAR1 & QSV Counters into `ax::GetTelemetry()` (`vk_rasterizer.cpp`, `vk_staging_buffer_pool.cpp`, `qsv_decoder.cpp`):** Populate `vram_used_bytes`, `vram_reaps`, `bar1_stalls`, `igpu_offloads`, and `qsv_frames_decoded` so `track_telemetry.ps1` streams live GPU/iGPU metrics from `Local\AX_Telemetry`[cite: 1].
* `[ ]` **6. L3 Cache ID (CCX) Grouping (`ax-sdk/src/topology.cpp`):** Group logical processors by Windows `RelationCache` (`Level == 3`) and Linux `cache/index3/shared_cpu_list` so `AudioTiming`, `VulkanSubmission`, and `GuestJit` stay pinned within a single L3 slice on multi-CCX Ryzen CPUs[cite: 1].
* `[ ]` **7. Re-Integrate Proactive `5.00 GiB` Async `VkVramShield` + Rolling VBLANK Slack (`vk_vram_shield.hpp`, `vk_rasterizer.cpp`):** Re-enable the `5.00 GiB` (`kPruneTriggerBytes`) async `VK_QUEUE_TRANSFER_BIT` reaper (`~4.50 GiB` hysteresis target) ahead of the `5.50 GiB` (`5.35 GiB` LSFG) ceiling with `RecordFrameCompletion(uint64_t gpu_time_ns)` and runtime `synchronization2` capability gating[cite: 1].
* `[ ]` **8. `256-Byte` Aligned Persistent `BAR1` Ring & Size-Class Texture Sub-Allocator (`vk_staging_buffer_pool.cpp`, `vulkan_memory_allocator.cpp`):** Enforce Maxwell `GM200`'s `256-byte` `minUniformBufferOffsetAlignment` bump-allocation inside the `128 MiB BAR1` ring and group textures into power-of-two size-class slabs (`64 KiB`–`16 MiB`)[cite: 1].
* `[ ]` **9. Cross-GPU QSV / VA-API Video Offload (`6.2`) & `64 KB` Sparse VMA Pool (`5.2`):** Wire `QSVHardwareDecoderRing` (`VK_KHR_external_memory_win32` + `VK_API_VERSION_1_3` UHD 630 check) and `vaapi_dmabuf_bridge.hpp`, and back dynamic render targets with a `64 KB` Sparse VMA pool[cite: 1].
* `[ ]` **10. Closed-Loop Telemetry-Guided PGO & Stage A vs. Stage B Bake:** Gate Stage 1 LLVM `-fprofile-generate` (`__llvm_profile_reset_counters()` / `__llvm_profile_dump()`) on live `VK_QUERY_TYPE_PIPELINE_STATISTICS` and `Local\AX_Telemetry` gameplay flags so cold menus never dilute `default.profdata`; run `3 × 15 min` gameplay sweeps (`PGO_stage1.bat`), pass `validate_profile.ps1`, bake `PGO_stage2.bat`, and log diff against Stage A `baseline_pgo_metrics.json`[cite: 1].

---

## 6. Sprint `v1.37` — UHD 630 FP16 `ASTC → BC7` Transcode, `ax-sdk` Lifecycle/CI, Hardware/DSP & `Piccolo-AX`

*Objective: Deploy hardware-accelerated `ASTC → BC7` 4:1 texture transcoding on Intel UHD 630, finalize `ax-sdk` `v1.37.0` lifecycle/unit-test infrastructure (`AX-1.5` → `AX-1.6`), complete `FLAC_AX` / *Frontier: Elite II* hardware/DSP polish, and bring up the bare-metal `Piccolo-AX` Linux appliance.*[cite: 1]

### 6.1 `ax-sdk` Repository (`galaxywarriordad/ax-sdk` → `v1.37.0`)
* `[ ]` **1. `AX-1.5` Explicit `ax::Context` & Lifecycle API (`include/ax/ax.hpp`, `src/ax.cpp`):** Replace implicit static initialization with deterministic `ax::Status ax::Init(const ax::Config&)` and `void ax::Shutdown()`[cite: 1].
* `[ ]` **2. `AX-1.6` Hardware-Mock Unit & Concurrency CI Suite (`tests/`, `.github/workflows/ci.yml`):** Abstract CPU/GPU topology queries behind injectable mock descriptors and build CMake `CTest` suites (`test_audio_ring.cpp`, `test_topology.cpp`, `test_vram_shield.cpp`) across Windows (`clang-cl`) and Linux (`Clang`/`GCC`) GitHub Actions runners[cite: 1].

### 6.2 Engine, Transcoding, Hardware/DSP & `Piccolo-AX` Appliance (`v1.37.0`)
* `[ ]` **3. Zero-Copy `VK_KHR_external_memory_win32` / `dma_buf` UHD 630 FP16 (`24 EUs`) `ASTC → BC7 / BC6H` Compute Transcoder (`5.3`):** Dispatch async `ASTC → BC7` block encoding onto Intel UHD 630's 24 native `2:1` FP16 EUs and transfer directly into `GTX 980 Ti` VRAM (`<= 3.50 GB/s` ring-bus cap) without a system-RAM bounce[cite: 1].
* `[ ]` **4. Batched Guest SMC Page-Protection Invalidations (`ax::memory`):** Batch `VirtualProtect` / `mprotect` updates to eliminate 4 KB page-fault storms during self-modifying code flushes[cite: 1].
* `[ ]` **5. `FLAC_AX` & *Frontier: Elite II* Hardware/DSP Polish:** Add native `.dsf`/`.dff` DoP (`DSD64`/`DSD128`) + `snd_mixer` ES9281AC Pro hardware volume + 8-wide `_mm256_fmadd_ps` AVX2 synth vectorization + Akai MPK Mini MK3 SysEx OLED parameter labels in `FLAC_AX`, plus *Frontier: Elite II* MPK3 cockpit controls.
* `[ ]` **6. `Piccolo-AX` Sprint 0–2 Bare-Metal Bring-Up & Cross-App USB DAC Arbitration (`/run/lock/ax-dac.lock`):** Boot Linux `6.12-rt` + pinned NVIDIA `580.*` driver inside `< 300 MB` `lz4hc,9` `toram` EROFS image, verify via `ax-sdk/tools/ax_doctor.sh`, coordinate `flock()` ownership across `Eden-AX`, `Cemu-AX`, and `FLAC_AX` on `hw:CARD=FC3,DEV=0`, and bring up `Cemu-Latte-AX`[cite: 1].
* `[ ]` **7. Kiosk & Appliance Hardening (`7.1` – `7.4`):** Wire direct-boot ROM path checks, borderless exclusive full-screen startup, controller profile freeze, write-protected virtual NAND sandboxing, and `[AX]` branding sanitization[cite: 1].

---

## 7. Cross-Repo Build & CI Verification Queue
- **Windows `Eden-Espresso-Wave-AX`:** Clean `PGO_stage0LD.bat` build (`build_stage0\bin\eden.exe`) and telemetry-gated `PGO_stage1.bat` / `PGO_stage2.bat` sweep.
- **Ubuntu VM CMake/Ninja Builds:** Verify `Generation-X-AX` (all 7 engines: `RMG`, `BlastEm`, `Cannonball`, `EDuke32`, `Frontier: Elite II`, `Flycast`, `Hatari`) and `FLAC_AX` (`flac_ax` & `ax_av_studio`).
- **Offline Buildroot Hermeticity (`Piccolo-AX`):** Verify `FETCHCONTENT_SOURCE_DIR_AX_SDK` offline hermeticity and add `eduke32-ax.mk` to `Piccolo-AX`.