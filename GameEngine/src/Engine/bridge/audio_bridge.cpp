#include "eno_bridge.hpp"

// gEngine is defined in eno_bridge.cpp
extern Engine::EnoEngine* gEngine;

namespace Bridge
{
    extern "C" {
        ENGINE_API void __stdcall PlayInstantAudio(const wchar_t* groupName, const wchar_t* soundName)
        {

            std::cout << "Entering PIA\n";
            if (!gEngine) {
                std::cerr << "[Audio Bridge] Engine not initialized!" << std::endl;
                return;
            }

            std::wstring wSound(soundName);
            std::string sound(wSound.begin(), wSound.end());

            auto& audioMgr = gEngine->getAudioManager();
            auto& sounds = audioMgr.getSounds();

            //Console.WriteLine("Get PIA");

            // If the sound isn't in the audio manager yet, request it from the loader.
            // publishSoundRequest -> loader responds with LoadSound -> audio manager stores it.
            // This is synchronous through the event bus.
            if (sounds.find(sound) == sounds.end())
            {
                audioMgr.publishSoundRequest(sound);
            }
            
            // Check again after the request
            //Console.WriteLine("Find Sound PIA");
            std::cout << "Find Sound PIA\n";

            auto it = sounds.find(sound);

            if (it == sounds.end())
            {
                std::cerr << "[Audio Bridge] Sound '" << sound << "' not found!" << std::endl;
                return;
            }

            // Play directly via channel (no group needed for instant SFX)
            //Console.WriteLine("Going to play by channel PIA");
            std::cout << "Going to play by channel PIA\n";
            audioMgr.playByChannel(it->second, false);
        }
    }
}
