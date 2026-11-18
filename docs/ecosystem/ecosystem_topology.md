##End-to-End Ecosystem & Hardware Topology Map

The full multi-repository and silicon topology is structured across four primary layers—ranging from the upper emulator cartridges and shared C++20 runtime core to the bare-metal Linux console OS appliance and the target reference silicon (Intel i5-9600K combined with a GTX 980 Ti, UHD 630 iGPU, and HiBy FC3 DAC)—with complete architectural documentation and configuration details available in the source repositories.

1. End-to-End 4-Layer Ecosystem & Silicon Topology Diagram
---
====================================================================================================
 LAYER 1: EMULATOR CARTRIDGES (Two-Tier Release Discipline: Stage A Win10 vs. Stage B Piccolo [AX])
====================================================================================================
  +-----------------------------------------------+   +-------------------------------------------+

  |        Eden Espresso Wave [AX] (Switch)       |   |       Cemu Latte [AX] (Wii U)             |
  |  Primary: git.eden-emu.dev (master)           |   |  Primary: github.com (main)               |
  |  Mirror : github.com (master) | GPL-3.0       |   |  Upstream: cemu-project/Cemu | MPL-2.0    |
  |  - Stage A: Win10 Clang 23.1+ ThinLTO + PGO   |   |  - Stage A: Win10 Coffee Lake PGO + Vk 1.4  |
  |  - Stage B: Piccolo Linux + ax::core linked   |   |  - Stage B: Piccolo Linux + ax::core linked |
  +-----------------------+-----------------------+   +---------------------+---------------------+

                          |                                                 |
                          +------------------------+------------------------+
                                                   | Static Link (ax::core / libax_core.a)
                                                   v
====================================================================================================
 LAYER 2: SHARED DETERMINISTIC C++20 RUNTIME SDK (MIT License — Zero External Dependencies)
