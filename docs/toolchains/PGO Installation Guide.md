
Eden Emulator: High-Performance PGO Installation Guide

This guide outlines how to execute the automated Profile-Guided Optimisation (PGO) pipeline. This process is specifically tailored for the Intel Core i5-9600K architecture to unlock maximum emulation frame rates, tighten 1% low frame times, and minimize micro-stuttering.
🛠️ Prerequisites & Setup
Before running the automated scripts, ensure the following toolchains are installed on your Windows system:
• LLVM/Clang for Windows (v23.1.4+): System compiler engine and profiling toolkit.
• CMake (v3.31+): Configuration tree generator.
• Ninja (v1.12+): Parallel build automation engine.
[!IMPORTANT]
Ensure all the above tools are added to your system environment variables (%PATH%) so the scripts can locate and call them automatically.
🚀 Architectural PGO Execution Sequence
Follow these three phases sequentially to generate, train, and build your micro-optimised emulator binary.

Phase 1: Compile the Instrumented Binary

1. Open your workspace directory (C:\Users\nick_\Desktop\eden).
2. Launch the x64 Native Tools Command Prompt for VS from your Windows Start Menu. (This environment is mandatory so that Clang can inherit vital MSVC runtime and STL include variable paths).
3. Run the first automated script:cmd
PGO_stage1.bat
Use code with caution.
4. The script will automatically purge stale configuration caches, clear out previous profile data, and run a clean parallel compilation loop targeting your hardware layout.
• Language Flag Separation Safety: The script splits core compilation properties into explicit language buckets. This completely strips C++ exclusive features away from pure C source units, preventing toolchain crashes inside Boost dependencies.
• Universal C++20 Standardization: Enforces strict -DCMAKE_CXX_STANDARD=20 and -Xclang -std=c++20 parameters globally. This ensures third-party dependencies pull into the correct language standard, preventing Microsoft standard header parsing breaks.
• Memory Safety Alignment: The script automatically applies a strict 32-byte global over-allocation guard (-Xclang -faligned-allocation -D_CRT_ALIGNMENT=32) to shield the application from pointer misalignment errors on startup.
5. Wait until the window displays the [PGO STAGE 1 COMPLETE] success banner.

Phase 2: Generate High-Fidelity Hardware Profiles (Multi-Session Training)

To build a highly accurate, micro-optimized optimization map, you will execute 3 to 4 distinct gameplay training runs. This ensures Clang records varied architectural footprints (Vulkan translation paths, vector audio math, and cache-heavy logic blocks).
1. Ensure Stage 1 has completed successfully and generated your instrumented binary.
2. Launch your x64 Native Tools Command Prompt for VS and enter the output workspace layout:cmd
cd build_pgo\bin
Use code with caution.
3. Setup Environment Path Enclosure: Before executing the training binary directly via the prompt, re-establish the environment variable mapping if you are launching it manually (Note: inside the batch files, these require double % symbols, but use single % symbols if typing directly in the prompt terminal):cmd
set "LLVM_PROFILE_FILE=C:\Users\nick_\Desktop\eden\pgo_data\eden-%m-%p.profraw"
Use code with caution.
4. Execute Training Run 1: Launch eden.exe. Load a heavy gameplay profile. Actively explore, fight, and cache shaders for 15 minutes. Cleanly exit the emulator to flush all metrics hooks to your drive.
5. Navigate to your pgo_data\ folder. You will see an active .profraw file with process identifiers. Rename this file immediately to preserve it:cmd
ren "C:\Users\nick_\Desktop\eden\pgo_data\eden-*.profraw" "session1.profraw"
Use code with caution.
6. Repeat the sequence for Sessions 2, 3, and 4:
• Run eden.exe → Profile a different high-load zone for 15 minutes → Exit cleanly.
• Rename the newly generated trace before launching the next one:
• For Run 2: Rename the trace file to session2.profraw
• For Run 3: Rename the trace file to session3.profraw
• For Run 4: Rename the trace file to session4.profraw
[!WARNING]
Multi-Threaded Concurrency Boundary Safeguard
Because the Intel Core i5-9600K utilizes six discrete processing threads executing concurrently, never force a unified, static output name during live instrumentation loops. Always leverage automated formatting identifiers inside your execution environment to isolate structural worker logs: eden-%%m-%%p.profraw. Missing the %%m (module identity) or %%p (Process ID) wildcards can cause multiple physical cores to overwrite the same sector map concurrently, completely corrupting your data block layout upon closing down the application interface.
[!IMPORTANT]
Hardware Resource Allocation Parameters
Do not assign artificial processor constraints, performance overrides, or manual CPU affinity masks via Windows Task Manager during an instrumented tracking session. Clang requires completely unhindered range across your physical core layout to accurately chart how your architecture scales hot branch vectors across your memory lanes and cache pools.
🎯 Quick Checklist for the Training Run:
• Go down the most intense paths in your game (heavy shader generation zones, high-density combat pools, fast travel asset loading).
• Verify that .profraw files are actively populating inside your pgo_data\ folder as you play.
• Close the emulator cleanly using standard application menus when done. This guarantees that internal runtime memory buffers flush completely and cleanly onto your drive.

