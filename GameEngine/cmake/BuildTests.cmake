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
    ${CMAKE_CURRENT_LIST_DIR}/${APP_NAME}/*.rc
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

if (MSVC)
    set_target_properties(${APP_NAME} PROPERTIES
        LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE"
        LINK_FLAGS_RELWITHDEBINFO "/SUBSYSTEM:WINDOWS"
        LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS"
        LINK_FLAGS_MINSIZEREL "/SUBSYSTEM:WINDOWS"
    )
endif()

add_dependencies(${APP_NAME} ${SCRIPT_NAME}_cs)

# Set common include directories
target_include_directories(${APP_NAME} PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories(${APP_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src/Engine)

# Link the library to the executable using the CMake target
target_link_libraries(${APP_NAME} PUBLIC
    ${ENGINE_NAME}_static
    ${ENGINE_NAME}_shared
)
_fmod_copy_dll(${APP_NAME})
_dotnet_copy_dll(${APP_NAME})
_engine_copy_dll(${APP_NAME})

ENDMACRO(SETUP_APP)