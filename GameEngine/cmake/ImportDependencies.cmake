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
        set_library_folder(imgui)
    endif()
endmacro()

# ---------------------------------------------------------
# nlohmann_json
# ---------------------------------------------------------
macro(import_nlohmann_json)
    if(NOT TARGET nlohmann_json)  # Guard to prevent multiple inclusion
        include(FetchContent)

        FetchContent_Declare(
            nlohmann_json
            GIT_REPOSITORY https://github.com/nlohmann/json.git
            GIT_TAG v3.11.3  # Stable release, change as needed
        )
        FetchContent_MakeAvailable(nlohmann_json)
        set_library_folder(nlohmann_json)
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

# =========================================================
# .NET runtime
# =========================================================
function(install_dotnet_runtime)
    set(DOTNET_DIR ${FETCHCONTENT_BASE_DIR}/dotnet)

    if (EXISTS "${DOTNET_DIR}")
        message(STATUS ".NET runtime already present at ${DOTNET_DIR}")
        return()
    endif()

    # Create destination directory
    file(MAKE_DIRECTORY "${DOTNET_DIR}")

    if (WIN32)
        set(DOTNET_INSTALLER "${DOTNET_DIR}/dotnet-install.ps1")
        file(DOWNLOAD
            "https://builds.dotnet.microsoft.com/dotnet/scripts/v1/dotnet-install.ps1"
            "${DOTNET_INSTALLER}"
            SHOW_PROGRESS
        )
        execute_process(
            COMMAND powershell -NoLogo -NoProfile -ExecutionPolicy Bypass
                    -File "${DOTNET_INSTALLER}"
                    -Runtime dotnet
                    -Version latest
                    -InstallDir "${DOTNET_DIR}"
            RESULT_VARIABLE install_result
        )
    else()
        set(DOTNET_INSTALLER "${DOTNET_DIR}/dotnet-install.sh")
        file(DOWNLOAD
            "https://builds.dotnet.microsoft.com/dotnet/scripts/v1/dotnet-install.sh"
            "${DOTNET_INSTALLER}"
            SHOW_PROGRESS
        )
        execute_process(
            COMMAND bash "${DOTNET_INSTALLER}" --runtime dotnet --version latest --install-dir "${DOTNET_DIR}"
            RESULT_VARIABLE install_result
        )
    endif()

    if (NOT install_result EQUAL 0)
        message(FATAL_ERROR "Failed to install .NET runtime into ${DOTNET_DIR}")
    endif()
endfunction()


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

    # Comment for now (Filename too long issue...)
    # message(STATUS "Importing nlohmann/json...")
    # import_nlohmann_json()
    # message(STATUS "nlohmann/json imported successfully.")

    message(STATUS "Importing Freetype...")
    import_freetype()
    message(STATUS "Freetype imported successfully.")

    message(STATUS "Importing stb_image...")
    import_stb_image()
    message(STATUS "stb_image imported successfully.")

    message(STATUS "Installing .NET runtime...")
    install_dotnet_runtime()
    message(STATUS ".NET runtime installed successfully.")

    message(STATUS "All dependencies have been imported successfully.")
    
endmacro()