#include "file_system.hpp"
#include <windows.h>

std::filesystem::path FileSystem::exeDir_;
std::filesystem::path FileSystem::rootDir_;
std::filesystem::path FileSystem::assetsDir_;
std::filesystem::path FileSystem::scriptsDir_;

void FileSystem::Initialize()
{
    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
    exeDir_ = std::filesystem::path(buffer).parent_path();

    // Navigate up from build/projects/App/Debug to GameEngine root (4 levels up)
    rootDir_ = exeDir_.parent_path().parent_path().parent_path().parent_path();
    //rootDir_ = std::filesystem::path(buffer).root_path();

    assetsDir_ = exeDir_ / "assets";
    scriptsDir_ = exeDir_ / "scripts/GameScripts/user_scripts";
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