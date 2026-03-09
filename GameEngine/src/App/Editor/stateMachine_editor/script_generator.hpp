#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <sstream>

namespace Script {

    class StateMachineScriptGenerator {
    public:
        // Generate state methods in a C# script
        static bool generateStateMethods(const std::string& scriptPath,
            const std::string& stateName) {
            std::fstream file(scriptPath, std::ios::in | std::ios::out);
            if (!file) {
                return false;
            }

            // Read entire file
            std::stringstream buffer;
            buffer << file.rdbuf();
            std::string content = buffer.str();
            file.close();

            // Generate the three methods
            std::string enterMethod = generateEnterMethod(stateName);
            std::string updateMethod = generateUpdateMethod(stateName);
            std::string exitMethod = generateExitMethod(stateName);

            // Check if methods already exist
            std::string enterMethodSig = "void " + stateName + "_enter()";
            std::string updateMethodSig = "void " + stateName + "_update()";
            std::string exitMethodSig = "void " + stateName + "_exit()";

            bool enterExists = content.find(enterMethodSig) != std::string::npos;
            bool updateExists = content.find(updateMethodSig) != std::string::npos;
            bool exitExists = content.find(exitMethodSig) != std::string::npos;

            // Only add methods that don't exist
            std::string methodsToAdd;
            if (!enterExists) methodsToAdd += enterMethod;
            if (!updateExists) methodsToAdd += updateMethod;
            if (!exitExists) methodsToAdd += exitMethod;

            if (methodsToAdd.empty()) {
                return true; // All methods already exist
            }

            // Find the class and insert methods
            if (!insertMethodsIntoClass(content, methodsToAdd)) {
                return false;
            }

            // Write back to file
            std::ofstream outFile(scriptPath, std::ios::trunc);
            if (!outFile) {
                return false;
            }
            outFile << content;
            outFile.close();

            return true;
        }

        // Remove state methods from script
        static bool removeStateMethods(const std::string& scriptPath,
            const std::string& stateName) {
            std::fstream file(scriptPath, std::ios::in);
            if (!file) {
                return false;
            }

            std::stringstream buffer;
            buffer << file.rdbuf();
            std::string content = buffer.str();
            file.close();

            // Remove all three methods
            removeMethod(content, stateName + "_enter");
            removeMethod(content, stateName + "_update");
            removeMethod(content, stateName + "_exit");

            // Write back
            std::ofstream outFile(scriptPath, std::ios::trunc);
            if (!outFile) {
                return false;
            }
            outFile << content;
            outFile.close();

            return true;
        }

        // Check if script has all required state methods
        static bool hasStateMethods(const std::string& scriptPath,
            const std::string& stateName) {
            std::ifstream file(scriptPath);
            if (!file) {
                return false;
            }

            std::stringstream buffer;
            buffer << file.rdbuf();
            std::string content = buffer.str();

            bool hasEnter = content.find(stateName + "_enter()") != std::string::npos;
            bool hasUpdate = content.find(stateName + "_update()") != std::string::npos;
            bool hasExit = content.find(stateName + "_exit()") != std::string::npos;

            return hasEnter && hasUpdate && hasExit;
        }

        static std::vector<std::string> extractStatesFromScript(const std::string& scriptPath) {
            std::ifstream file(scriptPath);
            if (!file) return {};

            std::stringstream buffer;
            buffer << file.rdbuf();
            std::string content = buffer.str();

            std::unordered_map<std::string, int> stateMask;
            // bitmask: 1 = enter, 2 = update, 4 = exit

            auto scan = [&](const std::string& suffix, int bit) {
                size_t pos = 0;
                while ((pos = content.find(suffix, pos)) != std::string::npos) {
                    // walk backwards to get state name
                    size_t end = pos;
                    size_t start = end;

                    while (start > 0 &&
                        (isalnum(content[start - 1]) || content[start - 1] == '_')) {
                        --start;
                    }

                    std::string state = content.substr(start, end - start);
                    stateMask[state] |= bit;
                    pos = end + suffix.size();
                }
                };

            scan("_enter()", 1);
            scan("_update()", 2);
            scan("_exit()", 4);

            std::vector<std::string> result;
            for (auto& [state, mask] : stateMask) {
                if (mask == 0b111) { // enter + update + exit
                    result.push_back(state);
                }
            }

            return result;
        }

