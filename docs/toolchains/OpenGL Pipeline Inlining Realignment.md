Fix Clang-cl token parsing failures in gl_graphics_pipeline

Collapse trailing lambda macro definitions when compiling under a standalone
LLVM frontend. 

Strict ISO C++ standard grammar engines reject trailing vendor macro tokens
placed between a lambda expression definition and its statement body. 
Under Clang-cl, this structural mismatch triggers fatal grammar 
validation loops.

Refactored the cross-compiler definitions to collapse LAMBDA_FORCEINLINE down
to empty space strictly within Clang context scopes. This successfully 
bypasses trailing token constraints while preserving legacy native 
[[msvc::forceinline]] behaviors for pure MSVC toolchain environments.

Files modified:
  - src/video_core/renderer_opengl/gl_graphics_pipeline.cpp
