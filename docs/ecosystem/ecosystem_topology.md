# 🗺️ End-to-End Ecosystem & Hardware Topology Map

**Programme:** The Silicon Preservation Society (`[AX]`)  
**Mission:** Keep proven PC hardware out of landfills through deterministic bare-metal optimization.  
**SDK & Engine Baseline:** `ax-sdk` `v1_36_1` (`v1.36.1`) | `Eden-Espresso-Wave-AX` `v1.35.1`  
**Last Updated:** `2026-11-20`

The full multi-repository and silicon topology is structured across **four primary layers** and **five repositories**—ranging from the upper application cartridges (`Eden`, `Cemu`, `FLAC_AX`) and shared C++20 runtime core/tooling (`ax-sdk` `v1_36_1`) to the bare-metal Linux console OS appliance (`Piccolo-AX`) and the target Stream 1 reference silicon (`Intel Core i5-9600K` + `NVIDIA GTX 980 Ti` + `Intel UHD 630` + `HiBy FC3 USB DAC`).

---

## 1. 🏗️ End-to-End 4-Layer Ecosystem & Silicon Topology

| Layer | Component / Target | Repository & License | Architectural Role & Execution Specifications |
| :--- | :--- | :--- | :--- |
| **Layer 1: Application Cartridges** | • **Eden Espresso Wave `[AX]`** (Switch)<br>• **Cemu Latte `[AX]`** (Wii U)<br>• **`FLAC_AX`** (Hi-Res Audio) | • Forgejo / GitHub: `galaxywarriordad/Eden-Espresso-Wave-AX` (`GPL-3.0`)<br>• GitHub: `galaxywarriordad/Cemu-Latte-AX` (`MPL-2.0`)<br>• GitHub: `galaxywarriordad/FLAC_AX` (`MIT`) | **Two-Tier Release Discipline (Emulators):**<br>• **Stage A (Win10 Control):** Clang 23.1+ ThinLTO + 3-Phase PGO (`-march=skylake`), Vulkan 1.4.<br>• **Stage B (`[AX]` Production):** Links `ax::core`, loads `ax_engine.ini`, and activates full silicon isolation.<br>**Audio Appliance (`FLAC_AX`):** Direct `libFLAC++` → `S32_LE` → `SCHED_FIFO 80` ALSA (`2.00 ms`) pipeline. |
| **Layer 2: Shared C++20 SDK, Settings & Tooling** | **`ax-sdk`** (`libax_core` / `ax::core`) + **`ax-sdk/tools/`** | GitHub: `galaxywarriordad/ax-sdk` (`MIT`) | • **C++20 Runtime:** `config` (`ax_engine.ini`), shared-memory `telemetry` + microsecond audio jitter profiler, `topology`, `memory`, `vram`, `igpu`, `audio`.<br>• **Declarative Truth:** `config/hardware_profiles.json` (`schema_version: 2.1`).<br>• **Shared Tools (`tools/`):** Process-agnostic `Get-AxProfile.ps1`, `check_hardware.ps1`, `set_irq_affinity.ps1`, `track_telemetry.ps1`, `check_regression.ps1`, `clean_all.ps1`, and Linux `ax_doctor.sh`. |
| **Layer 3: Bare-Metal Console OS** | **Piccolo `[AX]`** (`BR2_EXTERNAL`) | GitHub: `galaxywarriordad/Piccolo-AX` (`GPL-3.0`) | `< 300 MB` RAM-resident EROFS (`lz4hc,9`) appliance on pinned `batocera.linux` + Linux `6.12-rt` (`PREEMPT_RT`, `NO_HZ_FULL`, `cgroup v2`, ZRAM LZ4, uncomposited Xorg). |
| **Layer 4: Reference Silicon** | **Intel Core i5-9600K** (`6C/6T`, Zero SMT)<br>**GTX 980 Ti** + **UHD 630** + **HiBy FC3** | Bare-Metal Stream 1 Hardware | **Isolated `cgroup v2` Domain (`Cores 0–4`):** Core 0 (`0x01`) fused Audio/Timing (`SCHED_FIFO 80–99`), Core 1 (`0x02`) Vulkan Queue + Dirty Gather, Cores 2–4 (`0x1C`) tickless Guest JIT.<br>**Housekeeping Sink (`Core 5` / `0x20`):** Kernel `kworker`s, `rcu_nocbs`, SATA `SwitchHDD`, QSV/VA-API, ZSTD L18, and FLAC decoding.<br>**GPUs & DAC:** GTX 980 Ti (`PCI:1:0:0`, Driver `582.78` / `580.*`), UHD 630 (`PCI:0:2:0`, `iHD` VA-API `NV12` `dma_buf`), HiBy FC3 DAC (`hw:CARD=FC3,DEV=0`). |

