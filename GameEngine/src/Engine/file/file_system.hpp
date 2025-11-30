#pragma once
#include <filesystem>
#include <string>
#include <iostream>

class FileSystem
{
public:
    static void Initialize();

    static const std::filesystem::path& GetExecutableDir();
    static const std::filesystem::path& GetAssetsDir();
    static const std::filesystem::path& GetScriptsDir();

private:
    static std::filesystem::path exeDir_;
    static std::filesystem::path assetsDir_;
    static std::filesystem::path scriptsDir_;
};