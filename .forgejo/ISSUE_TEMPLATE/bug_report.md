---
name: "🐛 Bug Report & Telemetry Audit"
about: "Report a runtime, audio, Vulkan, or PGO build issue with hardware topology details"
title: "[Bug]: "
labels: ["bug"]
---

## ⚖️ Hardware Ownership & Copyright Confirmation
*Please confirm before submitting (issues referencing pirated ROMs, firmware, or keys will be closed immediately):*
- [ ] I own an original Nintendo Switch console and dumped my own firmware, keys, and legally purchased game directly from my hardware.
- [ ] No copyrighted keys, firmware binaries, game files, or proprietary DLLs (`Lossless.dll`) are attached to this report.

---

## 💻 Host Silicon & Driver Topology
* **Build Stage & Commit:** *(e.g., Stage 0 / Stage 2 PGO — commit `5af00d7983`)*
* **CPU & Core Layout:** *(e.g., Intel Core i5-9600K 6C/6T Non-SMT, Core 0 @ 4.9 GHz, Cores 1–5 @ 4.8 GHz)*
* **Primary Discrete GPU & Driver:** *(e.g., NVIDIA GTX 980 Ti 6 GB — Driver 566.xx)*
* **Secondary iGPU & DVMT:** *(e.g., Intel UHD Graphics 630 — 512 MB DVMT Pre-Allocated)*
* **Audio DAC & `FlexASIO.toml` Buffer:** *(e.g., HiBy FC3 USB DAC — `wasapiExclusiveMode = true`, `bufferSizeSamples = 128`)*
* **IRQ Steering (`check_hardware.ps1`):** *(e.g., PASS — USB `0x01`, dGPU `0x02`, NVMe/SATA `0x20`)*

---

## ⚙️ Emulator Configuration (`Emulation → Configure`)
* **ASTC Recompression:** *(e.g., Uncompressed RGBA8 / BC3)*
* **Direct UBO Passthrough (`fast_ubo_passthrough`):** *(Enabled / Disabled)*
* **Hardware Video Decoder:** *(Intel Quick Sync Video / GPU)*
* **Lossless Scaling (`LSFG`):** *(Disabled / LSFG x2 / LSFG x3)*

---

## 📝 Description & Reproduction Steps
*Describe what happened, which core/subsystem showed contention, and how to reproduce it:*

1. 
2. 
3. 

---

## 📊 Telemetry & Log Output
*Paste relevant lines from `%APPDATA%\eden\log\eden_log.txt` and `build_stage_telemetry_metrics.json`:*

```json
{
  "audio_underruns": 0,
  "shader_worker_threads": 1
}
```
