# 🛠️ Eden Espresso Wave `[AX]` — Adversarial Architectural Audit & Resolution Plan

An unhedged, adversarial engineering and architectural audit of **Eden Espresso Wave `[AX]`** evaluated against the master repositories of its direct peers (Citron, Stock Eden, and upstream trailblazers like yuzu or RPCS3) reveals both world-class systems design and historical systemic liabilities that have now been systematically addressed through `ax-sdk` (`v1_36_1`) and the 7-repository Silicon Preservation Society topology.

> **Clarification on Policy Compliance:** Only documentation files are edited or corrected directly within the Git web portal using Markdown (`.md`) formatting corrections. This specific documentation practice is explicitly permitted under repository hygiene guidelines and carries zero risk to runtime binary execution logs. Code-level changes continue to require local verification scripts.

---

## 📋 Executive Summary

The provided architectural resolution plan for **Eden Espresso Wave `[AX]`** delivers a peer critique and remediation roadmap comparing the engine against alternatives like Citron, Stock Eden, yuzu, and RPCS3:

* **Microarchitectural & Audio/Video Performance:** Acknowledges triumphs such as low-latency ASIO / direct ALSA thread binding (`Core 0`, `2.00–2.67 ms`) and Intel QSV / VA-API zero-copy rendering, while resolving risks like SMT/Hyper-Threading scheduling conflicts and VRAM compaction bottlenecks during VBLANK.
* **Process & QA Evaluation:** Addresses the maintenance friction of rigid, hardware-locked forks (via the zero-dependency MIT `ax-sdk` `v1_36_1` separation) and the limitations of generic cloud CI/CD runners (`windows-2022`) that cannot validate bare-metal audio, GPU `dma_buf`, or TSC clocks.
* **Phased Resolution Plan:** Tracks completed and queued corrective sprints covering dynamic SMT-aware topology parsing (`ax-sdk/src/topology.cpp`), asynchronous Vulkan transfer queue VRAM management (`ax::vram` + `VkVramShield`), cross-process shared-memory telemetry (`Local\AX_Telemetry` / `/dev/shm/ax_telemetry`), and self-hosted bare-metal testing gates.

---

## 🔍 1. Architectural Audit & Peer Critique

### Domain 1: Microarchitectural Engineering & Silicon Tuning

* 💎 **The Triumph (Where `[AX]` Beats the Competition):** Stock Eden and Citron rely on generic audio wrappers (Cubeb, WASAPI Shared) running through the Windows audio mixer (`audiodg.exe`) or Linux PipeWire/PulseAudio, which forces a 48 kHz engine clock to resample to the desktop rate. Your implementation of ASIO direct thread binding (`asio::asio` / `FlexASIO`) and direct ALSA (`hw:CARD=FC3,DEV=0`, `S32_LE`) locked to Core 0 (`0x01`) at `THREAD_PRIORITY_CRITICAL` / `SCHED_FIFO 80–99` completely bypasses OS latency, achieving a sub-3 ms audio ring response (`2.00 ms` Linux / `2.67 ms` Windows) that no generic desktop emulator can match.
* 🛑 **The Failure Point (The SMT/Hyper-Threading Performance Trap — Resolved):** Hardcoding the execution affinity mask to a static 6C/6T map calibrated for an Intel Core i5-9600K introduces massive regression risks for users running SMT (Hyper-Threading) enabled chips (e.g., Core i7/i9 or AMD Ryzen). If a guest JIT fiber is scheduled on a logical thread sharing a physical core with a heavy background process, inter-thread pipeline stalls will completely erase your hardware-locked pacing advantages. Upstream emulators solve this cleanly using dynamic, architecture-aware topology generation rather than hardcoded bitmasks.

### Domain 2: Core Subsystem Architecture

* 💎 **The Triumph:** Most emulation engines handle video playback by decoding frames on the CPU via FFmpeg and copying them row-by-bit across the PCIe bus to a Vulkan storage image. Your zero-copy Intel QSV NT Shared Handle (`VK_KHR_external_memory_win32`) and Linux `iHD` VA-API `dma_buf` pipeline completely frees up your main GPU's memory bandwidth during heavy in-game cinematic cutscenes.
* 🛑 **The Failure Point (The VRAM Shield Race Condition — Resolved):** Running your memory compaction loop (`InterceptAndPruneVRAM`) directly inside the VBLANK window creates a tight timing bottleneck. If a modern game requires a large, sudden burst of allocations that exceeds the remaining space in your `5.50 GB` envelope, executing a 10% eviction pass during VBLANK will overflow the frame time budget. This induces a noticeable drop in frame rate, replacing a rare memory-spill event with a predictable micro-stutter. Top emulators resolve this by moving resource deletion to persistent asynchronous worker queues rather than cramming them into the display sync window.

### Domain 3: Development Process & Repository Governance

* 🛑 **Critical Process Structural Critique (Addressed via Polyrepo `ax-sdk` Separation):** Upstream tools like stock Eden and Citron roll out core tracking changes on a daily basis. By hardcoding your architecture directly onto specific hardware layouts inside the main emulator tree, a monolithic fork struggles to merge upstream optimizations or security fixes. Extracting all hardware isolation primitives into a standalone MIT-licensed C++20 SDK (`ax-sdk` `v1_36_1`) keeps consumer forks (`Eden-Espresso-Wave-AX`, `Cemu-Latte-AX`, `Generation-X-AX`, `FLAC_AX`) thin and rebase-friendly.

### Domain 4: Quality Assurance & Testing Pipeline

