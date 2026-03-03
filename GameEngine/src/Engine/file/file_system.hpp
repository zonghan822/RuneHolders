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
    static const std::filesystem::path& GetSourceAssetsDir();
    static const std::filesystem::path& GetSourceScriptsDir();

private:
    static std::filesystem::path exeDir_;
    static std::filesystem::path rootDir_;
    static std::filesystem::path assetsDir_;
    static std::filesystem::path scriptsDir_;
    static std::filesystem::path sourceAssetsDir_;
    static std::filesystem::path sourceScriptsDir_;
};