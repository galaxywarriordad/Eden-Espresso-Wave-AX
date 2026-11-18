Fix Clang-cl compilation failure in yuzu_room due to zero-as-null-pointer-constant

Modernise the structural array definition for `long_options` inside 
yuzu_room.cpp. Passing raw `0` literals into pointer elements within the 
POSIX `getopt` struct causes fatal diagnostic failures when building with 
newer standalone LLVM/Clang toolchains under strict warning profiles.

Swapped integer zero literals to `nullptr` for the `name` and `flag` fields 
within the structural configurations and terminal elements. This maintains 
exact functional consistency while satisfying modern C++ type-conformance rules.