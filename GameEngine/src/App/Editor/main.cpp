#include "editor_logic.hpp"

void EnableMemoryLeakChecking(int breakAlloc = -1)
{
    //Set the leak checking flag
    int tmpDbgFlag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
    tmpDbgFlag |= _CRTDBG_LEAK_CHECK_DF;
    _CrtSetDbgFlag(tmpDbgFlag);

    //If a valid break alloc provided set the breakAlloc
    if (breakAlloc != -1) _CrtSetBreakAlloc(breakAlloc);
}

#ifdef _DEBUG
int main() {
#else
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
#endif
    try {
#ifdef _DEBUG
        EnableMemoryLeakChecking();
#endif

        FileSystem::Initialize(ENO_APP_NAME);

        // Create game instance
        EditorApplication editor;

        // Initialize all game systems
        editor.initialize();

        // Run the game
        editor.run();

        // Terminate game
        editor.shutdown();

    }
    catch (const std::exception& e) {
        std::cerr << "Editor error: " << e.what() << std::endl;
        CrashLogger::logError(e.what());
        return -1;
    }
    catch (...) {
        CrashLogger::logError("Unknown Error");
    }
    return 0;
}