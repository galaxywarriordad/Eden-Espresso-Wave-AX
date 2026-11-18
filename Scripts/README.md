# Eden Espresso Wave [AX] — Pipeline Automation & Staging Scripts

This directory houses the core automation engine, telemetry nodes, hardware validators, and performance gating suites for the **Eden Emulator Profile-Guided Optimization (PGO) Pipeline**. 

The entire framework is structurally optimized for an **Intel Core i5-9600K 6-Core Non-SMT Host Topology (Skylake ISA Baseline)** paired with an **NVIDIA Maxwell GTX 980 Ti** graphics architecture.

---

## 🏗️ Architectural Topology: The Elastic 2:3:1 Execution Model

To guarantee frame-pacing stability and eradicate real-time low-latency audio dropouts, this pipeline orchestrates the host machine's 6 physical, non-hyperthreaded cores into isolated execution zones via hard registry and OS thread scheduling bounds:

*   **Core 0 [Mask 0x01] (Audio & High-Precision Timing):** Exclusively hosts the **Intel USB 3.0/3.1 eXtensible Host Controller** linked to the low-latency **HiBy FC3 Hardware DAC**. Pinning the USB controller here isolates time-critical audio hardware interrupts from execution spikes.
*   **Core 1 [Mask 0x02] (Vulkan Engine Command Submission):** Dedicated entirely to the **NVIDIA GeForce GTX 980 Ti** kernel mode drivers for raw Vulkan submission processing.
*   **Cores 2, 3, 4 [Mask 0x1C]:** The primary processing arena reserved for the emulator's core dynamic compilation, execution translation loops, and state machines.
*   **Core 5 [Mask 0x20] (Storage Engine Quarantine):** Acts as an auxiliary containment cell for all mechanical and flash storage hardware interrupts. The **Standard NVM Express Controllers** and **SATA AHCI Controller** are bound here to prevent asynchronous disk IO read operations (on your Level-18 mechanical storage layout) from corrupting real-time frame threads.

---

## 📂 Core Automation Matrix Component Directory

### 1. `set_irq_affinity.ps1`
*   **Role:** Administrative Hardware MSI Interrupt Steering Engine.
*   **Mechanics:** Modifies the system hardware registry tree (`Affinity Policy\AssignmentSetOverride`) to target PnP device instances directly. Forces the Intel USB host controllers, NVIDIA kernel nodes, NVMe chips, and SATA channels into their designated **Elastic 2:3:1** core layout boundaries.
*   **Execution Rule:** Must run in an elevated context (`Run as Administrator`). A system reboot is required post-execution to re-route hardware message-signaled interrupts.

### 2. `clean_all.ps1`
*   **Role:** Asynchronous Master Workspace Purge Suite.
*   **Mechanics:** Terminates active compiler nodes (`ninja`, `cmake`, `clang-cl`, `lld-link`) and targets corrupted build trees (`build_stage0`, `build_stage1`, `build_stage2`). To bypass persistent file-handle locks that delay compilation, it renames active directories to randomized `EDENTRASH_` strings and hands them off to an independent background worker via a hidden, base64-encoded command.
*   **Execution Scope:** Automatically flattens the local compiler state, including the local `.cache` layout directory.
### 3. `track_telemetry.ps1`
*   **Role:** Zero-Overhead Background Telemetry Daemon Node.
*   **Mechanics:** Runs an infinite observation loop while `eden.exe` is active. Pulls memory footprints, active process handle arrays, non-paged kernel driver pools, and disk transfer counts directly from kernel pointers. Simultaneously queries the GPU registers (`nvidia-smi`) and hooks into the thread engine to monitor per-core thread density loops.
*   **Low-Overhead Parsing:** Leverages a non-locking `IO.FileStream` pointer to asynchronously track the end of `eden_current.log`, catching low-latency audio buffer underruns (FlexASIO/Steinberg) and Vulkan shader compile spikes in real time without stealing cycles from the engine. Outputs metrics directly to `build_stage_telemetry_metrics.json`.

### 4. `check_hardware.ps1`
*   **Role:** Host Topology Compliance Firewall.
*   **Mechanics:** Executed during the initialization block of your build phases. Validates that the active compiler shell has verified access to required hardware states and reviews active system configurations to confirm host compliance before allowing long-duration compilation streams to start.

### 5. `validate_profile.ps1`
*   **Role:** LLVM Telemetry Matrix Quality Gate.
*   **Mechanics:** Orchestrated before starting Stage 2 production compilation. Evaluates raw performance data frames (`.profraw`) to protect against file truncation (minimum 128 bytes limit). Invokes `llvm-profdata.exe show` to ensure function counters map perfectly onto the Instrumented Stage 1 binary targets, executing a dry-run consolidation pass to prevent compiler crashes.

### 6. `check_regression.ps1`
*   **Role:** Automated Performance Regression Evaluator.
*   **Mechanics:** Grades the structured output of the telemetry daemon against your saved `baseline_telemetry.json`. Evaluates hard limits on system handle footprints and logged shader stutters while calculating mathematical drift limits on memory consumption and system CPU load.
*   **Output:** Returns explicit exit code tokens (`exit 0` for health pass, `exit 1` for regression fault), allowing you to automate performance control within your continuous pipeline.

---

## 🔄 Compilation Staging Pipeline Synchronization

When you trigger a stage build sequence through the root configuration hub, the utilities engage in the following chronological sequence:
[Configurator Hub] -> Launches Stage Script
│
├──> 1. Call clean_all.ps1 (Clear environment caches, unlock open handles)
├──> 2. Call check_hardware.ps1 (Verify architecture compliance registers)
├──> 3. Spawns track_telemetry.ps1 (Asynchronously detaches to watch background frame health)
├──> 4. Executes Toolchain Compilation Loop (cmake + ninja -j 6)
└──> 5. Call check_regression.ps1 (Grade dynamic run health metrics against targets)

---

## 🛠️ Diagnostics & Pipeline Tuning

If your downstream regression checker flags a build failure:
1. Examine the generated `build_stageX_verbose.log` or `build_stage0_triage.log` left in the root path.
2. Read the structured results dumped in `profile_validation_report.log` to trace exact function alignment counts.
3. Check `build_stage_telemetry_metrics.json` to verify if thread densities drifted off the designated **Elastic 2:3:1** processor layout.

