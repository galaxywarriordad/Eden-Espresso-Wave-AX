# 📜 Third-Party Notices, Legal Disclaimer & Attribution

## ⚖️ Legal Disclaimer & Hardware Ownership Notice

**Eden Espresso Wave [AX]** is an independent open-source software preservation and systems-engineering research project licensed under the **GNU General Public License v3.0 (GPLv3 or any later version)**.

* **No Bundled Proprietary Files:** This repository and its build scripts **do not include, host, or distribute** any copyrighted Nintendo Switch system firmware, cryptographic decryption keys (`prod.keys` / `title.keys`), proprietary BIOS images, or commercial game software.
* **Hardware Ownership Requirement:** In accordance with the official policies of the upstream **Eden** and **Citron** emulation projects, users **must legally own an original Nintendo Switch console** and dump their own system firmware, cryptographic keys, and legally purchased game cartridges/digital titles directly from their own hardware.
* **No Affiliation:** Nintendo and Nintendo Switch are registered trademarks of Nintendo Co., Ltd. This project is not affiliated with, endorsed by, or sponsored by Nintendo Co., Ltd.
* **Third-Party Commercial Binaries (`Lossless.dll`):** Native Lossless Scaling Frame Generation (`LSFG`) hooks require a legally purchased copy of **Lossless Scaling** installed by the user from Steam. No proprietary `Lossless.dll` files are bundled with this repository.

---

## 🙏 Upstream Emulation Foundations

**Eden Espresso Wave [AX]** stands on the shoulders of the open-source Nintendo Switch emulation community:

* **Eden Emulator Project (`eden-emu/eden`):** Core Horizon OS HLE implementation, Qt6 frontend, Vulkan/OpenGL renderers, and build infrastructure (GPLv3).
* **yuzu Emulator Project:** Foundational ARM64 emulation architecture, Maxwell GPU command processor, and audio DSP microkernel research (GPLv3).
* **Citron / Citron Neo Contributors:** Upstream Maxwell GM200 descriptor safety guards, subresource view range clamping, procedural vertex-less draw fastpaths, and async query recycling research (GPLv3).

---

## 🧩 Integrated Libraries, SDKs & Toolchains

| Component | License | Role in Eden Espresso Wave [AX] |
| :--- | :--- | :--- |
| **Steinberg ASIO SDK** | Proprietary Steinberg ASIO SDK License | Low-latency audio driver interface (`asio::asio` target ingested via CMake `FetchContent`). |
| **FlexASIO** | MIT License | Universal ASIO-to-WASAPI Exclusive Mode bridge (`wasapiExclusiveMode = true`) driving the HiBy FC3 USB DAC. |
| **Cubeb** | ISC License | Cross-platform audio backend providing automatic WASAPI secondary failover (`CubebSink`). |
| **Dynarmic** | 0BSD / MIT License | Dynamic ARM64-to-x86-64 JIT recompiler executing across home-biased host fibers (`0x1C`). |
| **Sirit** | BSD-3-Clause License | Runtime SPIR-V shader assembly and emission backend (`emit_spirv.cpp`). |
| **Xbyak** | BSD-3-Clause License | Runtime x86-64/AVX2 JIT assembler used for host vector routines and GPU Macro-JIT compilation. |
| **LLVM / Clang (`clang-cl` & `lld-link`)** | Apache-2.0 with LLVM Exception | C++20 compiler, ThinLTO linker, and Profile-Guided Optimisation (`llvm-profdata`) toolchain. |
| **Vulkan Memory Allocator (VMA) & Vulkan Headers** | MIT / Apache-2.0 License | GPU device memory management and Vulkan 1.4 dynamic dispatch definitions (`VK_NO_PROTOTYPES`). |
| **FFmpeg** | LGPLv2.1+ / GPLv3 | Multimedia container and codec framework paired with Windows Media Foundation / Intel QSV offload. |
| **Qt 6** | LGPLv3 / GPLv3 | Cross-platform desktop UI, configuration dialogs, and window surface hosting. |
| **Boost (`Boost.Context` / `Boost.Heap`)** | Boost Software License 1.0 | User-mode fiber context switching (`Common::Fiber`) and $O(1)$ Fibonacci event heap (`CoreTiming`). |
| **Zstandard (ZSTD) & LZ4** | BSD-3-Clause / BSD-2-Clause | Level-18 `.chd`/`.ncz` archive decompression (`ScopedNczQuarantine`) and pipeline cache compression. |
| **fmt & nlohmann/json** | MIT License | Type-safe diagnostic logging (`fmt::underlying`) and atomic JSON telemetry serialisation (`g_metrics`)[cite: 26]. |