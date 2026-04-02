/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         main.cpp
* @author       Muhammad Raziq Bin Mohd Rafig (muhammadraziq.b@digipen.edu)   50%
* @co-author    ZongHan Law (zonghan.law@digipen.edu)   50%
* @brief        See file name.
*//*========================================================================================*/

#include <windows.h>
#include <shlobj.h>
#include <string>
#include <map>
#include <fstream>
#include <filesystem>
#include "resource.h"

std::string selectedResolution = "1280x720";
bool isFullscreen = false;
bool isVSync = false;

BOOL OnInitDialog(HWND hwnd)
{
    HWND combo = GetDlgItem(hwnd, IDC_RESOLUTION);

    const char* resolutions[] = {
        "1280x720",
        "1600x900",
        "1920x1080",
        "2560x1440",
        "3840x2160"
    };

    for (auto r : resolutions)
        SendMessage(combo, CB_ADDSTRING, 0, (LPARAM)r);

    SendMessage(combo, CB_SETCURSEL, 0, 0);
    return TRUE;
}

std::filesystem::path GetLauncherDir()
{
    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
    std::filesystem::path p(buffer);
    return p.parent_path();
}

// Returns the writable config directory.
// In dev builds (assets/ next to exe), writes next to the exe.
// In shipped builds, writes to %APPDATA%/RuneWeavers.
std::string GetConfigDir()
{
    // Dev mode: assets folder exists next to the exe
    std::string localAssets = GetLauncherDir().string() + "\\assets\\settings";

    char appDataBuf[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, appDataBuf)))
    {
        std::string appDataDir = std::string(appDataBuf) + "\\RuneWeavers\\settings";
        std::filesystem::create_directories(appDataDir);
        return appDataDir;
    }

    // Fallback to local
    std::filesystem::create_directories(localAssets);
    return localAssets;
}

void SaveConfig()
{
    int width = stoi(selectedResolution.substr(0, selectedResolution.find('x')));
    int height = stoi(selectedResolution.substr(selectedResolution.find('x') + 1));

    std::string folder = GetConfigDir();
    std::string filepath = folder + "\\config.txt";

    // Read existing config to preserve audio volumes and other saved keys
    std::map<std::string, std::string> existing;
    {
        std::ifstream in(filepath);
        std::string line;
        while (std::getline(in, line))
        {
            auto colon = line.find(": ");
            if (colon != std::string::npos)
                existing[line.substr(0, colon)] = line.substr(colon + 2);
        }
    }

    // Update launcher-controlled keys
    existing["Engine Version"] = "0.0001a";
    existing["Window Width"]   = std::to_string(width);
    existing["Window Height"]  = std::to_string(height);
    existing["Fullscreen"]     = isFullscreen ? "true" : "false";
    existing["VSync"]          = isVSync ? "true" : "false";

    // Ensure tilemap defaults exist
    if (existing.find("Tilemap Width") == existing.end())    existing["Tilemap Width"]    = "64";
    if (existing.find("Tilemap Height") == existing.end())   existing["Tilemap Height"]   = "64";
    if (existing.find("Tilemap CellSize") == existing.end()) existing["Tilemap CellSize"] = "64";

    std::ofstream file(filepath);
    for (auto& [key, val] : existing)
        file << key << ": " << val << "\n";
    file.close();
}

void LaunchGame()
{
    std::string gameExe = GetLauncherDir().string() + "\\RuneWeavers.exe";

    STARTUPINFO si = { sizeof(si) };
    PROCESS_INFORMATION pi;

    BOOL result = CreateProcessA(
        gameExe.c_str(),   // path to EXE (same directory)
        NULL, NULL, NULL,
        FALSE,
        0, NULL, NULL,
        &si, &pi
    );

    if (result) {
        // We do NOT wait for the game to exit.
        // Close thread and process handles immediately.
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
    else {
        MessageBoxA(NULL, ("Failed to launch:\n" + gameExe).c_str(), "Error", MB_ICONERROR);
    }
}

INT_PTR CALLBACK DialogProc(HWND hwnd, UINT msg, WPARAM w, LPARAM)
{
    switch (msg)
    {
        case WM_INITDIALOG:
            return OnInitDialog(hwnd);

        case WM_COMMAND:
        {
            switch (LOWORD(w))
            {
                case IDPLAY:
                {
                    // Read resolution
                    HWND combo = GetDlgItem(hwnd, IDC_RESOLUTION);
                    int sel = (int)SendMessage(combo, CB_GETCURSEL, 0, 0);

                    int len = (int)SendMessage(combo, CB_GETLBTEXTLEN, sel, 0);
                    if (len > 0)
                    {
                        std::string buffer;
                        buffer.resize(len + 1);
                        SendMessage(combo, CB_GETLBTEXT, sel, (LPARAM)buffer.data());
                        selectedResolution = buffer.c_str();
                    }

                    // Read checkboxes
                    isFullscreen = IsDlgButtonChecked(hwnd, IDC_FULLSCREEN);
                    isVSync      = IsDlgButtonChecked(hwnd, IDC_VSYNC);

                    // Save settings
                    SaveConfig();

                    // Launch game
                    LaunchGame();

                    EndDialog(hwnd, 0);
                    return TRUE;
                }

                case IDCANCEL:
                    EndDialog(hwnd, 0);
                    return TRUE;
            }
        }
    }
    return FALSE;
}

#ifdef _DEBUG

int main()
{
    return (int)DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_LAUNCHER), NULL, DialogProc);
}

#else

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int)
{
    return (int)DialogBox(hInst, MAKEINTRESOURCE(IDD_LAUNCHER), NULL, DialogProc);
}

#endif