set(ENGINE_NAME Engine)

file(GLOB_RECURSE ENGINE_SRC_FILES
    ${CMAKE_CURRENT_LIST_DIR}/*.[ch]pp
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

# Create the executable (or library)
add_library(${ENGINE_NAME} ${ENGINE_SRC_FILES})

set_property(TARGET ${ENGINE_NAME}  PROPERTY CXX_STANDARD 20) 
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${ENGINE_NAME} PRIVATE -Wall ${DisableWarnings})
elseif (MSVC)
    target_compile_options(${ENGINE_NAME} PRIVATE /W4 /WX-)
endif()

target_include_directories(${ENGINE_NAME} SYSTEM PUBLIC ${PROJECT_SOURCE_DIR})
target_include_directories(${ENGINE_NAME} PUBLIC
    ${FMOD_INCLUDE_DIRS}
    ${rapidjson_INCLUDE_DIRS}
)
target_compile_definitions(${ENGINE_NAME} 
    PRIVATE ASSETS_DIR_PATH="${CMAKE_SOURCE_DIR}/assets/"
    -DGLEW_STATIC
    -D_CRT_NONSTDC_NO_DEPRECATE
)

target_link_libraries(${ENGINE_NAME}
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

_fmod_copy_dll(${ENGINE_NAME})
_dotnet_copy_dll(${ENGINE_NAME})

# Call the function to copy assets
set(ASSETS_SOURCE_DIR "${CMAKE_SOURCE_DIR}/assets")
set(ASSETS_DEST_DIR "${CMAKE_BINARY_DIR}/assets")
COPY_FOLDER_IF_DIFFERENT(${ENGINE_NAME} ${ASSETS_SOURCE_DIR} ${ASSETS_DEST_DIR})