#include "eno_bridge.hpp"
#include "../component/components/drawInfo.hpp"
#include "../component/components/ai_component.hpp"

static Engine::EnoEngine* gEngine = nullptr;

namespace Bridge
{
    extern "C" {
        ENGINE_API void __stdcall Bridge_Init(void* enginePtr)
        {
            gEngine = reinterpret_cast<Engine::EnoEngine*>(enginePtr);
            if (!gEngine) {
                std::cerr << "[Engine DLL] Bridge not initialized!" << std::endl;
                return;
            }
        }

        ENGINE_API bool __stdcall IsKeyPressed(int key)
        {
            if (!gEngine) {
                std::cerr << "[Engine DLL] Bridge not initialized!" << std::endl;
                return false;
            }
            auto input = gEngine->getInputManager();
            return input.isKeyDown(key);
        }

        ENGINE_API bool __stdcall IsKeyHeld(int key)
        {
            if (!gEngine) {
                std::cerr << "[Engine DLL] Bridge not initialized!" << std::endl;
                return false;
            }
            auto input = gEngine->getInputManager();
            return input.isKeyPressed(key);
        }

        ENGINE_API bool __stdcall IsKeyReleased(int key)
        {
            if (!gEngine) {
                std::cerr << "[Engine DLL] Bridge not initialized!" << std::endl;
                return false;
            }
            auto input = gEngine->getInputManager();
            return input.isKeyUp(key);
        }

        ENGINE_API int __stdcall ModsApplied()
        {
            if (!gEngine) {
                std::cerr << "[Engine DLL] Bridge not initialized!" << std::endl;
                return 0;
            }
            auto input = gEngine->getInputManager();
            return input.currentMods();
        }

        ENGINE_API void ChangeState(const wchar_t* stateName, uint32_t entityID) 
        {
            if (!gEngine) {
                std::cerr << "[Engine DLL] Bridge not initialized!" << std::endl;
                return;
            }

            std::wstring wState(stateName);
            std::string state(wState.begin(), wState.end());

            // Request state change through the system
            gEngine->getStateMachineSys().requestStateChange(entityID, state);
        }
        
        //Animation Bridging -> C# should call this function whenever theres a need to change animation
        ENGINE_API bool __stdcall SetEntityAnimation(uint32_t entityID, const wchar_t* animationName)
        {
            if (!gEngine) {
                std::cerr << "[Engine DLL] Bridge not initialized!" << std::endl;
                return false;
            }

            // Convert wide string to std::string
            std::wstring wname(animationName);
            std::string name(wname.begin(), wname.end());

            auto& entMng = gEngine->getEntityManager();
            auto& animRegistry = gEngine->getAnimationRegistry();

            // Check if entity exists and has animation component
            ECS::Entity entity(entityID);
            if (!entity.has<ECSComponents::AnimationControllerComponent>(entMng)) {
                std::cerr << "[Bridge] Entity " << entityID << " has no AnimationControllerComponent!" << std::endl;
                return false;
            }

            auto& animComp = entity.get<ECSComponents::AnimationControllerComponent>(entMng);
            Animation::AnimationController* ctrl = animRegistry.get(animComp.controllerID);

            if (!ctrl) {
                std::cerr << "[Bridge] No AnimationController found for entity " << entityID << std::endl;
                return false;
            }

            bool success = ctrl->setAnimation(name);

            // Also update the component's current animation name
            if (success) {
                strncpy_s(animComp.currentAnimationName, name.c_str(), sizeof(animComp.currentAnimationName) - 1);
                animComp.currentAnimationName[sizeof(animComp.currentAnimationName) - 1] = '\0';

                // CRITICAL: Also update the entity's texture to match the animation
                if (entity.has<ECSComponents::DrawInfoComponent>(entMng)) {
                    auto& drawComp = entity.get<ECSComponents::DrawInfoComponent>(entMng);
                    auto tex = ctrl->getCurrentTexture();
                    if (tex) {
                        drawComp.texture = tex;
                        std::cout << "[Bridge] Set animation '" << name << "' and updated texture for entity " << entityID << std::endl;
                    }
                }
            }

            return success;
        }

        // Static buffer for returning AI state string
        static wchar_t aiStateBuffer[64];

        ENGINE_API const wchar_t* __stdcall GetEntityAIState(uint32_t entityID)
        {
            if (!gEngine) {
                std::cerr << "[Engine DLL] Bridge not initialized!" << std::endl;
                return L"";
            }

            auto& entMng = gEngine->getEntityManager();
            ECS::Entity entity(entityID);

            if (!entity.has<ECSComponents::AIComponent>(entMng)) {
                return L"";
            }

            auto& ai = entity.get<ECSComponents::AIComponent>(entMng);

            // Convert char[] to wchar_t[]
            size_t len = strlen(ai.currentState);
            for (size_t i = 0; i < len && i < 63; ++i) {
                aiStateBuffer[i] = static_cast<wchar_t>(ai.currentState[i]);
            }
            aiStateBuffer[len < 63 ? len : 63] = L'\0';

            return aiStateBuffer;
        }
    }
}