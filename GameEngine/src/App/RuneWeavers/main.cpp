#include "engine/eno_engine.hpp"

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

        // Engine instance
        Engine::EnoEngine engine_;

        // Initialize all game systems
        engine_.setup();

        // Run the game
        engine_.run();

        // Terminate game
        engine_.shutDown();

    }
    catch (const std::exception& e) {
        CrashLogger::logError(e.what());
        return -1;
    }
    catch (...) {
        CrashLogger::logError("Unknown Error");
    }
    return 0;
}