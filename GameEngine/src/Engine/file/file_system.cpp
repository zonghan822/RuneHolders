/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         file_system.cpp
* @author       ZongHan Law (zonghan.law@digipen.edu)   46%
* @co-author    Muhammad Raziq Bin Mohd Rafig (muhammadraziq.b@digipen.edu)   23%
* @co-author    Caven Koh (caven.koh@digipen.edu)   15%
* @co-author    Koe Yun Ji (yunji.koe@digipen.edu)   15%
* @brief        See file name.
*//*========================================================================================*/

#include "file_system.hpp"
#include <windows.h>
#include <shlobj.h>

std::filesystem::path FileSystem::exeDir_;
std::filesystem::path FileSystem::rootDir_;
std::filesystem::path FileSystem::assetsDir_;
std::filesystem::path FileSystem::scriptsDir_;
std::filesystem::path FileSystem::writableDir_;

void FileSystem::Initialize(const std::string& appName)
{
    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
    exeDir_ = std::filesystem::path(buffer).parent_path();

    // Navigate up from build/projects/App/<AppName>/Debug to GameEngine root (5 levels up)
    rootDir_ = exeDir_.parent_path().parent_path().parent_path()
                       .parent_path().parent_path();

    std::filesystem::path sourceAssets = rootDir_ / "assets" / appName;
    if (std::filesystem::exists(sourceAssets))
    {
        // Development: use per-app source dirs
        assetsDir_   = sourceAssets;
        scriptsDir_  = rootDir_ / "scripts" / appName / "user_scripts";
        writableDir_ = assetsDir_;
    }
    else
    {
        // Shipped: assets/ and scripts/ sit next to the exe
        assetsDir_  = exeDir_ / "assets";
        scriptsDir_ = exeDir_ / "scripts" / "user_scripts";

        // Use %APPDATA%/<appName> for writable files (config, logs, temp DLLs)
        char appDataBuf[MAX_PATH];
        if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, appDataBuf)))
        {
            writableDir_ = std::filesystem::path(appDataBuf) / appName;
            std::filesystem::create_directories(writableDir_);
        }
        else
        {
            // Fallback to exe directory
            writableDir_ = exeDir_;
        }
    }
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

const std::filesystem::path& FileSystem::GetWritableDir()
{
    return writableDir_;
}