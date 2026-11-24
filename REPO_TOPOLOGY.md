## 🌐 The Silicon Preservation Society: 9-Repository Topology

The **[AX] Ecosystem** separates shared low-level C++20 silicon abstractions from platform-, OS-, and emulator-specific repositories so upstream rebases stay clean while hardware isolation logic remains unified across Windows 10/11, bare-metal `x86_64` Linux, and `aarch64` handhelds.

```text
                    +-------------------------------------------------------+

                    |                   ax-sdk (v1_36_2)                    |
                    |  Shared C++20 Silicon Core, Dynamic SMT/CCX & 1:2:1   |
                    |  Topology, Evdev Gamepad Input & SCHED_FIFO Audio     |
                    +-------------------------------------------------------+
                       /              /                 \               \
      FetchContent    /  FetchContent/                   \ FetchContent  \ BR2_EXTERNAL
                     v              v                     v               v
+-------------------------+ +-------------------+ +-----------------+ +-------------------+

| Eden-Espresso-Wave-AX   | |  Cemu-Latte-AX    | |   Amiberry-AX   | |     FLAC_AX       |
| (Nintendo Switch [AX])  | | (Wii U / Latte)   | | (Amiga 500/1200)| | S32_LE/DoP & TUI  |
+-------------------------+ +-------------------+ +-----------------+ +-------------------+
                                                                                |
                    +-----------------------------------+-----------------------+

                    |                                   |                       |
                    v                                   v                       v
          +-------------------+               +-------------------+   +-------------------+

          | Generation-X-AX   |               |    Piccolo-AX     |   |    Piccolo-2go    |
          | (Hatari, BlastEm, |               | (x86_64 6.12-rt   |   | (ARM64 RK3326     |
          |  Flycast, RMG)    |               |  EROFS toram OS)  |   |  1:2:1 Handheld)  |
          +-------------------+               +---------+---------+   +-------------------+
                                                        |
                                                        v
                                              +-------------------+

                                              |piccolo-ax-desktop |
                                              | (LXQt/X11 Core 5) |
                                              +-------------------+
```

### Repository Breakdown (All 9 Repositories)

| # | Repository | Primary Role & Contents |
| :--- | :--- | :--- |
| **1** | **`ax-sdk`** | Shared C++20 runtime library (`ax::core`), dynamic Intel SMT / AMD CCX / RK3326 `1:2:1` core topology resolver, `EvdevNavReader` gamepad input, HugeTLB fastmem, and telemetry schemas. |
| **2** | **`Piccolo-AX`** | Standalone `< 300 MB` `toram` EROFS Batocera Linux `x86_64` console appliance (`Linux 6.12-rt`, `2:3:1` isolation, direct ALSA `SCHED_FIFO` audio, `FBNeo` + `[AX]` emulator wheel). |
| **3** | **`Piccolo-2go`** | Deterministic `aarch64` Buildroot handheld OS (`128 KB`-block `zstd` SquashFS `< 140 MB`) for **BATLEXP G350** & **R36S** (`RK3326`, `1:2:1` isolation, `1024 MB LZ4` ZRAM, `Panfrost` KMS/DRM, dual SanDisk `SDR50` guard, **HiBy FC3 `1.3 Vrms`** OTG audio). |
| **4** | **`piccolo-ax-desktop`** | On-demand, local-first LXQt/X11 desktop overlay (`< 42 MB` Tier A footprint) for `Piccolo-AX`, isolated on housekeeping Core `5` away from real-time audio/emulation cores. |
| **5** | **`FLAC_AX`** | Bit-perfect `S32_LE` / `DoP` audiophile transport (`ES9281PRO` / `ES9281AC Pro`), `libopenmpt` Amiga/Atari ST tracker player, `ax_av_studio`, `ax_video_upscaler`, and `ax_appliance_tui` launcher. |
| **6** | **`Amiberry-AX`** | Deterministic Commodore Amiga 500 / 1200 / CD32 engine linked against `ax-sdk` (`v1_36_2`). |
| **7** | **`Generation-X-AX`** | Unified C++20 `[AX]` adapter suite powering **Hatari** (Atari ST), **BlastEm** (Mega Drive), **Flycast** (Dreamcast/NAOMI), **RMG** (N64), **Cannonball**, **EDuke32**, and **Frontier**. |
| **8** | **`Eden-Espresso-Wave-AX`** | Deterministic Nintendo Switch `[AX]` engine (`clang-cl` Skylake/BMI2, ThinLTO, PGO, Elastic `2:3:1` isolation, Intel QSV offload). |
| **9** | **`Cemu-Latte-AX`** | Deterministic Nintendo Wii U (Latte GPU) `[AX]` engine linked against `ax-sdk` (`v1_36_2`). |
