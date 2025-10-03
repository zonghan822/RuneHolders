/*============================================================================================
* @file         crashlogger.cpp
* @author       Yun Ji Koe (yunji.koe@digipen.edu)
* @date         03-10-2025
* @brief        Logs exception into a crash_log.txt file with timestamps
*//*========================================================================================*/

#include "crashlogger.hpp"
#include <iostream>
#include <ctime>

void CrashLogger::logError(const std::string& message) {
    std::ofstream log("../../../assets/crashlogs/crash_log.txt", std::ios::app);

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