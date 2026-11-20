Captures low-level INI schemas, ax-sdk v1.35.0 runtime contracts (5.00 GiB async VRAM shield, GetLogicalProcessorInformationEx / sysfs 2:3:1 P-core scaler), LLVM/Clang-cl 23+ & MSVC 14.51+ ABI guards, shader preprocessor rules, telemetry internals, and PowerShell/Batch automation specifications.
---
## 1. ax_engine.ini Runtime Configuration Schema
ax_engine.ini provides an external, cross-platform runtime control layer (shared by Windows desktop and Eden Piccolo [AX]) for thread-affinity enforcement, proportional physical-core scaling, Win32 scheduling classes, and hardware capability overrides without recompiling:
```ini
[EngineTopology]
; Enforces hard-locked thread affinity mapping boundaries if hardware permits
EnableThreadAffinityMasks=true
; Clamps guest execution environment variables gracefully below physical ceilings
EnforceStrictCoreClamps=true
; Minimum physical P-core count required before applying hard allocation splits
MinCoresForIsolationSplit=6
; Proportional 2:3:1 physical-core scaler (Audio+GPU : Guest JIT : Aux Quarantine)
; Automatically excludes SMT siblings and E-cores via ax::topology (Activity 5.6)
CoreRatio=2:3:1
EnableProportionalScaler=true
[HardwareFeatureFlags]
; Runtime hardware capability detection (auto | force_on | force_off) (Activity 5.7)
; Encapsulates Maxwell/QSV paths cleanly from core dispatch to minimise upstream rebase friction
MaxwellSafeUBOPassthrough=auto
VramShieldPruneTriggerMB=5000
VramShieldCeilingMB=5500
VramShieldLsfgCeilingMB=5350
IntelQSVVideoOffload=auto
[WindowsPlatformHacks]
; Escalates the HostTiming event ring directly into the multimedia scheduler layer
EnableMMCSSProAudioScaling=true
; Thread scheduling class priorities (REALTIME, HIGH, ABOVENORMAL, NORMAL, BELOWNORMAL)
; Kept at HIGH so MMCSS Pro Audio manages preemption safely without starving USB xHCI IRQs
HostTimingPriorityClass=HIGH
VulkanDriverPriorityClass=HIGH
GuestExecutionPriorityClass=ABOVENORMAL
AuxQuarantinePriorityClass=BELOWNORMAL
```
---

## 2. Shader Preprocessor & Build Validation Pipeline

To eliminate Vulkan SPIR-V compilation failures and preprocessor token conflicts across shader wrappers, all shaders must pass automated macro normalization and pre-build validation before compilation.

### 2.1 GLSL Header & Version Alignment
* **Target Standard:** All shader wrappers and core GLSL files are standardized to `#version 460 core`.
* **Include Directive:** Wrappers enforce standardized `#extension GL_GOOGLE_include_directive : require` header insertion to ensure uniform include resolution across `glslangValidator` and runtime Vulkan drivers.

### 2.2 `macro_update.ps1` (Numeric Macro Harmonization)
* **Purpose:** Enforces explicit numeric defines across all shader wrappers to prevent boolean/empty token collisions in the Vulkan preprocessor.
* **Enforced Definitions:**
  * `USE_EASU` (`0` or `1`)
  * `USE_RCAS` (`0` or `1`)
  * `YUZU_USE_FP16` (`0` or `1` — forced `0` on Maxwell GM200 paths)
  * `VERSION` (numeric integer matching target GLSL version)

### 2.3 `shader_validate.ps1` (Pre-Build SPIR-V Gate)
* **Purpose:** Invokes `glslangValidator` across all shader sources prior to C++ compilation to catch syntax, uniform block declaration, and SPIR-V validation errors early.
* **Pipeline Integration:** Automatically triggered as a mandatory pre-build step; non-zero exit codes immediately halt the build pipeline.

### 2.4 FidelityFX FSR Preprocessor Firewall & CMake Injection Guards
During `v1.34` Stage 0 bring-up (objects `[498/1820]–[500/1820]` in `build_stage0_triage.log`), deep triage isolated three structural rules required when rebasing upstream AMD FidelityFX FSR shaders:

