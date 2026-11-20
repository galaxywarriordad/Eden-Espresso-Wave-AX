# Eden Espresso Wave [AX] is a deterministic, hardware-pinned emulation platform built around a single guiding principle: treat a PC like a dedicated console appliance rather than a general-purpose desktop. Anchored by a standalone C++20 systems library (ax-sdk), the ecosystem spans Windows desktop builds (Eden-Espresso-Wave-AX and Cemu-Latte-AX) and a sub-300 MB immutable Linux bootable USB appliance (Piccolo-AX), engineered specifically to extract deterministic, zero-stutter performance from Coffee Lake + Maxwell hardware (Intel i5-9600K / i5-8600K + NVIDIA GTX 980 Ti + Intel UHD 630 + HiBy FC3 DAC).

## What Makes This Architecture Genuinely Unique

# Spatial Core Partitioning (CoreRatio=2:3:1) & 1:1 IRQ Steering:
Instead of letting the OS scheduler bounce threads across cores, ax-sdk dynamically detects physical cores, filters out SMT siblings, and isolates workloads into dedicated silicon lanes: Core 0 (0x01) fuses host timing, guest audio DSP, and bit-perfect USB DAC output (2.00–2.67 ms) inside a single private 256 KB L2 cache; Core 1 (0x02) owns Vulkan command submission; Cores 2–4 (0x1C) execute guest ARM64/PowerPC JIT threads; and Core 5 (0x20) quarantines OS housekeeping, NVMe I/O, and background tasks.

# Heterogeneous Dual-GPU Cooperation (GTX 980 Ti + Intel UHD 630):
Rather than leaving the integrated Intel UHD 630 dormant, the architecture routes hardware video decoding (H.264/HEVC/VP8/VP9 via Windows Media Foundation QSV or Linux iHD VA-API) onto the iGPU over zero-copy shared handles (VK_KHR_external_memory_win32 / dma_buf), while keeping all 2,816 Maxwell CUDA cores focused purely on 3D rendering with automatic FP16 → FP32 shader promotion and 128 MiB BAR1 aperture protection.

# Proactive 5.00 GiB Asynchronous VRAM Management:
Designed around the GTX 980 Ti's 6 GB GDDR5 ceiling, async transfer-queue eviction (VK_QUEUE_TRANSFER_BIT + Vulkan 1.4 split barriers) drains stale textures off Core 5 before VBLANK, preventing 20–45 ms system-RAM paging freezes and VK_ERROR_DEVICE_LOST crashes.

# Sub-300 MB Immutable Linux Appliance (Piccolo-AX):
Compiles a PREEMPT_RT (6.12-rt) kernel, strips Linux down to an EROFS image that copies completely into DDR4-3600 RAM (toram) in under a second, locks emulation memory onto 1 GB / 2 MB HugePages, and eliminates mid-game USB flash writes.

# Sustainable Hardware Preservation:
Demonstrates that disciplined systems software, Clang ThinLTO + 3-phase PGO, and respect for cache/IRQ/memory boundaries can turn proven 8th/9th-Gen K-series and Maxwell hardware into a whisper-quiet, ultra-responsive console experience.
