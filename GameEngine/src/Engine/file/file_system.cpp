#include "file_system.hpp"

std::filesystem::path FileSystem::exeDir_;
std::filesystem::path FileSystem::assetsDir_;
std::filesystem::path FileSystem::scriptsDir_;

void FileSystem::Initialize(const char* argv0)
{
    exeDir_ = std::filesystem::canonical(argv0).parent_path();

    assetsDir_ = exeDir_ / "assets";
    scriptsDir_ = exeDir_ / "scripts/GameScripts/user_scripts";
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