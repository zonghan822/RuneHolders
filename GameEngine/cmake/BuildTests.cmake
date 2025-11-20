MACRO(SETUP_APP APP_NAME ENGINE_NAME)

set(APP_NAME ${APP_NAME})

# Project name
project(${APP_NAME} VERSION 1.0.0 LANGUAGES CXX)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

file(GLOB_RECURSE ${APP_NAME}_source_files
    ${CMAKE_CURRENT_LIST_DIR}/${APP_NAME}/*.[ch]pp
    ${CMAKE_CURRENT_LIST_DIR}/${APP_NAME}/*.[ch]
)

file(GLOB_RECURSE ${APP_NAME}_template_files
    ${CMAKE_CURRENT_LIST_DIR}/${APP_NAME}/*.tpp
)

set(${APP_NAME}_all_files
    ${${APP_NAME}_source_files} 
    ${${APP_NAME}_template_files}
)

# Group files
GROUP_FILES_BY_FOLDER(${${APP_NAME}_all_files})

# Add the executable
add_executable(${APP_NAME} 
    ${${APP_NAME}_source_files} 
)

set_property(TARGET ${APP_NAME}  PROPERTY CXX_STANDARD 20) 
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${APP_NAME} PRIVATE -Wall ${DisableWarnings})
elseif (MSVC)
    target_compile_options(${APP_NAME} PRIVATE /W4 /WX-)
endif()

# Set common include directories
target_include_directories(${APP_NAME} PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories(${APP_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src/${ENGINE_NAME})
target_compile_definitions(${APP_NAME} PRIVATE
    ASSETS_DIR_PATH="${CMAKE_SOURCE_DIR}/assets/"
    CONFIG_DIR_PATH="${CMAKE_SOURCE_DIR}/src/App/${APP_NAME}/"
    SCRIPT_SRC_PATH="${SCRIPT_SOURCE_DIR}/user_scripts/"
    SCRIPT_BIN_PATH="${SCRIPT_BINARY_DIR}"
    DOTNET_ROOT="${DOTNET_RUNTIME_PATH}"
    ASSET_SCENE_PATH="${CMAKE_SOURCE_DIR}/assets/saves/scenes/"
)

# Link the library to the executable using the CMake target
target_link_libraries(${APP_NAME} PUBLIC
    ${ENGINE_NAME}
)
_fmod_copy_dll(${APP_NAME})
_dotnet_copy_dll(${APP_NAME})

ENDMACRO(SETUP_APP)