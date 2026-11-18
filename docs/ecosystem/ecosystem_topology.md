# 🗺️ End-to-End Ecosystem & Hardware Topology Map

**Programme:** The Silicon Preservation Society (`[AX]`)  
**Mission:** Keep proven PC hardware out of landfills through deterministic bare-metal optimization.  
**Last Updated:** `2026-11-18`

The full multi-repository and silicon topology is structured across four primary layers—ranging from the upper emulator cartridges and shared C++20 runtime core to the bare-metal Linux console OS appliance and the target reference silicon (Intel Core i5-9600K combined with an NVIDIA GTX 980 Ti, Intel UHD 630 iGPU, and HiBy FC3 USB DAC).

---

## 1. 🏗️ End-to-End 4-Layer Ecosystem & Silicon Topology

| Layer | Component / Target | Repository & License | Architectural Role & Execution Specifications |
| :--- | :--- | :--- | :--- |
| **Layer 1: Emulator Cartridges** | **Eden Espresso Wave `[AX]`** (Switch)<br>**Cemu Latte `[AX]`** (Wii U) | [Eden Forgejo](https://git.eden-emu.dev/galaxywarriordad/Eden-Espresso-Wave-AX.git) / [GitHub Mirror](https://github.com/galaxywarriordad/Eden-Espresso-Wave-AX) (`GPL-3.0`)<br>[Cemu-Latte-AX](https://github.com/galaxywarriordad/Cemu-Latte-AX) (`MPL-2.0`) | **Two-Tier Release Discipline:**<br>• **Stage A (Win10 Control):** Clang 23.1+ ThinLTO + 3-Phase PGO (`-march=skylake`), Vulkan 1.4.<br>• **Stage B (Piccolo Linux):** Statically links `ax::core` (`libax_core.a`) for full hardware isolation. |
| **Layer 2: Shared C++20 Runtime SDK** | **`ax-sdk`** (`libax_core`) | [github.com/galaxywarriordad/ax-sdk](https://github.com/galaxywarriordad/ax-sdk) (`MIT`) | Dependency-free C++20 hardware abstraction layer (`topology`, `memory`, `vram`, `igpu`, `audio`) integrated via Buildroot `BR2_EXTERNAL` (`ax-sdk.mk`). |
| **Layer 3: Bare-Metal Console OS** | **Piccolo `[AX]`** (`BR2_EXTERNAL`) | [github.com/galaxywarriordad/Piccolo-AX](https://github.com/galaxywarriordad/Piccolo-AX) (`GPL-3.0`) | `< 300 MB` RAM-resident EROFS (`lz4hc,9`) appliance on pinned `batocera.linux` + Linux `6.12-rt` (`PREEMPT_RT`, `NO_HZ_FULL`, `cgroup v2`, ZRAM LZ4, uncomposited Xorg). |
| **Layer 4: Reference Silicon** | **Intel Core i5-9600K** (`6C/6T`, Zero SMT)<br>**GTX 980 Ti** + **UHD 630** + **HiBy FC3** | Bare-Metal Stream 1 Hardware | **Isolated `cgroup v2` Domain (`Cores 0–4`):** Core 0 (`0x01`) fused Audio/Timing (`SCHED_FIFO 99`), Core 1 (`0x02`) Vulkan Queue + Dirty Gather, Cores 2–4 (`0x1C`) tickless Guest JIT.<br>**Housekeeping Sink (`Core 5` / `0x20`):** Kernel `kworker`s, `rcu_nocbs`, SATA `SwitchHDD`, FFmpeg/SPIR-V.<br>**GPUs & DAC:** GTX 980 Ti (`PCI:1:0:0`, Driver `580.*`), UHD 630 (`PCI:0:2:0`, `iHD` VA-API `NV12` `dma_buf`), HiBy FC3 DAC (`hw:CARD=FC3,DEV=0`). |

---

## 2. 🔍 Detailed Layer-by-Layer Architecture Breakdown

### Layer 1: Emulator Cartridges (`Eden Espresso Wave [AX]` & `Cemu Latte [AX]`)

* **Two-Tier Build Discipline (Stage A vs. Stage B):**
  * **Stage A (Windows 10 Control Baseline):** Compiled from unmodified upstream code using Clang 23.1+ (ThinLTO + 3-Phase PGO), Coffee Lake flags (`-march=skylake`), Vulkan 1.4 SDK, and FFmpeg 7.1.x LTS to establish public Windows releases and `baseline_pgo_metrics.json` benchmarks.
  * **Stage B (Batocera Linux `[AX]` Production):** Links `ax::core` (`ax-sdk`) inside Piccolo `[AX]` to activate core isolation, HugeTLB fastmem, Intel iGPU offloading, VRAM shielding, and bit-perfect USB DAC audio.
* **Host-Matched Repository Strategy:** Primary forks live on the same platform as their upstream projects (`git.eden-emu.dev` with a GitHub mirror for Eden; `github.com` for Cemu) so upstream syncs stay clean.

### Layer 2: Shared Deterministic C++20 SDK (`ax-sdk` / `libax_core`)

* **License & Portability:** Published under the **MIT License** with zero external dependencies so it links cleanly into `GPL-3.0`, `MPL-2.0`, and `GPLv2` emulators.
* **Committed Modules (`include/ax/` & `src/`):**
  * `topology.hpp` + `topology.cpp`: Enforces `2:3:1` spatial core masks (`0x01`, `0x02`, `0x1C`, `0x20`), detects SMT buddies, applies `pthread_setaffinity_np` + `SCHED_FIFO` on Linux (isolating `<windows.h>` + MMCSS Pro Audio inside `topology.cpp`), and probes `/dev/cpu/0/msr` (`0xc8f`) for Intel Cache Allocation Technology (CAT).
  * `memory.hpp` + `memory.cpp`: Defines `1G` / `2M` HugeTLB fastmem arenas, single-cycle BMI1 (`_bextr_u64`) guest page extraction, and `_MM_HINT_NTA` non-temporal streaming hints when CAT `0xc8f` is unavailable.
  * `vram.hpp`: Enforces VBLANK VRAM ceilings (`5.50 GiB` standard, `5.35 GiB` with `lsfg-vk 2.0.0+` bindless active) and exposes Maxwell GM200 capability flags (`FP16 → FP32` promotion, `VK_QUEUE_TRANSFER_BIT`, `256 MB BAR1` WC pool, `SHFL` subgroup reductions).
  * `igpu.hpp`: Defines zero-copy `DRM_FORMAT_MOD_LINEAR` `NV12` `dma_buf` descriptors for UHD 630 → GTX 980 Ti handoffs and enforces the `<= 3.50 GB/s` DDR4 ring-bus bandwidth guardrail.
  * `audio.hpp`: Provides a cache-line-aligned (`alignas(64)`) lock-free SPSC stereo `S32_LE` ring buffer sized for a 96-frame (`2.00 ms` @ `48 kHz`) exclusive USB DAC loop.

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
* **Dual-GPU & USB Audio Interconnect:** Pairs GTX 980 Ti (`580.*` driver, Vulkan 1.4 primary display) with Intel UHD 630 (`iHD` VA-API `NV12` `dma_buf` decode) and HiBy FC3 USB DAC (`hw:CARD=FC3,DEV=0` on Core 0).

---

## 3. 🌿 Multi-Host Git Architecture, Branching & Licensing Strategy

| Rule / Principle | How It Is Applied Across Your 4 Repositories |
| :--- | :--- |
| **"Follow the Upstream Host" Rule** | Primary forks live on the same platform as upstream (`git.eden-emu.dev` + GitHub mirror for Eden; `github.com` for Cemu, `ax-sdk`, and `Piccolo-AX`) so upstream PRs and syncs stay frictionless. |
| **Aligned Default Branches** | Matches upstream branch conventions (`master` for `Eden-Espresso-Wave-AX`; `main` for `Cemu-Latte-AX`, `ax-sdk`, and `Piccolo-AX`) so Buildroot `.mk` recipes and CMake `FetchContent` tags never mismatch. |
| **Standalone `BR2_EXTERNAL` Design** | Keeps `Piccolo-AX` tiny (`< 10 MB`) by pointing at a pinned upstream `batocera.linux` release tag instead of carrying gigabytes of unrelated ARM/handheld board history. |
| **Cross-Repo License Matrix** | `ax-sdk` (`MIT`) links cleanly into `Eden-Espresso-Wave-AX` (`GPL-3.0`), `Cemu-Latte-AX` (`MPL-2.0`), and `Piccolo-AX` (`GPL-3.0`) without license conflicts. |

---

## 4. 📂 Complete Repository File Structures

### Repository 1: `Piccolo-AX` ([github.com/galaxywarriordad/Piccolo-AX](https://github.com/galaxywarriordad/Piccolo-AX)) — `GPL-3.0`

```text
Piccolo-AX/
├── LICENSE                                         # GPL-3.0
├── README.md                                       # Console OS overview & 4-repo network table
└── batocera-ax-external/
    ├── external.desc                               # name: PICCOLO_AX
    ├── Config.in                                   # Kconfig options (ax-sdk, eden-ax, cemu-latte-ax, lsfg-vk, vmtouch)
    ├── external.mk                                 # Includes package/*/*.mk
    ├── configs/
    │   └── linux-6.12-rt-skylake.fragment          # PREEMPT_RT, NO_HZ_FULL, cgroup v2, HugeTLB, EROFS, ZRAM
    ├── package/
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
                    └── axGenerator.py              # Multi-emulator launcher, vmtouch & SIGSTOP ES
```

### Repository 2: `ax-sdk` ([github.com/galaxywarriordad/ax-sdk](https://github.com/galaxywarriordad/ax-sdk)) — `MIT`

```text
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
│   └── memory.cpp                                  # Linux MAP_HUGE_1GB/2MB & Win32 LargePage mmap
└── ax_toolchain/
    └── piccolo-clang-x86_64.cmake                  # Shared Clang 23.1+ ThinLTO + 3-Phase PGO file
```

### Repository 3: `Cemu-Latte-AX` ([github.com/galaxywarriordad/Cemu-Latte-AX](https://github.com/galaxywarriordad/Cemu-Latte-AX)) — `MPL-2.0`

```text
Cemu-Latte-AX/
├── README.md                                       # Merged dual-edition README + upstream Cemu links + network table
├── CMakeLists.txt                                  # Optional -DENABLE_AX_SDK=ON FetchContent hook
└── src/                                            # Clean upstream Cemu source ready for Stage A Win10 PGO bake
```

### Repository 4: `Eden-Espresso-Wave-AX` ([Eden Forgejo](https://git.eden-emu.dev/galaxywarriordad/Eden-Espresso-Wave-AX.git) Primary + [GitHub Mirror](https://github.com/galaxywarriordad/Eden-Espresso-Wave-AX)) — `GPL-3.0`

```text
Eden-Espresso-Wave-AX/
├── README.md                                       # Linked in 4-repo network table
└── src/ & Scripts/                                 # Active v1.34 codebase with Sprint 0 automation (consumes ax-sdk in v1.35.0)
```