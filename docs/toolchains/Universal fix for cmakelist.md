# Universal fix for MSVC 14.51+ STL and Clang-cl compatibility
if (MSVC AND CXX_CLANG)
    add_compile_options("/D_DISABLE_VECTOR_ANNOTATION")
    add_compile_definitions(_DISABLE_VECTOR_ANNOTATION)
    add_link_options("/BigObj") 
    message(STATUS "[GLOBAL STL FIX] Hard-enforced MSVC find-vectorizer bypass across all engine targets.")

    # Target-specific override to protect Opus from global fast math instability and inject Coffee Lake optimizations
    # Eden/Yuzu derivatives can name the internal target 'opus', 'externals::opus', or 'audio_core' sub-libraries
    set(OPUS_TARGET_VARIATIONS "opus" "externals::opus" "bundled_libopus" "audio_core")

    foreach(_opus_target IN LISTS OPUS_TARGET_VARIATIONS)
        if(TARGET ${_opus_target})
            # 1. Fetch, scrub /fp:fast, and mandate /fp:precise for floating-point stability
            get_target_property(_current_flags ${_opus_target} COMPILE_OPTIONS)
            if(_current_flags)
                list(REMOVE_ITEM _current_flags "/fp:fast")
            else()
                set(_current_flags "")
            endif()
            
            # 2. Inject precise math alongside dedicated architecture flags for your Intel 9600K (AVX2/FMA/BMI2)
            list(APPEND _current_flags "/fp:precise" "-mavx2" "-mfma" "-mbmi2")
            
            set_target_properties(${_opus_target} PROPERTIES COMPILE_OPTIONS "${_current_flags}")
            message(STATUS "[OPUS OPTIMIZATION] Isolated /fp:precise, AVX2, and FMA for target: ${_opus_target}")
        endif()
    endforeach()
endif()