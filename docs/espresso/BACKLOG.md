# 🗂️ Eden Espresso Wave [AX] & Piccolo [AX] — Master Product & Engineering Backlog

**Programme Banner:** [The Silicon Preservation Society](https://ko-fi.com)  
**Current Baseline:** Master `v1.35.1` / `ax-sdk` `v1_36_1` | **Unified Plan:** `v1.4` | **Piccolo Spec:** `v0.6` (Sprint 0–5) | **Last Updated:** `2026-11-21`

**Workflow:** Active sprint gate tasks are tracked in `WhatsNext.md`. Completed items move to `CHANGELOG.md`. All Stage A/Stage B baselines, `v1.35.0–v2.00` releases, UHD 630 silicon offloads, Piccolo appliance milestones, 7-engine classic expansion (`Generation-X-AX`), A/V Studio (`FLAC_AX`), opt-in desktop workbench (`piccolo-ax-desktop`), and Linux-first multi-emulator expansion (`ax-sdk`) are organized below.

---

## 🖥️ Remaining Actions to Set Up `piccolo-ax-desktop` (Priority Top)

* `[✓]` **Wire `piccolo-ax-desktop` Into `Piccolo-AX` Buildroot (`Config.in`, `external.mk`, `post-build.sh`):** Added opt-in `BR2_PACKAGE_PICCOLO_AX_DESKTOP` (`default n`), conditional `openbox*` preservation, and `/usr/local/bin` + `/etc/profile.d` workbench launcher installation in `Piccolo-AX/batocera-ax-external/`.
* `[ ]` **Finalize Root `topology.md` & Clean Up Sections:** Confirm the `#` title, intro paragraph, Part 1 (Compute/Session diagram, Repository & Delivery tree, Boundary Invariants), and Part 2 (Goals/Non-Goals, Tier A/Tier B package tiers, State/Environment/Launcher flow, GPLv3 governance) are intact at the root with consistent heading levels (`#` > `##` > `###` > `####`) and no duplicate blocks.
* `[ ]` **Add Remaining Launcher & Staging Scripts (`scripts/`):** Commit `scripts/launch-falkon.sh` (initializing `SHARE`/XDG environment and handling `QtWebEngine` root sandbox flags) and `scripts/stage-share-apps.sh` (creating `${AX_SHARE_ROOT:-/userdata}/desktop/apps` and staging/marking executable AppImages or portable tools) alongside `scripts/launch-desktop.sh`.
* `[ ]` **Verify Profiles & XDG Overlay:** Confirm `profiles/desktop-core.mk`, `profiles/desktop-heavy.mk`, and `overlays/common/etc/profile.d/ax-desktop-xdg.sh` export `XDG_CONFIG_HOME`, `XDG_DATA_HOME`, and `XDG_CACHE_HOME` under `${AX_SHARE_ROOT:-/userdata}/desktop` and provision `xdg-config`, `xdg-data`, `xdg-cache`, `apps`, `docs`, `books`, and `src` when `SHARE` is mounted.
* `[ ]` **Draft `docs/packages.md` & `buildroot/README.md`:** Document Tier A (`LXQt`/`XFCE`, **Falkon**, **AbiWord**, **Gnumeric**, **Foliate**, lightweight PDF viewer, `git`, terminal/archive tools) vs. Tier B (`GIMP`, `Krita`, `Shotcut`, `HandBrake`, `appimage-share` vs. `package` delivery) and upstream license attribution, and document external layer/Buildroot integration notes.
* `[ ]` **Run Local Shell & Sandboxed `SHARE` / XDG Checks:** Run `sh -n` across `ax-desktop-xdg.sh` and all `scripts/*.sh`, and test with a temporary `AX_SHARE_ROOT` path to verify directory and XDG variable setup without touching `/userdata` or rootfs.
* `[ ]` **Validate Desktop Layer on Target Hardware (Bootloader Last):** Validate desktop startup and `SHARE` persistence on target hardware while keeping default real-time appliance mode isolated and `EROFS` immutable before adding bootloader (`grub.cfg` / session flag) entries.

---

## 🏃 Next Sprint & Current Focus

**`v1.34` Bare-Metal Gate Confirmation → `v1.35.1` / `ax-sdk v1_36_1` (Shared-Memory Telemetry, 7-Engine Classic Suite, A/V Studio & Bare-Metal Gate)**

### Milestone Checklist (Reconciled With `WhatsNext.md`)
* `[✓]` **Milestone 1 — Environment & Core Partitioning:** `clang-cl 23.1+`, `Vulkan 1.4 SDK`, `FFmpeg 7.1.x` LTS, `0x01`/`0x02`/`0x1C`/`0x20` core masks, and IRQ steering configured.
* `[✓]` **Milestone 2 — `PGO_stage0` Build & Pipeline Automation:** `build_stage0\bin\eden.exe` compiled and linked cleanly; `Scripts\macro_update.ps1`, `Scripts\shader_validate.ps1`, and live IRQ menu audits integrated across all PGO stage batch scripts.
* `[✓]` **Milestone 2.5 — 7-Repository Polyrepo Topology & `ax-sdk` (`v1_36_1`) Consolidation:** Established and aligned `Eden-Espresso-Wave-AX`, `ax-sdk` (`v1_36_1` tagged), `Piccolo-AX`, `Cemu-Latte-AX`, `Generation-X-AX`, `FLAC_AX`, and `piccolo-ax-desktop` repositories; aggregated shared host tooling into `ax-sdk/tools/` (`Get-AxProfile.ps1`, `check_hardware.ps1`, `set_irq_affinity.ps1 -Revert`, `track_telemetry.ps1`, `check_regression.ps1`, `clean_all.ps1 -Wait`, and Linux `ax_doctor.sh`).
* `[✓]` **Milestone 2.6 — Async `5.00 GiB` Vulkan 1.4 VRAM Shield, Dynamic `2:3:1` Topology, Unified Settings & Shared-Memory Telemetry:** Implemented `ax::vram` (`5.00 GiB` async prune trigger + `~4.50 GiB` hysteresis), `VkVramShield`, `ax::topology`, `ax::config` (`ax_engine.ini`), and 64B-padded cross-process shared-memory telemetry (`Local\AX_Telemetry` / `/dev/shm/ax_telemetry`) + microsecond audio jitter profiler (`ax::RecordAudioCallbackMetrics`) wired into `Eden` (`sink_stream.cpp`, `flex_asio_manager.cpp`), `Generation-X-AX` (`ax_retro_bridge`), and `FLAC_AX`.
* `[✓]` **Milestone 2.7 — `Generation-X-AX` 7-Engine Unification & `FLAC_AX` Audio/Visual Studio (`ax_av_studio`):**
  * Imported `Flycast` (Dreamcast/NAOMI/Atomiswave) and `Hatari` (Atari ST/STE/TT/Falcon) into `Generation-X-AX` alongside `RMG`, `BlastEm`, `Cannonball`, `EDuke32`, and `Frontier: Elite II`; wired `ax_flycast_hook` and `ax_hatari_hook` onto `ax_retro_bridge` (`EngineId 0..6`), packaged `flycast-ax` and `hatari-ax` into `Piccolo-AX`, and mapped `dreamcast`, `naomi`, `atomiswave`, and `atarist` in `axGenerator.py`.
  * Built `ax_av_studio` inside `FLAC_AX` (`Core 5` Akai MPK Mini MK3 ALSA RawMIDI listener → `Cores 2–4` 16-voice saw+sub synthesizer, Chamberlin 2-pole SVF filter & `1024`-pt Hann FFT tap → `Core 0` `2.00 ms` `SCHED_FIFO 80` `S32_LE` ESS Sabre ALSA output + `Core 1` VBlank-locked GTX 980 Ti OpenGL Studio HUD), packaged `flac-ax` into `Piccolo-AX`, and wired `/run/lock/ax-dac.lock` + `SIGUSR1` DAC handoff in `axGenerator.py`.
* `[★]` **CURRENT POSITION — Milestone 3: `PGO_stage0LD` Build Verification, VM Compile Gate & Stage A / Stage B Bare-Metal Gate:**
  * Run clean Ninja test-builds of `Generation-X-AX` (`flycast`, `hatari`, and 5 classic engines) and `FLAC_AX` (`flac_ax` + `ax_av_studio`) on Ubuntu VM to verify zero header/linker friction before Buildroot image assembly.
  * Complete Stage 0LD (`PGO_stage0LD.bat`) clean build verification with `ax::core` (`v1_36_1`) linked and `Local\AX_Telemetry` active.
  * Compile & log **Stage A Vanilla Control Baseline** (`Clang 23.1+` 3-Phase PGO + `Vulkan 1.4` + `FFmpeg 7.1.x`, zero AX code) → `Logs\baseline_pgo_metrics.json`.
  * Complete Stage 0 runtime triage (`0x01`/`0x02`/`0x1C`/`0x20` affinity verification + `audio_underruns: 0` + live shared-memory jitter metrics).
* `[⏳]` **Milestone 4 — Stage 1 & Stage 2 PGO Bake (Telemetry-Guided PGO):** Run `3 × 15 min` sandboxed gameplay sweeps (`LLVM_PROFILE_FILE="eden_%p_%m.profraw"`), qualify active gameplay windows via `Local\AX_Telemetry`, pass `Scripts\validate_profile.ps1` (`EXCELLENT_STREAM_READY`), and bake final L1i binary (`PGO_stage2.bat`) diffed against Stage A.
* `[⏳]` **Milestone 5 — Piccolo Sprint 0 Bare-Metal Gate Cleared:** Confirm BIOS `IGD Multi-Monitor: Enabled`, boot Linux `6.12-rt` + pinned NVIDIA `580.*` Live USB bench, run `ax-sdk/tools/ax_doctor.sh`, confirm `rdmsr 0xc8f` Intel CAT status, verify `0` `LOC` ticks on Cores `0–4`, confirm `0` ALSA xruns (`2.00 ms` on `hw:CARD=FC3,DEV=0` with HiBy FC3 and Akai MPK Mini MK3 on separate USB root hubs), and validate UHD 630 (`VK_API_VERSION_1_3`) `NV12` `dma_buf` ingestion.

---

## 🛠️ Tooling, Build Pipeline & Repo Hygiene Backlog (`2026-11-21`)

* `[✓]` **Synchronous Cleanup Gate (`ax-sdk/tools/clean_all.ps1 -Wait`):** Added `-Wait` parameter to `clean_all.ps1` and wired it into `PGO_config.bat`, `PGO_stage0LD.bat`, and `PGO_stage1.bat` so `.cache` and `build_stage*` purges complete deterministically before CMake configuration begins.
* `[✓]` **Configurator Window Positioning Fix (`Scripts\PGO_stage*.bat`):** Switched `Add-Type -TypeDefinition $w_code -Name WWindowUtil` to `-MemberDefinition $w_code -Name WWindowUtil` across `PGO_stage0.bat`, `PGO_stage0LD.bat`, and `PGO_stage1.bat` to eliminate the `AmbiguousParameterSet` warning on launch.
* `[✓]` **Local `ax-sdk` Dev Override & Tag Pinning (`CMakeLists.txt`):** Added automatic `AX_SDK_DIR` and `../ax-sdk` resolution ahead of `FetchContent_Declare(ax_sdk ... GIT_TAG v1_36_1 GIT_SHALLOW FALSE)` across `Eden`, `Cemu-Latte-AX`, `Generation-X-AX`, and `FLAC_AX`.
* `[✓]` **Aggregated Cross-App Tooling & Config Decoupling (`ax-sdk/tools/` & `ax_engine.ini`):** Extracted shared scripts (`Get-AxProfile.ps1`, `check_hardware.ps1`, `set_irq_affinity.ps1 -Revert`, `track_telemetry.ps1`, `check_regression.ps1`, `clean_all.ps1`, `ax_doctor.sh`) into `ax-sdk/tools/` via `Scripts\resolve_ax_tools.bat`, and decoupled personal drive checks (`J:\Switch`) and DAC names into `ax_engine.ini` / `hardware_profiles.json`.
* `[✓]` **Opt-In Safety Guards & Reversible IRQ Rollback (`set_irq_affinity.ps1 -Revert`):** Added `-Revert` support to `set_irq_affinity.ps1` to restore stock Windows PnP interrupt routing cleanly.
* `[✓]` **7-Repository Ecosystem Topology Alignment (`ecosystem_topology.md` & `REPO_TOPOLOGY.md`):** Synchronized documentation, licenses, and repository structures across all 7 repositories (`ax-sdk`, `Eden-Espresso-Wave-AX`, `Piccolo-AX`, `Generation-X-AX`, `Cemu-Latte-AX`, `FLAC_AX`, `piccolo-ax-desktop`).
* `[ ]` **Offline Buildroot `ax-sdk` Hermeticity Check (`flycast-ax.mk`, `hatari-ax.mk`, `flac-ax.mk`):** Verify `FETCHCONTENT_SOURCE_DIR_AX_SDK` points to Buildroot's local `ax-sdk` build/staging directory so offline `Piccolo-AX` image builds never require network access during CMake configure.
* `[ ]` **Cross-Platform Line-Ending Normalization (`.gitattributes`):** Add/verify a root `.gitattributes` across all seven repositories (`* text=auto`, `*.bat eol=crlf`, `*.ps1 eol=crlf`, `*.sh eol=lf`, `*.md eol=lf`) to protect Linux appliance scripts (`post-build.sh`, `ax_doctor.sh`, `verify_ax_appliance.sh`) from accidental CRLF corruption on Windows.
* `[ ]` **Vertical-Slice Execution Order:** Complete Stage 0LD + 3-Phase PGO verification on `Eden-Espresso-Wave-AX` (Windows 10) end-to-end before switching context to `Cemu-Latte-AX` PGO sweeps and the `Piccolo-AX` Buildroot `BR2_EXTERNAL` image build.

---

## 🔬 Reference Silicon & Two-Tier Release Discipline

### Stream 1 Reference Hardware & Driver Floor (`Last Supported Ceiling`)
* **CPU / ISA:** `Intel Core i5-9600K` (`6C/6T`, Zero SMT, `AVX2` + `BMI1/2`, no `AVX-512`) — **active on-die Intel iGPU (`IGD Multi-Monitor` enabled) is mandatory**.
* **Discrete GPU:** `NVIDIA GTX 980 Ti` (`Maxwell GM200`, `6 GB` GDDR5) — pinned to **`582.78 WHQL`** (Windows 10) and **`580.*` legacy branch** (Linux Piccolo), supporting **Vulkan `1.4.x` core** (`590+` dropped Maxwell).
* **Integrated GPU:** `Intel UHD Graphics 630` (`Coffee Lake Gen9.5 GT2`, `24 EUs`, native `2:1` FP16, `VDBOX`, `VEBOX`, `PAK`) — pinned to **`31.0.101.2135`** (Windows 10, capped at **`VK_API_VERSION_1_3`**) and Mesa `ANV` + `intel-media-driver` (`iHD`) on Linux Piccolo.
* **USB Audio & MIDI Peripherals:** **HiBy FC3** (`ESS Sabre ES9281AC Pro` / `ES9281PRO`, `hw:CARD=FC3,DEV=0` on rear PCH USB root hub → `Core 0` IRQ) + **Akai MPK Mini MK3** (`hw:CARD=MPKmini3,DEV=0` on secondary/front USB port → `Core 5` RawMIDI poll).
* **Multimedia Stack:** **`FFmpeg 7.1.x` LTS** + **`nv-codec-headers 12.2.72.0`**.
* **Community Adaptation Boundary:** iGPU-less CPUs (`F`-series) are outside the official full-die preservation envelope; `Docs\SINGLE_GPU_ADAPTATION.md` documents how community self-builders can strip `vaapi_dmabuf_bridge.hpp` hooks from source.

### Two-Tier Baseline Policy (`Stage A` vs. `Stage B`)
* **Stage A (Public / Scientific Control Baseline):** Untouched upstream emulator release (`Eden`, `Cemu`, `RPCS3`, `shadPS4`, `Xenia Canary`) compiled with **`Clang 23.1+` (`ThinLTO` + 3-Phase PGO)** against **`Vulkan 1.4 SDK`** and **`FFmpeg 7.1.x` LTS** with zero `AX` patches → outputs `baseline_pgo_metrics.json`.
* **Stage B (Full `[AX]` & `Piccolo [AX]` Release):** Layers our tiny **`ax-sdk`** master core and `< 512 MB` `PREEMPT_RT` Linux appliance onto Stage A, measuring exact $P_{99}$ jitter, VRAM stability, and audio latency deltas against the PGO-baked control.

---

## 🛠️ Completed Subsystem Additions (Sprint 0–3 Foundation)

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

### 5. Four-Repository Polyrepo Scaffolding & `ax-sdk` Integration (Resolved — `2026-11-18`)
* **2.9 Multi-Repository Network Live:** Established `Piccolo-AX` (`GPL-3.0`), `ax-sdk` (`MIT`, tagged `v1.35.0`), `Cemu-Latte-AX` (`MPL-2.0`), and `Eden-Espresso-Wave-AX` (`GPL-3.0` on Forgejo) with unified root governance files (`AGENTS.md`, `CONTRIBUTING.md`, `SECURITY.md`, `REPO_TOPOLOGY.md`).
* **2.10 CMake `FetchContent` (`GIT_SHALLOW FALSE`) & `ax::core` Linkage:** Integrated `ax-sdk` (`v1.35.0`) into `Eden-Espresso-Wave-AX` (`CMakeLists.txt`) and `Cemu-Latte-AX` (`src/CMakeLists.txt`), and aligned `cemu-latte-ax.mk` for Cemu 2.x (`-DENABLE_VCPKG=OFF`, `-DALLOW_PORTABLE=OFF`, `sdl3`, `wxwidgets`).

### 6. `ax-sdk` `v1_36_1` Shared-Memory Telemetry, 7-Engine Classic Suite & A/V Studio (`2026-11-20` – `2026-11-21`)
* **2.11 Shared-Memory Telemetry & Audio Jitter Histogram (`ax::telemetry`):** Mapped 64B-aligned `ax::TelemetryCounters` into `Local\AX_Telemetry` (Windows) and `/dev/shm/ax_telemetry` (Linux) with lock-free `min / P99 / max` microsecond audio callback histograms (`ax::RecordAudioCallbackMetrics`), wired into `Eden` (`sink_stream.cpp`, `flex_asio_manager.cpp`) and `FLAC_AX` (`src/main.cpp`).
* **2.12 Unified Runtime Configuration (`ax::config` & `ax_engine.ini`):** Added zero-dependency `ax_engine.ini` parsing (`ax::LoadEngineConfig`) and standardized ISA/platform macros (`AX_PLATFORM_WINDOWS`, `AX_PLATFORM_LINUX`, `AX_HAS_AVX2`, `AX_HAS_BMI1`).
* **2.13 7-Repository Boundary & Shared Tooling (`ax-sdk/tools/` & `Scripts\resolve_ax_tools.bat`):** Onboarded `Generation-X-AX`, `FLAC_AX`, and `piccolo-ax-desktop` into the 7-repo network, added Buildroot recipes (`cemu-latte-ax`, `rmg-ax`, `blastem-ax`, `cannonball-ax`, `frontier-ax`, `flycast-ax`, `hatari-ax`, `flac-ax`) to `Piccolo-AX`, and consolidated all shared host auditing/telemetry/purge scripts into `ax-sdk/tools/` + `ax_doctor.sh`.
* **2.14 `Generation-X-AX` 7-Engine Unification & `FLAC_AX` `ax_av_studio`:** Added `Flycast` and `Hatari` C/C++20 hooks onto `ax_retro_bridge`, and implemented `ax_av_studio` (Akai MPK Mini MK3 ALSA RawMIDI + 16-voice polyphonic synth + `1024`-pt Hann FFT + VBlank GTX 980 Ti OpenGL Studio HUD + `/run/lock/ax-dac.lock` DAC handoff).

---

## 🔄 Piccolo Sprint-to-Master Release Crosswalk (`v1.35.0` – `v2.00`)

| Piccolo Sprint | Piccolo Version | Master Release | Target Medium | Core Focus & Status |
| :--- | :--- | :--- | :--- | :--- |
| **Sprint 0** | Gate | `v1.34` (3.8) | Live USB Bench | `[ACTIVE GATE]` Bare-metal `6.12-rt` + NVIDIA `580.*` + BIOS iGPU enabled + invariant TSC + `NV12` `dma_buf` + HiBy FC3 + MPK Mini MK3 gate. |
| **Sprint 1** | MVP (`v0.6`) | `v1.35.0` (5.6, 5.7, 5.10) | Samsung FIT Plus | `[UP NEXT]` `< 512 MB` `toram` EROFS USB boot, A/B fallback, `ax-sdk` extraction, runtime capability probes, GM200 `FP16 → FP32` promotion. |
| **Sprint 2** | `v0.7` | `v1.35.1` (4.6, 5.5) | Samsung FIT Plus | `[STAGED]` `6 × 1G` + `1024 × 2M` HugeTLB (`5.0 GB` floor), `io_uring` memory pinning, `2.00 ms` PLL ALSA, **Telemetry-Guided PGO**, **Cemu-AX** & **Generation-X-AX** bring-up. |
| **Sprint 3** | `v0.8` | `v1.35.1` (5.1–5.3) | Samsung FIT Plus | `[STAGED]` `5.50 GB` / `5.35 GB` VRAM shield, `64 KB` sparse VMA pool, **UHD 630 FP16 (`24 EUs`) `ASTC → BC7/BC6H`** transcode. |

| **Sprint 4** | `v0.9` | `v1.36` (6.2, 6.8, 6.9, 8.1–8.2) | Samsung FIT Plus | `[STAGED]` `lsfg-vk 2.0.0+` bindless layers, **UHD 630 `VEBOX` (`NV12 → RGBA` + deinterlace)**, `VK_QUEUE_TRANSFER_BIT`, `256 MB BAR1` WC, `ax-top` live SHM TUI monitor, and `ax::power` thermal/frequency throttle detector. |
| **Sprint 5** | `v1.0` → `v2.00` | `v1.37` – `v2.00` (7.1–7.4, 8.3, 9.1–10.2) | Samsung FIT Plus | `[ROADMAP]` Kiosk hardening, universal UAC2 USB DAC auto-probe, zero-copy ALSA `MMAP` / WaveRT audio ring, native DoP DSD & UAC2 hardware volume, `VK_KHR_present_wait` VBLANK phase lock, release SBOM/SHA-256 attestation, **RPCS3-AX → shadPS4-AX → Xenia-AX** Linux rollout, and **Live Re-LLVM Tiered JIT**. |

---

## 📊 Scheduled Future Releases (`v1.35.0` – `v2.00`)

> **Phase 3 Dependency Gate Matrix:** Activity **5.7 (Runtime Hardware Flags)** is the active prerequisite gate for **5.3, 5.10, 6.2, 6.8, and 6.9**. Completing `5.7` in `v1.35.0` guarantees that Maxwell `GM200` overrides and UHD 630 cross-GPU interop paths are dynamically verified before `v1.35.1` memory/offload work lands.

| Release | Phase / Sprint | Activity ID & Name | Target Components | Specific Engineering Activities |
| :--- | :--- | :--- | :--- | :--- |
| **`v1.35.0`** | Pipeline | **4.1 Unified Stage 0–2 Orchestrator** | `Scripts\build_all.ps1` | Wrap Stage A & Stage B Stage 0/1/2 builds + `validate_profile.ps1` with automatic SHA256 hashing and side-by-side telemetry diffing against `baseline_pgo_metrics.json`. |
| **`v1.35.0`** | Pipeline | **4.2 `sccache` & Build Manifest** | `CMakeLists.txt`, `build_manifest.json` | Enable `sccache` compiler caching for `~15s` incremental C++20 rebuilds and emit a JSON manifest. |
| **`v1.35.0`** | Pipeline | **4.3 Portable Release Packager** | `Scripts\package_release.ps1` | `[VERIFIED]` Bundle single-permutation SPIR-V bytecode, required DLLs, and configs into versioned archives. |
| **`v1.35.0`** | Pipeline | **4.4 Automated Frametime & PAK Capture Harness** | `Scripts\benchmark_frametimes.ps1`, Piccolo CI | `[VERIFIED]` Execute repeatable 60-second deterministic replay runs tracking 1% and 0.1% low stutters; wire UHD 630 PAK (`h264_vaapi`) hardware video capture on Piccolo for zero-overhead benchmark recording. |
| **`v1.35.0`** | Pipeline / Shaders | **4.5 Telemetry, Rollback & `VkSpecializationInfo` Shaders** | `Scripts\telemetry_daemon.ps1`, `PGO_stage*.bat`, shader wrappers | Add ASIO underrun and voice-drop counters; expand batch error rollback checks; expose `USE_EASU` and `USE_RCAS` via Vulkan `VkSpecializationInfo` (`layout(constant_id = N)`) so `eden_engine.ini` toggles take effect at pipeline creation without recompiling `.spv` binaries. |
| **`v1.35.1`** | Pipeline / `ax-sdk` | **4.6 Closed-Loop Telemetry-Guided PGO Qualification** | `ax-sdk/include/ax/telemetry.hpp`, `track_telemetry.ps1`, `validate_profile.ps1` | **Sub-Sprint 4.6a:** Tag active execution windows (`InGameplay`, `ShaderCompile`, `AudioCallback`, `VramReap`) in `ax::TelemetryCounters` so Stage 1 PGO sweeps trigger `__llvm_profile_reset_counters()` after menu/boot transitions and flush `.profraw` only during qualified hot-loop windows.<br>**Sub-Sprint 4.6b:** Generalize the 3-stage Clang ThinLTO + Telemetry-Guided PGO template into `ax-sdk/tools/` for `Cemu-Latte-AX`, `Generation-X-AX` (`Flycast`, `RMG`), and `FLAC_AX` (`ax_av_studio`). |
| **`v1.35.0`** | Phase 3 / Sprint 1 | **5.6 `ax-sdk` Extraction & Config-Driven Scaler** | `ax-sdk` repo (`libax_core`), `topology.cpp`, `eden_engine.ini` | `[REPO LIVE]` Extracted standalone `ax-sdk` master repo (`v1_36_1`); verify `LTP_PC_SMT` flag check and `<windows.h>` header isolation in `topology.cpp`; read `2:3:1` ratios dynamically on Intel & AMD Ryzen. |
| **`v1.35.0`** | Phase 3 / Sprint 1 | **5.7 Runtime Hardware Flags & iGPU Pre-Flight Gate** | `vk_device.cpp`, `vulkan_device.h`, `Docs\SINGLE_GPU_ADAPTATION.md` | `[CODE READY]` Encapsulate Maxwell `GM200` queries behind runtime vendor checks; verify active on-die Intel iGPU (`0x8086`, `VK_API_VERSION_1_3` safe query, `shaderFloat16`, `dma_buf`/NT handle export/import) at startup and emit clear BIOS `IGD Multi-Monitor` diagnostics if absent; publish `Docs\SINGLE_GPU_ADAPTATION.md` for self-builders on iGPU-less CPUs. |
| **`v1.35.0`** | Phase 3 | **5.10 `GM200` `FP16 → FP32` Promotion** | `emit_spirv.cpp`, `vk_device.cpp` | Gated by `5.7`: detect Maxwell `GM200` at runtime to promote Switch 3D shader `FP16` ops to native `FP32`, bypassing half-precision penalties. |
| **`v1.35.1`** | Phase 3 / Sprint 3 | **5.1 VRAM Shield (`5.50 / 5.35 GB`)** | `vk_rasterizer.cpp`, `vk_vram_shield.hpp` | `[CODE READY]` Poll DXGI and `VK_EXT_memory_budget` to intercept execution inside VBLANK windows for cache eviction. |
| **`v1.35.1`** | Phase 3 / Sprint 3 | **5.2 `64 KB` Sparse VMA Pool** | `vk_rasterizer.cpp`, `texture_reaper.cpp` | `[CODE READY]` Back dynamic render targets with a pre-allocated `64 KB` Sparse pool paired with VBLANK defragmentation. |
| **`v1.35.1`** | Phase 3 / Sprint 3 | **5.3 UHD 630 FP16 `ASTC → BC7/BC6H` Transcode** | `texture_cache.*`, `astc_to_bc7.comp`, `vaapi_dmabuf_bridge.hpp` | Gated by `5.7`: dispatch `ASTC → BC7/BC6H` compute shaders onto Intel UHD 630's 24 native-FP16 EUs (`VK_API_VERSION_1_3` capped) via `dma_buf` / NT Shared Handle to eliminate Maxwell `GM200` `1:64` FP16 and compute time-slicing stalls. |
| **`v1.35.1`** | Phase 3 / Sprint 2 | **5.5 Piccolo Linux Core & ALSA** | `ax_platform_shim.hpp`, `lockfree_audio_queue.hpp` | `[CODE READY]` Attach high-priority direct ALSA interconnects onto `hw:CARD=FC3,DEV=0` under `SCHED_FIFO 99` (`2.00 ms` + `±0.005%` micro-PLL). |
| **`v1.36`** | Phase 4 / Sprint 4 | **6.2 Async Compute & `VEBOX` Media Offload** | `vk_device.h/.cpp`, `vaapi_dmabuf_bridge.hpp` | `[CODE READY]` Open secondary `VK_QUEUE_COMPUTE_BIT` queues with timeline semaphores; route cutscene `NV12 → RGBA` conversion, deinterlacing (`480i`/`1080i`), and scaling through UHD 630's fixed-function `VEBOX` pipe (`VAEntrypointVideoProc`) using zero EU shaders. |
| **`v1.36`** | Phase 4 | **6.8 Dedicated Transfer Bit** | `vk_staging_buffer_pool.cpp` | `[CODE READY]` Bind fine-grained Vulkan split barriers (`VK_QUEUE_TRANSFER_BIT`) to overlap async DMA transfers with active 3D passes. |
| **`v1.36`** | Phase 4 | **6.9 `256 MB BAR1` Write-Combined Pool** | `vk_buffer_cache.cpp`, `vk_memory_allocator.cpp` | `[CODE READY]` Stream dynamic uniforms directly over PCIe into host-visible device-local VRAM without staging buffer bounces. |
| **`v1.36`** | Tooling / Observability | **8.1 `ax-top` Live Shared-Memory TUI Monitor** | `ax-sdk/tools/ax_top.cpp` (or Python curses) | **Sub-Sprint 8.1a:** Map `Local\AX_Telemetry` / `/dev/shm/ax_telemetry` (`magic = 0x4158544D`) read-only at `10 Hz` with zero Core 0/1 overhead.<br>**Sub-Sprint 8.1b:** Render live per-core affinity status, `VramUsed_GiB` vs. `5.00 GiB` prune bar, `Bar1Stalls`, `QsvFramesDecoded`, and `min / P99 / max` audio callback jitter bars (`us`) over SSH or a second monitor. |
| **`v1.36`** | `ax-sdk` / Telemetry | **8.2 Package Power & Thermal Throttle Detector (`ax::power`)** | `ax-sdk/include/ax/telemetry.hpp`, `src/telemetry.cpp`, `check_regression.ps1` | **Sub-Sprint 8.2a:** Add low-frequency (`1 Hz` Core 5) readers for Linux `IA32_THERM_STATUS` (`MSR 0x19C`) / `powercap` RAPL and Windows NVML / PDH frequency counters.<br>**Sub-Sprint 8.2b:** Track `thermal_throttle_events` and `cpu_effective_mhz` in `ax::TelemetryCounters`, flagging any drop below `98%` of BIOS `x48` (`4.8 GHz`) during `Stage 1` PGO sweeps. |
| **`v1.37`** | Audio / Plug-and-Play | **9.1 Universal USB Audio Class 2.0 (UAC2) Auto-Probe** | `ax-sdk`, `Piccolo-AX`, `FLAC_AX`, `flex_asio_manager.cpp` | **Sub-Sprint 9.1a:** Scan `/proc/asound/cards` and `stream0` capabilities on Linux to detect any connected UAC2 DAC supporting native `S32_LE` / `24-bit` (`44.1–384 kHz`), falling back gracefully when `hw:CARD=FC3,DEV=0` is absent.<br>**Sub-Sprint 9.1b:** Query Win32 `IMMDeviceEnumerator` in `FlexASIOManager` to auto-select any active UAC2 endpoint when `ExclusiveDeviceName` is blank in `ax_engine.ini`, persisting the discovered device name on first boot. |
| **`v1.37`** | Audio / Kernel-Bypass | **9.2 Zero-Copy Audio Ring (`ALSA MMAP` & `WaveRT`)** | `FLAC_AX/src/main.cpp`, `ax-sdk/include/ax/audio.hpp`, `Eden` | **Sub-Sprint 9.2a:** Switch `FLAC_AX` `ConfigureAlsaForEss()` to `SND_PCM_ACCESS_MMAP_INTERLEAVED` (`snd_pcm_mmap_begin` / `commit`) to write `S32_LE` frames directly into the USB xHCI DMA ring without `snd_pcm_writei()` buffer copies.<br>**Sub-Sprint 9.2b:** Expose a direct DMA span adapter in `ax::audio::LockFreeStereoRing` and evaluate native WaveRT / Steinberg ASIO buffer-pointer handoff in `Eden` to eliminate an intermediate stereo copy on Core 0. |
| **`v1.37`** | Audio / A/V Studio | **9.3 Native DoP (`DSD64`/`DSD128`), UAC2 Hardware Volume & AVX2 Synth SIMD** | `FLAC_AX/src/ess_dac_hooks.hpp`, `src/dsp/avx2_synth.cpp`, `src/midi/mpk_mini3.cpp` | **Sub-Sprint 9.3a:** Add `.dsf` / `.dff` 1-bit DSD framing inside `24-bit` DoP (`0x05` / `0xFA` markers) at `176.4 kHz` / `352.8 kHz` and bind `snd_mixer_selem_set_playback_volume_all()` to the ES9281AC Pro's internal 32-bit HyperStream digital attenuator.<br>**Sub-Sprint 9.3b:** Vectorize `PolySynthEngine` across 8-wide `_mm256_fmadd_ps` AVX2 voice batches (`< 15 µs` per `96`-frame period) and push Akai MPK Mini MK3 SysEx (`F0 47 7F 49 ... F7`) parameter labels (`K1–K8`) + pad LED feedback on startup. |

| **`v1.38`** | Display / Frame Pacing | **10.1 Display-Refresh Phase-Locked Pacing (`VK_KHR_present_wait`)** | `Eden`, `Cemu-Latte-AX`, `Piccolo-AX`, `ax::telemetry` | **Sub-Sprint 10.1a:** Query `VK_KHR_present_id` and `VK_KHR_present_wait` on GTX 980 Ti (`Driver 580+`), recording exact queue-to-scanout deltas into `ax::TelemetryCounters::last_frametime_us`.<br>**Sub-Sprint 10.1b:** Implement an adaptive Core 1 sleep-spin sync point (`~300 µs` pre-VBLANK wake) that locks frame presentation directly to display refresh without external frame limiters (`RTSS`). |
| **`v1.38`** | DevOps / Supply Chain | **10.2 Automated Release SBOM & Checksum Attestation** | `.github/workflows/release.yml`, `Piccolo-AX/post-image.sh`, `ax_doctor.sh` | **Sub-Sprint 10.2a:** Generate `SHA256SUMS.txt` and a CycloneDX JSON SBOM (`sbom.cdx.json`) on every tagged `ax-sdk` and `FLAC_AX` GitHub release.<br>**Sub-Sprint 10.2b:** Emit signed `SHA256SUMS` for `rootfs.erofs` and `bzImage` in `Piccolo-AX/post-image.sh`, verified by `ax_doctor.sh` at appliance boot. |
| **`v2.00`** | Phase 6 / Sprint 5 | **8.3 Live Re-LLVM Tiered JIT** | `dynarmic`, `live_reopt_daemon.cpp` | `[ROADMAP]` Runtime hot-block profiling and asynchronous background LLVM recompilation on Core 5 (`0x20`) with `seq_cst` I-cache pointer hot-swapping (superseding static AOT disk caches / `PA-05`). |

---

## 🌍 Linux-Only Multi-Emulator Expansion (`ax-sdk` + `Piccolo [AX]`)

**Strategy:** Windows 10 support concludes with **Eden Espresso Wave `[AX]`** (`v1.37` / `v2.00`). All subsequent emulators target Linux natively inside **Piccolo `[AX]`** via thin adapter forks linked to the tiny `ax-sdk` master repository (`libax_core`), each validated against its own Stage A Vanilla LLVM + Vulkan 1.4 + 3-Phase PGO baseline.

| Order | Target Fork | Build & Toolchain Guardrails | Core `ax-sdk` & Hardware Stream Integration |
| :--- | :--- | :--- | :--- |
| **#1** | **Cemu-AX** (Wii U) | Native CMake + Ninja + Clang 23.1+ (ThinLTO + 3-Phase PGO); developed alongside Piccolo Sprint 2–3. | Pins 3-core PowerPC Espresso JIT onto `0x1C`; routes `cubeb` to Core 0 `2.00 ms` ALSA ring; reuses `vk_vram_shield.hpp` and `VEBOX` deinterlacing. |
| **#2** | **RPCS3-AX** (PS3) | Mandatory `-DBUILD_LLVM=ON -DSTATIC_LINK_LLVM=ON` (isolates SPU JIT from host Clang 23.1+ / LLVM 22+ regressions); `-DUSE_NATIVE_INSTRUCTIONS=OFF`; patch LTO guard for headless SDL2 appliance build. | Isolates 1 PPU + primary SPUs on `0x1C` / CCX 0; quarantines SPU LLVM workers on Core 5 (`0x20`) with `_MM_HINT_NTA`; enables `mitigations=off` + `VEBOX` `1080i` deinterlacing. |
| **#3** | **shadPS4-AX** (PS4) | Modern C++20 + CMake + Clang 23.1+ (ThinLTO + 3-Phase PGO); developed on Stream 3 (11th Gen AVX-512 + `8 GB+` VRAM) before down-porting. | Backs `8 GB` unified memory via `6 × 1G` + `1024 × 2M` HugeTLB + `17.5 GB` `vmtouch -dql`; backports to Stream 1 (`9600K` + `980 Ti 6 GB`) via `5.35 GB` VRAM Shield + `64 KB` sparse VMA aliasing + UHD 630 FP16 transcode. |
| **#4** | **Xenia-AX** (Xbox 360) | Clang 23.1+ + Ninja (`xb` build); sequenced after `shadPS4-AX` to allow upstream Linux Vulkan backend maturity. | Maps Xenon worker threads onto `0x1C` (`0x3C` spillover), optimizes ROV split barriers, and streams constant buffers via `256 MB BAR1` WC pool. |

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

* **Two-Tier Baseline Discipline (Stage A vs. Stage B):** Never ship or benchmark an `[AX]` build without first logging its unmodified upstream Stage A (`Clang 23.1+` + `Vulkan 1.4` + `FFmpeg 7.1.x` + 3-Phase PGO) control baseline (`baseline_pgo_metrics.json`).
* **Mandatory Active On-Die Intel iGPU Policy:** Stream 1 and Stream 3 builds require an active on-die Intel iGPU (`IGD Multi-Monitor` enabled) to harvest `VDBOX`, 24 EUs FP16, `VEBOX`, and `PAK`. Do not maintain dual fallback compute pipelines inside official `[AX]` builds; maintain `Docs\SINGLE_GPU_ADAPTATION.md` for self-builders on `F`-series SKUs.
* **No Post-Espresso Windows Porting & No Bloated Desktop Linux Packaging:** Windows 10 support ends at **Eden Espresso Wave `[AX]`**. All subsequent emulators (`Cemu`, `RPCS3`, `shadPS4`, `Xenia Canary`) target Linux natively inside the `< 512 MB` `Piccolo [AX]` `toram` EROFS appliance via `ax-sdk`—never general-purpose PipeWire/Wayland desktop Linux packages.
* **No Dynamic Work-Stealing Thread Pools:** Do not replace static core partitioning (`0x01`/`0x02`/`0x1C`/`0x20`) with dynamic work-stealing schedulers; synchronization overhead and cache-line migration penalties undermine the zero-jitter determinism of `[AX]`.
* **Strict `< 512 MB` `toram` EROFS Budget (Console Default):** Keep `piccolo-ax-desktop` strictly opt-in (`default n`) and never pull PipeWire, PulseAudio, NVIDIA CUDA/OptiX/NVENC blobs, or Mesa LLVM drivers into `batocera-ax-external/`.
* **Rejected — `PA-05` (AOT `ARM64 → x86_64` Persistent Disk Cache):** Rejected due to self-modifying code (SMC) hazards, heavy upstream Dynarmic/loader merge friction, and functional overlap with Activity `8.3` (Live Re-LLVM Tiered JIT).
