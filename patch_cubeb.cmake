# patch_cubeb.cmake

# 1. FIX THE CUBEB SUBMODULE CRASHES & EXECUTION BLOCKS
set(CUBEB_CMAKELISTS "${CPM_SOURCE_CACHE}/cubeb/fa02160712/CMakeLists.txt")
if(EXISTS "${CUBEB_CMAKELISTS}")
    file(READ "${CUBEB_CMAKELISTS}" CUBEB_CONTENTS)
    
    set(SEARCH_SAN "message(FATAL_ERROR \"Could not find sanitizers-cmake")
    set(REPLACE_SAN "message(STATUS \"Bypassing sanitizers-cmake check... # ")
    string(REPLACE "${SEARCH_SAN}" "${REPLACE_SAN}" CUBEB_MODIFIED "${CUBEB_CONTENTS}")
    
    set(SEARCH_GTEST "message(FATAL_ERROR \"Could not find googletest")
    set(REPLACE_GTEST "message(STATUS \"Bypassing googletest check... # ")
    string(REPLACE "${SEARCH_GTEST}" "${REPLACE_GTEST}" CUBEB_MODIFIED "${CUBEB_MODIFIED}")

    set(SEARCH_SUBDIR "add_subdirectory(googletest)")
    set(REPLACE_SUBDIR "# add_subdirectory(googletest)")
    string(REPLACE "${SEARCH_SUBDIR}" "${REPLACE_SUBDIR}" CUBEB_MODIFIED "${CUBEB_MODIFIED}")

    set(SEARCH_ADDSAN "add_sanitizers(")
    set(REPLACE_ADDSAN "# add_sanitizers(")
    string(REPLACE "${SEARCH_ADDSAN}" "${REPLACE_ADDSAN}" CUBEB_FINAL "${CUBEB_MODIFIED}")
                   
    file(WRITE "${CUBEB_CMAKELISTS}" "${CUBEB_FINAL}")
endif()

# 2. RESTORE THE VULKAN GRAPHICS PIPELINE BOOST INCLUDE
set(GRAPHICS_PIPELINE_H "${CMAKE_SOURCE_DIR}/src/video_core/renderer_vulkan/vk_graphics_pipeline.cpp")
if(EXISTS "${GRAPHICS_PIPELINE_H}")
    file(READ "${GRAPHICS_PIPELINE_H}" PIPELINE_CONTENTS)
    
    # Reverse the previous change to restore the original include perfectly
    string(REPLACE "#include <boost/container/vector.hpp>\n#include <boost/container/static_vector.hpp>"
                   "#include <boost/container/small_vector.hpp>" 
                   PIPELINE_RESTORED "${PIPELINE_CONTENTS}")
                   
    file(WRITE "${GRAPHICS_PIPELINE_H}" "${PIPELINE_RESTORED}")
endif()

# 3. FIX THE CUBEB TYPE-CAST WARNINGS
if(EXISTS "${FILE_PATH}")
    file(READ "${FILE_PATH}" FILE_CONTENTS)
    
    set(SEARCH_CAST "ret->devid = (cubeb_devid)devid;")
    set(REPLACE_CAST "ret->devid = (cubeb_devid)(uintptr_t)devid;")
    
    string(REPLACE "${SEARCH_CAST}" "${REPLACE_CAST}" MODIFIED_CONTENTS "${FILE_CONTENTS}")
                   
    file(WRITE "${FILE_PATH}" "${MODIFIED_CONTENTS}")
    message(STATUS "Successfully applied Eden Emulator build patches to dependencies.")
endif()
