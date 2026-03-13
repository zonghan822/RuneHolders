/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         crashlogger.hpp
* @author       Koe Yun Ji (yunji.koe@digipen.edu)   71%
* @co-author    Muhammad Raziq Bin Mohd Rafig (muhammadraziq.b@digipen.edu)   29%
* @date         01-10-2025
* @brief        Tracks crashes with a text file that includes date and time
*//*========================================================================================*/



#pragma once
#include <fstream>
#include <string>

// CrashLogger Class encapsulates the function logerror
class CrashLogger {
public:
    /*============================================================================================
     * \brief       Logs exception into a crash_log.txt file with timestamps
     * \param[in]   std::string const& message
     * \return      void
    *//*========================================================================================*/
    static void logError(const std::string& message);
};