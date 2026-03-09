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
function(COPY_FOLDER_IF_DIFFERENT target source_dir)
    # Remove first two arguments (target + source)
    set(options)
    set(oneValueArgs)
    set(multiValueArgs DESTINATIONS)
    cmake_parse_arguments(CF " " " " "DESTINATIONS" ${ARGN})

    if(NOT EXISTS "${source_dir}")
        message(FATAL_ERROR "[COPY_FOLDER_IF_DIFFERENT] Source not found: ${source_dir}")
    endif()

    foreach(dest ${CF_DESTINATIONS})
        # Ensure directory expansion works with generator expressions
        set(expanded_dest "${dest}")

        add_custom_command(
            TARGET ${target} PRE_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory "${expanded_dest}"
        )

        add_custom_command(
            TARGET ${target} PRE_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${source_dir}"
                "${expanded_dest}"
            COMMENT "Copying ${source_dir} -> ${expanded_dest}"
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

# ---------------------------------------------------------
# FFMPEG (extern)
# ---------------------------------------------------------
macro(set_ffmpeg)
    if(NOT TARGET ffmpeg)
        set(FFMPEG_DIR "${PROJECT_SOURCE_DIR}/lib/ffmpeg")

        add_library(ffmpeg INTERFACE IMPORTED)
        target_include_directories(ffmpeg INTERFACE ${FFMPEG_DIR}/include)
        target_link_libraries(ffmpeg INTERFACE
            ${FFMPEG_DIR}/lib/avcodec.lib
            ${FFMPEG_DIR}/lib/avformat.lib
            ${FFMPEG_DIR}/lib/avutil.lib
            ${FFMPEG_DIR}/lib/swscale.lib
            ${FFMPEG_DIR}/lib/swresample.lib
        )

        function(_ffmpeg_copy_dll TARGET)
            file(GLOB FFMPEG_DLLS "${FFMPEG_DIR}/bin/*.dll")
            foreach(DLL ${FFMPEG_DLLS})
                add_custom_command(TARGET ${TARGET} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        "${DLL}"
                        "$<TARGET_FILE_DIR:${TARGET}>"
                )
            endforeach()
        endfunction()

        set_library_folder(ffmpeg)
    endif()
endmacro()

# =========================================================
# .NET SDK setup helper function
# =========================================================
function(set_dotnet_sdk)
    set(DOTNET_LOCAL_DIR "${CMAKE_BINARY_DIR}/lib/dotnet")

    # Check for local installation
    if (EXISTS "${DOTNET_LOCAL_DIR}/sdk")
        message(STATUS ".NET SDK found at ${DOTNET_LOCAL_DIR}/sdk")
        set(DOTNET_SDK_PATH "${DOTNET_LOCAL_DIR}" PARENT_SCOPE)
        set(DOTNET_CMD "${DOTNET_LOCAL_DIR}/dotnet.exe" PARENT_SCOPE)
        return()
    endif()

    # Create destination directory
    file(MAKE_DIRECTORY "${DOTNET_LOCAL_DIR}")
    set(DOTNET_INSTALLER "${DOTNET_LOCAL_DIR}/dotnet-install.ps1")
    file(DOWNLOAD
        "https://builds.dotnet.microsoft.com/dotnet/scripts/v1/dotnet-install.ps1"
        "${DOTNET_INSTALLER}"
        SHOW_PROGRESS
    )

    message(STATUS "Checking for .NET SDK installation...")
    execute_process(
        COMMAND dotnet --version
        RESULT_VARIABLE DOTNET_RESULT
        OUTPUT_VARIABLE DOTNET_SDK_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )

    if (DOTNET_RESULT EQUAL 0)
        string(REGEX MATCH "^([0-9]+)\\.([0-9]+)" _ "${DOTNET_SDK_VERSION}")
        set(DOTNET_MAJOR "${CMAKE_MATCH_1}")
        set(DOTNET_MINOR "${CMAKE_MATCH_2}")
        message(STATUS "Detected .NET SDK version: ${DOTNET_MAJOR}.${DOTNET_MINOR}")

        if (DOTNET_MAJOR GREATER_EQUAL 8)
            execute_process(
                COMMAND dotnet --info
                OUTPUT_VARIABLE DOTNET_INFO
                RESULT_VARIABLE DOTNET_RESULT
                OUTPUT_STRIP_TRAILING_WHITESPACE
            )
            string(REGEX MATCH "Base Path:[ \t]*([^\r\n]+)" _match "${DOTNET_INFO}")
            set(BASE_PATH_LINE "${CMAKE_MATCH_1}")

            # Strip the trailing "\sdk\9.0.306\" part to get "C:\Program Files\dotnet\"
            string(REGEX REPLACE "[/\\]sdk[/\\][^/\\]+[/\\]?$" "" DOTNET_ROOT "${BASE_PATH_LINE}")
            message(STATUS "Using .NET SDK at: ${DOTNET_ROOT}")

            set(DOTNET_CMD "dotnet" PARENT_SCOPE)
            set(DOTNET_INSTALLER "${DOTNET_INSTALLER}" PARENT_SCOPE)
            set(DOTNET_SDK_PATH "${DOTNET_ROOT}" PARENT_SCOPE)
            return()
        endif()
        message(STATUS ".NET SDK version less than 8.0")
    endif()

    message(STATUS "Downloading .NET SDK to ${DOTNET_LOCAL_DIR}")
    execute_process(
        COMMAND powershell -ExecutionPolicy Bypass
                -File "${DOTNET_INSTALLER}"
                -Version 8.0.415
                -InstallDir "${DOTNET_LOCAL_DIR}"
        RESULT_VARIABLE install_result
    )

    if (NOT install_result EQUAL 0)
        message(FATAL_ERROR "Failed to install .NET SDK. Check your internet connection or PowerShell setup.")
    endif()
    message(STATUS ".NET SDK successfully installed at ${DOTNET_LOCAL_DIR}")
    set(DOTNET_INSTALLER "${DOTNET_INSTALLER}" PARENT_SCOPE)
    set(DOTNET_SDK_PATH "${DOTNET_LOCAL_DIR}" PARENT_SCOPE)
    set(DOTNET_CMD "${DOTNET_LOCAL_DIR}/dotnet.exe" PARENT_SCOPE)
endfunction()

# =========================================================
# .NET setup
# =========================================================
macro(set_dotnet)
    set_dotnet_sdk()
    set(DOTNET_RUNTIME_PATH "${CMAKE_BINARY_DIR}/lib/dotnet")
    # Install runtime if sdk not installed
    if(NOT EXISTS "${DOTNET_RUNTIME_PATH}/shared/Microsoft.NETCore.App")
        message(STATUS "Installing .NET runtime into ${DOTNET_RUNTIME_PATH} ...")

        # Maximum retries if file is locked
        set(MAX_RETRIES 3)
        set(RETRY_DELAY 5)  # seconds between retries
        set(INSTALL_SUCCESS FALSE)

        foreach(RETRY RANGE 1 ${MAX_RETRIES})
            message(STATUS "Attempt ${RETRY} to install .NET runtime...")

            execute_process(
                COMMAND powershell -ExecutionPolicy Bypass -Command "
                    $names = 'dotnet','MSBuild','VBCSCompiler','ServiceHub.*'
                    foreach ($n in $names) {
                        Get-Process -Name $n -ErrorAction SilentlyContinue | Stop-Process -Force -ErrorAction SilentlyContinue
                    }
                "
                OUTPUT_QUIET
                ERROR_QUIET
            )

            # Allow Windows to release DLL handles
            execute_process(COMMAND ${CMAKE_COMMAND} -E sleep 1)

            execute_process(
                COMMAND powershell -ExecutionPolicy Bypass
                    -File "${DOTNET_INSTALLER}"
                    -Runtime dotnet
                    -Version 8.0.21
                    -InstallDir "${DOTNET_RUNTIME_PATH}"
                RESULT_VARIABLE install_result
                OUTPUT_QUIET
                ERROR_VARIABLE install_error
            )

            if (install_result EQUAL 0)
                set(INSTALL_SUCCESS TRUE)
                break()
            else()
                message(WARNING "Attempt ${RETRY} to install .NET runtime failed.")
                message(WARNING "${install_error}")
                if (RETRY LESS ${MAX_RETRIES})
                    file(REMOVE_RECURSE "${DOTNET_RUNTIME_PATH}/shared/Microsoft.NETCore.App")
                    message(STATUS "Waiting ${RETRY_DELAY}s before retry...")
                    execute_process(COMMAND ${CMAKE_COMMAND} -E sleep ${RETRY_DELAY})
                endif()
            endif()
        endforeach()
            
        if (NOT INSTALL_SUCCESS)
            message(FATAL_ERROR "Failed to install .NET runtime into ${DOTNET_RUNTIME_PATH}")
        endif()
        message(STATUS ".NET runtime successfully installed in ${DOTNET_RUNTIME_PATH}")
    endif()

    file(GLOB DOTNET_NATIVE_DIRS
        "${DOTNET_SDK_PATH}/packs/Microsoft.NETCore.App.Host.win-x64/*/runtimes/win-x64/native"
    )
    list(GET DOTNET_NATIVE_DIRS 0 DOTNET_NATIVE_DIR)
    if(NOT DOTNET_NATIVE_DIR)
        message(FATAL_ERROR "Could not find .NET native host folder under ${DOTNET_SDK_PATH}")
    endif()

     # Create INTERFACE target for .NET host
    add_library(dotnet_host INTERFACE)

    target_include_directories(dotnet_host INTERFACE
        ${DOTNET_NATIVE_DIR}
    )

    target_link_libraries(dotnet_host INTERFACE
        ${DOTNET_NATIVE_DIR}/nethost.lib
    )

    set_library_folder(dotnet_host)

    file(GLOB DOTNET_HOST_DIRS
        "${DOTNET_RUNTIME_PATH}/host/fxr/*"
    )
    # get the latest version (last element)
    list(GET DOTNET_HOST_DIRS 0 DOTNET_HOST_DIR)
    if(NOT DOTNET_HOST_DIR)
        message(FATAL_ERROR "Could not find .NET host folder under ${DOTNET_RUNTIME_PATH}")
    endif()
    # Extract version from directory name (e.g. ".../host/fxr/8.0.21" -> "8.0.21")
    get_filename_component(DOTNET_HOST_FXR_VERSION "${DOTNET_HOST_DIR}" NAME)

    # Locate the .NET runtime shared framework directory
    file(GLOB DOTNET_SHARED_FX_DIRS
        "${DOTNET_RUNTIME_PATH}/shared/Microsoft.NETCore.App/*"
    )
    list(GET DOTNET_SHARED_FX_DIRS 0 DOTNET_SHARED_FX_DIR)
    if(NOT DOTNET_SHARED_FX_DIR)
        message(FATAL_ERROR "Could not find .NET shared framework under ${DOTNET_RUNTIME_PATH}/shared/")
    endif()
    # Extract version from directory name (e.g. ".../Microsoft.NETCore.App/8.0.21" -> "8.0.21")
    get_filename_component(DOTNET_SHARED_FX_VERSION "${DOTNET_SHARED_FX_DIR}" NAME)

    function(_dotnet_copy_dll TARGET)
        add_custom_command(TARGET ${TARGET} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${DOTNET_HOST_DIR}/hostfxr.dll"
                "${DOTNET_NATIVE_DIR}/nethost.dll"
                "$<TARGET_FILE_DIR:${TARGET}>"
        )
        # Copy the .NET runtime shared framework next to the executable
        add_custom_command(TARGET ${TARGET} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory
                "$<TARGET_FILE_DIR:${TARGET}>/dotnet_rt/shared/Microsoft.NETCore.App"
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${DOTNET_SHARED_FX_DIR}"
                "$<TARGET_FILE_DIR:${TARGET}>/dotnet_rt/shared/Microsoft.NETCore.App/${DOTNET_SHARED_FX_VERSION}"
            COMMENT "Copying .NET runtime framework to output directory"
        )
        # Copy the host/fxr directory for hostfxr resolution
        add_custom_command(TARGET ${TARGET} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory
                "$<TARGET_FILE_DIR:${TARGET}>/dotnet_rt/host/fxr"
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${DOTNET_HOST_DIR}"
                "$<TARGET_FILE_DIR:${TARGET}>/dotnet_rt/host/fxr/${DOTNET_HOST_FXR_VERSION}"
            COMMENT "Copying hostfxr to output directory"
        )
    endfunction()
endmacro()

function(build_csharp_scripts TARGET_NAME CS_PROJECT_PATH OUTPUT_DIR)
    if(ARGC GREATER 3)
        set(DLL_NAME ${ARGV3})
    else()
        set(DLL_NAME ${TARGET_NAME})
    endif()
    set(CONFIG_OUTPUT_DIR "${OUTPUT_DIR}")

    # Gather all C# source files in the project directory
    file(GLOB_RECURSE CS_FILES "${CS_PROJECT_PATH}/*.cs")

    # Group files in IDE (so they appear in Visual Studio)
    source_group(TREE ${CS_PROJECT_PATH} FILES ${CS_FILES})

    # =========================================================
    # Publish the C# project
    # =========================================================
    add_custom_command(
        OUTPUT ${CONFIG_OUTPUT_DIR}/${DLL_NAME}.dll
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CONFIG_OUTPUT_DIR}
        COMMAND ${DOTNET_CMD} publish ${CS_PROJECT_PATH}
            -f net8.0
            -c $<CONFIG>
            --self-contained false
            --output ${CONFIG_OUTPUT_DIR}
        DEPENDS
            ${CS_FILES}
            ${CS_PROJECT_PATH}/GameScripts.csproj
            ${CS_PROJECT_PATH}/obj/project.assets.json
        COMMENT "Publishing ${TARGET_NAME} for $<CONFIG>..."
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
    # Target depends on correct CONFIG output
    add_custom_target(${TARGET_NAME}_cs
        DEPENDS ${CONFIG_OUTPUT_DIR}/${DLL_NAME}.dll
    )
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

function(_engine_copy_dll TARGET)
    add_custom_command(TARGET ${TARGET} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:Engine_shared>"
            "$<TARGET_FILE_DIR:${TARGET}>"
    )
endfunction()