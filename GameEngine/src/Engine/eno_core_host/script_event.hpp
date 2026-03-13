/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         script_event.hpp
* @author       Muhammad Raziq Bin Mohd Rafig (muhammadraziq.b@digipen.edu)   50%
* @co-author    ZongHan Law (zonghan.law@digipen.edu)   50%
* @brief        See file name.
*//*========================================================================================*/

#pragma once
#include <cstdint>
#include <string>

namespace Core {

    // Event for triggering script methods
    struct ScriptEvent {
        uint32_t scriptID;               // Entity ID that owns the script
        std::wstring className;           // Path to the script file (for reference)
        std::string methodName;          // Name of the method to call

        ScriptEvent(uint32_t id, std::wstring classN, std::string method)
            : scriptID(id), className(classN), methodName(method) {
        }
    };

}