* 🛑 **The Failure Point (Hardware-Locked Test Invalidation):** Your automated test pipeline runner is locked to a generic virtualized GitHub Actions environment (`windows-2022`). Because cloud virtual machines lack the unique hardware configuration your architecture relies on (such as invariant TSC clocks, physical Intel QSV silicon, and an external USB DAC interface), the continuous integration run cannot validate your core thread affinity masks or low-latency ALSA/ASIO buffers. The tests pass successfully on the virtual machine while hiding subtle regressions that could break real bare-metal hardware.

---

## 🚀 2. Detailed Technical Resolution Plan

To elevate `[AX]` from a fragile hardware-specific fork into a robust, high-performance architecture, the following engineering corrections have been systematically applied across `v1.35.0–v1.35.1` (`ax-sdk v1_36_1`) and queued for `v1.36–v1.37`:

```text
[ PHASE A: TOPOLOGY RESOLUTION ] ──► [ PHASE B: ASYNC EVACUATION ] ──► [ PHASE C: TARGET HARNESSING ]
 (Removes Static Bitmask Traps)        (Decouples VRAM from VBLANK)      (Hardens Bare-Metal CI/CD Gates)
```

### Phase A: Dynamic SMT-Aware Topology & Unified Settings (`[RESOLVED in ax-sdk v1.35.0 → v1_36_1]`)

To eliminate the SMT pipeline stall hazard, hardcoded affinity bitmasks have been replaced by a cross-platform hardware topology resolver inside **`ax-sdk/src/topology.cpp`** (`ax::topology::DetectHostSilicon()`):

1. **Logical Thread Extraction:** Queries host CPU characteristics using `std::thread::hardware_concurrency()` and platform-specific OS registers (`GetLogicalProcessorInformationEx` with `LTP_PC_SMT` + `EfficiencyClass` on Windows; `/sys/devices/system/cpu` `thread_siblings_list` + `cpu_capacity` on Linux) to identify physical core boundaries.
2. **SMT Filtering Matrix:** When Hyper-Threading/SMT is detected at boot time, the engine masks out secondary logical buddy threads and maps workloads strictly across primary physical execution units (e.g., routing JIT threads exclusively to primary physical core indices).
3. **Proportional Core Scaler & Unified Settings:** Reads `CoreRatio=2:3:1` from `ax_engine.ini` (`ax::LoadEngineConfig()`) alongside declarative silicon profiles in `ax-sdk/config/hardware_profiles.json` (`schema_version: 2.1`) to generate runtime affinity bitmasks dynamically on boot across 6-core through 16-core processors.

### Phase B: Asynchronous VRAM Evacuation (`[RESOLVED in ax-sdk v1.35.0 → v1_36_1]`)

To remove memory compaction stalls from the critical display path, eviction routines have been decoupled from the synchronous VBLANK execution block via **`ax-sdk/include/ax/vram.hpp`** (`kPruneTriggerBytes = 5.00 GiB`) and **`src/video_core/renderer_vulkan/vk_vram_shield.hpp`** (`VK_QUEUE_TRANSFER_BIT` + Vulkan 1.4 `vkCmdSetEvent2`/`vkCmdWaitEvents2` split barriers + Core 5 `VK_KHR_timeline_semaphore` reaper):

1. **VRAM Budget Offloading:** Shifts the proactive cache eviction loop (`EvictStaleCacheStructures`) out of the synchronous display tick pass, triggering non-blocking pruning at `5.00 GiB` (`~4.50 GiB` hysteresis recovery) ahead of the `5.50 GiB` (`5.35 GiB` `lsfg-vk 2.0.0+`) hard ceiling.
2. **Dedicated Transfer Queue Binding:** Connects the memory compaction subsystem directly to an independent background Vulkan transfer queue utilizing `VK_QUEUE_TRANSFER_BIT` (`vulkan_device.cpp`).
3. **Asynchronous Split-Barrier Pruning:** Uses fine-grained Vulkan Synchronization2 split barriers (`vkCmdSetEvent2` and `vkCmdWaitEvents2`) and a timeline-semaphore-fenced Core 5 reaper to query, track, and prune stale texture cache layers continuously in the background without invading or reducing the frame time safety window.

### Phase C: Localized Hardware Testing, Telemetry-Guided PGO & Branch Protection (`[TOOLING READY in ax-sdk v1_36_1 — Runner Queued for v1.37]`)

To prevent unvalidated code configurations from breaking bare-metal performance baselines, testing infrastructure is decoupled from generic virtual cloud runners:

1. **Deploy Local Self-Hosted Testing Agents:** Run hardware-dependent affinity, QSV/VA-API, and USB DAC (`hw:CARD=FC3,DEV=0`) validations on a local bare-metal self-hosted runner matching the reference silicon profile (Intel Coffee Lake + Maxwell GTX 980 Ti + Intel UHD 630 + HiBy FC3 DAC), while keeping cloud CI strictly scoped to fast C++20/CMake compile verification (`Generation-X-AX`, `FLAC_AX`).
2. **Enforce Strict Remote Branch Protection:** Require developer modifications to pass through feature branch tracks (`feature/*`) with automated line-ending and macro hygiene checks before merging into `master` / `main`.
3. **Mandatory Pass Gates & Telemetry-Guided PGO:** Gate merges and Stage 2 PGO bakes on `ax-sdk/tools/check_hardware.ps1`, `ax-sdk/tools/track_telemetry.ps1` (`Local\AX_Telemetry` / `/dev/shm/ax_telemetry`), and `ax-sdk/tools/check_regression.ps1` (Windows) or `ax-sdk/tools/ax_doctor.sh` (Piccolo Linux) exiting with code `0`.
