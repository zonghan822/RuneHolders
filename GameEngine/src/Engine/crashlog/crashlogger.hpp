/*============================================================================================
* @file         crashlogger.hpp
* @author       Yun Ji Koe (yunji.koe@digipen.edu)
* @date         01-10-2025
* @brief        Tracks crashes with a text file that includes date and time
* 
*               This file contains the crash logging system including:
*               	- logError(const std::string& message)
*
*               USAGE:
*               	1. Whenever you run a process that may throw an error
*                    you can put the risky process into a try block
*
*               	2. Next you will end the try block and then catch the error with
*                    catch (const std::exception& e)
*
*                  //code example
*                  try {
*                        // risky process
*                    } catch (const std::exception& e) {
*                        CrashLogger::logError(e.what());
*                    }
*               	 //end of example
*
*                 3. Any error that is being logged will be written to "crash_log.txt" in
*                    "assets/crashlogs/crash_log.txt" file directory
*
*               	4. The file will display error in accordance of
*                    [YEAR-MONTH-DAY HOUR:MINUTES:SECONDS] "Error Message Placeholder"
*===========================================================================================*/

#pragma once
#include <fstream>
#include <string>


class CrashLogger {
public:
    static void logError(const std::string& message);
};