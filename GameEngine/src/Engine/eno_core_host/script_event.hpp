#pragma once
#include <cstdint>
#include <string>

namespace Core {

    // Event for triggering script methods
    struct ScriptEvent {
        uint32_t entityID;               // Entity ID that owns the script
        std::wstring className;           // Path to the script file (for reference)
        std::string methodName;          // Name of the method to call

        ScriptEvent(uint32_t id, std::wstring classN, std::string method)
            : entityID(id), className(classN), methodName(method) {
        }
    };

}
