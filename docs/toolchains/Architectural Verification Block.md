To guarantee this patch functions cleanly when you initiate your PGO compilation tree via your updated PGO_stage1.bat, ensure that the macro definition matching this commit layout is exactly what you have inside src/video_core/renderer_vulkan/vk_graphics_pipeline.cpp:
cpp
#if defined(__clang__) && (defined(NDEBUG) || defined(__FAST_MATH__))
    #define LAMBDA_FORCEINLINE __forceinline
#elif defined(_MSC_VER) && defined(NDEBUG) && !defined(__clang__)
    #define LAMBDA_FORCEINLINE [[msvc::forceinline]]
#else
    #define LAMBDA_FORCEINLINE
#endif
