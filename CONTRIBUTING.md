# 🤝 Contributing to Eden Espresso Wave [AX]

Thank you for your interest in **Eden Espresso Wave [AX]**! Because this fork is engineered around deterministic core partitioning, LLVM `clang-cl` 23+ (`-march=skylake`, `-Werror`), and heterogeneous GPU/DAC offloading, please review these guidelines before submitting issues or pull requests.

---

## ⚖️ Legal & Copyright Policy

In strict alignment with the upstream **Eden** and **Citron** projects:

* **Original Hardware Required:** You **must own an original Nintendo Switch console** and dump your own system firmware, cryptographic keys (`prod.keys` / `title.keys`), and legally purchased games directly from your own hardware.
* **Zero Piracy Tolerance:** Any issue, pull request, commit, or log attachment referencing or containing copyrighted Nintendo firmware, decryption keys, ROMs/NSPs/XCIs, or third-party commercial binaries (`Lossless.dll`) will be closed immediately.

---

## 🛠️ Engineering & Code Standards

All code contributions must compile cleanly under **LLVM `clang-cl` 23.1.4+** with strict diagnostics (`-Werror`) and pure Win32 `/MD` linkage:

1. **Vulkan 1.4 Dynamic Dispatch (`VK_NO_PROTOTYPES`):**
   * Never call raw `vulkan-1.lib` C symbols directly. Route all Vulkan device calls through `m_device.GetDispatchLoader()` and physical device queries through `device.GetPhysical()`.
2. **Win32 Header & Macro Hygiene:**
   * Always define `NOMINMAX` before including `<windows.h>` and add `#ifdef CreateEvent` / `#undef CreateEvent` immediately after `<windows.h>` or `<timeapi.h>` so Horizon OS `CreateEvent` methods do not collide with `CreateEventA`.
3. **Clang-cl Strict Warning Compliance:**
   * Initialise all raw Qt6 widget pointers (`= nullptr`), avoid unused lambda captures (`-Wunused-lambda-capture`), wrap scoped `enum class` values in `fmt::underlying(...)` for `fmt 12.1+`, and use `nullptr` rather than literal `0` in pointer fields (`-Wzero-as-null-pointer-constant`).
4. **Core-Lane & Audio Safety:**
   * Never block or allocate heap memory on **Core 0 (`0x01`, Critical MMCSS `Pro Audio`)** or **Core 1 (`0x02`, `VulkanWorker`)**. Heavy background work (shader compilation, ZSTD decompression, video decode) must remain quarantined on **Core 5 (`0x20`)**.

---

## 🖥️ Adapting Core Masks for Different CPUs

If you are porting the **Elastic 2:3:1 Non-SMT** layout to another processor topology:

* **6C/6T Non-SMT (i5-9600K / 8600K / 8400):** Use the default masks in `src/common/thread.h` (`0x01`, `0x02`, `0x1C`, `0x20`).
* **8C/8T Non-SMT (i7-9700K):** Expand `PrimaryGuestJit` to `0x3C` (Cores 2–5) and shift `AuxQuarantine` to `0xC0` (Cores 6–7).
* **6C/12T SMT (i5-10600K / 12400F / Ryzen 5 5600X):** Map bitmasks strictly to even logical indices (`0, 2, 4, 6, 8, 10` — physical cores only) so sibling hyper-threads never contend with Core 0 audio or Core 1 Vulkan submission.

---

## 🐛 Submitting Bug Reports & Telemetry

When opening an issue on **`galaxywarriordad/Eden-Espresso-Wave-AX`**, please include:

1. **Hardware & BIOS Topology:** CPU model/clocks, GPU model/driver version, iGPU DVMT allocation, and USB DAC model.
2. **Runtime Telemetry & Logs:** Attach `%APPDATA%\eden\log\eden_log.txt` and `build_stage_telemetry_metrics.json` (verifying `"audio_underruns"` and `"shader_worker_threads"`).
3. **Reproduction Steps:** Note whether the issue occurs with `fast_ubo_passthrough` enabled or disabled, and which **ASTC Recompression** mode (`Uncompressed` vs. `BC3`) is active[cite: 25].