---

## 2. 🔍 Detailed Layer-by-Layer Architecture Breakdown

### Layer 1: Application Cartridges (`Eden Espresso Wave [AX]`, `Cemu Latte [AX]`, `FLAC_AX`)

* **Two-Tier Build Discipline (Stage A vs. Stage B):**
  * **Stage A (Windows 10 Control Baseline):** Compiled from unmodified upstream code using Clang 23.1+ (ThinLTO + 3-Phase PGO), Coffee Lake flags (`-march=skylake`), Vulkan 1.4 SDK, and FFmpeg 7.1.x LTS to establish public Windows releases and `baseline_pgo_metrics.json` benchmarks.
  * **Stage B (`[AX]` Production):** Links `ax::core` (`ax-sdk` `v1_36_1`), loads `ax_engine.ini`, publishes live shared-memory telemetry (`Local\AX_Telemetry` / `/dev/shm/ax_telemetry`), and activates core isolation, HugeTLB fastmem, Intel iGPU offloading, VRAM shielding, and bit-perfect USB DAC audio.
* **Standalone Audio Appliance (`FLAC_AX`):** Pairs `Core 5` (`0x20`) non-RT `libFLAC++` decoding with `Core 0` (`0x01`) `SCHED_FIFO 80` ALSA playback (`hw:CARD=FC3,DEV=0`, `96` frames / `2.00 ms`) via `ax::audio::LockFreeStereoRing`, exporting live callback jitter (`min / P99 / max us`) to `/dev/shm/ax_telemetry`.
* **Host-Matched Repository Strategy:** Primary forks live on the same platform as their upstream projects (`git.eden-emu.dev` with a GitHub mirror for Eden; `github.com` for `ax-sdk`, `Cemu-Latte-AX`, `Piccolo-AX`, and `FLAC_AX`) so upstream syncs stay clean.

### Layer 2: Shared Deterministic C++20 SDK, Settings & Tooling (`ax-sdk` `v1_36_1`)

* **License & Portability:** Published under the **MIT License** with zero external dependencies so it links cleanly into `GPL-3.0`, `MPL-2.0`, and `GPLv2` consumers.
* **Committed C++20 Modules (`include/ax/` & `src/`):**
  * `config.hpp` + `config.cpp`: Standardized platform/ISA macros (`AX_PLATFORM_WINDOWS`, `AX_PLATFORM_LINUX`, `AX_HAS_AVX2`, `AX_HAS_BMI1`) and zero-dependency `ax_engine.ini` parser (`ax::LoadEngineConfig` / `ax::WriteDefaultEngineConfig`).
  * `telemetry.hpp` + `telemetry.cpp`: 64-byte cache-line-padded atomic counters (`ax::TelemetryCounters`) mapped into cross-process shared memory (`Local\AX_Telemetry` / `/dev/shm/ax_telemetry`) via `ax::InitTelemetryShm()`, plus a zero-allocation `min / P99 / max` microsecond audio callback jitter histogram (`ax::RecordAudioCallbackMetrics`).
  * `topology.hpp` + `topology.cpp`: Enumerates physical P-cores via `GetLogicalProcessorInformationEx` (`LTP_PC_SMT` + `EfficiencyClass`) on Windows and `/sys/devices/system/cpu` (`thread_siblings_list` + `cpu_capacity`) on Linux, dynamically scaling `CoreRatio=2:3:1` (`0x01`, `0x02`, `0x1C`, `0x20` on `i5-9600K`) with RAII `AvRevertMmThreadCharacteristics` cleanup on Windows, `pthread_setaffinity_np` + `SCHED_FIFO` on Linux, and `/dev/cpu/0/msr` (`0xc8f`) Intel CAT detection.
  * `memory.hpp` + `memory.cpp`: Defines `1G` / `2M` HugeTLB fastmem arenas, single-cycle BMI1 (`_bextr_u64`) guest page extraction, and `_MM_HINT_NTA` non-temporal streaming hints when CAT `0xc8f` is unavailable.
  * `vram.hpp`: Implements proactive **`5.00 GiB` (`kPruneTriggerBytes`) async VRAM pruning** (`~4.50 GiB` hysteresis recovery) ahead of the `5.50 GiB` standard (`5.35 GiB` `lsfg-vk 2.0.0+` / `5.20 GiB` Win32 LS) hard ceilings, paired with `VkVramShield` (`VK_QUEUE_TRANSFER_BIT` + Vulkan 1.4 `vkCmdSetEvent2`/`WaitEvents2` split barriers + Core 5 reaper).
  * `igpu.hpp`: Defines zero-copy `DRM_FORMAT_MOD_LINEAR` `NV12` `dma_buf` descriptors for UHD 630 → GTX 980 Ti handoffs and enforces the `<= 3.50 GB/s` DDR4 ring-bus bandwidth guardrail.
  * `audio.hpp`: Provides a 64-byte cache-line-isolated templated `LockFreeStereoRing<2048, SampleT>` (`PushBatch`, `PopBatch`, `PopPlanarBatch`) using contiguous `std::memcpy` wrap-around slices for both `96`-frame (`2.00 ms`) Piccolo ALSA and `128`-sample (`2.67 ms`) Win32 ASIO loops.
