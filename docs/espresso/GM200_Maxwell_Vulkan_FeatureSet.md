# 🎮 NVIDIA GTX 980 Ti (GM200 Maxwell) — Vulkan 1.3 & 1.4 Feature-Set Assessment

**Baseline:** `Master v1.35.1` / `ax-sdk v1_36_1` | **Driver Floor:** `582.78 WHQL` (Win10) & `580.*` Legacy Branch (Linux `Piccolo [AX]`)

---

## 1. GTX 980 Ti (GM200 Maxwell) Vulkan 1.3 & 1.4 Feature-Set Assessment

While the NVIDIA GeForce GTX 980 Ti is officially Khronos-conformant for both Vulkan 1.3 and Vulkan 1.4 (via NVIDIA's final R580 legacy driver branch—R590+ dropped Maxwell), API conformance only requires meeting mandatory baseline specifications—many major architectural features remain either emulated in driver software (partial) or unavailable due to physical 28 nm Maxwell (GM200) silicon limits.

### Vulkan 1.3 Feature Breakdown

| Feature / Extension | GTX 980 Ti Status | Technical Reality on Maxwell (GM200) |
| :--- | :--- | :--- |
| **Dynamic Rendering** (`VK_KHR_dynamic_rendering`) | **Full** | Supported natively in hardware/command processor; removes legacy `VkRenderPass` and `VkFramebuffer` object overhead. |
| **Synchronization 2** (`VK_KHR_synchronization2`) | **Full** | Supported cleanly in driver; simplifies pipeline barriers and queue submission without hardware penalties. |
| **Maintenance 4** (`VK_KHR_maintenance4`) | **Full** | Fully supported; enables cleaner local workgroup sizing and early destruction of shader modules after pipeline creation. |
| **Extended Dynamic State 1 & 2** | **Partial** | Core dynamic states (viewport, scissor, cull mode, depth bounds) work natively, while deeper dynamic state variants trigger internal driver pipeline re-binds. |
| **Graphics Pipeline Libraries** (`VK_EXT_graphics_pipeline_library`) | **Partial / Emulated** | Exposed by the R580 driver, but Maxwell lacks hardware pre-compiled shader stages. The driver still performs full monolithic pipeline linking on the CPU, resulting in compilation stutter. |
| **Descriptor Indexing / Bindless** (`VK_EXT_descriptor_indexing`) | **Partial (Hard-Capped)** | Supported up to strict Maxwell hardware descriptor table limits (far lower than Turing/Ampere), risking device loss (`VK_ERROR_DEVICE_LOST`) if exceeded. |

### Vulkan 1.4 Feature Breakdown

| Feature / Extension | GTX 980 Ti Status | Technical Reality on Maxwell (GM200) |
| :--- | :--- | :--- |
| **Push Descriptors** (`VK_KHR_push_descriptor`) | **Full** | Natively supported; allows direct uniform/texture updates inside command buffers without descriptor pool allocation. |
| **Scalar Block Layouts** (`VK_EXT_scalar_block_layout`) | **Full** | Natively supported; eliminates C-style memory padding overhead when uploading Switch uniform buffers. |
| **Maintenance 5 & 6** (`VK_KHR_maintenance5/6`) | **Full** | Fully supported API cleanups, including `VK_KHR_map_memory2` and streamlined pipeline flags. |
| **Shader Subgroup Rotate / Float Controls 2** | **Full** | Supported via Maxwell's 32-thread warp shuffle instructions (`SHFL`) and FP32/FP16 ALU blocks. |
| **Streaming Transfers** (`VK_EXT_host_image_copy`) | **Partial** | Uses standard DMA copy engines, but lacks modern resizable BAR (ReBAR) / direct host-to-VRAM layout tiling optimizations found on Turing+. |
| **Dynamic Rendering Local Read** | **Partial / Emulated** | Designed for mobile tile-based GPUs; on immediate-mode GM200 silicon, the driver emulates attachment reads via standard global VRAM barriers. |

---

## 2. Current Eden Espresso Wave `[AX]` & Ecosystem Utilisation Assessment (`v1.35.1`)

**Eden Espresso Wave `[AX]`** (`Master v1.35.1` / `ax-sdk v1_36_1`)—alongside **`Cemu-Latte-AX`**, **`Generation-X-AX`** (`Flycast`, `RMG`, `BlastEm`, `Cannonball`, `EDuke32`, `Frontier`, `Hatari`), and **`FLAC_AX`** (`ax_av_studio`)—is explicitly engineered around the exact hardware boundaries of Maxwell GPUs like the GTX 980 Ti rather than relying on newer Vulkan 1.3/1.4 abstractions that fall back to slow driver emulation.

### What Wave `[AX]` & `ax-sdk` Actively Utilise

* **Maxwell-Safe UBO Passthrough:** Directly exploits the GTX 980 Ti's native Push Descriptors and Scalar Block Layouts to stream Nintendo Switch Uniform Buffer Objects (UBOs) straight from host memory into command buffers. This bypasses Maxwell's tight descriptor-indexing hardware caps and avoids costly CPU-side buffer repacking.
* **Proactive `5.00 GiB` Async VRAM Shield (`ax::vram` + `VkVramShield`):** Exploits the GTX 980 Ti's full unsegmented `6 GB` GDDR5 pool (`384-bit` bus, `336.6 GB/s`) by triggering asynchronous background pruning at **`5.00 GiB`** (`~4.50 GiB` hysteresis recovery) ahead of the `5.50 GiB` (`5.35 GiB` `lsfg-vk 2.0.0+` / `5.20 GiB` Win32 LS) hard ceilings—while keeping `FLAC_AX`'s OpenGL Studio HUD strictly under `< 64 MB` VRAM so switching between A/V studio and emulation never fragments GM200 memory.
* **Zero-Copy Intel UHD 630 Dual-Die Offloading (`ax::igpu`):** Because the GTX 980 Ti's 1st-Gen NVDEC block lacks hardware decoding for newer video codecs and GM200 suffers a `1:64` FP16 throughput ratio, Wave `[AX]` offloads media decoding (`VDBOX`), `NV12 → RGBA` scaling/deinterlacing (`VEBOX`), and native `2:1` FP16 `ASTC → BC7/BC6H` texture transcoding onto the Coffee Lake **Intel UHD Graphics 630** (`24 EUs`, `VK_API_VERSION_1_3` capped) via zero-copy `DRM_FORMAT_MOD_LINEAR` `dma_buf` / NT Shared Handles (`<= 3.50 GB/s` ring-bus guardrail)—keeping GM200's 2,816 CUDA cores dedicated solely to 3D rasterization.
* **Vulkan Dynamic Dispatch, Hardware Pipeline Queries & Persistent Shader Caches:** Uses `VK_QUERY_TYPE_PIPELINE_STATISTICS` (`telemetry_osd.cpp`) for zero-overhead GPU observability, exports metrics to `Local\AX_Telemetry` / `/dev/shm/ax_telemetry`, and pairs persistent `zstd`-backed disk/tmpfs shader caches with `2:3:1` CPU core isolation (`Core 1` `0x02` Vulkan submission, `Core 5` `0x20` async SPIR-V workers) so compilation never stalls main emulation threads.

### What Wave `[AX]` Intentionally Avoids (or Leaves Unused)

* **Full Bindless / Descriptor Heap Paths:** Disabled or clamped (`maxPushDescriptors`) to stay safely inside GM200's hardware descriptor table ceilings.
* **Vulkan 1.4 Tile-Based Extensions (Dynamic Rendering Local Read):** Ignored, as desktop Maxwell cards gain zero bandwidth savings from tile-local attachment reads.
* **Native GPU Async Pipeline Linking:** Treated as unreliable on Maxwell R580 drivers; Wave `[AX]` compensates at the OS/CPU layer (core pinning + 3-stage Telemetry-Guided LLVM PGO) rather than trusting the driver to compile pipelines asynchronously without frametime spikes.

### Summary Verdict

For a GTX 980 Ti, **Eden Espresso Wave `[AX]` (`v1.35.1`)** operates near the theoretical efficiency ceiling of GM200 silicon: it actively leverages the genuine hardware-accelerated parts of Vulkan 1.3/1.4 (Dynamic Rendering, Push Descriptors, Scalar Block Layouts, Synchronization2 split barriers, and dedicated Transfer DMA queues) while routing around Maxwell's incomplete features (Graphics Pipeline Libraries, unbounded Descriptor Indexing, `1:64` FP16, and legacy NVDEC) via dual-die Intel UHD 630 co-processing.

---

## 3. Untapped Hardware Headroom & Optimisation Opportunities

When pairing a GTX 980 Ti (GM200 Maxwell) with Eden Espresso Wave `[AX]` and `Piccolo [AX]`, several native hardware capabilities on the GPU—and on the surrounding Coffee Lake platform—can be pushed even further beyond conservative lowest-common-denominator defaults (built to protect smaller `2 GB` / `4 GB` Maxwell cards like the GTX 960/970).

### 3.1 GM200 Hardware Capabilities & Status Matrix

| Native Capability | GTX 980 Ti Spec | Current Wave `[AX]` Behaviour (`v1.35.1`) | Opportunity for Further Utilisation |
| :--- | :--- | :--- | :--- |
| **6 GB Unsegmented VRAM & 384-bit Bus** | `6,144 MB` GDDR5 at `336.6 GB/s` (no split-pool penalty like the GTX 970's `3.5 + 0.5 GB` layout) | **`[IMPLEMENTED]`** Proactive `5.00 GiB` async pruning (`ax::vram` + `VkVramShield`) keeps `~5.0–5.35 GiB` safely utilized without WDDM/NVIDIA paging spills. | **Telemetry-Tuned Hysteresis:** Auto-tune the `4.50 GiB` recovery floor per title using `ax::TelemetryCounters` `Bar1Stalls` feedback. |
| **NVIDIA Maxwell Texture Compression (Delta Colour Compression)** | 3rd-Gen DCC (up to `4:1` lossless bandwidth compression) | **`[STAGED — Activity 5.2]`** `64 KB` Sparse VMA pool with VBLANK defragmentation (`texture_reaper.cpp`). | **Transient Attachment Memory Aliasing:** Reusing VRAM blocks for short-lived depth/stencil buffers keeps GM200's Delta Colour Compression warm in the `2 MB` L2 cache, boosting effective fill-rate at 1440p/4K docked scaling. |
| **Native FP16 / Half-Precision Packed Math** | Supported at `1:64` FP32 rate on GM200 (unlike Tegra X1's `2:1` fast FP16) | **`[STAGED — Activities 5.3 & 5.10]`** Offloads `ASTC → BC7/BC6H` to UHD 630's 24 native-FP16 EUs (`5.3`) and promotes Switch 3D shader FP16 to native FP32 (`5.10`). | **Complete `emit_spirv.cpp` FP32 Promotion Gate:** Finish Activity `5.10` (gated by `5.7` runtime hardware flags) so all remaining fragment/compute FP16 instructions execute at full GM200 FP32 throughput. |
| **Dedicated Async DMA Copy Engine** | Dual Copy Engines on GM200 (independent from the 3D/Compute graphics queue) | **`[IMPLEMENTED — Activity 6.8]`** Discovers `VK_QUEUE_TRANSFER_BIT` (`vulkan_device.cpp`) with Synchronization2 split barriers (`vkCmdSetEvent2`/`WaitEvents2`) + `VK_KHR_timeline_semaphore` Core 5 reaper. | **Overlap Streaming UBO/Texture Uploads:** Extend dedicated transfer queue usage to continuous background texture streaming across `Eden` and `Flycast` (`Generation-X-AX`). |

### 3.2 Vulkan 1.3 / 1.4 Native Features With Headroom Left

* **Full `VK_KHR_synchronization2` Split Barriers:** Maxwell's command processor handles fine-grained pipeline barriers cleanly. Extending split `synchronization2` events across all compute-to-graphics transitions allows GM200's 22 SMM units (2,816 CUDA cores) to overlap compute shader work with vertex/pixel shading rather than draining the entire GPU pipeline.
* **Subgroup Shuffle Instructions (`VK_KHR_shader_subgroup_vote` / arithmetic):** GM200 natively supports 32-thread warp-level register shuffles (`SHFL`) in hardware. Ensuring Eden's SPIR-V shader compiler emits native subgroup reductions instead of shared-memory (LDS) fallbacks cuts register pressure and boosts occupancy on complex Switch shaders.
* **Persistent Mapped Host-Visible / Device-Local Memory (`256 MB BAR1` WC Pool — Activity `6.9`):** With a `256 MB` BAR1 aperture on GM200, small, frequently updated Switch Uniform Buffer Objects (UBOs) and index buffers can be written directly by the CPU into WC (Write-Combined) mapped VRAM—eliminating an extra staging-buffer copy per frame.
* **Display-Refresh Phase-Locked Presentation (`VK_KHR_present_wait` — Activity `10.1`):** Querying `VK_KHR_present_id` / `VK_KHR_present_wait` on R580+ drivers enables exact queue-to-scanout delta measurement and `~300 µs` pre-VBLANK sync without external frame limiters (`RTSS`).

### 3.3 Platform-Level Synergy (Coffee Lake + UHD 630 + GTX 980 Ti)

* **Dual-Die FP16 vs. FP32 Specialization:** Rather than forcing GM200 (`1:64` FP16) or the 6-core Coffee Lake CPU (`2:3:1` pinned) to absorb heavy half-precision texture decompression and video scaling, **Intel UHD 630 (`24 EUs`, native `2:1` FP16 + fixed-function `VDBOX`/`VEBOX`)** handles ASTC transcoding and media decode over zero-copy `dma_buf`, leaving the GTX 980 Ti's `~6.0+ TFLOPS` FP32 shader array 100% focused on 3D geometry, pixel shading, and `lsfg-vk` frame generation.

### Summary Priority List & Implementation Status (`Master v1.35.1`)

1. **`[IMPLEMENTED in v1.35]` Proactive `5.00 GiB` Async VRAM Shield (`ax::vram` + `VkVramShield`):** Triggers non-blocking background pruning at `5.00 GiB` (`~4.50 GiB` hysteresis recovery) ahead of the `5.50 GiB` (`5.35 GiB` LSFG) hard ceiling so the GTX 980 Ti utilizes its full `6 GB` GDDR5 pool without WDDM/NVIDIA system-RAM paging stalls.
2. **`[IMPLEMENTED in v1.35]` Dedicated `VK_QUEUE_TRANSFER_BIT` & Vulkan 1.4 Split Barriers (`vkCmdSetEvent2` / `vkCmdWaitEvents2`):** Discovers GM200's dedicated hardware DMA copy queue (`vulkan_device.cpp`) and uses Synchronization2 split barriers (`ALL_GRAPHICS_BIT` → `TRANSFER_BIT`) + a `VK_KHR_timeline_semaphore`-fenced Core 5 reaper (`vk_vram_shield.hpp`) to eliminate VBLANK eviction stalls.
3. **`[NEXT — v1.35.1]` UHD 630 FP16 `ASTC → BC7/BC6H` Offload & Forced FP32 Shader Promotion (`Activities 5.3 & 5.10`):** Route half-precision ASTC decompression to UHD 630's 24 native-FP16 EUs via `dma_buf` and promote Switch 3D FP16 shader math to native FP32 on GM200 (`emit_spirv.cpp`).
4. **`[STAGED — v1.36]` `256 MB BAR1` Write-Combined Uniform Pool & `VK_KHR_present_wait` Phase Lock (`Activities 6.9 & 10.1`):** Stream dynamic uniforms directly over PCIe into WC-mapped VRAM and lock scanout pacing natively to display VBLANK.
---

## 4. Peripheral Silicon Assessment: HiBy FC3 (ESS Sabre) & Akai MPK Mini MK3

Beyond the CPU and dual-GPU die, the Stream 1 reference platform relies on two dedicated USB hardware peripherals—the **HiBy FC3 USB DAC** (`ESS Sabre ES9281AC Pro` / `ES9281PRO`) and the **Akai MPK Mini MK3** (`MPK mini 3`)—for sub-3ms audio conversion and real-time tactile control across `FLAC_AX`, `Eden`, `Cemu`, and `Generation-X-AX`.

### 4.1 HiBy FC3 (`ESS Sabre ES9281AC Pro` / `ES9281PRO`) Capability Breakdown

| Hardware Capability | ESS Sabre Spec | Current `[AX]` Behaviour (`v1.35.1`) | Remaining Optimization Headroom |
| :--- | :--- | :--- | :--- |
| **32-Bit HyperStream® II Quad DAC™ + Time Domain Jitter Eliminator** | Asynchronous USB Audio Class 2.0 (`UAC 2.0`) clock slave (`44.1 kHz`–`384 kHz`, `124 dB` SNR, `-112 dB` THD+N) | **`[IMPLEMENTED]`** Direct ALSA `hw:CARD=FC3,DEV=0` (`S32_LE`, `96`-frame / `2.00 ms` period, `snd_pcm_hw_params_set_rate_resample(0)`) on Linux and `FlexASIO` / Steinberg ASIO (`128` samples / `2.67 ms`) on Windows, pinned to **Core 0 (`0x01`)** with xHCI IRQ affinity. | **Zero-Copy `ALSA MMAP` & `WaveRT` (`Activity 9.2`):** Switch `FLAC_AX` from `snd_pcm_writei()` to `SND_PCM_ACCESS_MMAP_INTERLEAVED` (`snd_pcm_mmap_begin`/`commit`) to write `S32_LE` frames directly into the USB xHCI DMA ring. |
| **Native DoP (`DSD64` / `DSD128`)** | 1-bit DSD packed inside `24-bit` PCM frames (`0x05` / `0xFA` alternating marker bytes @ `176.4 kHz` / `352.8 kHz`) | **`[STAGED — Activity 9.3a]`** Ring buffer (`StereoFrameRing<65536>`) already transports `S32_LE` containers at `176.4–384 kHz` with `8x` buffer scaling. | **`.dsf` / `.dff` Bitstream Packer:** Add native 1-bit DSD-over-PCM framing into `ess_dac_hooks.hpp` without touching Core 0 write loops. |
| **30-Step Hardware Digital Attenuator** | Internal 32-bit ESS DSP volume control exposed via UAC 2.0 Feature Unit (`snd_mixer`) | **`[STAGED — Activity 9.3a]`** Host PCM stream stays `0 dBFS` bit-exact (`SampleToS32` MSB shift) without software gain scaling. | **`snd_mixer` Hardware Binding:** Bind `snd_mixer_selem_set_playback_volume_all()` to the ES9281AC Pro's internal attenuator so volume adjustments preserve full 24-bit dynamic range. |
| **Hardware Rate Lock & MQA / RGB Bitrate Status** | Automatic hardware clock locking and LED bitrate indicator driven by UAC 2.0 sample-rate negotiation | **`[IMPLEMENTED]`** `NegotiateEssRate()` and `OnEssHardwareConfigured()` drain and re-prepare ALSA cleanly on track sample-rate transitions while exporting `min / P99 / max` callback jitter to `/dev/shm/ax_telemetry`. | **Universal UAC2 Auto-Probe (`Activity 9.1`):** Auto-discover any connected UAC2 `S32_LE` DAC via `/proc/asound/cards` and Win32 `IMMDeviceEnumerator` when `hw:CARD=FC3,DEV=0` is absent. |

### 4.2 Akai MPK Mini MK3 (`MPK mini 3`) Capability Breakdown

| Hardware Capability | Akai MPK Mini MK3 Spec | Current `[AX]` Behaviour (`v1.35.1`) | Remaining Optimization Headroom |
| :--- | :--- | :--- | :--- |
| **Class-Compliant USB 1.1 Full-Speed MIDI (`hw:CARD=MPKmini3,DEV=0`)** | `12 Mbps` USB-MIDI (`1 ms` SOF polling) with zero proprietary driver dependencies | **`[IMPLEMENTED]`** `MpkMini3Listener` (`FLAC_AX/src/midi/mpk_mini3.cpp`) auto-detects `MPKmini3` via `/proc/asound/cards`, polls non-blocking (`SND_RAWMIDI_NONBLOCK`) on **Core 5 (`0x20`)**, handles running status + hot-unplug recovery, and pushes into a 64B-aligned SPSC ring (`1,024` events). | **Physical Root-Hub Isolation:** Keep `MPK mini 3` on a separate USB root port/hub from the HiBy FC3 so USB 1.1 `1 kHz` SOF frames never share an xHCI Transaction Translator with isochronous audio. |
| **25 Gen-2 Keys, 4-Way Thumbstick & Sustain Pedal** | Channel `1` velocity keys, X-axis `Pitch Bend` (`0xE0`), Y-axis `Mod Wheel` (`CC 1`), and `1/4"` `Sustain` (`CC 64`) | **`[IMPLEMENTED]`** Drives the 16-voice saw+sub synthesizer (`PolySynthEngine`) and `ax_av_studio` OpenGL HUD with zero audio-thread allocation. | **8-Wide AVX2 Voice Vectorization (`Activity 9.3b`):** Vectorize oscillator phase and filter loops with `_mm256_fmadd_ps` (`< 15 µs` per `96`-frame period) and add *Frontier: Elite II* cockpit thruster/scanner mapping (`Workstream 2.1`). |
| **8 Velocity/Pressure MPC Pads (`Banks A & B`)** | Channel `10` Notes `36–51`, Program Change (`0xC9`), CC (`0xB9`), and Channel Aftertouch (`0xD9`) | **`[IMPLEMENTED]`** Visualized live on the Core 1 OpenGL 2×4 velocity-reactive pad grid (`AvStudioHud`). | **Bidirectional Pad LED Feedback:** Send MIDI Note-On velocity LED commands back to `MPKmini3` to mirror active synth voices, sequencer steps, or emulator states. |
| **8 Endless 360° Rotary Encoders (`K1–K8`) & Onboard OLED** | Channel `1` `CC 70–77` (`0–127`) with real-time parameter name + value readout on the built-in OLED screen | **`[IMPLEMENTED]`** Mapped 1:1 to `K1` Cutoff, `K2` Resonance, `K3` Attack, `K4` Release, `K5` Sub-Oscillator, `K6` Harmonic Drive, `K7` Stereo Chorus, and `K8` Master Gain. | **Startup SysEx Program Push (`Activity 9.3b`):** Transmit Akai SysEx (`F0 47 7F 49 ... F7`) on `MpkMini3Listener::Start()` to auto-label `K1–K8` on the MK3's OLED display (`CUTOFF`, `RESO`, `ATTACK`, `RELEASE`, `SUB`, `DRIVE`, `CHORUS`, `MASTER`). |