    private:
        static std::string generateEnterMethod(const std::string& stateName) {
            return R"(
        // Called when entering the )" + stateName + R"( state
        public void )" + stateName + R"(_enter()
        {
            // TODO: Add enter logic for )" + stateName + R"( state
        }
)";
        }

        static std::string generateUpdateMethod(const std::string& stateName) {
            return R"(
        // Called every frame while in the )" + stateName + R"( state
        public void )" + stateName + R"(_update()
        {
            // TODO: Add update logic for )" + stateName + R"( state
        }
)";
        }

        static std::string generateExitMethod(const std::string& stateName) {
            return R"(
        // Called when exiting the )" + stateName + R"( state
        public void )" + stateName + R"(_exit()
        {
            // TODO: Add exit logic for )" + stateName + R"( state
        }
)";
        }

        static bool insertMethodsIntoClass(std::string& content, const std::string& methods) {
            // Find the class definition
            size_t classPos = content.find("class ");
            if (classPos == std::string::npos) {
                return false;
            }

            // Find the opening brace
            size_t openBrace = content.find("{", classPos);
            if (openBrace == std::string::npos) {
                return false;
            }

            // Find class indentation
            size_t lineStart = content.rfind('\n', classPos);
            if (lineStart == std::string::npos) lineStart = 0;
            else ++lineStart;

            size_t indentEnd = lineStart;
            while (indentEnd < content.size() &&
                (content[indentEnd] == ' ' || content[indentEnd] == '\t')) {
                ++indentEnd;
            }
            std::string classIndent = content.substr(lineStart, indentEnd - lineStart);

            // Find the matching closing brace
            int depth = 1;
            size_t pos = openBrace + 1;

            while (pos < content.size() && depth > 0) {
                if (content[pos] == '{') ++depth;
                else if (content[pos] == '}') --depth;
                ++pos;
            }

            if (depth != 0) {
                return false;
            }

            // Insert before the closing brace
            size_t insertPos = pos - 1;
            content.insert(insertPos, methods + classIndent);

            return true;
        }

        static void removeMethod(std::string& content, const std::string& methodName) {
            // Find method signature
            std::string searchStr = "void " + methodName + "()";
            size_t methodPos = content.find(searchStr);

            if (methodPos == std::string::npos) {
                return;
            }

            // Find start of method (including comments and attributes)
            size_t methodStart = methodPos;

            // Look backwards for the start of the method declaration
            while (methodStart > 0 && content[methodStart - 1] != '\n') {
                --methodStart;
            }

            // Skip any preceding comments or attributes
            while (methodStart > 0) {
                // Skip whitespace
                size_t lineStart = methodStart;
                while (lineStart > 0 && content[lineStart - 1] != '\n') {
                    --lineStart;
                }

                // Check if this line is a comment or attribute
                size_t firstChar = lineStart;
                while (firstChar < methodStart &&
                    (content[firstChar] == ' ' || content[firstChar] == '\t')) {
                    ++firstChar;
                }

                if (firstChar < methodStart &&
                    (content.substr(firstChar, 2) == "//" || content[firstChar] == '[')) {
                    methodStart = lineStart;
                }
                else {
                    break;
                }
            }

            // Find the opening brace
            size_t bracePos = content.find("{", methodPos);
            if (bracePos == std::string::npos) {
                return;
            }

            // Find the matching closing brace
            int depth = 1;
            size_t pos = bracePos + 1;

            while (pos < content.size() && depth > 0) {
                if (content[pos] == '{') ++depth;
                else if (content[pos] == '}') --depth;
                ++pos;
            }

            if (depth == 0) {
                // Include the newline after the closing brace if present
                if (pos < content.size() && content[pos] == '\n') {
                    ++pos;
                }
                content.erase(methodStart, pos - methodStart);
            }
        }
    };

}
