# Changelog

All notable changes, silicon-tuning milestones, and compiler fixes for **Eden Espresso Wave [AX]** are documented in this file, adhering to the [Keep a Changelog](https://keepachangelog.com) specification.

---

## [Unreleased] — Phases 2–6 Engineering Roadmap
* **Phase 2 (Throughput & Math):** L1-aligned Xbyak GPU Macro-JIT, GTX 980 Ti stride guards, physical page-contiguity DMA checks (Citron PR #449 / #383), vertex-less draw fastpaths (Citron PR #448), and uniform ring packing.
* **Phase 3 (VRAM & BC7/BC6H):** VRAM over-subscription shields, Fragment-Aware Texture Reaper, CPU-delta overlap invalidation (Eden PR #32), and hardware-native ASTC to BC7/BC6H texture recompression.
* **Phase 4 (Async Silicon Offloading):** WinRT Windows Graphics Capture swapchain integration for `lsfg-vk`, secondary `VK_QUEUE_COMPUTE_BIT` queue, and Intel UHD 630 co-processing.
* **Phase 5 (Closed-Loop Runtime & Dev-Kit Polish):** Predictive NVMe pre-faulting, frame-pacing governor, Maxwell register-pressure guards, Core 5 RawInput polling, and external USB-C telemetry screen support.
* **Phase 6 (Autonomous Silicon R&D):** Live background ARM64 superblock re-compilation on Core 5, Intel VME motion vector synthesis, and hardware VBLANK/audio sync dongles.

---

## [v1.34] — 2026-11-16 (Advanced Shader Macro Harmonization & Pipeline Telemetry Integration)
### Added
* **Shader Macro Automation (`Scripts\macro_update.ps1`):** Enforced consistent numeric macro defines (`USE_EASU`, `USE_RCAS`, `YUZU_USE_FP16`, `VERSION`) across all shader wrappers to eliminate preprocessor token conflicts and Vulkan SPIR-V compilation failures.
* **Pre-Build SPIR-V Validation (`Scripts\shader_validate.ps1`):** Added `glslangValidator`-powered shader syntax and SPIR-V validation prior to C++ build execution.
* **Async Telemetry Daemon (`Scripts\telemetry_daemon.ps1`):** Deployed out-of-process per-core profiling for `eden.exe` capturing memory, thread affinities, and initial audio buffer health metrics with JSON output, configurable sampling, and graceful shutdown controls.
* **Runtime Configuration Schema (`eden_engine.ini`):** Defined cross-platform runtime parameters for hard-locked thread affinity mapping (`EnableThreadAffinityMasks`), proportional `2:3:1` core scaling, Maxwell/QSV hardware capability flags, and Windows MMCSS Pro Audio / thread priority class tuning.

### Changed
* **Integrated PGO Workflow Batch Scripts (`PGO_stage0LD.bat`, `PGO_stage1.bat`, `PGO_stage2.bat`):** Standardised quality gates across all PGO stages by invoking `macro_update.ps1`, `shader_validate.ps1`, and `telemetry_daemon.ps1` regression checks between cleaning and build steps.
* **Interactive Root Configurator Menu:** Added live dynamic parsing and display of IRQ affinity audit results sourced from `check_hardware.ps1`.
* **Shader Preprocessor Alignment:** Standardised all shader wrappers to `#version 460 core` and enforced uniform `#extension GL_GOOGLE_include_directive` header insertion.

### Fixed
* **Batch & PowerShell Execution Guards:** Resolved Windows batch `else` statement syntax restrictions in multi-line nested conditionals using explicit separate checks and `setlocal EnableDelayedExpansion`, and applied scoped inline `-ExecutionPolicy Bypass` flags to PowerShell invocations.

---

## [2026-11-13] — Audio Telemetry Logging Hook Integration
### Added
* **Audio Telemetry Logging Hook (`PerformanceManager::TapFrame`):** Integrated real-time atomic tracking for `voices_dropped`, `voice_drop_parameter`, and `audio_processing_time_us` into `g_metrics` (`src/common/telemetry_types.hpp`), allowing the 250 ms telemetry daemon to audit ADSP voice shedding and buffer execution times on Host Core 0 (`0x01`).
* **FlexASIO Endpoint Alignment:** Standardised the output target string to `device = "Speakers (HiBy FC3)"` in `FlexASIO.toml` for direct WASAPI exclusive-mode binding.

---

## [2026-11-12] — Stage 0 Verified Baseline
### Added
* **Core Partitioning & IRQ Steering:** Enforced Win32 thread affinity masks (`src/common/thread.h/.cpp`) and steered hardware MSI IRQ policies via `set_irq_affinity.ps1`.
* **Memory & Vector Acceleration:** Enabled `SeLockMemoryPrivilege` 2 MB large-page fastmem, BMI1 `_bextr_u64` page-index extraction, lock-free `GPUDirtyMemoryManager`, and AVX2 non-temporal pipeline cache decompression.
* **Video & Audio Pipelines:** Zero-copy Intel QSV video offload for VP8/VP9 (`qsv_decoder.h/.cpp`) and a lock-free SPSC PCM ring buffer feeding FlexASIO WASAPI Exclusive Mode (`flex_asio_manager.cpp`, Eden PR #2).
* **Diagnostics & Docs:** Added a VT100 pinned console header, parallel `ninja -j 4` orchestration, live 250 ms JSON/console telemetry, PGO validation gates, and comprehensive documentation suite.

### Changed
* **Toolchain & Linker:** Standardised on LLVM `clang-cl` 23.1.4 (`-march=skylake`, ThinLTO, `-Werror`), CMake Policy `CMP0169` Steinberg ASIO SDK ingestion, and pure Win32 `/MD` runtime isolation.
* **VRAM & Unswizzle:** Scaled 3D compute unswizzle tiers, enforced a 5.5 GB DXGI VRAM ceiling, and restricted SPIR-V compilation to single-worker Core 5 (`ASYNC_WORKER_CORE_CLAMP = 1`, Eden PR #4664).

### Fixed
* **Vulkan & Qt Fixes:** Resolved `lld-link` unresolved externals in Vulkan 1.4 dynamic dispatch and fixed null-pointer guards/lambda captures in Qt6 configuration dialogs.
* **Header & Macro Hygiene:** Enforced `<windows.h>` ordering with `NOMINMAX` and `#undef CreateEvent` across core modules.

---

## [2026-11-03] — Initial Clang-cl 23 & Coffee Lake PGO Bring-Up
### Added
* Initial 3-stage LLVM PGO scripts (`PGO_stage1.bat`, `PGO_stage2.bat`) with automated forward-slash path escaping, 32-byte heap over-alignment, and MSVC STL vector-annotation guards.

### Fixed
* **Compiler & Audio Fixes:** Refactored `LAMBDA_FORCEINLINE` macros for strict Clang-cl rules, scrubbed C-flags in CMake, and resolved POSIX `getopt_long` null-pointer constants in `yuzu_room.cpp`[cite: 17].