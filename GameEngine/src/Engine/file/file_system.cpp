#include "file_system.hpp"
#include <windows.h>

std::filesystem::path FileSystem::exeDir_;
std::filesystem::path FileSystem::rootDir_;
std::filesystem::path FileSystem::assetsDir_;
std::filesystem::path FileSystem::scriptsDir_;
std::filesystem::path FileSystem::sourceAssetsDir_;

void FileSystem::Initialize()
{
    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
    exeDir_ = std::filesystem::path(buffer).parent_path();

    // Navigate up from build/projects/App/Debug to GameEngine root (4 levels up)
    rootDir_ = exeDir_.parent_path().parent_path().parent_path().parent_path();
    //rootDir_ = std::filesystem::path(buffer).root_path();

    assetsDir_ = exeDir_ / "assets";
    scriptsDir_ = exeDir_ / "scripts\\GameScripts\\user_scripts";
    sourceAssetsDir_ = rootDir_ / "assets";
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

void FileSystem::SyncToSource(const std::string& buildFilePath)
{
    try
    {
        std::filesystem::path buildPath{ buildFilePath };
        std::filesystem::path relative = std::filesystem::relative(buildPath, assetsDir_);
        std::filesystem::path destPath = sourceAssetsDir_ / relative;

        std::filesystem::create_directories(destPath.parent_path());
        std::filesystem::copy_file(buildPath, destPath, std::filesystem::copy_options::overwrite_existing);
    }
    catch (const std::filesystem::filesystem_error& e)
    {
        std::cerr << "[FileSystem] SyncToSource failed for '" << buildFilePath << "': " << e.what() << std::endl;
    }
}

void FileSystem::RemoveFromSource(const std::string& buildFilePath)
{
    try
    {
        std::filesystem::path buildPath{ buildFilePath };
        std::filesystem::path relative = std::filesystem::relative(buildPath, assetsDir_);
        std::filesystem::path destPath = sourceAssetsDir_ / relative;

        if (std::filesystem::exists(destPath))
        {
            std::filesystem::remove(destPath);
        }
    }
    catch (const std::filesystem::filesystem_error& e)
    {
        std::cerr << "[FileSystem] RemoveFromSource failed for '" << buildFilePath << "': " << e.what() << std::endl;
    }
}