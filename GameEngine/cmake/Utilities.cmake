# Define a macro to group source and header files by directory
macro(GROUP_FILES_BY_FOLDER)
    foreach(FILE ${ARGV}) 
        # Make path relative to project root
        if(DEFINED APP_NAME)
            file(RELATIVE_PATH REL_FILE "${CMAKE_CURRENT_LIST_DIR}/${APP_NAME}" "${FILE}")
        else()
            file(RELATIVE_PATH REL_FILE "${CMAKE_CURRENT_LIST_DIR}" "${FILE}")
        endif()
        
        # Get the directory of the file
        get_filename_component(PARENT_DIR "${REL_FILE}" DIRECTORY)

        # Replace '/' with '\' for Visual Studio group formatting
        string(REPLACE "/" "\\" GROUP "${PARENT_DIR}")
        
        # Add the file to the corresponding source group
        source_group("${GROUP}" FILES "${FILE}")
    endforeach()
endmacro()

# Define a macro to group source and header files by directory
macro(INCLUDE_DIR_BY_FOLDER ALL_FILES APP_NAME)
    set(include_dirs "")
    foreach(FILE ${ALL_FILES}) 
        get_filename_component(DIR "${FILE}" DIRECTORY)
        list(APPEND include_dirs "${DIR}")
    endforeach()

    # Deduplicate the list
    list(REMOVE_DUPLICATES include_dirs)

    # Add all discovered include directories to the target
    target_include_directories(${APP_NAME} PRIVATE ${include_dirs})
endmacro()

# Define function to copy assets with conditional copying
function(COPY_FOLDER_IF_DIFFERENT PROJECT_NAME source_dir dest_dir)
    file(GLOB_RECURSE ASSET_FILES "${source_dir}/*")

    foreach(asset ${ASSET_FILES})
        get_filename_component(relative_path ${asset} DIRECTORY)
        string(REPLACE "${source_dir}" "" relative_path "${relative_path}")
        set(dest_file "${dest_dir}/${relative_path}")
        
        add_custom_command(
            TARGET ${PROJECT_NAME}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory ${dest_file}
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${asset} ${dest_file}
        )
    endforeach()
endfunction()

# ---------------------------------------------------------
# FMOD (extern)
# ---------------------------------------------------------
macro(set_fmod)
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

# =========================================================
# .NET SDK
# =========================================================
function(set_dotnet)
    message(STATUS "Checking for .NET SDK installation...")

    # Check for system installation
    execute_process(
        COMMAND dotnet --version
        RESULT_VARIABLE DOTNET_RESULT
        OUTPUT_VARIABLE DOTNET_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )

    if (DOTNET_RESULT EQUAL 0)
        message(STATUS ".NET SDK found: version ${DOTNET_VERSION}")
        set(DOTNET_CMD "dotnet" PARENT_SCOPE)
        return()
    endif()


    # Check for local installation
    set(DOTNET_LOCAL_DIR "${CMAKE_BINARY_DIR}/lib/dotnet")
    set(DOTNET_LOCAL_BIN "${DOTNET_LOCAL_DIR}/dotnet.exe")
    if (EXISTS "${DOTNET_LOCAL_BIN}")
        message(STATUS ".NET SDK found at ${DOTNET_LOCAL_BIN}")
        set(DOTNET_CMD "${DOTNET_LOCAL_BIN}" PARENT_SCOPE)
        return()
    endif()

    message(STATUS ".NET SDK not found globally or locally — downloading to ${DOTNET_LOCAL_DIR}")
    # Create destination directory
    file(MAKE_DIRECTORY "${DOTNET_LOCAL_DIR}")

    set(DOTNET_INSTALLER "${DOTNET_LOCAL_DIR}/dotnet-install.ps1")
    file(DOWNLOAD
        "https://builds.dotnet.microsoft.com/dotnet/scripts/v1/dotnet-install.ps1"
        "${DOTNET_INSTALLER}"
        SHOW_PROGRESS
    )
    execute_process(
        COMMAND powershell -ExecutionPolicy Bypass
                -File "${DOTNET_INSTALLER}"
                -Version latest
                -InstallDir "${DOTNET_LOCAL_DIR}"
        RESULT_VARIABLE install_result
    )

    if (NOT install_result EQUAL 0)
        message(FATAL_ERROR "Failed to install .NET SDK. Check your internet connection or PowerShell setup.")
    endif()

    message(STATUS ".NET SDK successfully installed at ${DOTNET_LOCAL_BIN}")
    set(DOTNET_CMD "${DOTNET_LOCAL_BIN}" PARENT_SCOPE)
endfunction()

function(build_csharp_scripts TARGET_NAME CS_PROJECT_PATH OUTPUT_DIR)
    file(MAKE_DIRECTORY ${OUTPUT_DIR})

    add_custom_command(
        OUTPUT ${OUTPUT_DIR}/${TARGET_NAME}.dll
        COMMAND ${DOTNET_CMD} build ${CS_PROJECT_PATH}
                --configuration Release
                --output ${OUTPUT_DIR}
        COMMENT "Building ${TARGET_NAME} C# project..."
    )

    add_custom_target(${TARGET_NAME}_cs ALL DEPENDS ${OUTPUT_DIR}/${TARGET_NAME}.dll)
endfunction()


# Helper to group libraries in VS Solution Explorer
function(set_library_folder target)
    if (TARGET ${target})
        set_target_properties(${target} PROPERTIES FOLDER "Libraries")
    endif()
endfunction()

# Ensures include directories of a target are placed BEFORE system headers
function(prioritize_includes target_name)
    get_target_property(_incs ${target_name} INTERFACE_INCLUDE_DIRECTORIES)
    if(_incs)
        target_include_directories(${target_name} BEFORE INTERFACE ${_incs})
    endif()
endfunction()