* **Aggregated Cross-App Host Tooling (`ax-sdk/tools/`):**
  * `Get-AxProfile.ps1`, `check_hardware.ps1`, `set_irq_affinity.ps1` (`-Revert`), `track_telemetry.ps1` (reads `Local\AX_Telemetry` live atomics), `check_regression.ps1`, `clean_all.ps1` (`-Wait`), and bare-metal Linux `ax_doctor.sh`.

### Layer 3: Bare-Metal Linux Console OS Platform (`Piccolo-AX` / `BR2_EXTERNAL`)

* **Out-of-Tree Buildroot Overlay (`batocera-ax-external/`):**
  * **Core Metadata:** `external.desc` (`PICCOLO_AX`), `Config.in`, and `external.mk` register Piccolo `[AX]` cleanly on top of a pinned upstream `batocera.linux` release without maintaining a heavy full-tree fork.
  * **`linux-6.12-rt-skylake.fragment`:** Configures mainline Linux `6.12-rt` with `CONFIG_PREEMPT_RT=y`, tickless `NO_HZ_FULL`, `RCU_NOCB_CPU`, pure `cgroup v2`, `1G`/`2M` HugeTLB, EROFS, ZRAM (LZ4), and built-in (`=y`) SATA/xHCI/USB audio/`i915` drivers.
  * **`grub.cfg`, `post-build.sh` & `post-image.sh`:** Sets `isolcpus=managed_irq,domain,0-4`, `nohz_full=0-4`, `rcu_nocbs=0-4`, `6 × 1G + 1024 × 2M` HugePages, `mitigations=off`, and A/B `rootfs.erofs.prev` fallback; purges `~300 MB` of unused NVIDIA compute/Mesa LLVM/PipeWire/Openbox bloat and enforces the `<= 300 MiB` (`307,200 KB`) EROFS `lz4hc,9` budget gate.
* **Runtime Boot, Isolation & Hardware Gate (`rootfs-overlay/`):**
  * **`S01piccolo-isolate`:** Builds `/sys/fs/cgroup/eden.slice` (`0-4`, isolated partition), routes unbound workqueues and default IRQs to Core 5 (`0x20`), configures `512 MB` LZ4 `zram`, locks GTX 980 Ti PCIe power state, and runs pre-mount `e2fsck` on `SHARE`.
  * **`S02piccolo-hw`:** Sets `snd-usb-audio nrpacks=1 lowlatency=1`, pins HiBy FC3 xHCI IRQ to Core 0 (`0x01`) and GTX 980 Ti MSI IRQ to Core 1 (`0x02`), enables `nvidia-smi -pm 1`, stages `/dev/shm` shader `tmpfs` directories, and mounts `SwitchHDD` asynchronously on Core 5.
  * **`K01piccolo-sync`, `xorg.conf` & `verify_ax_appliance.sh`:** Atomically flushes RAM shader caches (`zstd -3` + `sync`) on shutdown, configures bare uncomposited Xorg page-flipping (`PCI:1:0:0` + `PCI:0:2:0`), and runs a 7-point bare-metal hardware gate at boot.

### Layer 4: Target Stream 1 Reference Silicon (`i5-9600K` + `GTX 980 Ti` + `UHD 630` + `HiBy FC3`)

* **Isolated Domain (`Cores 0–4`):** Dedicated to fused audio/timing (Core 0), Vulkan submission (Core 1), and tickless guest CPU JIT execution (Cores 2–4).
* **Housekeeping Sink (`Core 5` / `0x20`):** Absorbs kernel `kworker`s, `rcu_nocbs`, SATA `SwitchHDD` I/O, USB HID polling, shader workers, and FFmpeg slices.
* **Dual-GPU & USB Audio Interconnect:** Pairs GTX 980 Ti (`582.78` / `580.*` driver, Vulkan 1.4 primary display) with Intel UHD 630 (`iHD` VA-API `NV12` `dma_buf` decode) and HiBy FC3 USB DAC (`hw:CARD=FC3,DEV=0` on Core 0).

