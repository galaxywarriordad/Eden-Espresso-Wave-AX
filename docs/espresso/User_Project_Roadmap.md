# ☕ Eden Espresso Wave [AX] — End User Guide & Project Roadmap

Welcome to the official end-user overview for **Eden Espresso Wave [AX]**. If you are a gamer looking to get smooth, stutter-free performance out of your Nintendo Switch emulation setup without needing a computer science degree, you are in the right place.

This guide breaks down what this project is, what we have achieved, how it transforms your gaming experience, and what is coming next on our roadmap.

---

## 🎯 The Core Aim: Why Does This Project Exist?

Most modern PC software is built to share your computer's parts fluidly. Your web browser, Discord, and background windows all scramble for attention from your processor. For normal desktop apps, this is fine. For high-end gaming and emulation, it introduces **micro-stutter**—tiny, fraction-of-a-second hitches that ruin flat frame pacing and cause audio to crackle.

**Our Goal:** To transform a standard desktop PC into a **dedicated, hardware-locked gaming appliance**. Instead of letting your operating system guess where to run the emulator, we physically lock specific parts of your computer to handle specific jobs.

Make no mistake: rewriting an entire emulation engine to lock onto hardware like this is slow, highly methodical work. There is an old engineering proverb that asks: *"How do you eat an elephant?"* The classic answer is *"one bite at a time"*—but around here, **we are eating this microarchitectural elephant exactly one *byte* at a time.** By steadily debugging, optimizing, and fencing off system code bit by bit, we are systematically carving away latency to give you a flawless gaming experience.

---

## ⚠️ Mandatory Legal & Software Ownership Note

Eden Espresso Wave [AX] supports native frame generation through an internal link to `Lossless.dll`. To ensure absolute legal compliance, users must adhere to the following constraints:

