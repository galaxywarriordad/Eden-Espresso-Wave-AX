// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

#include <cassert>
#include <cstdint>
#include <iostream>

// =======================================================================
// TASK 1 DIAGNOSTIC TEST: Win32 x64 Non-Volatile Register Validation
// =======================================================================

// Forward declarations matching your structural architecture layout
namespace Common {
    struct Fiber {
        static void Switch(Fiber* target);
    };
}

void AuditFiberContextStability(Common::Fiber* target_fiber) {
    std::cout << "[DIAGNOSTIC] Initializing Task 1 microarchitectural register audit..." << std::endl;

    // Fixed magic bitmask markers to track register preservation across context switches
    const uint64_t expected_marker = 0xDEADBEEFCAFEBABEULL;

    uint64_t rbx_post = 0;
    uint64_t r12_post = 0;
    uint64_t r15_post = 0;

    #if defined(_MSC_VER) || defined(__clang__)
    // Force direct hardware mapping by inline-clobbering and backing up state vectors
    __asm__ volatile (
        // 1. Force pattern into non-volatile lanes before fiber yield handshake
        "movq %3, %%rbx\n\t"
        "movq %3, %%r12\n\t"
        "movq %3, %%r15\n\t"

        // 2. Trigger the active fiber context swap sequence target
        // Pass target fiber down via Win32 RCX/RDI ABI standard parameter rules
        "movq %4, %%rdi\n\t"     
        "call *%5\n\t"

        // 3. Extract the register state signatures post-resume loop execution pass
        "movq %%rbx, %0\n\t"
        "movq %%r12, %1\n\t"
        "movq %%r15, %2\n\t"
        : "=r"(rbx_post), "=r"(r12_post), "=r"(r15_post)
        : "r"(expected_marker), "r"(target_fiber), "r"(Common::Fiber::Switch)
        : "rbx", "r12", "r15", "rdi", "memory"
    );
    #endif

    // State Evaluation Verification Gate
    if (rbx_post != expected_marker || r12_post != expected_marker || r15_post != expected_marker) {
        std::cerr << "[@=FATAL] TASK 1 STABILITY CRASH: Non-volatile register corruption caught!" << std::endl;
        std::cerr << " -> Expected Signature Map: 0x" << std::hex << expected_marker << std::endl;
        std::cerr << " -> RBX State Post-Swap   : 0x" << std::hex << rbx_post << std::endl;
        std::cerr << " -> R12 State Post-Swap   : 0x" << std::hex << r12_post << std::endl;
        std::cerr << " -> R15 State Post-Swap   : 0x" << std::hex << r15_post << std::endl;
        assert(false && "Microarchitectural Fiber State Leak Detected.");
    } else {
        std::cout << "[SUCCESS] Task 1 Register Audit passed: Context assembly holds state cleanly." << std::endl;
    }
}
