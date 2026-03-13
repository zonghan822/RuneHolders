/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         crashlogger.cpp
* @author       Koe Yun Ji (yunji.koe@digipen.edu)   60%
* @co-author    ZongHan Law (zonghan.law@digipen.edu)   40%
* @date         03-10-2025
* @brief        Logs exception into a crash_log.txt file with timestamps
*//*========================================================================================*/

#include "crashlogger.hpp"
#include "../file/file_system.hpp"
#include <iostream>
#include <ctime>

void CrashLogger::logError(const std::string& message) {
    std::string logPath = FileSystem::GetAssetsDir().string() + "/crashlogs/crash_log.txt";
    std::ofstream log(logPath.c_str(), std::ios::app);

    if (!log) {
        std::cerr << "Failed to open crash_log.txt\n";
        return;
    }

    // Timestamp
    std::time_t t = std::time(nullptr);
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&t));

    log << "[" << buf << "] " << message << std::endl;
}