---

## 3. 🌿 Multi-Host Git Architecture, Branching & Licensing Strategy

| Rule / Principle | How It Is Applied Across Your 5 Repositories |
| :--- | :--- |
| **"Follow the Upstream Host" Rule** | Primary forks live on the same platform as upstream (`git.eden-emu.dev` + GitHub mirror for Eden; `github.com` for `Cemu-Latte-AX`, `ax-sdk`, `Piccolo-AX`, and `FLAC_AX`) so upstream PRs and syncs stay frictionless. |
| **Aligned Default Branches** | Matches upstream branch conventions (`master` for `Eden-Espresso-Wave-AX`; `main` for `Cemu-Latte-AX`, `ax-sdk`, `Piccolo-AX`, and `FLAC_AX`) so Buildroot `.mk` recipes and CMake `FetchContent` tags never mismatch. |
| **Standalone `BR2_EXTERNAL` Design** | Keeps `Piccolo-AX` tiny (`< 10 MB`) by pointing at a pinned upstream `batocera.linux` release tag instead of carrying gigabytes of unrelated ARM/handheld board history. |
| **Cross-Repo License Matrix** | `ax-sdk` (`MIT`) links cleanly into `Eden-Espresso-Wave-AX` (`GPL-3.0`), `Cemu-Latte-AX` (`MPL-2.0`), `Piccolo-AX` (`GPL-3.0`), and `FLAC_AX` (`MIT`) without license conflicts. |

---

## 4. 📂 Complete 5-Repository File Structures

### Repository 1: `Piccolo-AX` (`://github.com`) — `GPL-3.0`

```text
Piccolo-AX/
├── LICENSE                                         # GPL-3.0
├── README.md                                       # Console OS overview & 5-repo network table
└── batocera-ax-external/
    ├── external.desc                               # name: PICCOLO_AX
    ├── Config.in                                   # Kconfig options (ax-sdk, eden-ax, cemu-latte-ax, lsfg-vk, vmtouch)
    ├── external.mk                                 # Includes package/*/*.mk
    ├── configs/
    │   └── linux-6.12-rt-skylake.fragment          # PREEMPT_RT, NO_HZ_FULL, cgroup v2, HugeTLB, EROFS, ZRAM
    ├── package/
    │   ├── ax-sdk/ax-sdk.mk                        # Builds libax_core (v1_36_1) into Buildroot staging
    │   ├── eden-ax/eden-ax.mk                      # Builds Eden Espresso Wave [AX] (Clang ThinLTO + ax-sdk)
    │   └── cemu-latte-ax/                          # Config.in + cemu-latte-ax.mk (Clang ThinLTO + ax-sdk)
    └── board/piccolo-ax/
        ├── grub.cfg                                # isolcpus=0-4, HugePages, mitigations=off, A/B .prev fallback
        ├── post-build.sh                           # Strips ~300 MB NVIDIA compute, Mesa LLVM, PipeWire & Openbox
        ├── post-image.sh                           # Packs EROFS (lz4hc,9) & enforces <= 300 MiB size gate
        └── rootfs-overlay/
            ├── etc/
            │   ├── X11/xorg.conf                   # Bare uncomposited Xorg (PCI:1:0:0 + PCI:0:2:0)
            │   └── init.d/
            │       ├── S01piccolo-isolate          # cgroup v2 0-4 partition, workqueue 0x20, zram, e2fsck SHARE
            │       ├── S02piccolo-hw               # nrpacks=1, Core 0 xHCI & Core 1 GPU IRQ pin, async SwitchHDD
            │       └── K01piccolo-sync             # Atomic zstd -3 flush of /dev/shm caches on exit
            └── usr/
                ├── bin/verify_ax_appliance.sh      # 7-point Sprint 0 / Day-1 bare-metal hardware gate
                └── lib/python3/configgen/generators/ax/
                    └── axGenerator.py              # Multi-emulator launcher, vmtouch & SIGSTOP ES
```

### Repository 2: `ax-sdk` (`://github.com`) — `MIT`

