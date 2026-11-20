# Eden Espresso Wave [AX] — Pipeline Automation & Staging Scripts

This directory houses the Switch-specific build, shader validation, PGO staging, and live triage adapters for the **Eden Espresso Wave `[AX]`** Profile-Guided Optimization (PGO) pipeline—working alongside the shared cross-application tooling in **`ax-sdk/tools/`**.

The framework is optimized for an **Intel Core i5-9600K 6-Core Non-SMT Host Topology (Skylake ISA Baseline)** paired with an **NVIDIA Maxwell GTX 980 Ti** and **Intel UHD Graphics 630**, with declarative multi-CPU scaling across the **2015–2021 Intel Preservation Family** (`i5-9600K`, `i5-8600K`, `i7-9700K`, `i7-8700K`, `i5-10600K`, `i5-10400`, `i7-7700K`, `i7-6700K`).

---

## 🏗️ Architectural Topology: The Elastic 2:3:1 Execution Model

To guarantee frame-pacing stability and eliminate real-time audio dropouts, the pipeline partitions physical P-cores into isolated execution zones via PnP registry MSI steering and OS thread affinity:

* **Core 0 [Mask 0x01] (Audio & High-Precision Timing):** Exclusively hosts the **Intel USB 3.0/3.1 eXtensible Host Controller** linked to the **HiBy FC3 USB DAC** (`FlexASIO` / `Steinberg ASIO`), isolating real-time audio interrupts from execution spikes.
* **Core 1 [Mask 0x02] (Vulkan Engine Command Submission):** Dedicated to the **NVIDIA GeForce GTX 980 Ti** kernel-mode driver and dirty-gather submission.
* **Cores 2, 3, 4 [Mask 0x1C] (Guest Execution Arena):** Reserved for ARM64 JIT dynamic compilation and CPU worker fibers.
* **Core 5 [Mask 0x20] (Housekeeping & Storage Quarantine):** Contains **NVMe / SATA AHCI** interrupts, Intel QSV video decoding, and ZSTD L18 asset decompression so asynchronous disk I/O never disturbs real-time frame threads.

---

## 🔗 Shared Cross-Application Tooling (`ax-sdk/tools/`)

Per the **[AX] Policy-in-SDK / Adapter-in-Consumer** boundary contract (`ax-sdk/REPO_TOPOLOGY.md`), shared host utilities live in **`ax-sdk/tools/`** and are resolved automatically at runtime by **`Scripts\resolve_ax_tools.bat`** (via `$env:AX_SDK_DIR`, sibling `..\ax-sdk\tools`, or CMake `FetchContent` caches):

* **`Get-AxProfile.ps1`** — Resolves `ax-sdk/config/hardware_profiles.json` (`schema_version: 2.1`) and detects host CPU/GPU silicon profiles.
* **`check_hardware.ps1`** — Pre-flight audit verifying `SeLockMemoryPrivilege`, physical/SMT topology, PCIe/USB/SATA IRQ masks (`[Bus X:Y.Z]`), `100%` Minimum Processor State, and disabled USB Selective Suspend.
* **`set_irq_affinity.ps1`** — Administrative PCIe/USB/SATA MSI interrupt steering (`DevicePolicy = 4`) with `-Revert` rollback support.
* **`track_telemetry.ps1`** — Non-invasive background telemetry daemon reading `SampleIntervalMs` from `ax_engine.ini` and outputting `build_stage_telemetry_metrics.json`.
* **`check_regression.ps1`** — Automated performance, handle-leak, audio underrun, and GPU VRAM ceiling (`vram_ceiling_gib`) regression gate.
* **`clean_all.ps1`** — Instant-rename (`EDENTRASH_*`) workspace purge supporting asynchronous background deletion or `-Wait` synchronous completion.

---

## 📂 Local Eden / Switch Adapter Scripts (`Scripts/`)

### 1. `resolve_ax_tools.bat`
* **Role:** Shared Toolset Locator.
* **Mechanics:** Resolves `%AX_TOOLS_DIR%` across `%AX_SDK_DIR%\tools`, sibling `..\ax-sdk\tools`, and `build_stage*\_deps\ax_sdk-src\tools` so `PGO_config.bat` and all `PGO_stage*.bat` scripts share a single `ax-sdk` installation.

