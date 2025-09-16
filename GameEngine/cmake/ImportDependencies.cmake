include(FetchContent)

# --- Put all downloaded libraries inside lib/
set(FETCHCONTENT_BASE_DIR ${CMAKE_BINARY_DIR}/lib)

# Helper to group libraries in VS Solution Explorer
function(set_library_folder target)
    if (TARGET ${target})
        set_target_properties(${target} PROPERTIES FOLDER "Libraries")
    endif()
endfunction()


# ---------------------------------------------------------
# GLFW
# ---------------------------------------------------------
macro(import_glfw)
    if(NOT TARGET glfw)  # Guard to prevent multiple inclusion
        FetchContent_Declare(
            glfw
            GIT_REPOSITORY https://github.com/glfw/glfw.git
            GIT_TAG 3.3.8
        )
        set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
        set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
        set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(glfw)

        include_directories(${GLFW_SOURCE_DIR}/include)
        set_library_folder(glfw)
    endif()
endmacro()

# ---------------------------------------------------------
# GLM
# ---------------------------------------------------------
macro(import_glm)
    if(NOT TARGET glm)  # Guard to prevent multiple inclusion
        FetchContent_Declare(
            glm
            GIT_REPOSITORY https://github.com/g-truc/glm.git
            GIT_TAG master
        )
        FetchContent_MakeAvailable(glm)

        include_directories(${glm_SOURCE_DIR})
        set_library_folder(glm)
    endif()
endmacro()

# ---------------------------------------------------------
# GLEW
# ---------------------------------------------------------
macro(import_glew)
    if(NOT TARGET glew_s)  # Guard to prevent multiple inclusion
        FetchContent_Declare(
            glew
            GIT_REPOSITORY https://github.com/omniavinco/glew-cmake.git
            GIT_TAG master  # Or use a specific commit or tag if preferred
        )

        # Disable shared builds if needed
        set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libs" FORCE)
        FetchContent_MakeAvailable(glew)

        # Optional: include directories if needed manually (though target_link_libraries handles it)
        include_directories(${glew_SOURCE_DIR}/include)
        set_library_folder(libglew_static)
        set_library_folder(libglew_shared)
    endif()
endmacro()

