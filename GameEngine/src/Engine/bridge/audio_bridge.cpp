#include "eno_bridge.hpp"
#include "../component/components/audio.hpp"

// gEngine is defined in eno_bridge.cpp
extern Engine::EnoEngine* gEngine;

// Helper: resolve sceneObjectID to ECS::Entity via scene tree
static ECS::Entity GetAudioEntityFromID(uint32_t sceneObjectID)
{
    auto& sceneTree = gEngine->getEnoEditor().getSceneTree();
    Editor::Node* node = sceneTree.getNodeBySceneObjectID(sceneObjectID);
    if (!node)
        return ECS::Entity(UINT32_MAX);
    return node->entity_;
}

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

        ENGINE_API bool __stdcall PlayEntityAudio(uint32_t entityID)
        {
            if (!gEngine) {
                std::cerr << "[Audio Bridge] Engine not initialized!" << std::endl;
                return false;
            }

            auto& entMng = gEngine->getEntityManager();
            ECS::Entity entity = GetAudioEntityFromID(entityID);

            if (entity.id() == UINT32_MAX) {
                std::cerr << "[Audio Bridge] Entity " << entityID << " not found!" << std::endl;
                return false;
            }

            if (!entity.has<ECSComponents::AudioComponent>(entMng)) {
                std::cerr << "[Audio Bridge] Entity " << entityID << " has no AudioComponent!" << std::endl;
                return false;
            }

            auto& ac = entity.get<ECSComponents::AudioComponent>(entMng);

            if (ac.audioMod_.name_[0] == '\0') {
                std::cerr << "[Audio Bridge] Entity " << entityID << " AudioComponent has no sound attached!" << std::endl;
                return false;
            }

            auto& audioMgr = gEngine->getAudioManager();
            audioMgr.play(ac);
            return true;
        }

        ENGINE_API bool __stdcall StopEntityAudio(uint32_t entityID)
        {
            if (!gEngine) {
                std::cerr << "[Audio Bridge] Engine not initialized!" << std::endl;
                return false;
            }

            auto& entMng = gEngine->getEntityManager();
            ECS::Entity entity = GetAudioEntityFromID(entityID);

            if (entity.id() == UINT32_MAX) {
                std::cerr << "[Audio Bridge] Entity " << entityID << " not found!" << std::endl;
                return false;
            }

            if (!entity.has<ECSComponents::AudioComponent>(entMng)) {
                std::cerr << "[Audio Bridge] Entity " << entityID << " has no AudioComponent!" << std::endl;
                return false;
            }

            auto& ac = entity.get<ECSComponents::AudioComponent>(entMng);

            if (ac.audioMod_.ch_) {
                ac.audioMod_.ch_->stop();
                ac.audioMod_.ch_ = nullptr;
            }
            ac.isPlaying_ = false;

            return true;
        }
    }
}
