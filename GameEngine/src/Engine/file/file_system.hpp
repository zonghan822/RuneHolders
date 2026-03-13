/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         file_system.hpp
* @author       ZongHan Law (zonghan.law@digipen.edu)   50%
* @co-author    Muhammad Raziq Bin Mohd Rafig (muhammadraziq.b@digipen.edu)   20%
* @co-author    Caven Koh (caven.koh@digipen.edu)   20%
* @co-author    Koe Yun Ji (yunji.koe@digipen.edu)   10%
* @brief        See file name.
*//*========================================================================================*/

#pragma once
#include <filesystem>
#include <string>
#include <iostream>

class FileSystem
{
public:
    static void Initialize(const std::string& appName);

    static const std::filesystem::path& GetExecutableDir();
    static const std::filesystem::path& GetRootDir();
    static const std::filesystem::path& GetAssetsDir();
    static const std::filesystem::path& GetScriptsDir();

private:
    static std::filesystem::path exeDir_;
    static std::filesystem::path rootDir_;
    static std::filesystem::path assetsDir_;
    static std::filesystem::path scriptsDir_;
};