====================================================================================================
  +-----------------------------------------------------------------------------------------------+

  |                       ax-sdk (https://github.com)                     |
  |  [topology] 2:3:1 Masks (0x01/0x02/0x1C/0x20), SCHED_FIFO, Pro Audio MMCSS, CAT MSR 0xc8f     |
  |  [memory]   1G / 2M HugeTLB Fastmem Arenas, BMI1 (_bextr_u64), _MM_HINT_NTA Prefetch          |
  |  [vram]     VBLANK VRAM Shield (5.50 GiB Std / 5.35 GiB lsfg-vk), Maxwell GM200 Flags         |
  |  [igpu]     UHD 630 VA-API (iHD) -> DRM_FORMAT_MOD_LINEAR NV12 dma_buf (<= 3.50 GB/s Guard)   |
  |  [audio]    Cache-Line Aligned (64B) Lock-Free SPSC Stereo Ring (96 Frames / 2.00 ms @ 48 kHz) |
  +------------------------------------------------+----------------------------------------------+
                                                   | Buildroot BR2_EXTERNAL (ax-sdk.mk)
                                                   v
====================================================================================================
 LAYER 3: BARE-METAL LINUX CONSOLE OS APPLIANCE (GPL-3.0 — < 300 MB RAM-Resident EROFS)
====================================================================================================
  +-----------------------------------------------------------------------------------------------+

  |                    Piccolo-AX (https://github.com)                |
  |  - Standalone BR2_EXTERNAL overlay pinned to upstream batocera-linux/batocera.linux           |
  |  - Linux 6.12-rt (CONFIG_PREEMPT_RT=y, NO_HZ_FULL, RCU_NOCB_CPU, pure cgroup v2, ZRAM LZ4)    |
  |  - Boot: GRUB isolcpus=0-4 + 6x1G/1024x2M HugePages -> toram EROFS (lz4hc,9 <= 300 MiB)       |
  |  - Runtime: S01piccolo-isolate -> S02piccolo-hw -> Bare Uncomposited Xorg -> K01piccolo-sync  |
  +------------------------------------------------+----------------------------------------------+
                                                   | Direct Hardware & IRQ Affinity Binding
                                                   v
====================================================================================================
 LAYER 4: TARGET STREAM 1 REFERENCE SILICON (Intel Core i5-9600K 6C/6T — Zero SMT)
====================================================================================================
  +-----------------------------------------------------------------------+  +--------------------+

  |               ISOLATED CGROUP V2 DOMAIN (Cores 0–4)                   |  | HOUSEKEEPING SINK  |
  |  [Core 0 | 0x01] Audio + CoreTiming (SCHED_FIFO 99) <--- HiBy FC3 DAC |  |  [Core 5 | 0x20]   |
  |  [Core 1 | 0x02] Vulkan Command Queue + Dirty Gather --> GTX 980 Ti   |  |  - Kernel kworkers |
  |  [Cores 2–4 | 0x1C] Tickless Guest CPU JIT Execution (NO_HZ_FULL)     |  |  - rcu_nocbs / IRQ |
  +-----------------------------------------------------------------------+  |  - SATA SwitchHDD  |

                                                                             |  - FFmpeg / SPIR-V |
  +-----------------------------------+   +-------------------------------+  +--------------------+

  |  NVIDIA GTX 980 Ti (6 GB GM200)   |   |  Intel UHD 630 (24 EUs GT2)   |
  |  - PCI:1:0:0 | Driver 580.*       |   |  - PCI:0:2:0 | Mesa ANV + iHD |
  |  - Vulkan 1.4 Primary WSI Display |<==|  - NV12 dma_buf Video/ASTC    |
  +-----------------------------------+   +-------------------------------+
---
# 2. Detailed Layer-by-Layer Architecture Breakdown

The architecture is divided into four integrated layers:

Layer 1: Emulator Cartridges – Includes Eden Espresso Wave [AX] for Nintendo Switch emulation (git.eden-emu.dev primary with a GitHub mirror, GPL-3.0-or-later) and Cemu Latte [AX] for Wii U emulation (github.com primary, MPL-2.0), both operating under a two-tier release discipline (Stage A Win10 PGO vs. Stage B Batocera Linux). [1]

Layer 2: Shared C++20 Runtime SDK (ax-sdk) – A zero-dependency C++20 hardware abstraction layer hosted at ://github.com under the MIT License. It manages core topologies (2:3:1 masks, SCHED_FIFO), memory tiers (HugeTLB), VRAM shields, and lock-free SPSC audio rings, consumed via Buildroot BR2_EXTERNAL (ax-sdk.mk). [1]

Layer 3: Bare-Metal Linux Console OS Appliance (Piccolo-AX) – A standalone Buildroot BR2_EXTERNAL overlay on pinned upstream batocera.linux (://github.com, GPL-3.0). It boots directly into DDR4 RAM (toram <300MB) using Linux 6.12-rt with PREEMPT_RT, cgroup v2 hardware isolation, an uncomposited Xorg display server, and EROFS compression. [1]

Layer 4: Reference Silicon (Intel Core i5-9600K + GTX 980 Ti 6GB + UHD 630 + HiBy FC3) – Features an isolated cgroup v2 domain (0-4, NO_HZ_FULL) splitting audio timing, Vulkan command submission, and JIT guest CPU cores from a housekeeping sink (Core 5) and on-die UHD 630 iGPU for VA-API decoding. [1]

# 2. Detailed Layer-by-Layer Architecture Breakdown


Layer 1: Emulator Cartridges (Eden Espresso Wave [AX] & Cemu Latte [AX])

• Two-Tier Build Discipline (Stage A vs. Stage B):
	• Stage A (Windows 10 Control Baseline): Compiled from unmodified upstream code using Clang 23.1+ (ThinLTO + 3-Phase PGO), Coffee Lake flags (-march=skylake), Vulkan 1.4 SDK, and FFmpeg 7.1.x LTS to establish public Windows releases and baseline_pgo_metrics.json benchmarks.
	• Stage B (Batocera Linux [AX] Production): Links ax::core (ax-sdk) inside Piccolo-AX to activate core isolation, HugeTLB fastmem, Intel iGPU offloading, VRAM shielding, and bit-perfect USB DAC audio.
• Host-Matched Repository Strategy: Primary forks live on the same platform as their upstream projects (git.eden-emu.dev with a GitHub mirror for Eden; github.com for Cemu) so upstream syncs stay clean.

Layer 2: Shared Deterministic C++20 SDK (ax-sdk / libax_core)

• License & Portability: Published under the MIT License with zero external dependencies so it links cleanly into GPL-3.0, MPL-2.0, and GPLv2 emulators.
• Committed Modules (include/ax/ & src/):
	• topology.hpp + topology.cpp: Enforces 2:3:1 spatial core masks (0x01, 0x02, 0x1C, 0x20), detects SMT buddies, applies pthread_setaffinity_np + SCHED_FIFO on Linux (isolating <windows.h> + MMCSS Pro Audio inside topology.cpp), and probes /dev/cpu/0/msr (0xc8f) for Intel Cache Allocation Technology (CAT).
	• memory.hpp: Defines 1G / 2M HugeTLB fastmem arenas, single-cycle BMI1 (_bextr_u64) guest page extraction, and _MM_HINT_NTA non-temporal streaming hints when CAT 0xc8f is unavailable.
	• vram.hpp: Enforces VBLANK VRAM ceilings (5.50 GiB standard, 5.35 GiB with lsfg-vk 2.0.0+ bindless active) and exposes Maxwell GM200 capability flags (FP16→FP32 promotion, VK_QUEUE_TRANSFER_BIT, 256 MB BAR1 WC pool, SHFL subgroup reductions).
	• igpu.hpp: Defines zero-copy DRM_FORMAT_MOD_LINEAR NV12 dma_buf descriptors for UHD 630 → GTX 980 Ti handoffs and enforces the <= 3.50 GB/s DDR4 ring-bus bandwidth guardrail.
	• audio.hpp: Provides a cache-line-aligned (alignas(64)) lock-free SPSC stereo S32_LE ring buffer sized for a 96-frame (2.00 ms @ 48 kHz) exclusive USB DAC loop.

Layer 3: Bare-Metal Linux Console OS Platform (Piccolo-AX / BR2_EXTERNAL)

• Out-of-Tree Buildroot Overlay (batocera-ax-external/):
	• Core Metadata: external.desc (PICCOLO_AX), Config.in, and external.mk register Piccolo-AX cleanly on top of a pinned upstream batocera.linux release without maintaining a heavy full-tree fork.
	• linux-6.12-rt-skylake.fragment: Configures mainline Linux 6.12-rt with CONFIG_PREEMPT_RT=y, tickless NO_HZ_FULL, RCU_NOCB_CPU, pure cgroup v2, 1G/2M HugeTLB, EROFS, ZRAM (LZ4), and built-in (=y) SATA/xHCI/USB audio/i915 drivers.
	• grub.cfg, post-build.sh & post-image.sh: Sets isolcpus=managed_irq,domain,0-4, nohz_full=0-4, rcu_nocbs=0-4, 6 × 1G + 1024 × 2M HugePages, mitigations=off, and A/B rootfs.erofs.prev fallback; purges ~300 MB of unused NVIDIA compute/Mesa LLVM/PipeWire/Openbox bloat and enforces the <= 300 MiB (307,200 KB) EROFS lz4hc,9 budget gate.
• Runtime Boot, Isolation & Hardware Gate (rootfs-overlay/):
	• S01piccolo-isolate: Builds /sys/fs/cgroup/eden.slice (0-4, isolated partition), routes unbound workqueues and default IRQs to Core 5 (0x20), configures 512 MB LZ4 zram, locks GTX 980 Ti PCIe power state, and runs pre-mount e2fsck on SHARE.
	• S02piccolo-hw: Sets snd-usb-audio nrpacks=1 lowlatency=1, pins HiBy FC3 xHCI IRQ to Core 0 (0x01) and GTX 980 Ti MSI IRQ to Core 1 (0x02), enables nvidia-smi -pm 1, stages /dev/shm shader tmpfs directories, and mounts SwitchHDD asynchronously on Core 5.
	• K01piccolo-sync, xorg.conf & verify_ax_appliance.sh: Atomically flushes RAM shader caches (zstd -3 + sync) on shutdown, configures bare uncomposited Xorg page-flipping (PCI:1:0:0 + PCI:0:2:0), and runs a 7-point bare-metal hardware gate at boot.

Layer 4: Target Stream 1 Reference Silicon (i5-9600K + GTX 980 Ti + UHD 630 + HiBy FC3)

• Isolated Domain (Cores 0–4): Dedicated to fused audio/timing (Core 0), Vulkan submission (Core 1), and tickless guest CPU JIT execution (Cores 2–4).
• Housekeeping Sink (Core 5 / 0x20): Absorbs kernel kworkers, rcu_nocbs, SATA SwitchHDD I/O, USB HID polling, shader workers, and FFmpeg slices.
• Dual-GPU & USB Audio Interconnect: Pairs GTX 980 Ti (580.* driver, Vulkan 1.4 primary display) with Intel UHD 630 (iHD VA-API NV12 dma_buf decode) and HiBy FC3 USB DAC (hw:CARD=FC3,DEV=0 on Core 0).

3. Multi-Host Git Architecture, Branching & Licensing Strategy


Rule / Principle	How It Is Applied Across Your 4 Repositories
"Follow the Upstream Host" Rule	Primary forks live on the same platform as upstream (git.eden-emu.dev + GitHub mirror for Eden; github.com for Cemu, ax-sdk, and Piccolo-AX) so upstream PRs and syncs stay frictionless.
Aligned Default Branches	Matches upstream branch conventions (master for Eden-Espresso-Wave-AX; main for Cemu-Latte-AX, ax-sdk, and Piccolo-AX) so Buildroot .mk recipes and CMake FetchContent tags never mismatch.
Standalone BR2_EXTERNAL Design	Keeps Piccolo-AX tiny (< 10 MB) by pointing at a pinned upstream batocera.linux release tag instead of carrying gigabytes of unrelated ARM/handheld board history.
Cross-Repo License Matrix	ax-sdk (MIT) links cleanly into Eden-Espresso-Wave-AX (GPL-3.0), Cemu-Latte-AX (MPL-2.0), and Piccolo-AX (GPL-3.0) without license conflicts.


4. Complete Repository File Structures


Repository 1: Piccolo-AX (https://github.com) — GPL-3.0

text
Piccolo-AX/
├── LICENSE                                         # GPL-3.0
├── README.md                                       # Console OS overview & 4-repo network table
└── batocera-ax-external/
    ├── external.desc                               # name: PICCOLO_AX
    ├── Config.in                                   # Kconfig options (ax-sdk, eden-ax, cemu-latte-ax, lsfg-vk, vmtouch)
    ├── external.mk                                 # Includes package/*/*.mk
    ├── configs/
    │   └── linux-6.12-rt-skylake.fragment          # PREEMPT_RT, NO_HZ_FULL, cgroup v2, HugeTLB, EROFS, ZRAM
    ├── package/                                    # [STAGED NEXT]
    │   ├── ax-sdk/ax-sdk.mk                        # Builds libax_core into Buildroot staging
    │   ├── eden-ax/eden-ax.mk                      # Builds Eden Espresso Wave [AX] (Clang ThinLTO + ax-sdk)
    │   └── cemu-latte-ax/cemu-latte-ax.mk          # Builds Cemu Latte [AX] (Clang ThinLTO + ax-sdk)
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
                    └── axGenerator.py              # [STAGED NEXT] Multi-emulator launcher, vmtouch & SIGSTOP ES
Use code with caution.

Repository 2: ax-sdk (https://github.com) — MIT

text
ax-sdk/
├── CMakeLists.txt                                  # Builds static library target ax::core (C++20)
├── LICENSE                                         # MIT License
├── README.md                                       # SDK overview & 4-repo network table
├── include/ax/
│   ├── audio.hpp                                   # 64B-aligned lock-free SPSC stereo ring (96 frames / 2.00 ms)
│   ├── igpu.hpp                                    # DRM_FORMAT_MOD_LINEAR NV12 dma_buf & 3.50 GB/s guardrail
│   ├── memory.hpp                                  # 1G/2M HugeTLB fastmem API, BMI1 _bextr_u64, _MM_HINT_NTA
│   ├── topology.hpp                                # 2:3:1 core masks (0x01/0x02/0x1C/0x20) & SiliconProfile
│   └── vram.hpp                                    # 5.50 / 5.35 GiB VBLANK VRAM shield & GM200 flags
├── src/
│   ├── topology.cpp                                # Linux pthread/SCHED_FIFO/MSR 0xc8f + Win32 MMCSS shim
│   └── memory.cpp                                  # [STAGED NEXT] Linux MAP_HUGE_1GB/2MB & Win32 LargePage mmap
└── ax_toolchain/
    └── piccolo-clang-x86_64.cmake                  # [STAGED NEXT] Shared Clang 23.1+ ThinLTO + 3-Phase PGO file
Use code with caution.

Repository 3: Cemu-Latte-AX (https://github.com) — MPL-2.0

text
Cemu-Latte-AX/
├── README.md                                       # Merged dual-edition README + upstream Cemu links + network table
├── CMakeLists.txt                                  # [STAGED NEXT] Optional -DENABLE_AX_SDK=ON FetchContent hook
└── src/                                            # Clean upstream Cemu source ready for Stage A Win10 PGO bake
Use code with caution.

Repository 4: Eden-Espresso-Wave-AX (https://eden-emu.dev Primary + https://github.com Mirror) — GPL-3.0

text
Eden-Espresso-Wave-AX/
├── README.md                                       # Linked in 4-repo network table
└── src/ & Scripts/                                 # Active v1.34 codebase with Sprint 0 automation (consumes ax-sdk in v1.35.0)
Use code with caution.

# 🗺️ The Silicon Preservation Society — Ecosystem & Silicon Topology (`ECOSYSTEM_TOPOLOGY.md`)

**Programme:** The Silicon Preservation Society (`[AX]`)  
**Mission:** Keep proven PC hardware out of landfills through deterministic bare-metal optimization.  
**Last Updated:** `2026-11-18`

---

## 🔗 Overview

The Silicon Preservation Society (`[AX]`) maintains an open-source ecosystem dedicated to bare-metal hardware optimization, lightweight Linux appliances, and cross-platform emulator integration. 

### Core Components
* **Piccolo `[AX]`:** A `< 300 MB` bare-metal Linux console OS (`BR2_EXTERNAL`) based on `batocera.linux` (`GPL-3.0`), coming soon to https://github.com.
* **`ax-sdk`:** A dependency-free C++20 runtime core (`libax_core`) under the `MIT` license, available at https://github.com.
* **Eden Espresso Wave `[AX]`:** A Nintendo Switch bridge integrating Windows 10 and Piccolo Linux (`GPL-3.0`), hosted at https://eden-emu.dev with a mirror at https://github.com.
* **Cemu Latte `[AX]`:** A Nintendo Wii U optimization baseline (`MPL-2.0`), available at https://github.com.

For complete directory structures, build configurations, and technical specifications across the 4-layer architecture, please consult the respective repositories linked above.