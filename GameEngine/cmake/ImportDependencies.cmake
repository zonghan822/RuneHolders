include(FetchContent)

# --- Put all downloaded libraries inside lib/
set(FETCHCONTENT_BASE_DIR ${CMAKE_BINARY_DIR}/lib)

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
            GIT_TAG glew-cmake-release  # Or use a specific commit or tag if preferred
        )

        # Disable shared builds
        set(glew-cmake_BUILD_SHARED OFF CACHE BOOL "Disable GLEW shared library" FORCE)
        set(glew-cmake_BUILD_STATIC ON CACHE BOOL "Enable GLEW static library" FORCE)
        FetchContent_MakeAvailable(glew)
        set_library_folder(libglew_static)
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
            GIT_TAG docking
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
        set_library_folder(imgui)
    endif()
endmacro()

# ---------------------------------------------------------
# RapidJSON (header-only library)
# ---------------------------------------------------------
macro(import_rapidjson)
    if(NOT TARGET rapidjson)  # Guard to prevent multiple inclusion
        include(FetchContent)

        FetchContent_Declare(
            rapidjson
            GIT_REPOSITORY https://github.com/Tencent/rapidjson.git
            GIT_TAG master
        )
        FetchContent_MakeAvailable(rapidjson)
        set_library_folder(rapidjson)
    endif()
endmacro()

# ---------------------------------------------------------
# Freetype
# ---------------------------------------------------------
macro(import_freetype)
    if(NOT TARGET freetype)  # Guard to prevent multiple inclusion
        include(FetchContent)

        FetchContent_Declare(
            freetype
            GIT_REPOSITORY https://gitlab.freedesktop.org/freetype/freetype.git
            GIT_TAG VER-2-13-2  # Pick a stable release
        )
        FetchContent_MakeAvailable(freetype)
        set_library_folder(freetype)

        # Ensure FreeType headers come before system headers
        prioritize_includes(freetype)
    endif()
endmacro()

# ---------------------------------------------------------
# stb_image
# ---------------------------------------------------------
macro(import_stb_image)
    if(NOT TARGET stb_image)  # Guard to prevent multiple inclusion
        include(FetchContent)

        FetchContent_Declare(
            stb_image
            GIT_REPOSITORY https://github.com/nothings/stb.git
            GIT_TAG master  # Or specify a commit/tag, e.g., e.g., 2023-09-07
        )
        FetchContent_MakeAvailable(stb_image)

        # Since stb_image is header-only, create an INTERFACE library
        add_library(stb_image INTERFACE)
        target_include_directories(stb_image INTERFACE
            ${stb_image_SOURCE_DIR}
        )
        set_library_folder(stb_image)
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

    message(STATUS "Importing RapidJSON...")
    import_rapidjson()
    message(STATUS "RapidJSON imported successfully.")

    message(STATUS "Importing Freetype...")
    import_freetype()
    message(STATUS "Freetype imported successfully.")

    message(STATUS "Importing stb_image...")
    import_stb_image()
    message(STATUS "stb_image imported successfully.")

    message(STATUS "All dependencies have been imported successfully.")
    
endmacro()