```text
ax-sdk/
├── CMakeLists.txt                                  # Builds static library target ax::core (v1.36.1, C++20)
├── LICENSE                                         # MIT License
├── README.md                                       # SDK overview & 5-repo network table
├── REPO_TOPOLOGY.md                                # 5-repo boundary contract & lineage
├── config/
│   └── hardware_profiles.json                      # Declarative silicon matrix (schema_version: 2.1)
├── include/ax/
│   ├── audio.hpp                                   # 64B-aligned lock-free SPSC stereo ring (96 frames / 2.00 ms)
│   ├── config.hpp                                  # Standardized platform/ISA macros & ax_engine.ini schema
│   ├── igpu.hpp                                    # DRM_FORMAT_MOD_LINEAR NV12 dma_buf & 3.50 GB/s guardrail
│   ├── memory.hpp                                  # 1G/2M HugeTLB fastmem API, BMI1 _bextr_u64, _MM_HINT_NTA
│   ├── telemetry.hpp                               # 64B-padded SHM counters & min/P99/max audio jitter stats
│   ├── topology.hpp                                # 2:3:1 core masks (0x01/0x02/0x1C/0x20) & SiliconProfile
│   └── vram.hpp                                    # 5.00 GiB proactive prune + 5.50 / 5.35 GiB VRAM shield
├── src/
│   ├── config.cpp                                  # Zero-dependency ax_engine.ini parser
│   ├── telemetry.cpp                               # Local\AX_Telemetry & /dev/shm/ax_telemetry + jitter histogram
│   ├── topology.cpp                                # Linux pthread/SCHED_FIFO/MSR 0xc8f + Win32 MMCSS shim
│   └── memory.cpp                                  # Linux MAP_HUGE_1GB/2MB & Win32 LargePage mmap
├── tools/
│   ├── Get-AxProfile.ps1                           # Silicon profile resolver for hardware_profiles.json
│   ├── check_hardware.ps1                          # Windows C-state, USB suspend & PCIe/USB/SATA IRQ audit
│   ├── set_irq_affinity.ps1                        # PnP MSI IRQ steering (0x01 / 0x02 / 0x20) with -Revert
│   ├── track_telemetry.ps1                         # Live Shared-Memory & log fallback telemetry daemon
│   ├── check_regression.ps1                        # Audio, handle-leak & GPU VRAM ceiling regression gate
│   ├── clean_all.ps1                               # Synchronous (-Wait) & async workspace purge utility
│   └── ax_doctor.sh                                # Bare-metal Linux PREEMPT_RT, HugePage & USB DAC auditor
└── ax_toolchain/
    └── piccolo-clang-x86_64.cmake                  # Shared Clang 23.1+ ThinLTO + 3-Phase PGO file
```

### Repository 3: `Cemu-Latte-AX` (`://github.com`) — `MPL-2.0`

```text
Cemu-Latte-AX/
├── README.md                                       # Merged dual-edition README + upstream Cemu links + 5-repo table
├── CMakeLists.txt                                  # Optional -DENABLE_AX_SDK=ON FetchContent hook (v1_36_1)
└── src/                                            # Clean upstream Cemu source ready for Stage A Win10 PGO bake
```

### Repository 4: `Eden-Espresso-Wave-AX` (`git.eden-emu.dev/galaxywarriordad/Eden-Espresso-Wave-AX.git` Primary + GitHub Mirror) — `GPL-3.0`

```text
Eden-Espresso-Wave-AX/
├── README.md                                       # Linked in 5-repo network table
├── CMakeLists.txt                                  # FetchContent ax_sdk (v1_36_1) + WIN32-gated asio_sdk & FlexASIO
├── PGO_config.bat                                  # Interactive configurator & 3-stage LLVM PGO hub
├── ax_engine.ini                                   # Unified [AX] CoreRatio=2:3:1 & HostTimingPriorityClass=HIGH settings
├── Scripts/                                        # resolve_ax_tools.bat, PGO_stage*.bat, macro_update, shader_validate, reset_eden_config, tail_eden_log
└── src/                                            # Active v1.35.1 codebase (VkVramShield, FlexASIO/SinkStream SHM telemetry, QSV offload)
```

### Repository 5: `FLAC_AX` (`://github.com`) — `MIT`

```text
FLAC_AX/
├── CMakeLists.txt                                  # C++20 CMake build with -DFLAC_AX_USE_AX_SDK=ON (v1_36_1)
├── LICENSE                                         # MIT License
├── README.md                                       # Bare-metal Linux ESS Sabre ES9281PRO / ES9281AC Pro player guide
└── src/main.cpp                                    # Core 5 FLAC decoder -> LockFreeStereoRing -> Core 0 SCHED_FIFO 80 ALSA + SHM jitter stats
```
