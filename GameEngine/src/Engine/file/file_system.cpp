#include "file_system.hpp"
#include <windows.h>

std::filesystem::path FileSystem::exeDir_;
std::filesystem::path FileSystem::rootDir_;
std::filesystem::path FileSystem::assetsDir_;
std::filesystem::path FileSystem::scriptsDir_;
std::filesystem::path FileSystem::sourceAssetsDir_;
std::filesystem::path FileSystem::sourceScriptsDir_;

void FileSystem::Initialize()
{
    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
    exeDir_ = std::filesystem::path(buffer).parent_path();

    // Navigate up from build/projects/App/Debug to GameEngine root (4 levels up)
    rootDir_ = exeDir_.parent_path().parent_path().parent_path().parent_path();

    // Use the source directory directly when it exists (development).
    // Fall back to the exe directory when shipped (source tree not present).
    std::filesystem::path sourceAssets = rootDir_ / "assets";
    if (std::filesystem::exists(sourceAssets))
    {
        assetsDir_  = sourceAssets;
        scriptsDir_ = rootDir_ / "scripts\\GameScripts\\user_scripts";
    }
    else
    {
        assetsDir_  = exeDir_ / "assets";
        scriptsDir_ = exeDir_ / "scripts\\GameScripts\\user_scripts";
    }

    // Keep source aliases pointing to the same location (no longer separate paths)
    sourceAssetsDir_  = assetsDir_;
    sourceScriptsDir_ = scriptsDir_;
}

const std::filesystem::path& FileSystem::GetRootDir()
{
    return rootDir_;
}

const std::filesystem::path& FileSystem::GetExecutableDir()
{
    return exeDir_;
}

const std::filesystem::path& FileSystem::GetAssetsDir()
{
    return assetsDir_;
}

const std::filesystem::path& FileSystem::GetScriptsDir()
{
    return scriptsDir_;
}

const std::filesystem::path& FileSystem::GetSourceAssetsDir()
{
    return sourceAssetsDir_;
}

const std::filesystem::path& FileSystem::GetSourceScriptsDir()
{
    return sourceScriptsDir_;
}