### 2. `PGO_stage0.bat`, `PGO_stage0I.bat`, `PGO_stage0LD.bat`, `PGO_stage1.bat`, `PGO_stage2.bat`
* **Role:** 3-Phase LLVM `clang-cl` 23+ PGO Orchestration Pipelines.
* **Mechanics:** Executes synchronous pre-build workspace cleanup (`clean_all.ps1 -Wait`), hardware compliance checks, shader macro harmonization, SPIR-V validation, Ninja compilation (`-j 4`), post-build regression gating, and downstream staging of `ax_engine.ini` and `FlexASIO.toml`.

### 3. `reset_eden_config.ps1`
* **Role:** Deterministic `qt-config.ini` Enforcer.
* **Mechanics:** Terminates lingering `eden.exe` instances, queries `Get-AxHardwareProfile` from `ax-sdk/tools/`, merges `common_qt_config` + GPU-specific `qt_config` overrides, and rewrites `qt-config.ini` section-by-section while preserving user controller mappings, NAND paths, and ROM directories.

### 4. `macro_update.ps1` & `shader_validate.ps1`
* **Role:** Host Shader Harmonizer & Pre-Build SPIR-V Gate.
* **Mechanics:** Standardizes `#version 460 core` and `#ifndef` macro guards across FidelityFX FSR fragment wrappers (`macro_update.ps1`) and validates all Vulkan fragment shaders via `glslangValidator.exe` (`--target-env spirv1.3`) prior to Ninja compilation (`shader_validate.ps1`).

### 5. `validate_profile.ps1`
* **Role:** LLVM PGO Profile Counter Quality Gate.
* **Mechanics:** Executed before Stage 2 optimization. Verifies `.profraw` integrity (`> 128` bytes), confirms symbol binding against `build_stage1\bin\eden.exe` via `llvm-profdata.exe show`, and runs a dry-run merge test.

### 6. `tail_eden_log.ps1`
* **Role:** Single-Window Run & Live Runtime Triage Monitor (`[TL]`).
* **Mechanics:** Launches `build_stage0\bin\eden.exe` via `explorer.exe` to drop Administrator elevation cleanly for user-mode Vulkan ICDs and USB DAC drivers, streams `eden_log.txt` via non-locking `FileShare.ReadWrite` with repeat-assert deduplication, and returns to `PGO_config.bat` on exit.

### 7. `dev_tools/`
* **Role:** Archived Switch shader/codec inspection utilities (`Macro_search_shader_codecs.ps1`, `update_frag.bat`, `build_with_macro_update.bat`, `macro_and_build.bat`).

---

## 🔄 Compilation Staging Pipeline Synchronization

```text
[PGO_config.bat Hub] -> Resolves %AX_TOOLS_DIR% via resolve_ax_tools.bat
│
├──> 1. %AX_TOOLS_DIR%\clean_all.ps1 -Wait (Synchronous cache & build tree purge)
├──> 2. %AX_TOOLS_DIR\(\%\check_hardware.\)ps1 (Verify C-states, USB suspend & PCIe IRQ masks)
├──> 3. Scripts\macro_update.ps1 + shader_validate.ps1 (Harmonize & validate SPIR-V shaders)
├──> 4. %AX_TOOLS_DIR%\track_telemetry.ps1 / Scripts\tail_eden_log.ps1 (Monitor runtime health)
├──> 5. Executes LLVM clang-cl 23+ + Ninja Compilation Loop (-j 4)
└──> 6. %AX_TOOLS_DIR\(\%\check_regression.\)ps1 (Grade runtime metrics against VRAM/audio limits)
```

---

## 🛠️ Diagnostics & Pipeline Tuning

1. Run `Scripts\reset_eden_config.ps1` to verify `qt-config.ini` matches your active `hardware_profiles.json` silicon preset.
2. Launch **`[TL]`** (`tail_eden_log.ps1`) from `PGO_config.bat` to inspect Vulkan device selection, `Arch_Maxwell` (`FP16 → FP32`) active state, `128 MiB BAR1` staging bounds, and HLE applet transitions.
3. Inspect `build_stageX_verbose.log`, `build_stage0_triage.log`, `profile_validation_report.log`, or `build_stage_telemetry_metrics.json` in the repository root for detailed diagnostics.
