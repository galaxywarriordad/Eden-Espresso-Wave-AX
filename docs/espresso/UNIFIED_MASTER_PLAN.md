# 🌐 Unified Dual-Target Master Plan (v1.3)

**Updated:** 2026-11-17 (Post-Recovery Clean Build, Shader Macro Harmonization & Pipeline Telemetry Sprint)

This master plan consolidates **Eden Espresso Wave [AX]** (Windows 10 desktop build, `MASTER v1.34` → `v2.00`) and **Eden Piccolo [AX]** (Batocera Linux USB 3.0 appliance, `v0.6` → `v2.00`) into a single coordinated C++20 engineering programme. Rather than maintaining divergent forks, both targets compile from one shared silicon core while isolating OS-specific primitives behind `#ifdef _WIN32` and `#ifdef __linux__` platform shims.

## 📑 Table of Contents
- [Chapter 1: Dual-Target Convergence Architecture (Espresso Wave [AX] + Piccolo [AX])](#-chapter-1-dual-target-convergence-architecture-espresso-wave-ax--piccolo-ax)
- [Chapter 2: Shared C++20 Repository, Toolchain & Branch Strategy](#-chapter-2-shared-c20-repository-toolchain--branch-strategy)
- [Chapter 3: Unified Cross-Platform Sprint Sequence (Sprint 0 → Sprint 6)](#-chapter-3-unified-cross-platform-sprint-sequence-sprint-0--sprint-6)
- [Chapter 4: Cross-Platform Regression & Telemetry Gate Matrix](#-chapter-4-cross-platform-regression--telemetry-gate-matrix)
- [Chapter 5: Architectural & Technical Analysis and Future Optimisations](#-chapter-5-architectural--technical-analysis-and-future-optimisations)

---

# 🌐 Chapter 1: Dual-Target Convergence Architecture (Espresso Wave [AX] + Piccolo [AX])

## ⚙️ 1.1 Architectural Convergence Principle

Both **Eden Espresso Wave [AX]** (Windows 10) and **Eden Piccolo [AX]** (Batocera Linux `< 300 MB` `toram` USB 3.0 appliance) target identical host silicon:

* **CPU:** Intel Core i5-9600K (6C/6T, zero SMT; **locked symmetrically at `x48` / `4.8 GHz` across all 6 physical cores**, Uncore Ring at `x45` / `4.5 GHz`, 9 MB L3)
* **RAM:** 32 GB Dual-Channel DDR4-3600 (`tREFI = 65535`; `VCCSA/VCCIO = 1.15–1.20 V`)
* **dGPU:** NVIDIA GeForce GTX 980 Ti (6 GB GDDR5, Maxwell GM200; `5.50 GB` / `5.35 GB` hard VRAM ceiling)
* **iGPU:** Intel UHD Graphics 630 (`0x8086`)
* **Audio:** HiBy FC3 USB DAC (ESS Sabre ES9281AC Pro; 24-bit / 48 kHz native)

**Convergence Strategy:**
* **Unified Core (~90% Shared):** The C++20 codebase compiles identically across both platforms using `-march=skylake -mavx2 -mfma -mbmi -mbmi2 -O3 -flto=thin` and two-stage Profile-Guided Optimisation (`clang-cl 23+` on Windows; `clang-23` + `lld-23` on Linux).
* **Isolated OS Boundary:** All OS-specific syscalls, memory allocation flags, hardware video handles, and audio drivers remain strictly encapsulated within three `#ifdef _WIN32` / `#ifdef __linux__` shim headers (see Section 2.1).

## 🧩 1.2 Shared Platform-Agnostic Silicon Core Modules (Zero OS Forking)

| Subsystem Cluster | Primary Source Files | Shared Microarchitectural Implementation (`i5-9600K` + `GM200`) |
| :--- | :--- | :--- |
| **Virtual Memory & BMI1 Translation** | `device_memory.h/.cpp`<br>`bmi_util.h`<br>`memory.h/.cpp`<br>`device_memory_manager.h/.cpp` | • Reserves a 512 GB contiguous guest virtual address space (`1ULL << 39`).<br>• Performs single-cycle page extraction via 3-arg BMI1 `_bextr_u64(vaddr, YUZU_PAGEBITS, 36)`.<br>• Uses split O(1) vs. `MULTI_FLAG` GMMU resolution and incorporates **Eden PR #3817** (`HeapTracker` leak removal). |
| **Lock-Free Dirty Memory Tracking** | `gpu_dirty_memory_manager.h/.cpp` | • Tracks dirty pages on guest threads via 8-byte `TransformAddress` weak-CAS (`compare_exchange_weak`).<br>• Executes zero-copy `front_buffer.swap(back_buffer)` and `TZCNT`/`BLSR` bit-scans inside `Gather()` on Core 1 (`0x02`). |
| **Fiber Scheduler & Fibonacci Timeline** | `cpu_manager.h/.cpp`<br>`core_timing.h/.cpp`<br>`speed_limiter.h/.cpp`<br>`eden_engine.ini` | • Schedules user-mode `Common::Fiber` contexts across a home-biased `0x1C` pool (`C0`→`0x04`, `C1`→`0x08`, `C2`→`0x10`) with Core 3 elastic spillover (`0x3C`) and config-driven 2:3:1 physical P-core scaling.<br>• Drives `CoreTiming` via an O(1) Boost Fibonacci Heap protected by `std::scoped_lock{advance_lock, basic_lock}`. |
| **Zero-Heap Audio DSP Graph & Micro-PLL** | `audio_core.h/.cpp`<br>`system_manager.h/.cpp`<br>`command_generator.h/.cpp`<br>`lockfree_audio_queue.hpp` | • Runs an inline `std::optional` `AudioManager` and ADSP microkernel backed by a 64B-aligned lock-free PCM ring (2048 slots), `WorkbufferAllocator` + `std::construct_at`, and `0xCAFEBABE` command validation.<br>• Uses `libswresample` AVX2 FIR (`SWR_DITHER_TRIANGULAR_HIGHPASS`) as a software micro-pitch PLL (`±0.005%`) driven by ring-buffer fill watermarks to absorb Z390-to-DAC crystal drift, plus **Eden PR #2** biquad bounds checks. |
| **Maxwell GM200 Vulkan, Sparse VMA & Shader Safety** | `macro_jit_compiler.cpp`<br>`vk_ubo_passthrough.cpp`<br>`vk_texture_cache.cpp`<br>`texture_cache.h`<br>`emit_spirv.cpp`<br>`texture_reaper.cpp` | • Emits 64B-aligned, 32-bit sign-extended Xbyak GPU Macro-JIT code.<br>• Enforces page-contiguity UBO passthrough and `maxPushDescriptors` clamps (**Citron PR #4688, #449, #383**), procedural draw fastpaths (**#448**), subresource clamps (**#385, #447**), and `128 MiB` / `16 MiB` / 128-slice unswizzle limits.<br>• Backs dynamic render targets with a 64 KB Sparse Page-Aliased VMA pool (`VK_KHR_bind_memory2`) to eliminate mid-game `vkAllocateMemory` stalls.<br>• Supports `BC3`/`BC7`/`BC6H` plus Option C `< 4.5 GB` `RGBA8`, `emit_spirv.cpp` `lmem` register-spill condensation, uniform `#version 460 core` + `GL_GOOGLE_include_directive` shader headers, and **Eden PR #32** CPU-delta overlap invalidation. |
| **Quarantine Workers, Loading Burst & Telemetry** | `ncz_decompressor.cpp`<br>`vk_pipeline_worker.h/.cpp`<br>`vk_cache_decompression.cpp`<br>`perf_stats.h/.cpp` | • Quarantines background tasks onto Core 5 (`0x20`) via `ScopedNczQuarantine`, with loading-screen core bursting across `0x1C` before guest fibers spin up for 3× faster `.ncz` extraction.<br>• Enforces `ASYNC_WORKER_CORE_CLAMP = 1`, 32B-aligned AVX2 non-temporal cache decompression (`_mm256_stream_si256` + `_MM_HINT_NTA`), and `FFmpeg::execute2` slice routing.<br>• Exports lock-free `g_metrics` atomic counters consumed by `telemetry_daemon.ps1`. |

## 🔀 1.3 Platform Shim Abstraction Matrix

### 📌 Shim 1: `src/common/ax_platform_shim.hpp` (Core OS, Memory, Audio, Timing & HID)

| Capability | `#ifdef _WIN32` (Espresso Wave [AX]) | `#ifdef __linux__` (Piccolo [AX]) | Architectural Equivalence |
| :--- | :--- | :--- | :--- |
| **Core Affinity & Isolation** | `SetThreadAffinityMask` + `SetThreadIdealProcessor` (2:3:1 config-driven mask; reversible `irq_runtime_helper.ps1`) | `pthread_setaffinity_np` inside cgroups v2 (`cpuset.cpus.partition = isolated` on `0-4`, `nohz_full=0-4`, `rcu_nocbs=0-4`, `0x20` quarantine) | Enforces identical Core 0 / Core 1 / Cores 2–4 / Core 5 spatial boundaries across a completely symmetrical frequency domain. |
| **Large / Huge Page Fastmem** | `EnableLockMemoryPrivilege` (`SeLockMemoryPrivilege`) + `VirtualAlloc(..., MEM_LARGE_PAGES)` (2 MB pages) | `mmap(..., MAP_HUGETLB \| MAP_HUGE_1GB)` (6.0 GB pool) + `MAP_HUGE_2MB` (2.0 GB pool) + `mlockall(MCL_CURRENT \| MCL_FUTURE)` | Eliminates Skylake DTLB/STLB page-table walks across the 512 GB guest fastmem arena and JIT cache. |
| **Bit-Perfect Audio Output** | FlexASIO / WASAPI Exclusive (128 samples / `2.67 ms`) + `ScopedProAudio` (MMCSS: `"Pro Audio"` `AVRT_PRIORITY_CRITICAL`) | • Fused single-thread Core 0 (4.8 GHz) `SCHED_FIFO 99` direct ALSA `hw:CARD=FC3,DEV=0` (`S32_LE`, 48 kHz, 96 frames × 2 = `2.00 ms`, 128-frame auto-fallback)<br>• Core 0 xHCI MSI IRQ (`itr=0`) | Keeps the entire PCM ring inside Core 0's private 256 KB L2 cache with zero frequency scaling or jitter variations. |
| **Sub-Microsecond Frame Pacing** | `timeBeginPeriod(1)` + `CreateWaitableTimerExW(..., HIGH_RESOLUTION)` + `_mm_pause()` spin tail | `clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, ...)` down to `150 µs` before VBLANK + calibrated `_mm_pause()` spin tail | Achieves `< 1 µs` frame-pacing accuracy without burning 100% CPU in busy-wait loops. |
| **Direct USB HID & Exit Combo** | Win32 `RawInput` / `hidapi` pinned to Core 5 (`0x20`) | Core 5 `SCHED_FIFO 80` `/dev/hidraw*` reader (`usbhid.jspoll=1`, `-1.5 ms` VBLANK latch + `Select+Start` 1.5s exit hook) | Shaves 1–2 frames off input-to-photon latency and provides keyboard-free console exit on bare Xorg. |

### 📌 Shim 2: `src/video_core/renderer_vulkan/vk_vram_shield.hpp` (VRAM Budget & Frame Gen)

| Capability | `#ifdef _WIN32` (Espresso Wave [AX]) | `#ifdef __linux__` (Piccolo [AX]) | Architectural Equivalence |
| :--- | :--- | :--- | :--- |
| **VRAM Over-Subscription Guard** | `IDXGIAdapter3::QueryVideoMemoryInfo` polled at VBLANK (`5.50 GB` ceiling; `5.20 GB` with Lossless Scaling) | `VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT` + VBLANK `VK_EXT_memory_budget` poll (`5.50 GB` ceiling; `5.35 GB` with `lsfg-vk 2.0.0+`) | Prevents WDDM/NVIDIA system-RAM VRAM paging stalls (`20–45 ms` freezes) via 64 KB sparse VMA aliasing & `< 64 KB` defragmentation. |
| **Frame Generation Interop** | Native Win32 `Lossless.dll` via zero-copy WinRT `Windows.Graphics.Capture` (`vk_wgc_capture.cpp`) | Implicit Vulkan 1.2 bindless layer `lsfg-vk 2.0.0+` (forced FP32 shader path for Maxwell GM200, BYO Steam `Lossless.dll`) on bare uncomposited Xorg | Delivers 2× interpolated smoothness while avoiding Maxwell's throttled 1:64 FP16 ALUs. |

### 📌 Shim 3: `src/video_core/host1x/vaapi_dmabuf_bridge.hpp` (Cross-GPU Video & VME Offload)

| Capability | `#ifdef _WIN32` (Espresso Wave [AX]) | `#ifdef __linux__` (Piccolo [AX]) | Architectural Equivalence |
| :--- | :--- | :--- | :--- || **iGPU Video Decode (UHD 630)** | Media Foundation / Intel QSV + `vpp_qsv` filter graph on Core 5 (`0x20`, `BelowNormal`) | Intel `intel-media-driver` (`iHD` VA-API) H.264/HEVC/VP8/VP9 decode on Core 5 (`0x20`) | Keeps 100% of GTX 980 Ti GDDR5 bandwidth free for 3D rendering during cutscenes. |
| **Zero-Copy Cross-GPU Handoff** | `IDXGIResource1` Windows NT Shared Handle → `VK_KHR_external_memory_win32` | • Exports linear NV12 surfaces via `vaExportSurfaceHandle` (`DRM_PRIME_2`, `DRM_FORMAT_MOD_LINEAR`) and imports into Vulkan via `VkImageDrmFormatModifierListCreateInfoEXT`.<br>• Enforces producer-consumer fencing via `vaSyncSurface()` on Core 5 (`0x20`) before passing the `dma_buf` FD to Core 1 (`0x02`).<br>• Transitions `VkImage` to `SHADER_READ_ONLY_OPTIMAL` via `VK_KHR_synchronization2` before sampling. | Transfers decoded NV12 frames from Intel UHD 630 directly into GTX 980 Ti Vulkan VRAM with zero CPU copy. |
| **Phase 6 VME Optical Flow (v2.00)** | GTX 980 Ti 720p/540p `R8_UNORM` luma downscale → UHD 630 NT Shared Handle → Intel VME motion vectors → `Lossless.dll` | GTX 980 Ti 720p/540p `R8_UNORM` luma downscale → UHD 630 `dma_buf` FD → Intel VME motion vectors → `lsfg-vk 2.0.0+` | Downscaling to luma-only beforehand cuts PCIe Gen 3 bandwidth by ~75–85% while eliminating HUD warping on Maxwell. |

## 💾 1.4 Storage & Asset Pipeline Convergence (NVMe/HDD vs. FIT Plus + io_uring/vmtouch)

* **Windows (Espresso Wave [AX]):**
  * **NVMe #1:** Windows OS, `eden.exe` binary, and compiled Vulkan/NVIDIA shader caches.
  * **NVMe #2:** Title updates/DLCs (`< 200 MB`) and active `.ncz` hot ring.
  * **7200 RPM HDD:** Bulk `SwitchHDD` cold archive (`.chd` and `.ncz` library).
* **Linux Appliance (Piccolo [AX]):**
  * **Samsung FIT Plus 64 GB USB 3.1:** `4.0 GB` FAT32 `BATOCERA` partition (`< 300 MB` EROFS `toram` OS + A/B fallback slot) + `~55.6 GB` journal-free (`^has_journal`) ext4 `SHARE` partition for atomic `.tar.zst` `EdenCache` + NVIDIA `GLCache` archives.
  * **32 GB DDR4-3600 System RAM:** `17.5 GB` `io_uring` (`MADV_POPULATE_READ`) + `vmtouch -dql` container-filtered NCA page-cache buffer + `1.5 GB` `tmpfs` (`EdenCache` + `/dev/shm/nv_cache` via `__GL_SHADER_DISK_CACHE_PATH`).
  * **7200 RPM HDD:** `SwitchHDD` cold archive backing store (with optional outer-cylinder `SwitchHOT` primary partition for `~190–210 MB/s` linear read velocity).

---

# 🔀 Chapter 2: Shared C++20 Repository, Toolchain & Branch Strategy

## 📁 2.1 Single-Source Repository Layout

Both **Eden Espresso Wave [AX]** (Windows 10) and **Eden Piccolo [AX]** (Batocera Linux USB Appliance) build from one unified C++20 repository (`Eden4CoffeeLake` / `Eden-Espresso-Wave-AX`):

```text
Eden-Espresso-Wave-AX/
├── .gitattributes                      # Enforces CRLF on *.bat/*.ps1; LF on C++/MD/Shell/Buildroot files
├── ARCHITECTURE_SPEC.md                # Windows Espresso Wave [AX] Master Spec (v1.34)
├── EDEN_PICCOLO_SPEC.md                # Linux Piccolo [AX] Appliance Spec (v0.5)
├── UNIFIED_MASTER_PLAN.md              # Dual-Target Convergence Master Plan (v1.3)
├── BACKLOG.md                          # Cross-Synced Engineering Backlog (v1.34 -> v2.00)
├── WhatsNext.md                        # Active Rolling Sprint Plan (v1.34)
├── eden_engine.ini                     # Runtime 2:3:1 physical-core scaler & hardware feature flags
├── CMakeLists.txt                      # Unified CMake 3.28+ entry (WIN32 vs. LINUX target switches)
├── PGO_config.bat                      # Interactive build configurator, IRQ auditor & PGO stage launcher
├── src/
│   ├── common/
│   │   ├── ax_platform_shim.hpp        # [SHIM 1] Cores, 1G/2M HugePages, ALSA/FlexASIO, Timers, HID
│   │   ├── bmi_util.h                  # [SHARED] Single-cycle 3-arg _bextr_u64 BMI1 translation
│   │   └── lockfree_audio_queue.hpp    # [SHARED] 64B-aligned atomic PCM ring (2048 slots) + micro-PLL telemetry
│   ├── core/
│   │   ├── cpu_manager.cpp             # [SHARED] Home-biased Common::Fiber (0x1C) + Core 3 (0x3C)
│   │   ├── core_timing.cpp             # [SHARED] Boost Fibonacci Heap + closed-loop governor
│   │   └── memory.cpp                  # [SHARED] 512 GB VA fastmem + PR #3817 HeapTracker removal
│   ├── video_core/
│   │   ├── renderer_vulkan/
│   │   │   ├── vk_vram_shield.hpp      # [SHIM 2] DXGI vs. VK_EXT_memory_budget (5.50 / 5.35 / 5.20 GB)
│   │   │   ├── vk_ubo_passthrough.cpp  # [SHARED] Page-contiguity & maxPushDescriptors Maxwell guard
│   │   │   └── texture_reaper.cpp      # [SHARED] 64 KB Sparse VMA aliasing + < 64 KB VBLANK defrag
│   │   └── host1x/
│   │       └── vaapi_dmabuf_bridge.hpp # [SHIM 3] QSV NT Handle vs. iHD VA-API NV12 dma_buf FD
│   └── shader_recompiler/
│       └── backend/spirv/
│           └── emit_spirv.cpp          # [SHARED] Maxwell GM200 lmem register-spill condensation
├── Scripts/                            # Consolidated Windows automation & telemetry tooling:
│   ├── PGO_stage0.bat                  # Stage 0 high velocity build
│   ├── PGO_stage0LD.bat                # Stage 0 build + macro_update + shader_validate + telemetry gate
│   ├── PGO_stage1.bat                  # Stage 1 PGO instrumented build + automated quality gates
│   ├── PGO_stage2.bat                  # Stage 2 PGO L1i production bake + automated quality gates
│   ├── check_hardware.ps1              # Pre-flight hardware, privilege & IRQ readiness check
│   ├── set_irq_affinity.ps1            # Registry MSI IRQ affinity enforcement (0x01 / 0x02 / 0x20)
│   ├── irq_runtime_helper.ps1          # Reversible runtime MSI IRQ assignment helper
│   ├── macro_update.ps1                # Numeric shader macro harmonizer (USE_EASU, USE_RCAS, FP16, VERSION)
│   ├── validate_profile.ps1             # Pre-build GLSL 460 / SPIR-V validation via glslangValidator
│   └── track_telemetry.ps1             # Async per-core eden.exe profiler & JSON regression reporter
├── scripts/linux/                      # test_nv12_dmabuf.c, verify_ax_appliance.sh, fetch_piccolo_sources.sh
└── batocera-ax-external/               # Out-of-tree Batocera Buildroot overlay (Piccolo [AX])
    ├── external.desc
    ├── external.mk
    ├── Config.in
    ├── configs/
    │   └── piccolo_x86_64_defconfig
    ├── board/piccolo/
    │   ├── linux.config                # Skylake-only 6.12.x-rt kernel config
    │   ├── grub.cfg                    # isolcpus, nohz_full, 1G+2M HugePages, max_cstate=0, mitigations=off, A/B fallback
    │   ├── post-build.sh               # Strips CUDA/OptiX/NVENC/LLVM blobs (< 300 MB budget)
    │   └── post-image.sh               # Packs EROFS (-zlz4hc,9) + hard 307,200 KB size gate
    ├── package/
    │   ├── eden/                       # Clang 23+ ThinLTO + 2-stage PGO Buildroot package
    │   └── nvidia-driver-legacy580/    # Hard-pinned 580.* Maxwell driver (IGNORE_PREEMPT_RT_PRESENCE=1)
    └── rootfs-overlay/
        ├── etc/init.d/                 # S01piccolo-isolate, S02piccolo-hw, K01piccolo-sync (EdenCache + nv_cache)
        └── usr/share/batocera/configgen/generators/eden/
            └── edenGenerator.py
```

---

## 🌿 2.2 Branching & Commit Discipline (`master` + Short-Lived Feature Branches)

* **Protected Dual-Target Trunk (`master`):** Every commit merged to `master` must compile cleanly on both Windows (`clang-cl 23+`) and Linux (`clang-23` staging sysroot). Shared core fixes in `src/core/`, `src/audio_core/`, and `src/video_core/` land once and immediately benefit both targets.
* **Topic Branch Naming Convention:**
  * `feat/shared-*`: Platform-agnostic C++20 silicon optimisations (e.g., `feat/shared-bc7-compute`, `feat/shared-sparse-vma`).
  * `feat/win32-*`: Windows-only Espresso Wave [AX] shims or PowerShell/batch scripts.
  * `feat/piccolo-*`: Linux-only Piccolo [AX] shims, `batocera-ax-external/` Buildroot recipes, or init scripts.
* **Atomic Upstream Cherry-Picks:** Backports from upstream Eden (`#3817`, `#4819`, `#4210`, `#4664`, `#2`, `#32`) or Citron (`#4688`, `#449`, `#383`, `#448`, `#385`, `#447`, `#4966`, `#5102`, `#5311`) land as isolated cherry-pick commits with original PR references preserved in each commit message.
* **Reproducible Supply-Chain Pinning (`*.hash` & Exact Commit SHAs):** Pin every Buildroot package (`linux`, `lsfg-vk`, `vmtouch`, `media-driver`, and `eden`) to an immutable 40-character Git commit SHA or signed release tag accompanied by a Buildroot `.hash` (`sha256`) checksum file and the cached `NVIDIA-Linux-x86_64-580.126.09.run` archive.

---

## 🧹 2.3 Strict Preprocessor & Header Hygiene Rules (`#ifdef` Boundary)

1. **Zero OS Headers Outside Shim Files:** System headers (`<windows.h>`, `<timeapi.h>`, `<avrt.h>`, `<dxgi1_4.h>`, `<pthread.h>`, `<sched.h>`, `<sys/mman.h>`, `<alsa/asoundlib.h>`, and `<va/va_drmcommon.h>`) may only be `#include`d inside the three shim headers (`ax_platform_shim.hpp`, `vk_vram_shield.hpp`, `vaapi_dmabuf_bridge.hpp`) and their corresponding `.cpp` translation units.
2. **Uniform C++20 Interface Signatures:** Every function exposed by a shim header presents an identical C++20 signature to the shared core:
   ```cpp
   namespace Ax::Platform {
   enum class CoreRole : u8 {
       AudioTiming0,
       VulkanSubmit1,
       GuestJit234,
       Quarantine5
   };
   [[nodiscard]] bool PinCurrentThread(CoreRole role) noexcept;
   [[nodiscard]] void* AllocateFastmemArena(std::size_t bytes) noexcept;
   void PreciseSleepUntil(std::chrono::steady_clock::time_point target) noexcept;
   }
   ```

---

## 🔨 2.4 Lockstep Dual-Target Compiler & Two-Stage PGO Pipeline

| Build Parameter | Windows (Espresso Wave [AX]) | Linux (Piccolo [AX] via `eden.mk`) | Shared Microarchitectural Effect |
| :--- | :--- | :--- | :--- |
| **Compiler & Linker** | `clang-cl 23+` + `lld-link` | `clang++-23` + `ld.lld-23` (`--sysroot=$(STAGING_DIR)`) | Identical LLVM 23+ Skylake scheduler & ThinLTO optimiser. |
| **ISA & Intrinsics** | `/clang:-march=skylake /clang:-mavx2 /clang:-mfma /clang:-mbmi /clang:-mbmi2` | `-march=skylake -mavx2 -mfma -mbmi -mbmi2 -O3` | Unlocks single-cycle 3-arg `_bextr_u64` (BMI1), `TZCNT`/`BLSR` (BMI1), & AVX2 FIR/non-temporal stores (`_MM_HINT_NTA`). |
| **LTO Mode** | `-flto=thin` | `-flto=thin -fuse-ld=lld` | Enables cross-module inlining across `CoreTiming`, `AudioCore`, `VulkanWorker`, & `Dynarmic`. |
| **Pre-Build Shader Gate** | `macro_update.ps1` (`USE_EASU`, `USE_RCAS`, `YUZU_USE_FP16`, `VERSION`) + `shader_validate.ps1` (`glslangValidator`) | Build-time GLSL 460 core & `GL_GOOGLE_include_directive` header check | Prevents Vulkan SPIR-V preprocessor token conflicts and catches shader syntax errors before C++ compilation. |
| **PGO Stage 1 (Collect)** | `-fprofile-instr-generate` → writes `default.profraw` on NVMe #1; profiled via `telemetry_daemon.ps1` | `BR2_PACKAGE_EDEN_PGO_STAGE=1` (`-fprofile-instr-generate`) → writes `.profraw` to `EdenCache` `tmpfs` (zero USB flash wear) | Instruments hot branch edges during 3 × 15 min representative gameplay sweeps while verifying core/memory telemetry. |
| **PGO Merge Tool** | `llvm-profdata merge -output=eden.profdata *.profraw` | `llvm-profdata-23 merge -output=eden.profdata *.profraw` | Produces portable LLVM indexed profile data. |
| **PGO Stage 2 (Bake)** | `-fprofile-instr-use=eden.profdata` → `eden.exe` + automated telemetry regression check | `BR2_PACKAGE_EDEN_PGO_STAGE=2` (`-fprofile-use=eden.profdata`) → `rootfs.erofs` (`< 300 MB`) | Linearises hot basic blocks inside Skylake's 32 KB L1i cache. |

---

# 🚀 Chapter 3: Unified Cross-Platform Sprint Sequence (Sprint 0 → Sprint 6)

## 📊 3.1 Dual-Target Progression Flow

```text
+-----------------------+     +-----------------------+     +-----------------------+     +-----------------------+
| SPRINT 0: BARE-METAL  |     | SPRINT 1: MVP & CORE  |     | SPRINT 2: CACHE &     |     | SPRINT 3: VRAM &      |
| GATE [ACTIVE]         | --> | LOCK                  | --> | AUDIO                 | --> | BC7/BC6H              |
| (Win32 Stage 0 Triage |     | (Win32 v1.34 +        |     | (Win32 v1.35 +        |     | (Win32 v1.35 +        |
|  & Piccolo RT Gate)   |     |  Piccolo v0.6)        |     |  Piccolo v0.7)        |     |  Piccolo v0.8)        |
+-----------------------+     +-----------------------+     +-----------------------+     +-----------------------+
                                                                                                      |
+-----------------------+     +-----------------------+     +-----------------------+                 |
| SPRINT 6: BLUE-SKY    |     | SPRINT 5: PGO         |     | SPRINT 4: CROSS-GPU   |                 |
| R&D                   | <-- | PRODUCTION            | <-- | & FG                  | <---------------+
| (Unified v2.00        |     | (Win32 v1.37 +        |     | (Win32 v1.36 +        |
|  Luma VME & Dongle)   |     |  Piccolo v1.0)        |     |  Piccolo v0.9)        |
+-----------------------+     +-----------------------+     +-----------------------+
```
### 🚦 Sprint 0: Bare-Metal Silicon & Driver Risk-Retirement Gate *(Current Active Gate)*

**Objective:** Validate the highest-severity hardware, kernel, and cross-GPU assumptions on the live i5-9600K + Z390 + GTX 980 Ti + UHD 630 + HiBy FC3 test rig before writing full Buildroot or C++ integration code.

* **Shared Silicon Tasks:**
  * **Lock Z390 BIOS parameters:** Symmetrical execution lock at `x48` (`4.8 GHz`) on Cores 0–5, Uncore/Ring ratio locked to `x45` (`4.5 GHz`), Ring Downbin = Disabled, AVX Offset = `0`, XMP DDR4-3600 (`tREFI = 65535`), and `VCCSA/VCCIO = 1.15–1.20 V`.
  * **Verify USB controller isolation:** Inspect `lspci -nn | grep -i usb` (Linux) and Device Manager (Windows) to confirm that the HiBy FC3 DAC and Samsung FIT Plus 64 GB reside on independent xHCI PCIe root hubs.
  * **Intel CAT (`resctrl`) Silicon MSR Check:** Test `rdmsr 0xc8f` on bare metal. Because desktop Coffee Lake CPUs typically fuse off CAT MSRs in hardware, confirm status early and engage `_mm_prefetch(..., _MM_HINT_NTA)` non-temporal streaming reads on Core 5 (`0x20`) as the zero-overhead software L3 protection path.
* **Windows (Espresso Wave [AX]) Track:**
  * *(Completed Tooling)* Automated `macro_update.ps1`, `shader_validate.ps1`, `telemetry_daemon.ps1`, and live IRQ affinity audit display inside the root configurator menu, integrated across `PGO_stage0LD.bat`, `PGO_stage1.bat`, and `PGO_stage2.bat`.
  * *(Active Execution)* Compile `PGO_stage0` binary via `PGO_stage0LD.bat`, run Stage 0 regression triage with `telemetry_daemon.ps1`, and confirm `Scripts\check_hardware.ps1` (`SeLockMemoryPrivilege`, `0x01`/`0x02`/`0x20` IRQ masks, and `128`-sample / `2.67 ms` FlexASIO exclusive output).
* **Linux (Piccolo [AX]) Track (Temporary Ubuntu/Arch Live USB Bench):**
  * Boot mainline Linux 6.12.x LTS (`CONFIG_PREEMPT_RT=y` and `CONFIG_DRM_TTM_HELPER=y`) with the hard-pinned NVIDIA `580.*` legacy driver (`IGNORE_PREEMPT_RT_PRESENCE=1`) and persistence mode enabled (`nvidia-smi -pm 1`, P0 boost state).
  * Compile and run standalone `test_nv12_dmabuf.c` (`VkImageDrmFormatModifierListCreateInfoEXT`) to prove zero-copy `DRM_FORMAT_MOD_LINEAR` NV12 surface handoff from Intel UHD 630 (`iHD` VA-API) into GTX 980 Ti Vulkan.
  * Execute a 60-second `speaker-test` on `hw:CARD=FC3,DEV=0` (`S32_LE`, 48 kHz, `nrpacks=1`) and confirm `0` xruns.
* **Mandatory Exit Gate:** Both Windows `Scripts\check_hardware.ps1` + Stage 0 regression triage and Linux `test_nv12_dmabuf.c` + 0-xrun ALSA checks pass on bare metal.

### 🧱 Sprint 1: Core Partitioning, Shim Foundation & First Bootable USB MVP

**Target Versions:** Windows `v1.34` / `v1.35` Hardened Baseline | Linux Piccolo `v0.6` Bootable USB MVP

* **Shared C++20 Core Tasks:**
  * Commit the `ax_platform_shim.hpp` interface (`CoreRole` affinity, fastmem allocation, precision sleep) and enforce `#ifdef _WIN32` / `#ifdef __linux__` header isolation.
  * Lock the home-biased `Common::Fiber` guest pool (`0x1C`: `C0`→`0x04`, `C1`→`0x08`, `C2`→`0x10`) alongside Guest Core 3 elastic spillover (`0x3C`).
  * Quarantine background workers to Core 5 (`0x20`) via `ScopedNczQuarantine`—adding loading-screen core bursting across `0x1C` before guest fibers spin up for ~3× faster `.ncz` extraction.
  * Enforce `ASYNC_WORKER_CORE_CLAMP = 1`, `FFmpeg::execute2` slice routing, uniform `#version 460 core` + `#extension GL_GOOGLE_include_directive` shader headers, and Eden PRs `#4819`, `#4210`, `#4664`, `#2`, and `#3817` (`HeapTracker` leak removal).
* **Windows (Espresso Wave [AX]) Track:**
  * Verify `clang-cl 23+` (`-march=skylake`, ThinLTO) build stability and Vulkan 1.4 dynamic dispatch compliance (`VK_NO_PROTOTYPES`) on `v1.34`, and harden `telemetry_daemon.ps1` with ASIO underrun and voice drop counters.
* **Linux (Piccolo [AX]) Track:**
  * **Buildroot & Size Gate:** Scaffold `batocera-ax-external/` using a split toolchain (GCC for base OS packages; Clang 23+ for `eden`), strip excluded packages (PipeWire, PulseAudio, Openbox, CUDA/OptiX/NVENC, Mesa LLVM/Gallium), and enforce the `< 300 MB` `post-image.sh` size assertion.
  * **Storage & Boot:** Partition the Samsung FIT Plus 64 GB drive (`4.0 GB` FAT32 `BATOCERA` + `~55.6 GB` `^has_journal` ext4 `SHARE`) and implement `initrd` `dd bs=4M` `toram` EROFS (`lz4hc,9`) boot with `mitigations=off` in `grub.cfg` (eliminating 8–18% Spectre/Meltdown/Retbleed syscall overhead on this offline appliance).
  * **Runtime Services & Audio/GPU:**
    * Deploy `S01piccolo-isolate`, `S02piccolo-hw` (async Core 5 `SwitchHDD` mount), and `K01piccolo-sync` (atomic `zstd -3` `.tar.zst` persistence of both `EdenCache` and `__GL_SHADER_DISK_CACHE_PATH=/dev/shm/nv_cache` so NVIDIA `580.*` Maxwell microcode caches persist across reboots).
    * Configure bare uncomposited Xorg (`SIGSTOP` EmulationStation), `512 MB` LZ4 `zram`, `8.0 GB` 2M HugePages, 128-frame (`2.67 ms`) Core 0 ALSA output, and Hybrid BC3 compression.
  * **A/B Image Rollback:** Retain the previous verified image on the `4.0 GB` FAT32 `BATOCERA` partition as `/boot/boot/rootfs.erofs.prev` (`~600 MB` combined for A/B slots) with a secondary GRUB entry (`Eden Piccolo [AX] -- Safe Fallback`).
* **Mandatory Exit Gate:** Cold-boot from the Samsung FIT Plus USB 3.0 drive (`toram < 1.0s`, `mitigations=off` active), launch a reference Switch title via EmulationStation, play with `0` ALSA xruns, exit cleanly via the controller combo (`Select+Start` 1.5s hold), and verify both `.tar.zst` shader cache and NVIDIA `nv_cache` persistence across reboot.

### ⚡ Sprint 2: Cache Locality, 1G HugePages, io_uring/vmtouch & 2.00 ms Micro-PLL Audio

**Target Versions:** Windows `v1.34` / `v1.35` Throughput & Math | Linux Piccolo `v0.7` Cache-Local Release

* **Shared C++20 Core Tasks:**
  * Port the 64B-aligned, 32-bit sign-extended Xbyak GPU Macro-JIT (`macro_jit_compiler.cpp`), page-contiguity UBO passthrough, and `maxPushDescriptors` clamps (Citron PRs `#4688`, `#449`, `#383`).
  * Integrate procedural draw fastpaths (`#448`), subresource range clamping (`#385`, `#447`), and `libswresample` AVX2 FIR resampling (`SWR_DITHER_TRIANGULAR_HIGHPASS`) configured as a software micro-pitch PLL (`±0.005%`) driven by `lockfree_audio_queue.hpp` fill watermarks to absorb Z390-to-DAC crystal drift.
* **Windows (Espresso Wave [AX]) Track:**
  * Validate Xbyak Macro-JIT, page-contiguity UBO passthrough, and the config-driven 2:3:1 physical-core scaler (`eden_engine.ini`) across `SwitchHDD` and NVMe #2 titles with zero `VK_ERROR_DEVICE_LOST` faults.
* **Linux (Piccolo [AX]) Track:**
  * **1G/2M Hybrid HugePages:** Upgrade GRUB to hybrid `6.0 GB` (1G) + `2.0 GB` (2M) HugePages (`8.0 GB` HugeTLB) with a `5.0 GB` Host Safety Floor, mapping `Core::DeviceMemory` (512 GB fastmem) and the Dynarmic JIT cache onto `MAP_HUGE_1GB`.
  * **Tickless Core 0 Fusion:** Fuse `CoreTiming`, `AudioCore`, and `SCHED_FIFO 99` ALSA (`S32_LE`, 96 frames × 2 = `2.00 ms`, with 128-frame auto-fallback) into a single runnable thread on Core 0 (symmetrically locked at `4.8 GHz`, `max_cstate=0`) so Linux `nohz_full` remains 100% tickless.
  * **Storage & HID:** Enable smart container filtering in `edenGenerator.py` (`0xFF` padding strip and Update/DLC prioritisation), `io_uring` (`IORING_OP_MADVISE` `MADV_POPULATE_READ`) NCQ pre-warming paired with `taskset -c 5 vmtouch -dql` (`MAP_POPULATE`, up to `17.5 GB` with dynamic clamping), and Core 5 `SCHED_FIFO 80` `/dev/hidraw*` polling latched `-1.5 ms` before VBLANK (`usbhid.jspoll=1`).
* **Mandatory Exit Gate:** Zero `LOC` timer ticks on Cores 0–4 under active load, zero mechanical `SwitchHDD` seek stalls during world streaming, and `0` xruns at a `2.00 ms` ALSA period over extended play sessions.

### 🛡️ Sprint 3: Maxwell GM200 VRAM Shield, 64 KB Sparse VMA Aliasing & BC7/BC6H

**Target Versions:** Windows `v1.35` | Linux Piccolo `v0.8`

* **Shared C++20 Core Tasks:**
  * Commit the `vk_vram_shield.hpp` interface enforcing a `5.50 GB` VBLANK VRAM ceiling, 64 KB Sparse Page-Aliased VMA pooling (`VK_KHR_bind_memory2`) in `texture_reaper.cpp` (eliminating mid-game `vkAllocateMemory` stalls alongside `< 64 KB` fractional defragmentation), and `> 180`-frame LRU eviction (`predictive_eviction.cpp`).
  * Implement Option C Lossless `RGBA8` (`< 4.5 GB` title toggle) alongside hardware-native BC7 (`VK_FORMAT_BC7_UNORM_BLOCK`) and BC6H (`VK_FORMAT_BC6H_UFLOAT_BLOCK`) (`1.0 BPP`). Gate compute transcoding on Maxwell GM200 to VBLANK slack and loading screens (with a Core 5 AVX2 SIMD fallback) so Maxwell's single hardware engine never stalls mid-frame.
  * Integrate `emit_spirv.cpp` `lmem` register-spill condensation (`VK_PIPELINE_CREATION_FEEDBACK_BIT`), Eden PR `#32` CPU-delta overlap invalidation, and Citron PRs `#4966`, `#5102`, and `#5311`.
* **Windows (Espresso Wave [AX]) Track:**
  * Bind `vk_vram_shield.hpp` to `IDXGIAdapter3::QueryVideoMemoryInfo` (`5.50 GB` ceiling; `5.20 GB` when Lossless Scaling is active).
* **Linux (Piccolo [AX]) Track:**
  * Bind `vk_vram_shield.hpp` to `VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT` and VBLANK `VK_EXT_memory_budget` polling (`5.50 GB` ceiling), and expose per-title `RGBA8` / `BC7` / `BC3` overrides in `edenGenerator.py`.
* **Mandatory Exit Gate:** Heavy open-world titles (*Tears of the Kingdom*, *Xenoblade Chronicles 3*) hold peak VRAM strictly `<= 5.50 GB` with zero gradient banding (BC7), zero `vkAllocateMemory` mid-game spikes, and zero `lmem` shader-spill frame drops.

### ⚡ Sprint 4: Cross-GPU Video Offload (UHD 630) & Frame Generation (`lsfg-vk 2.0.0+`)

**Target Versions:** Windows `v1.36` | Linux Piccolo `v0.9`

* **Shared C++20 Core Tasks:**
  * Commit the unified `vaapi_dmabuf_bridge.hpp` cross-GPU video interface and expose an `AV_HWDEVICE_TYPE_VULKAN` (`VK_KHR_video_decode_queue`) fallback.
* **Windows (Espresso Wave [AX]) Track:**
  * Implement zero-copy WinRT `Windows.Graphics.Capture` (`vk_wgc_capture.cpp`) feeding `Lossless.dll` with `0 ms` PCIe copy overhead, a COM-isolated Core 5 `QSVVideoFeeder` (`vpp_qsv` → `IDXGIResource1` NT Shared Handle), and the reversible runtime MSI IRQ helper (`Scripts\irq_runtime_helper.ps1`).
* **Linux (Piccolo [AX]) Track:**
  * Implement Intel UHD 630 `iHD` VA-API decode on Core 5, followed by a `vaSyncSurface()` completion fence, `DRM_FORMAT_MOD_LINEAR` NV12 `dma_buf` FD export, and GTX 980 Ti Vulkan import via `VkImageDrmFormatModifierListCreateInfoEXT` + a `VK_KHR_synchronization2` barrier.
  * Integrate the implicit `lsfg-vk 2.0.0+` layer (Vulkan 1.2 bindless, forced FP32 shader path for Maxwell GM200, BYO `Lossless.dll`) on bare uncomposited Xorg, automatically tightening `vk_vram_shield.hpp` from `5.50 GB` to `5.35 GB` when active.
* **Mandatory Exit Gate:** Cutscenes decode on Intel UHD 630 with `0%` GTX 980 Ti video-engine load and `0` CPU `sws_scale` copies, while 2× frame generation runs smoothly within the `5.35 GB` VRAM ceiling.

### 🏆 Sprint 5: Closed-Loop Governor, Automated Gate & Two-Stage PGO Production Bake

**Target Versions:** Windows `v1.37` | Linux Piccolo `v1.0` Production Appliance

* **Shared C++20 Core Tasks:**
  * Implement the closed-loop adaptive governor using lock-free `g_metrics` atomic counters to dynamically tune `SpeedLimiter` `_mm_pause()` spin tails (`0.4–1.5 ms`) and protect Core 0 audio headroom.
  * Export lock-free telemetry (`/dev/shm` and Windows shared memory) for the optional 3.5" USB serial dev-kit display.
* **Windows (Espresso Wave [AX]) Track:**
  * Run the Pre-PGO sanitizer verification pass, execute `PGO_stage1.bat` across 3 × 15 min gameplay sweeps (with `macro_update.ps1`, `shader_validate.ps1`, and `telemetry_daemon.ps1` active), pass `Scripts\validate_profile.ps1`, merge profiles with `llvm-profdata`, and build the `clang-cl 23+` Stage 2 production `eden.exe` binary (`PGO_stage2.bat`).
* **Linux (Piccolo [AX]) Track:**
  * Run `BR2_PACKAGE_EDEN_PGO_STAGE=1` sweeps writing `.profraw` traces strictly into `EdenCache` `tmpfs` (zero USB flash wear), merge `eden.profdata`, bake the `STAGE=2` immutable `rootfs.erofs` image (`< 300 MB`) alongside A/B fallback slot `rootfs.erofs.prev`, and pass `verify_ax_appliance.sh --runtime` (`EXCELLENT_APPLIANCE_READY`).
* **Mandatory Exit Gate:** Both platforms pass all automated telemetry assertions (`0` audio underruns, `0` mid-game USB writes on Piccolo, `<= 5.35 GB` VRAM, and `< 300 MB` EROFS image) with PGO-linearised L1i binaries.

### 🔬 Sprint 6: Autonomous Silicon & Hardware Phase-Sync R&D (`v2.00` Unified Parity)

**Target Version:** Unified `v2.00` Next-Gen Release

* **Shared & Dual-Target R&D Deliverables:**
  1. **Live Background Superblock Re-Compilation (`live_reopt_daemon.cpp`):** Stream hot branch-edge counters from Cores 2–4 (`0x1C`) to a Core 5 (`0x20`) LLVM daemon that compiles unrolled AVX2 superblocks and atomically hot-swaps Dynarmic jump-table pointers between frames.
  2. **Cross-Silicon Intel UHD 630 Luma-Downscaled VME Optical Flow (`igpu_optical_flow.cpp`):** Execute a lightweight 720p/540p `R8_UNORM` luma-only compute pass on the GTX 980 Ti before exporting via `dma_buf` / NT Shared Handle (Win32)—cutting PCIe Gen 3 bandwidth by ~75–85%—to compute fixed-function Intel VME motion vectors on the idle iGPU and feed them into `Lossless.dll` / `lsfg-vk 2.0.0+`.
  3. **Custom RP2040 / ESP32-S3 USB Phase-Sync Dongle (`firmware/usb_sync_bridge/`):** Deploy a composite 1000 Hz USB HID + CDC microcontroller providing hardware VBLANK / HiBy FC3 clock phase alignment and physical toggle switches (`fast_ubo_passthrough`, `async_shader_worker_enabled`, and FPS caps) across both Windows and Piccolo.

---

# 🔀 Chapter 4: Cross-Platform Regression & Telemetry Gate Matrix

## 🔍 4.1 Unified Hardware, Kernel & Driver Quality Gate (Pre-Launch Assertions)

| Audit Domain | Acceptance Threshold (Pass Criteria) | Windows Assertion (`Scripts\check_hardware.ps1` & Pre-Build Gates) | Linux Appliance Assertion (`verify_ax_appliance.sh`) |
| :--- | :--- | :--- | :--- |
| **CPU Clocks, C-States & Mitigations** | Cores 0–5 locked symmetrically at `4.8 GHz` (`x48`), Uncore Ring at `4.5 GHz` (`x45`), zero SMT, deep C-states disabled, `mitigations=off` on appliance | `Get-CimInstance Win32_Processor` (`NumberOfCores=6`, `NumberOfLogicalProcessors=6`) + Power Plan High Performance (`0%` idle throttle) | • `/proc/cpuinfo` `cpu MHz >= 4790` on Cores 0–5.<br>• `cpuidle/current_driver = none` (`processor.max_cstate=0`).<br>• `/sys/devices/system/cpu/vulnerabilities/*` confirms `mitigations=off`. |
| **Core Isolation & Tickless Mode** | Cores 0–4 shielded from background OS tasks/IRQs; Core 5 (`0x20`) absorbs all housekeeping with `_MM_HINT_NTA` reads | `Scripts\set_irq_affinity.ps1` registry audit (`0x01` DAC, `0x02` GTX 980 Ti, `0x20` NVMe/SATA) + live root menu IRQ display | • `/sys/fs/cgroup/isolated/cpuset.cpus.partition = isolated`.<br>• `/sys/devices/virtual/workqueue/cpumask = 20`.<br>• `0` unexpected `LOC` ticks on Cores 0–4 in `/proc/interrupts`. |
| **Shader Macro & SPIR-V Hygiene** | Uniform `#version 460 core`, explicit numeric shader macros (`USE_EASU`, `USE_RCAS`, `YUZU_USE_FP16`, `VERSION`), zero SPIR-V validation errors | `Scripts\macro_update.ps1` + `Scripts\shader_validate.ps1` (`glslangValidator`) pass with exit code `0` inside PGO stage scripts | Build-time GLSL 460 core & `GL_GOOGLE_include_directive` validation pass with zero SPIR-V errors. |
| **Large / HugePage Reservation** | `8.0 GB` HugeTLB locked (`6.0 GB` 1G + `2.0 GB` 2M on Linux; `SeLockMemoryPrivilege` on Win32); Host Safety Floor `>= 5.0 GB` | `whoami /priv` confirms `SeLockMemoryPrivilege = Enabled`; free physical RAM `>= 22 GB` | • `/proc/meminfo` confirms `HugePages_Total: 6` (`1048576 kB`) + `1024` (`2048 kB`).<br>• `zram0 = 512M` (`vm.swappiness=1`). |
| **dGPU Driver & PCIe Link** | GTX 980 Ti (`01:00.0`) on PCIe x16 Gen 3, P0 persistence active, ASPM disabled, legacy Maxwell branch locked, `GLCache` persisted | `nvidia-smi` confirms PCIe Gen 3 x16, P0 state, `580.*` branch, `__GL_YIELD=NOTHING` | • `nvidia-smi` confirms `580.*` legacy driver + `Persistence-M = On`.<br>• `0000:01:00.0/power/control = on`.<br>• `__GL_SHADER_DISK_CACHE_PATH=/dev/shm/nv_cache`. |
| **iGPU Video Decode (UHD 630)** | Intel UHD 630 (`00:02.0`, `0x8086`) hardware H.264/HEVC/VP8/VP9 decode active | Media Foundation / Intel QSV enumerates `0x8086` hardware MFT | `vainfo --display drm --device /dev/dri/renderD128` confirms `iHD` driver + `VAProfileVP9Profile0` / `VAProfileHEVCMain` |
| **USB DAC & Bus Separation** | HiBy FC3 (`ES9281AC Pro`) on separate xHCI controller from storage; 24-bit / 48 kHz exclusive lock | `mmsys.cpl` endpoint locked to 24-bit, `48000 Hz`, Exclusive Mode enabled; separate xHCI device tree | • `lspci -nn \| grep -i usb` confirms separate xHCI BDFs.<br>• `/proc/irq/<dac_irq>/smp_affinity = 01`.<br>• 60s `speaker-test` at `S32_LE` 96 frames yields `0` xruns. |
| **Immutable Boot & Storage Integrity** | Zero mid-game USB flash writes (Piccolo); journal-free `SHARE` partition clean | NVMe #1 / #2 TRIM active; `SwitchHDD` online | • `mount` confirms `rootfs` on `/dev/shm` (EROFS `< 300 MB`).<br>• `e2fsck -n /dev/sdX2` reports clean. |

---

## 📈 4.2 Runtime Telemetry & Frame-Pacing Regression Gate (`g_metrics` & `telemetry_daemon.ps1`)

| Telemetry Metric (`g_metrics`) | Target Threshold | Failure / Regression Trigger | Automated Remediation / Action |
| :--- | :--- | :--- | :--- |
| **`audio_underruns`** *(ALSA / FlexASIO)* | `0` underruns over 15-min sweep | Any `EPIPE` xrun, voice drop, or audible click on `hw:CARD=FC3,DEV=0` | Engage `libswresample` micro-PLL (`±0.005%` watermark drift compensation), auto-step ALSA period `96` → `128` frames if needed, and verify Core 0 `nohz_full` / `telemetry_daemon.ps1` buffer health. |
| **`vram_used_bytes`** *(`VK_EXT_memory_budget` / DXGI)* | `<= 5.50 GB` (base)<br>`<= 5.35 GB` (`lsfg-vk`)<br>`<= 5.20 GB` (Win32 LS) | Exceeding ceiling or triggering WDDM/NVIDIA system-RAM paging (`> 5 ms` alloc stall) | Reuse pre-allocated 64 KB sparse VMA pages (`VK_KHR_bind_memory2`), run `< 64 KB` defragmentation, flush `> 180`-frame LRU entries, or switch title from Option C `RGBA8` to BC7/BC3. |
| **`frametime_p99_jitter_ms`** | `< 0.50 ms` variance at locked 30/60 FPS | Frame-time spike `> 2.0 ms` outside cold shader compilation | Closed-loop governor dynamically adjusts `SpeedLimiter` `_mm_pause()` spin-tail (`0.4–1.5 ms`) and audits Core 1 `Gather()` / Maxwell compute queue slack. |
| **`host_free_ram_bytes`** *(Safety Floor)* | `>= 5.00 GB` free on Linux (`>= 4.50 GB` Win32) | Free RAM dipping below floor or `zram0` swap activity `> 64 MB` | `edenGenerator.py` dynamically clamps `io_uring` / `vmtouch -dql` ceiling (`17.5 GB` → `15.0 GB`), `telemetry_daemon.ps1` flags working-set spikes, and Eden PR `#3817` (`HeapTracker` leak fix) is verified active. |
| **`usb_runtime_writes_sectors`** *(Piccolo Only)* | `0` sectors written to Samsung FIT Plus during gameplay | Any write activity on `/dev/sdX` between title launch and title exit | Audit `/tmp`, `/var`, `/dev/shm/nv_cache`, and `EdenCache` mounts to ensure `tmpfs` (`1.5 GB`) captures all `.profraw` and shader cache writes until `K01piccolo-sync`. |
| **`shader_lmem_spill_count`** *(`emit_spirv.cpp`)* | `0` high-pressure `lmem` spills on Maxwell GM200 | `VK_PIPELINE_CREATION_FEEDBACK_BIT` flags register count `> 64` per thread | Run `emit_spirv.cpp` live-range condensation pass during PGO training to keep temporaries inside Maxwell's on-chip register file. |

---

## ✅ 4.3 Release Promotion Sign-Off Rules (Sprint 0 → Sprint 6)

1. **Dual-Target Build, Shader Gate & Pre-PGO Sanitizer Check:** Both `clang-cl 23+` (Windows) and `clang-23` + `lld-23` (`batocera-ax-external/`) compile with zero errors and zero `-Werror` warnings, `macro_update.ps1` + `shader_validate.ps1` pass with zero SPIR-V diagnostics, and `ldd` confirms zero forbidden library dependencies. Execute a Debug sanitizer pass (`-fsanitize=address,undefined` and `-fsanitize=thread`) across `lockfree_audio_queue.hpp`, `gpu_dirty_memory_manager.cpp`, and `vaapi_dmabuf_bridge.hpp` prior to Stage 1 PGO sweeps.
2. **Appliance Footprint & A/B Rollback Gate (Piccolo [AX]):** `post-image.sh` confirms `rootfs.erofs` (`lz4hc,9`) `<= 307,200 KB` (`300 MB`), `toram` boot copy `< 1.0s`, and `/boot/boot/rootfs.erofs.prev` A/B fallback slot present in `grub.cfg`.
3. **Hardware & Telemetry Gate:** `Scripts\check_hardware.ps1` + `telemetry_daemon.ps1` JSON regression checks and `verify_ax_appliance.sh` output `EXCELLENT_APPLIANCE_READY` with `0` audio underruns, `0` unexpected `LOC` timer interrupts on Cores 0–4, `mitigations=off` confirmed on Piccolo, and peak VRAM strictly within budget.
4. **Persistence & Recovery Gate:** Exiting a title via the controller combo (`Select+Start` 1.5s hold) cleanly terminates `io_uring`/`vmtouch`, atomically writes `.tar.zst` (`zstd -3` covering `EdenCache` + `/dev/shm/nv_cache`), resumes EmulationStation (`SIGCONT < 50 ms`), and passes `e2fsck -n` on `SHARE` after reboot.

---

# 🔬 Chapter 5: Architectural & Technical Analysis and Future Optimisations

## ⚖️ 5.1 Architectural & Technical Analysis (Strengths, Trade-Offs & Physical Limits)

### 🧠 1. Determinism Through Spatial & Temporal Partitioning (i5-9600K Non-SMT Advantage)
* **Why Zero-SMT Silicon Excels:** Six completely independent physical execution engines with private 32 KB L1d and 256 KB L2 caches avoid sibling thread contention.
* **Cache-Coherency Payoff via Symmetrical Multipliers:**
  * **Core 0 (`4.8 GHz`, `nohz_full`):** Keeps the `2.00 ms` (96-frame) ALSA ring + micro-PLL inside its private 256 KB L2 cache. By removing the old frequency step delta, thread-handoff latency steps between Core 0 and the primary JIT pool are structurally flattened.
  * **Core 1 (`4.8 GHz`):** Isolates `VulkanWorker` + `Gather()`.
  * **Cores 2–4 (`0x1C`):** Shield home-biased guest fibers (while assisting `.ncz` bursts strictly during loading screens before guest fibers spin up).
* **Architectural Trade-Off:** Concentrating OS housekeeping, SPIR-V compilation, `FFmpeg::execute2` slices, and 1000 Hz `/dev/hidraw*` polling onto Core 5 (`0x20`) requires strict priority tiering (`SCHED_FIFO 80` HID > `SCHED_OTHER` FFmpeg > `SCHED_BATCH`/Low SPIR-V) plus `_MM_HINT_NTA` non-temporal prefetching so Core 5 streaming reads never evict hot L3 lines used by Cores 0–4.

### 🧮 2. Memory Hierarchy & Bus Economics (32 GB DDR4-3600 + 6 GB GDDR5)
* **Eliminating Skylake TLB Walks (1G + 2M HugePages):** Backing `Core::DeviceMemory` and Dynarmic JIT caches with `6.0 GB` of 1 GB HugePages (`MAP_HUGE_1GB`) hits Skylake's dedicated 4-entry 1 GB DTLB, eliminating STLB page walks.
* **Overcoming USB 3.0 & Mechanical HDD Limits:**
  * `toram` EROFS detaches the OS from flash at boot (`~0.75s`).
  * `io_uring` (`MADV_POPULATE_READ`) + `vmtouch -dql` (`17.5 GB`) leverages 7200 RPM Native Command Queuing (NCQ) to eliminate 10–14 ms seek stalls.
  * Atomic `.tar.zst` exit flushes persist both `EdenCache` and NVIDIA `GLCache` (`/dev/shm/nv_cache`) safely onto `^has_journal` `SHARE`.
* **Maxwell GM200 (6 GB GDDR5) Envelope Management:** Backing dynamic render targets with a pre-allocated 64 KB Sparse Page-Aliased VMA pool (`VK_KHR_bind_memory2`) alongside VBLANK `VK_EXT_memory_budget` (`5.50 GB` / `5.35 GB` `lsfg-vk`) and Option C `RGBA8` vs. BC7/BC3 eliminates mid-game `vkAllocateMemory` stalls and WDDM/NVIDIA system-RAM paging freezes.

### 🎮 3. Dual-GPU Asymmetric Offload (GTX 980 Ti + Intel UHD 630)
* **Zero-Copy Cross-Silicon Video:** Offloading cutscene decode to the idle Intel UHD 630 (`iHD` VA-API / QSV) and importing `DRM_FORMAT_MOD_LINEAR` NV12 surfaces into GTX 980 Ti Vulkan via `VkImageDrmFormatModifierListCreateInfoEXT` / NT Shared Handles preserves 100% of Maxwell's GDDR5 bandwidth for 3D geometry.
* **Single-Engine Maxwell Constraint:** Because Maxwell GM200 time-slices compute and graphics on one hardware engine and throttles FP16 ALUs (1:64), pinning `lsfg-vk 2.0.0+` to FP32 mode and gating ASTC → BC7 compute transcodes to VBLANK slack avoids mid-frame contention.

---

## 🔭 5.2 Targeted Future Optimisations (Post-v1.0 & v2.00+ Roadmap)

| Optimisation Target | Technical Mechanism | Expected Gain |
| :--- | :--- | :--- |
| **1. Intel CAT (`resctrl`) Silicon Check + `_MM_HINT_NTA` Non-Temporal Prefetching** | Verify `rdmsr 0xc8f` in Sprint 0; where desktop Coffee Lake fuses off CAT MSRs in silicon, enforce `_mm_prefetch(..., _MM_HINT_NTA)` alongside `_mm256_stream_si256` on Core 5 (`0x20`) streaming reads so `io_uring`/`vmtouch` and `.ncz` reads bypass L1/L2 and minimise 9 MB L3 cache pollution. | Protects hot Dynarmic JIT blocks and `AudioCore` state in L3 without server-only Xeon MSRs. |
| **2. Cross-Silicon Intel UHD 630 Luma-Downscaled VME Optical Flow (Sprint 6 / v2.00)** | Execute a lightweight 720p/540p `R8_UNORM` luma-only compute pass on the GTX 980 Ti before exporting via `dma_buf` / NT Shared Handle to Intel UHD 630 VME, feeding hardware motion vectors back into `lsfg-vk 2.0.0+` / `Lossless.dll`. | Cuts PCIe Gen 3 transfer volume by ~75–85% (`< 0.3 ms`) while eliminating HUD warping during 2× frame generation. |
| **3. Live Background Superblock Re-Compilation (`live_reopt_daemon.cpp`)** | Stream lightweight guest branch-edge counters from Cores 2–4 over a lock-free ring to a low-priority LLVM JIT worker on Core 5 (`0x20`) that compiles unrolled AVX2 superblocks and atomically swaps Dynarmic jump-table pointers between frames. | Cuts CPU-bound frame times by 8–15% in complex simulation loops without mid-frame compilation stalls. |
| **4. Shared Zstandard Dictionary Compression for `EdenCache` Archives** | Train a static 256 KB `zstd` dictionary (`zstd --train`) across representative Switch SPIR-V/Maxwell shader caches and bake it into `rootfs.erofs`, invoking `zstd -3 -D /etc/piccolo/shader.dict` in `K01piccolo-sync` and `edenGenerator.py`. | Improves shader cache compression ratio by 25–35% and accelerates exit `.tar.zst` USB writes and launch decompression. |
| **5. Outer-Cylinder Physical Platter Alignment on `SwitchHDD`** | Partition `SwitchHDD` (7200 RPM mechanical drive) with a dedicated outer-cylinder `~250 GB` primary partition (`SwitchHOT`) for active v0 titles and updates, where linear platter velocity peaks at `~190–210 MB/s` (vs. `~95 MB/s` on inner tracks). | Cuts initial `17.5 GB` cold-load times nearly in half (`~180s` → `~90s`) with zero hardware cost. |
| **6. Hardware VBLANK & HiBy FC3 Clock Phase-Sync Dongle (RP2040 / ESP32-S3)** | Deploy a composite USB HID + CDC microcontroller pinned to Core 0 / Core 5 acting as a deterministic 1000 Hz controller bridge, VBLANK-to-DAC phase comparator, and physical appliance toggle switch bank (`lsfg-vk` 2x, Option C `RGBA8`/BC7, 30/60 FPS cap). | Eliminates software polling drift and provides instant physical console toggles without touching config files. |

---
