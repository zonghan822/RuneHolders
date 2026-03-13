set(ENGINE_NAME Engine)

file(GLOB_RECURSE ENGINE_SRC_FILES
    ${CMAKE_CURRENT_LIST_DIR}/*.[cht]pp
    ${CMAKE_CURRENT_LIST_DIR}/*.[ch]
)

# Identify the bridging files
file(GLOB BRIDGE_FILES
    "${CMAKE_CURRENT_LIST_DIR}/bridge/*"
)

set(ENGINE_SRC_NO_BRIDGE ${ENGINE_SRC_FILES})
list(REMOVE_ITEM ENGINE_SRC_NO_BRIDGE ${BRIDGE_FILES})

# Call the macro to group files by folder
GROUP_FILES_BY_FOLDER(${ENGINE_SRC_FILES})

# Call the macro to include directories by folder
# INCLUDE_DIR_BY_FOLDER(${ALL_FILES} ${ENGINE_NAME})

# ----------------------------------------------------------
# Build both STATIC and SHARED versions of the Engine
# ----------------------------------------------------------

# --- Static Library (for Editor / C++ Apps) ---
add_library(${ENGINE_NAME}_static STATIC ${ENGINE_SRC_NO_BRIDGE})
set_target_properties(${ENGINE_NAME}_static PROPERTIES OUTPUT_NAME ${ENGINE_NAME}_static)

# --- Shared Library (for .NET Interop / PInvoke) ---
add_library(${ENGINE_NAME}_shared SHARED ${BRIDGE_FILES})
set_target_properties(${ENGINE_NAME}_shared PROPERTIES OUTPUT_NAME ${ENGINE_NAME}_shared)


# ----------------------------------------------------------
# Compiler warning setup
# ----------------------------------------------------------
foreach(target ${ENGINE_NAME}_static ${ENGINE_NAME}_shared)
    set_property(TARGET ${target} PROPERTY CXX_STANDARD 20)

    if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(${target} PRIVATE -Wall ${DisableWarnings})
    elseif (MSVC)
        target_compile_options(${target} PRIVATE /W4 /WX- /external:anglebrackets /external:W0)
    endif()
endforeach()


# ----------------------------------------------------------
# Include directories
# ----------------------------------------------------------
target_include_directories(${ENGINE_NAME}_static SYSTEM PUBLIC ${PROJECT_SOURCE_DIR})
target_include_directories(${ENGINE_NAME}_static PUBLIC
    ${FMOD_INCLUDE_DIRS}
    ${rapidjson_INCLUDE_DIRS}
)

# ----------------------------------------------------------
# Link libraries
# ----------------------------------------------------------
target_link_libraries(${ENGINE_NAME}_static
    glm
    glfw
    libglew_static
    imgui
    fmod::core 
    fmod::studio
    freetype
    stb_image
    dotnet_host
    ffmpeg
)
_fmod_copy_dll(${ENGINE_NAME}_static)
_dotnet_copy_dll(${ENGINE_NAME}_static)
_ffmpeg_copy_dll(${ENGINE_NAME}_static)
target_link_libraries(${ENGINE_NAME}_shared PRIVATE 
    ${ENGINE_NAME}_static
)

# ----------------------------------------------------------
# Export definitions
# ----------------------------------------------------------
foreach(target ${ENGINE_NAME}_static ${ENGINE_NAME}_shared)
    target_compile_definitions(${target} PRIVATE
        DOTNET_ROOT="${DOTNET_RUNTIME_PATH}/"
        -DGLEW_STATIC
        -D_CRT_NONSTDC_NO_DEPRECATE
    )
endforeach()


target_compile_definitions(${ENGINE_NAME}_shared PRIVATE 
    ENGINE_BUILD_DLL
    ENGINE_USE_DLL
)

