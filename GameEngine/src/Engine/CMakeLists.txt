set(ENGINE_NAME Engine)

file(GLOB_RECURSE ENGINE_SRC_FILES
    ${CMAKE_CURRENT_LIST_DIR}/*.[cht]pp
    ${CMAKE_CURRENT_LIST_DIR}/*.[ch]
)

file(GLOB_RECURSE ENGINE_TEMP_FILES
    ${CMAKE_CURRENT_LIST_DIR}/*.tpp
)

set(ALL_FILES
    ${ENGINE_SRC_FILES} 
    ${ENGINE_TEMP_FILES}
)

# Call the macro to group files by folder
GROUP_FILES_BY_FOLDER(${ALL_FILES})

# Call the macro to include directories by folder
# INCLUDE_DIR_BY_FOLDER(${ALL_FILES} ${ENGINE_NAME})

# ----------------------------------------------------------
# Build both STATIC and SHARED versions of the Engine
# ----------------------------------------------------------

# --- Static Library (for Editor / C++ Apps) ---
add_library(${ENGINE_NAME}_static STATIC ${ALL_FILES})
set_target_properties(${ENGINE_NAME}_static PROPERTIES OUTPUT_NAME ${ENGINE_NAME})

# --- Shared Library (for .NET Interop / PInvoke) ---
add_library(${ENGINE_NAME}_shared SHARED ${ALL_FILES})
set_target_properties(${ENGINE_NAME}_shared PROPERTIES OUTPUT_NAME ${ENGINE_NAME})


# ----------------------------------------------------------
# Compiler warning setup
# ----------------------------------------------------------
foreach(target ${ENGINE_NAME}_static ${ENGINE_NAME}_shared)
    set_property(TARGET ${target} PROPERTY CXX_STANDARD 20)

    if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(${target} PRIVATE -Wall ${DisableWarnings})
    elseif (MSVC)
        target_compile_options(${target} PRIVATE /W4 /WX-)
    endif()
endforeach()


# ----------------------------------------------------------
# Include directories
# ----------------------------------------------------------
foreach(target ${ENGINE_NAME}_static ${ENGINE_NAME}_shared)
    target_include_directories(${target} SYSTEM PUBLIC ${PROJECT_SOURCE_DIR})
    target_include_directories(${target} PUBLIC
        ${FMOD_INCLUDE_DIRS}
        ${rapidjson_INCLUDE_DIRS}
    )
    target_compile_definitions(${target} PRIVATE 
        ASSETS_DIR_PATH="${CMAKE_SOURCE_DIR}/assets/"
        SCRIPT_SRC_PATH="${SCRIPT_SOURCE_DIR}/user_scripts/"
        SCRIPT_BIN_PATH="${SCRIPT_BINARY_DIR}/"
        WSCRIPT_SRC_PATH=L"${SCRIPT_SOURCE_DIR}/user_scripts/"
        WSCRIPT_BIN_PATH=L"${SCRIPT_BINARY_DIR}/"
        DOTNET_ROOT="${DOTNET_RUNTIME_PATH}/"
        -DGLEW_STATIC
        -D_CRT_NONSTDC_NO_DEPRECATE
    )
endforeach()

# ----------------------------------------------------------
# Export definitions for shared library
# ----------------------------------------------------------
target_compile_definitions(${ENGINE_NAME}_shared PRIVATE ENGINE_BUILD_DLL)

# ----------------------------------------------------------
# Link libraries
# ----------------------------------------------------------
foreach(target ${ENGINE_NAME}_static ${ENGINE_NAME}_shared)
    target_link_libraries(${target}
        glm
        glfw
        libglew_static
        imgui
        fmod::core 
        fmod::studio
        freetype
        stb_image
        dotnet_host
    )
endforeach()

_fmod_copy_dll(${ENGINE_NAME}_shared)
_dotnet_copy_dll(${ENGINE_NAME}_shared)

# ================================================================
# Copy Engine.dll to GameScripts output for P/Invoke
# ================================================================
add_custom_command(
    TARGET ${ENGINE_NAME}_shared POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:${ENGINE_NAME}_shared>
        ${SCRIPT_BINARY_DIR}
)

# Call the function to copy assets
set(ASSETS_SOURCE_DIR "${CMAKE_SOURCE_DIR}/assets")
set(ASSETS_DEST_DIR "${CMAKE_BINARY_DIR}/assets")
COPY_FOLDER_IF_DIFFERENT(${ENGINE_NAME}_shared ${ASSETS_SOURCE_DIR} ${ASSETS_DEST_DIR})