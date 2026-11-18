# 📋 Sprint Progress Report — Ecosystem Scaffolding & Multi-Repository Architecture (`2026-11-18`)

## 🎯 Executive Summary

This sprint established the four-repository open-source infrastructure for **The Silicon Preservation Society (`[AX]`)** on GitHub (`galaxywarriordad`), finalized the dual-edition strategy for **Cemu Latte `[AX]`**, completed and verified the standalone **`ax-sdk`** C++20 runtime library (`libax_core`), committed the full **`Piccolo-AX`** (`BR2_EXTERNAL`) Linux console OS appliance tree and package recipes, and integrated the optional `ENABLE_AX_SDK` CMake hook into **`Cemu-Latte-AX`**.

---

## ✅ Completed Deliverables

### 1. Unified Four-Repository Network Live on GitHub
* **[`Piccolo-AX`](https://github.com/galaxywarriordad/Piccolo-AX) (`GPL-3.0`):** Standalone `< 300 MB` bare-metal Batocera `BR2_EXTERNAL` console OS platform repository created, pinned, and documented (`README.md`).
* **[`ax-sdk`](https://github.com/galaxywarriordad/ax-sdk) (`MIT`):** Standalone, dependency-free C++20 runtime library (`libax_core`) created, pinned, and documented (`README.md`) for frictionless linking across `GPL-3.0`, `MPL-2.0`, and `GPLv2` emulators.
* **[`Cemu-Latte-AX`](https://github.com/galaxywarriordad/Cemu-Latte-AX) (`MPL-2.0`):** Clean upstream fork created, pinned, and documented (`README.md`) with dual-track framing (Windows 10 PGO/Coffee Lake/Vulkan 1.4 Stage A baseline + Batocera Linux `Piccolo [AX]` full `[AX]` edition).
* **[`Eden-Espresso-Wave-AX`](https://github.com/galaxywarriordad/Eden-Espresso-Wave-AX) (`GPL-3.0`):** Linked across all repository network tables alongside its primary Forgejo host ([git.eden-emu.dev](https://git.eden-emu.dev/galaxywarriordad/Eden-Espresso-Wave-AX.git)).

### 2. `ax-sdk` C++20 Runtime Core & Shared Clang Toolchain Finalized (16 Commits)
* Committed `CMakeLists.txt` (`ax::core`), `include/ax/topology.hpp` + `src/topology.cpp` (`2:3:1` spatial masks `0x01`/`0x02`/`0x1C`/`0x20`, Linux `rdmsr 0xc8f` CAT probe, `<windows.h>` MMCSS isolation), `include/ax/memory.hpp` + `src/memory.cpp` (`MAP_HUGE_1GB` / `MAP_HUGE_2MB` HugeTLB `mmap` fastmem allocator, `_bextr_u64` BMI1, `_MM_HINT_NTA`), `include/ax/vram.hpp` (`5.50` / `5.35 GiB` VBLANK shield + GM200 flags), `include/ax/igpu.hpp` (`DRM_FORMAT_MOD_LINEAR` `dma_buf` + `3.50 GB/s` guardrail), and `include/ax/audio.hpp` (`96-frame` / `2.00 ms` lock-free stereo ring).
* Committed `ax_toolchain/piccolo-clang-x86_64.cmake` (shared Clang 23.1+ ThinLTO + `-march=skylake` + 3-Phase PGO toolchain) and cleaned stray overlay directories so `ax-sdk` remains self-contained.

### 3. `Piccolo-AX` Buildroot Overlay, Isolation Layer & Package Recipes Committed
* Committed `external.desc` (`PICCOLO_AX`), `Config.in`, `external.mk`, `configs/linux-6.12-rt-skylake.fragment`, `board/piccolo-ax/grub.cfg` (with A/B `rootfs.erofs.prev` rollback), `post-build.sh` (`~300 MB` NVIDIA/Mesa/PipeWire/Openbox purge), and `post-image.sh` (`EROFS` `lz4hc,9` `< 300 MiB` hard size gate).
* Committed early init hooks (`S01piccolo-isolate`, `S02piccolo-hw`, `K01piccolo-sync`), uncomposited `etc/X11/xorg.conf`, and the 7-point bare-metal validation gate (`usr/bin/verify_ax_appliance.sh`).
* Committed Buildroot `Config.in` and `.mk` recipes for `package/ax-sdk/`, `package/eden-ax/` (pointing to Forgejo `master`), and `package/cemu-latte-ax/` (pointing to GitHub `main`), plus `axGenerator.py` (dynamic `/proc/meminfo` `vmtouch` RAM pinning, atomic `zstd -3` `/dev/shm` ↔ `/userdata/cache` persistence, and EmulationStation `SIGSTOP`/`SIGCONT` lifecycle).

### 4. `Cemu-Latte-AX` CMake Integration Started
* Updated root `CMakeLists.txt` with `option(ENABLE_AX_SDK ... OFF)` and `find_package(ax-sdk)` / `FetchContent` fallback (`CEMU_ENABLE_AX_SDK=1`) while keeping Stage A (`ENABLE_AX_SDK=OFF`) unmodified.

---

## 🚀 Immediate Carry-Over Actions (Next Session)

1. **Finalize Two Small CMake/Buildroot Alignment Edits (~2 minutes):**
   * Append the `target_link_libraries(CemuBin PRIVATE ax::core)` + `target_link_libraries(CemuCommon PUBLIC ax::core)` block to the bottom of `Cemu-Latte-AX/src/CMakeLists.txt`.
   * Update `Piccolo-AX/batocera-ax-external/package/cemu-latte-ax/cemu-latte-ax.mk` flags to match Cemu 2.x (`-DENABLE_VCPKG=OFF`, `-DALLOW_PORTABLE=OFF`, `sdl3`, `wxwidgets`).
2. **Execute Sprint 0 Stage A Windows 10 PGO Baseline Sweeps:**
   * Build Stage A (`-DENABLE_AX_SDK=OFF`) for **Eden Espresso Wave `[AX]`** and **Cemu Latte `[AX]`** using Clang 23.1+ ThinLTO + 3-Phase PGO (`-march=skylake`) and capture `baseline_pgo_metrics.json`.
3. **Validate Live USB Hardware Gate & Buildroot `BR2_EXTERNAL` Compile:**
   * Run `verify_ax_appliance.sh` on the Stream 1 reference silicon (`i5-9600K` + `GTX 980 Ti` + `UHD 630` + `HiBy FC3 DAC` under Linux `6.12-rt` + NVIDIA `580.*`) and trigger the first `Piccolo-AX` Buildroot image build (`<= 300 MiB` EROFS gate)[cite: 8, 9].