Phase 3: Execute the Final Optimization Build Pass

1. Verify that your emulator is closed cleanly and that your pgo_data\ folder contains newly generated, non-zero .profraw files.
2. Return to your primary workspace folder (C:\Users\nick_\Desktop\eden):cmd
cd ..\..
Use code with caution.
3. Execute the second automated script:cmd
PGO_stage2.bat
Use code with caution.
4. The script will automatically execute the following pipeline transitions:
• Profile Merging: Calls the LLVM profile engine (llvm-profdata.exe) to flatten and merge all separate gameplay traces (*.profraw) sitting in the folder simultaneously into a master optimization map (eden.profdata).
• Automated Forward-Slash Conversion: Programmatically strips native Windows backslashes out of the absolute profile path, replacing them with standard Unix forward slashes (/). This shields the Clang front-end driver from parsing string escape character faults.
• Optimization Ceiling Upgrades: Wipes out the dedicated optimized directory tree (build_pgo_opt) to ensure no variable inheritance pollution occurs, steps the compiler up to raw maximum optimization ceilings (/O3 /Ob3 /DNDEBUG /fp:fast), and schedules targeted Coffee Lake instruction set extensions via Clang's native -march=skylake flag combined with explicitly mapped vector targets (-mavx2 -mfma -mbmi2).
• Linker Geometry Restructuring: Routes the linkage step natively through lld-link.exe. LLD safely linearizes code branching geometry—packing high-frequency loop instructions directly into your CPU's L1 Instruction Cache to completely minimize frame-pacing latency.
🚀 Expected Performance Gains
Once Phase 3 completes, your freshly compiled native Intel binary located inside build_pgo_opt\bin will yield dramatic performance upgrades over stock legacy builds:
• Linear Code Branching: Real-time emulated translation pathways are physically linearized by Clang based on your actual gameplay data, drastically reducing pipeline flushes and branch mispredictions on your CPU.
• Enhanced Frametime Consistency: Your 1% low frame time metrics will lock in much tighter, heavily stabilizing on-the-fly shader generation stuttering during intensive compute cycles.
🛠️ Automated Troubleshooting Matrix

1. The Large Directory Database Parsing Stall (Phase 1 Training Halt)

• Symptom: The Instrumented Stage 1 binary crashes immediately during initialization, throws a silent heap exception on boot, or hangs indefinitely when reading the game selection panels.
• Root Cause: If your active game path points to a massive storage library (e.g., hundreds of games combined with vast networks of DLC and updates), the instrumented binary will attempt to run runtime checking hooks on every single file query. The background meta-parsing paths will rapidly deplete default compiler counter spaces or stack bounds, causing an abrupt termination before Phase 2 training can begin.
• Resolution: Isolate the scanning directory down to a minimal training profile. Create a temporary testing directory (e.g., Desktop\PGO_Train) and copy exactly 1 to 3 demanding target games into it without copying their DLC or update files. Open the emulator interface, add only this isolated testing folder, and complete your profiling run safely. Once the Phase 3 optimization pass is fully complete, you can safely swap back to your master database path.

2. Instrumented Application Instant Crash (Zero Terminal Output)

• Symptom: Double-clicking eden.exe flashes a blank window or terminates immediately without printing errors to the active command line prompt window.
• Root Cause: The Windows PE Loader or internal runtime environment is killing the process before execution enters main logic hooks. This is commonly caused by running an instrumented binary outside an active developer environment path, or by thread vector misalignment stemming from aggressive math definitions matching /fp:fast during unoptimized passes.
• Resolution:
1. Always run the executable within the active x64 Native Tools Command Prompt for VS session.
2. Switch your Stage 1 compilation script variables to precise math fallback parameters (set "BASE_FLAGS=/Od /Ob0 /DNDEBUG /W1 /fp:precise ... -Zm2000"). This forces the tracking array matrix to calculate linearly on startup without triggering vector memory faults.

3. File System Tracking Failure / Access Violations

• Symptom: LLVM profile hooks fail to dump metric tracks to disk, or throw string processing errors upon closing down the UI.
• Root Cause: The absolute variable string mapped to LLVM_PROFILE_FILE contains hidden spaces, special symbol chains, or native Windows backslashes that scramble the internal preprocessor formatting codes (%m, %p).
• Resolution: Ensure the formatting symbols are escaped correctly for the active terminal shell context environment by applying doubled percentage symbols (%%m and %%p) inside explicit batch deployment configurations to output the correct downstream parsing maps smoothly.