* **Proprietary Files Excluded:** This emulator does not bundle, include, redistribute, or host copyrighted binaries, game assets, or proprietary third-party files (such as `Lossless.dll`).
* **Steam Ownership Required:** To use the built-in LSFG ×2 and LSFG ×3 frame interpolation features, **you must legally purchase and install *Lossless Scaling* directly from its official storefront on Steam**.
* **Linking Your Local File:** To enable these features inside the emulator, go to `Emulation` → `Configure` → `Graphics` and point the file path directly to the legitimate `Lossless.dll` file located within your local Steam installation folder (`...\Steam\steamapps\common\Lossless Scaling\`).

Reports, issues, or requests referencing cracked files, pirated copies, or unverified asset mirrors will be immediately locked and deleted.

---

## 🔊 Quick-Start Exclusive Audio Alignment Checklist

To achieve a zero-crackle audio output stream, your Windows desktop environment must match the emulator's real-time clock pacing system. Complete these simple setup steps before launching a game:

- [ ] **Match System Sample Rate:** Open Windows Control Panel → Sound → Playback, select your primary USB DAC or audio interface endpoint, click *Properties* → *Advanced*, and force the default format strictly to **2 Channel, 24-bit or 32-bit, 48000 Hz (Studio Quality)** to sync 1:1 with the emulator's native 48 kHz engine clock.
- [ ] **Authorize Exclusive Access:** In that same *Advanced* settings tab, check the box to **"Allow applications to take exclusive control of this device"** and **"Give exclusive applications priority"**. This permits FlexASIO to bypass the Windows `audiodg.exe` processing mixer.
- [ ] **Verify Telemetry Output:** Launch a game, wait 60 seconds, check your `%APPDATA%\eden\log\eden_log.txt` or `build_stage_telemetry_metrics.json` cache file, and verify that `"AudioUnderruns"` remains locked at `0` (via live Local\AX_Telemetry shared memory).

---

## 🛠️ What We Have Done (Sprint 0 Baseline Completed)

We have just completed our foundational development cycle (**Sprint 0**), heavily optimizing the software for mid-to-high-end setups. Here is what has been built and verified behind the scenes:

1. **Anti-Stutter Firewall Deployed:** We built an automated checking system that scans every line of code before release to make sure no performance-destroying bugs sneak into the emulator.
2. **Proactive `5.00 GiB` Async VRAM Shield (Zero VBLANK Stalls):** When a high-end game fills up the memory on a 6 GB graphics card (like the GTX 980 Ti), Windows normally locks up for 20–45 milliseconds to swap data into system RAM. We built a next-gen **Asynchronous VRAM Shield** powered by **Vulkan 1.4 split barriers** and a dedicated hardware DMA transfer queue. As soon as VRAM reaches **5.00 GiB**, a background worker on Core 5 quietly prunes unneeded textures down to ~4.50 GiB completely off the main rendering path—giving you a massive safety buffer with zero frame-time hiccups.
3. **Smart `2:3:1` Physical-Core Locking (`ax-sdk`):** The emulator automatically scans your CPU at startup, filters out hyperthreaded/SMT sibling threads and efficiency cores, and hard-pins your audio, Vulkan graphics, and game code onto separate physical performance cores (`CoreRatio=2:3:1`) so they never fight for cache or CPU time.
4. **Flawless Video Interop:** Cutscenes can cause massive stutters when switching from gameplay. We built a zero-copy direct bridge that forces your main graphics card to wait for your computer's built-in video decoder, completely eliminating video tearing and stutters during cutscenes.

---

## 🎮 What the System Does for Your Games Right Now

When you launch a game with Eden Espresso Wave [AX], your hardware behaves completely differently than it would with stock software:

* **Locked 60 FPS with Flat Frame Pacing:** Games like *Bayonetta 3* that drop frames on a standard console are stabilized to a locked 60 frames per second at 2x or 3x original resolution.
* **Zero Audio Crackle:** Emulation audio often pops and crackles when the hard drive or processors are working hard. Because your audio system now has an exclusive, unbothered pipeline straight to your USB DAC (audio device), sound remains bit-perfect and completely crisp.
* **Silent Background Loading:** When a game streams textures from an old mechanical hard drive, the system usually hitches. We offloaded these data management loops onto an isolated background thread that cleans up data quietly, so you never feel it on your controller.

---

## 🚀 Coming in the Next Sprint (`v1.35` Backlog)

We are immediately transitioning into our next optimization cycle. Here are the features gamers will see landing very soon:

* **Automated One-Click Installer & Updater:** We are building a master configuration tool (`PGO_config.bat + ax-sdk/tools/`) and packager that will cleanly bundle the emulator, required link libraries, and optimized settings into a plug-and-play package.
* **Next-Gen Texture Compression (`BC7` & `BC6H`):** We are adding support for modern, near-lossless compression. This will fix blocky textures and color "banding" in dark scenes without blowing past your graphics card's memory limits.
* **The Bare-Metal Linux Plug-in:** For enthusiasts, we are launching an early version of our signature "USB Appliance". You will be able to flash the emulator onto a fast USB thumb drive, plug it into your PC, and boot directly into a highly stripped-down, ultra-fast gaming operating system that bypasses Windows entirely.

---

## 🗺️ Subsystem Feature-to-Benefit Roadmap

To bridge the gap between our code and your actual controller, the timeline below details exactly what our planned updates do and how they directly translate into a better gaming experience for you:

```text
[ v1.35: GRAPHICS & STABILITY ] ──► [ v1.36: CINEMATIC FLOW ] ──► [ v1.37: REAL-TIME PACING ]
  (Fixes Dark Color Banding)          (Zero-Lag Double Frames)        (Predictive Micro-Stutter Fixes)
```

### 📦 Milestone 1: Graphics Fidelity & VRAM Safety (Target: `v1.35` Release)
* **The Feature:** Native `BC7`/`BC6H` texture processing and automated `64 KB` sparse memory allocation filters.
  * *What it means for you:* Eliminates blocky graphics and ugly pixelated colour patterns in dark, shadowy levels. Additionally, the sparse memory manager removes the tiny, fraction-of-a-second "hitches" that traditionally happen when you cross loading zones in massive open-world games.
* **The Feature:** Config-driven physical-core balancing and automatic feature configurations.
  * *What it means for you:* The emulator automatically detects your exact processor setup and assigns your cores correctly on launch day. No manual configuration or complex custom file-tweaking is required to prevent background tasks from stealing your performance.

### 🎬 Milestone 2: Cinematic Interop & Smooth Frame Ingestion (Target: `v1.36` Release)
* **The Feature:** WinRT Windows Graphics Capture and internal multi-device computing queue.
  * *What it means for you:* Enables seamless, hardware-level **2x Frame Generation (LSFG)**. Your PC will intelligently insert extra frames into your video output stream without introducing heavy input lag, turning a demanding 30 FPS title into a visually fluid 60 FPS spectacle.
* **The Feature:** `256 MB` Write-Combined direct indexing fastpath.
  * *What it means for you:* Speeds up data transfers between your memory and your graphics card. This means instant asset loading with zero performance drops when games cycle through thousands of minor environmental objects rapidly.

### ⏱️ Milestone 3: Real-Time Pacing & Predictive HDD Streaming (Target: `v1.37` Release)
* **The Feature:** Strided HDD-to-NVMe predictive background pre-faulting ring.
  * *What it means for you:* If you store your game collection on an old, slow mechanical hard drive, the emulator will predict where you are walking and cache game assets silently in the background. This allows you to play open-world games smoothly off cheap spinning drives without rendering lag.
* **The Feature:** Closed-loop high-precision pacing governor tracking.
  * *What it means for you:* Monitors your PC's real-time internal clock down to the microsecond. If your computer begins to lag under intense action, the governor will automatically shift your performance targets to prioritize your controller inputs and keep your audio crystal-clear.

---

## 🔌 The Eden Piccolo [AX] Appliance: Pure Bare-Metal Gaming

For players demanding the absolute highest tier of real-time performance, Windows itself can sometimes act as a bottleneck due to mandatory background OS updates and telemetries. To bypass this entirely, we are engineering **Eden Piccolo [AX]**—a custom, hyper-optimized Linux operating system designed to run entirely inside your computer's high-speed RAM channels:

```text
[ Cheap USB 3.0 Stick ] ──(Boots directly to RAM)──► [ Stripped Linux 6.12-rt Kernel ]
                                                                   │
0 ms WINDOWS SCHEDULER OVERHEAD ◄──────────────────────────────────┘
```

* **Boot and Play from a Thumb Drive:** You can flash the entire operating system onto a fast, inexpensive USB thumb drive (like a Samsung FIT Plus). On boot, the entire appliance loads directly into a tiny `< 300 MB` slice of your computer's memory (`toram`), allowing you to unplug or ignore your main Windows hard drives completely while playing.
* **Zero Windows Overhead:** Piccolo strips away desktop window managers, background search indexers, and bloated audio servers. Running on a customized, real-time Linux `6.12-rt` kernel, your PC stops acting like a workstation and transforms 100% into a dedicated, low-latency gaming machine.
* **Hardware-Level Isolation:** Piccolo enforces your hardware partitioning automatically. It locks the Linux kernel and system inputs completely onto Core 5, leaving your main processor cores entirely unbothered and free to focus purely on rendering your game at absolute maximum speed.

---

## 🌌 The Long-Term Plans (The Blue-Sky Vision)

Our ultimate goal is absolute perfection in real-time execution. In the long term, we are researching two highly advanced optimization tracks:

1. **AI-Driven Live Code Tuning:** A smart, background daemon that watches the game as you play. If it catches an unoptimized or heavy piece of game code executing frequently, it will re-compile it on the fly into hyper-fast, modern PC instructions between frames with zero gameplay interruption.
2. **Cross-Silicon Motion Vector Interpolation:** We are going to harness the power of your idle integrated graphics chip (the video port on your motherboard you usually ignore). While your main graphics card handles 3D rendering, the emulator will offload advanced motion vector tracking to that idle chip, driving flawless frame generation (`lsfg-vk`) to boost perceived smoothness effortlessly.

---

### 📚 Further Project Information

For detailed installation guides, technical deep-dives, or to review our code safety check logs, explore the documentation links located directly within the [Eden Espresso Wave [AX] Repository](https://git.eden-emu.dev/galaxywarriordad/Eden-Espresso-Wave-AX#readme).