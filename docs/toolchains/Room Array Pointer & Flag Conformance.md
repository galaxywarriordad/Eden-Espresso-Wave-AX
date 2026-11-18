Modernize yuzu_room structural layout for strict pointer conformance

Overhaul the legacy POSIX getopt_long configuration array (long_options) to 
satisfy modern LLVM type-safety parameters.

The standalone Clang-cl 23 driver enforces modern ISO C++ rules that reject 
the use of raw integer 0 literals as placeholder null pointers inside standard 
initialization blocks, throwing fatal -Wzero-as-null-pointer-constant diagnostics.

Applied structural type modernizations to unblock compilation trees:
  - Swapped raw literal 0 pointer configurations into explicit C++ nullptr indicators.
  - Replaced the invalid literal 0 flag placement row with the correct system macro no_argument definition.

Files modified:
  - src/dedicated_room/yuzu_room.cpp