* **Two-Tier Macro Separation (Wrappers vs. AMD Vendor Header):** Outer wrapper shaders (`vulkan_fidelityfx_fsr_*.frag`) require explicit numeric defines (`USE_EASU=1`, `USE_RCAS=1`, `YUZU_USE_FP16=0`, `VERSION=460`) managed by `macro_update.ps1`. Conversely, internal AMD `ffx_fsr1.h` feature switches (`FSR_EASU_F`, `FSR_RCAS_F`) inside `fidelityfx_fsr.frag` must remain **blank tokens** wrapped in `#ifndef` guards so vendor macro token-pasting expands cleanly.
* **Include-Only Template Isolation (`fidelityfx_fsr.frag`):** Because `fidelityfx_fsr.frag` is `#include`d into wrapper shaders rather than compiled standalone, it must remain free of `#version` directives (which reside exclusively in the outer wrappers) and declare `InputTexture` and `con0–con3` inside an `#ifndef FSR_CONSTANTS_BOUND` guard.
* **`StringShaderHeader.cmake` & Ninja Cache Hygiene:** `StringShaderHeader.cmake` implicitly injects `-DUseUniformBlock=1` at build time. Any structural edits to `.cmake` shader generators or included `.frag` templates require purging `build_stage0` (`rd /s /q build_stage0`) so Ninja regenerates fresh SPIR-V headers from scratch.

---

## 3. Async Telemetry Daemon (%AX_TOOLS_DIR%\track_telemetry.ps1)
The async telemetry daemon provides low-overhead, per-core live profiling of eden.exe during PGO training sweeps and regression runs.
* Execution Model: Runs asynchronously out-of-process using a non-locking IO.FileStream tail reader on eden_current.log alongside kernel/GPU counters to prevent frame-pacing interference.
* Captured Metrics (build_stage_telemetry_metrics.json):
* Per-Core CPU Utilisation & Affinities: Verifies thread pinning against ax_engine.ini EngineTopology masks (Core 0 Audio/Timing, Core 1 Vulkan, Cores 2–4 Guest JIT, Core 5 Quarantine).
* Memory Footprint: Tracks working set, private bytes, and large-page allocation health.
* Audio & Async VRAM Health: Captures batch SPSC audio buffer switches (asio_buffer_switches), audio_underruns, and VkVramShield 5.00 GiB async transfer reaper activity.
* Regression Integration (%AX_TOOLS_DIR%\check_regression.ps1): Output JSON logs are graded automatically against baseline_telemetry.json to block affinity drift or memory regressions.
---
## 4. Build Orchestration & Scripting Standards (Windows)
### 4.1 Integrated PGO Batch Pipeline (PGO_config.bat → PGO_stage0.bat, PGO_stage0LD.bat, PGO_stage1.bat, PGO_stage2.bat)
All PGO stage scripts enforce a standardized quality gate sequence:
1. Clean & Hardware Audit: Purge stale build artefacts (clean_all.ps1 -Wait on 0LD/1) and validate host topology (%AX_TOOLS_DIR%\check_hardware.ps1).
2. Harmonize & Validate: Invoke macro_update.ps1 followed by shader_validate.ps1 (plus validate_profile.ps1 and default.profdata verification before Stage 2).
3. Compile / Bake: Execute clang-cl 23+ (-march=skylake -mavx2 -mfma -mbmi -mbmi2, lld-link, and PGO instrumentation or -fprofile-use bake).
4. Stage Configs & Regression Gate: Copy ax_engine.ini and FlexASIO.toml into %BUILD_DIR%\bin\ and validate runtime metrics via check_regression.ps1.

### 4.2 Interactive Root Configurator Menu
* Dynamically queries and renders live IRQ affinity audit results from `%AX_TOOLS_DIR%\check_hardware.ps1` and `%AX_TOOLS_DIR%\set_irq_affinity.ps1` directly inside the CLI menu.
* Confirms real-time hardware readiness (DAC on `0x01`, GTX 980 Ti on `0x02`, Storage on `0x20`, `SeLockMemoryPrivilege` status) prior to launching builds or gameplay sweeps.

### 4.3 Windows Batch & PowerShell Syntax Guards
To prevent script breakage across restricted Windows 10 environments, all repository scripts must adhere to two strict rules established during `v1.34` tooling hardening:
* **Inline PowerShell Execution Policy Bypass:** Never rely on global machine execution policies. Always invoke PowerShell scripts from batch wrappers using scoped, process-level bypass flags:
  ```bat
  powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0Scripts\macro_update.ps1"
  ```
* **Batch Multi-Line `else` & Delayed Expansion Safety:** Avoid complex multi-line nested `if (...) else (...)` blocks in `.bat` files where parenthesis parsing edge cases cause silent syntax failures. Use explicit separate conditional checks (`if %ERRORLEVEL% neq 0 (...)`) paired with `setlocal EnableDelayedExpansion` (`!VAR!`) inside loops.