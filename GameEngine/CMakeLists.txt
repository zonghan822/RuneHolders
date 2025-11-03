# Projects Settings

cmake_minimum_required(VERSION 3.4...3.28 FATAL_ERROR)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

project("RuneHolders" VERSION 0.1 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set the directory where your custom modules are located
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

include(Utilities)
include(ImportDependencies)
include(BuildTests)

importDependencies()

# Set External Libraries
set_fmod()
set(rapidjson_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/lib/rapidjson)
set_dotnet()

# Build c# project
set(SCRIPT_SOURCE ${CMAKE_SOURCE_DIR}/scripts/GameScripts/GameScripts.csproj)
set(SCRIPT_BINARY_DIR ${CMAKE_BINARY_DIR}/scripts/GameScripts)
build_csharp_scripts(
    GameScripts
    ${SCRIPT_SOURCE}
    ${SCRIPT_BINARY_DIR}
)

ADD_SUBDIRECTORY(src/Engine ${CMAKE_BINARY_DIR}/projects/Engine)
ADD_SUBDIRECTORY(src/App ${CMAKE_BINARY_DIR}/projects/App)