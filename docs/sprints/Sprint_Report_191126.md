# 📋 Sprint Progress Report — `ax-sdk` `v1_36_1` & Wider Silicon Preservation Society Topology

**Date:** `2026-11-19`  
**SDK Baseline:** `ax-sdk` `v1_36_1` (`v1.36.1`)  
**Target Reference Silicon:** Intel Core i5-9600K (`6C/6T` Non-SMT) + 32 GB RAM + GTX 980 Ti (`6 GB` Maxwell) + UHD 630 + HiBy FC3 DAC

---

## 🎯 Executive Summary

The sprint consolidated cross-application instrumentation, configuration schemas, and host automation into **`ax-sdk` (`v1_36_1`)** and aligned all **five `[AX]` repositories** (`ax-sdk`, `Eden-Espresso-Wave-AX`, `Cemu-Latte-AX`, `FLAC_AX`, and `Piccolo-AX`) around a unified policy-and-adapter boundary contract. Key deliverables include a zero-overhead Shared-Memory Telemetry Bridge, a Lock-Free Microsecond Audio Jitter Profiler, a Linux Bare-Metal Pre-Flight Auditor, and complete `cemu-latte-ax` Buildroot package definitions.

---

## ✅ Completed Deliverables

1. **`ax-sdk` `v1_36_1` Release & Ecosystem-Wide Synchronization**
   * Standardized ISA macros (`AX_PLATFORM_WINDOWS`, `AX_PLATFORM_LINUX`, `AX_HAS_AVX2`, `AX_HAS_BMI1`) and a zero-dependency `ax_engine.ini` parser in `include/ax/config.hpp` and `src/config.cpp`.
   * Implemented 64-byte cache-line-isolated `ax::TelemetryCounters` (`Local\AX_Telemetry` on Win32, `/dev/shm/ax_telemetry` on Linux) and zero-allocation `min / P99 / max` callback latency histograms (`include/ax/telemetry.hpp`).
   * Bumped CMake to `VERSION 1.36.1` with `cxx_std_20`, scoped Win32 macros, and updated version tags across all 5 repositories (`ax-sdk`, `Eden-Espresso-Wave-AX`, `Cemu-Latte-AX`, `FLAC_AX`, `Piccolo-AX`).
   * Updated canonical architecture and security documentation (`REPO_TOPOLOGY.md`, `End2end_Ecosystem_Architecture.md`, `SECURITY.md`, etc.).

2. **Aggregated Cross-App Host Tooling (`ax-sdk/tools/`) & Consumer Wiring**
   * Consolidated duplicate scripts into `ax-sdk/tools/` (`Get-AxProfile.ps1`, `check_hardware.ps1`, `set_irq_affinity.ps1` with `-Revert`, `track_telemetry.ps1`, `check_regression.ps1`, `clean_all.ps1`, and `ax_doctor.sh`).
   * Wired `ax::LoadEngineConfig`, `ax::InitTelemetryShm()`, and `ax::RecordAudioCallbackMetrics()` into `Eden-Espresso-Wave-AX` (`sink_stream.cpp`, `flex_asio_manager.cpp`) and `FLAC_AX` (`src/main.cpp` with Core 0 `SCHED_FIFO 80` tracking).

3. **`Cemu-Latte-AX` & `Piccolo [AX]` Buildroot Integration Blueprint**
   * Registered `cemu-latte-ax` in `batocera-ax-external` (`Config.in`, `cemu-latte-ax.mk`) targeting Clang ThinLTO and `-march=skylake`.
   * Split base OS compilation (GCC) from application targets (Clang + LLD + ThinLTO + PGO) while enforcing a `5.50 GB` VRAM ceiling, UHD 630 H.264 offloading, and USB DAC (`hw:CARD=FC3,DEV=0`) arbitration.

4. **Hybrid Development & Testing Pipeline Architecture**
   * Established a three-tier environment split spanning Windows 10 Host (C++20 development/PGO), Ubuntu Build VM (headless CMake/sanitizer verification), and Bare-Metal `Piccolo [AX]` Target (hardware ground-truth validation).

---

## 🚀 Immediate Carry-Over Actions (Next Session)

1. **Push Local `Eden-Espresso-Wave-AX` Changes & Run Stage 0LD Verification** (`git push`, cleanup, and `PGO_config.bat` -> `[0LD]` check).
2. **Wire GPU VRAM & QSV Counters into `ax::GetTelemetry()`** (`vram_used_bytes`, `vram_reaps`, `bar1_stalls`, and `qsv_frames_decoded`).
3. **Complete Ubuntu VM Provisioning & `Cemu-Latte-AX` Source Linkage** (`clang`, `lld`, `libflac++-dev`, `libasound2-dev`, and CMake target wiring).
4. **Implement Cross-App USB DAC Arbitration (`/run/lock/ax-dac.lock`) & Bare-Metal `Piccolo [AX]` Boot Gate** (`flock()` integration and `ax_doctor.sh` verification).