# ---------------------------------------------------------
# ImGui
# ---------------------------------------------------------
macro(import_imgui)
    if(NOT TARGET imgui)  # Guard to prevent multiple inclusion
        FetchContent_Declare(
            imgui
            GIT_REPOSITORY https://github.com/ocornut/imgui.git
            GIT_TAG v1.90.4
        )
        if(NOT imgui_POPULATED)
            FetchContent_Populate(imgui)
        endif()

        # Create a CMakeLists.txt file inside imgui source (if not exists)
        if(NOT EXISTS ${imgui_SOURCE_DIR}/CMakeLists.txt)
            file(WRITE ${imgui_SOURCE_DIR}/CMakeLists.txt
"cmake_minimum_required(VERSION 3.15)
project(imgui LANGUAGES CXX)

add_library(imgui STATIC
    imgui.cpp
    imgui_demo.cpp
    imgui_draw.cpp
    imgui_tables.cpp
    imgui_widgets.cpp
    backends/imgui_impl_glfw.cpp
    backends/imgui_impl_opengl3.cpp
)

target_include_directories(imgui PUBLIC
    \${CMAKE_CURRENT_SOURCE_DIR}
    \${CMAKE_CURRENT_SOURCE_DIR}/backends
)

target_link_libraries(imgui PUBLIC glfw libglew_static)

set_property(TARGET imgui PROPERTY CXX_STANDARD 11)
set_property(TARGET imgui PROPERTY CXX_STANDARD_REQUIRED ON)
")
        endif() 

        set(IMGUI_BINARY_DIR ${CMAKE_BINARY_DIR}/lib/imgui-build)

        add_subdirectory(${imgui_SOURCE_DIR} ${IMGUI_BINARY_DIR})
        endif()

        set_library_folder(imgui)
    
endmacro()

# ---------------------------------------------------------
# Vulkan (not used)
# ---------------------------------------------------------
macro(find_vulkan)
    # 1. Set VULKAN_SDK_PATH in .env.cmake to target specific vulkan version
    if (DEFINED VULKAN_SDK_PATH)
        set(Vulkan_INCLUDE_DIRS "${VULKAN_SDK_PATH}/Include") # 1.1 Make sure this include path is correct
        set(Vulkan_LIBRARIES "${VULKAN_SDK_PATH}/Lib") # 1.2 Make sure lib path is correct
        set(Vulkan_FOUND "True")
    else()
        find_package(Vulkan REQUIRED) # throws error if could not find Vulkan
        message(STATUS "Found Vulkan: $ENV{VULKAN_SDK}")
        include_directories(/Users/ps/VulkanSDK/1.3.268.1/macOS/include)
    endif()
    if (NOT Vulkan_FOUND)
        message(FATAL_ERROR "Could not find Vulkan library!")
    else()
        message(STATUS "Using vulkan lib at: ${Vulkan_LIBRARIES}")
    endif()
endmacro()

# ---------------------------------------------------------
# Vulkan Memory Allocator (not used)
# ---------------------------------------------------------
macro(import_vma)
    if(NOT TARGET vma)  # Guard to prevent multiple inclusion
        FetchContent_Declare(vma
            GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
            GIT_TAG v3.0.1
        )
        FetchContent_MakeAvailable(vma)
        include_directories(${vma_SOURCE_DIR}/include)
        set_library_folder(VulkanMemoryAllocator)
    endif()
endmacro()

# ---------------------------------------------------------
# FMOD
# ---------------------------------------------------------
macro(import_fmod)
    if (NOT TARGET fmod::core)
        set(FMOD_DIR "${PROJECT_SOURCE_DIR}/lib/fmod")

        # FMOD include directories
        set(FMOD_INCLUDE_DIRS
            ${FMOD_DIR}/core/inc
            ${FMOD_DIR}/studio/inc
        )

        # FMOD library paths (different for Debug/Release)
        set(FMOD_CORE_DEBUG_LIB    ${FMOD_DIR}/core/lib/x64/fmodL_vc.lib)
        set(FMOD_CORE_RELEASE_LIB  ${FMOD_DIR}/core/lib/x64/fmod_vc.lib)
        set(FMOD_STUDIO_DEBUG_LIB  ${FMOD_DIR}/studio/lib/x64/fmodstudioL_vc.lib)
        set(FMOD_STUDIO_RELEASE_LIB ${FMOD_DIR}/studio/lib/x64/fmodstudio_vc.lib)

        # Create interface targets for easier linking
        add_library(fmod::core INTERFACE IMPORTED)
        target_include_directories(fmod::core INTERFACE ${FMOD_INCLUDE_DIRS})
        target_link_libraries(fmod::core INTERFACE
            debug ${FMOD_CORE_DEBUG_LIB} optimized ${FMOD_CORE_RELEASE_LIB}
        )

        add_library(fmod::studio INTERFACE IMPORTED)
        target_include_directories(fmod::studio INTERFACE ${FMOD_INCLUDE_DIRS})
        target_link_libraries(fmod::studio INTERFACE
            debug ${FMOD_STUDIO_DEBUG_LIB} optimized ${FMOD_STUDIO_RELEASE_LIB}
        )

        # Post-build step to copy DLLs depending on configuration
        function(_fmod_copy_dll TARGET)
            add_custom_command(TARGET ${TARGET} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "$<$<CONFIG:Debug>:${FMOD_DIR}/core/lib/x64/fmodL.dll>"
                    "$<$<CONFIG:Release>:${FMOD_DIR}/core/lib/x64/fmod.dll>"
                    "$<TARGET_FILE_DIR:${TARGET}>"
            )
            add_custom_command(TARGET ${TARGET} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "$<$<CONFIG:Debug>:${FMOD_DIR}/studio/lib/x64/fmodstudioL.dll>"
                    "$<$<CONFIG:Release>:${FMOD_DIR}/studio/lib/x64/fmodstudio.dll>"
                    "$<TARGET_FILE_DIR:${TARGET}>"
            )
        endfunction()

    endif()
endmacro()

# Macro to import all dependencies
macro(importDependencies)
message(STATUS "Starting to import dependencies...")

    message(STATUS "Importing GLFW...")
    import_glfw()
    message(STATUS "GLFW imported successfully.")

    message(STATUS "Importing GLM...")
    import_glm()
    message(STATUS "GLM imported successfully.")

    message(STATUS "Importing GLEW...")
    import_glew()
    message(STATUS "GLEW imported successfully.")

    message(STATUS "Importing ImGui...")
    import_imgui()
    message(STATUS "ImGui imported successfully.")

    message(STATUS "Finding Vulkan SDK...")
    find_vulkan()
    message(STATUS "Vulkan set up successfully.")

    message(STATUS "Importing Vulkan VMA...")
    import_vma()
    message(STATUS "Vulkan VMA imported successfully.")

    message(STATUS "Importing FMOD...")
    import_fmod()
    message(STATUS "FMOD imported successfully.")

    message(STATUS "All dependencies have been imported successfully.")
    
endmacro()