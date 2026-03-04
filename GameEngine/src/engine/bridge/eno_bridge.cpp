#include "eno_bridge.hpp"
#include "../component/components/drawInfo.hpp"
#include "../component/components/ai_component.hpp"
#include "../component/components/state_machine_component.hpp"

// Global engine pointer shared across all bridge files
Engine::EnoEngine* gEngine = nullptr;

namespace Bridge
{
    // Helper function to get entity from sceneObjectID
    static ECS::Entity GetEntityFromSceneObjectID(uint32_t sceneObjectID, Editor::EnoSceneTree& sceneTree)
    {
        Editor::Node* node = sceneTree.getNodeBySceneObjectID(sceneObjectID);
        if (!node) {
            return ECS::Entity(UINT32_MAX); // Invalid entity
        }
        return node->entity_;
    }

    extern "C" {
        ENGINE_API void __stdcall Bridge_Init(void* enginePtr)
        {
            gEngine = reinterpret_cast<Engine::EnoEngine*>(enginePtr);
            if (!gEngine) {
                std::cerr << "[Engine DLL] Bridge not initialized!" << std::endl;
                return;
            }

            // DLL has its own copy of FileSystem statics (separate from the EXE's).
            // Initialize them here so inlined paths like selectScene resolve correctly.
            // FileSystem::Initialize();
        }

        ENGINE_API float __stdcall GetDeltaTime()
        {
            if (!gEngine) {
                std::cerr << "[Engine DLL] Bridge not initialized!" << std::endl;
                return 0.0f;
            }
            return static_cast<float>(gEngine->getDeltaTime());
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

        ENGINE_API void ChangeState(uint32_t sceneObjectID, const wchar_t* stateName)
        {
            if (!gEngine) {
                std::cerr << "[Engine DLL] Bridge not initialized!" << std::endl;
                return;
            }

            std::wstring wState(stateName);
            std::string state(wState.begin(), wState.end());

            // Get entity ID from sceneObjectID
            auto& sceneTree = gEngine->getEnoEditor().getSceneTree();
            Editor::Node* node = sceneTree.getNodeBySceneObjectID(sceneObjectID);

            if (!node) {
                std::cerr << "[Bridge] ChangeState: No node found for sceneObjectID " << sceneObjectID << std::endl;
                return;
            }

            uint32_t entityID = node->entity_.id();
            std::cout << "[Bridge] ChangeState: sceneObjectID " << sceneObjectID << " -> entityID " << entityID << " -> state '" << state << "'" << std::endl;

            // Request state change through the system (uses ECS entity ID)
            gEngine->getStateMachineSys().requestStateChange(entityID, state);
        }
        
        //Animation Bridging -> C# should call this function whenever theres a need to change animation
        ENGINE_API bool __stdcall SetEntityAnimation(uint32_t NodeID, const wchar_t* animationName)
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
            auto& sceneTree = gEngine->getEnoEditor().getSceneTree();
            ECS::Entity entity = GetEntityFromSceneObjectID(NodeID, sceneTree);

            if (!entity.has<ECSComponents::AnimationControllerComponent>(entMng)) {
                std::cerr << "[Bridge] Entity " << NodeID << " has no AnimationControllerComponent!" << std::endl;
                return false;
            }

            auto& animComp = entity.get<ECSComponents::AnimationControllerComponent>(entMng);
            Animation::AnimationController* ctrl = animRegistry.get(animComp.controllerID);

            if (!ctrl) {
                std::cerr << "[Bridge] No AnimationController found for entity " << NodeID << std::endl;
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
                        std::cout << "[Bridge] Set animation '" << name << "' and updated texture for entity " << NodeID << std::endl;
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

            if (!entity.has<ECSComponents::StateMachineComponent>(entMng)) {
                return L"";
            }

            auto& stateMachine = entity.get<ECSComponents::StateMachineComponent>(entMng);

            // Convert char[] to wchar_t[]
            size_t len = strlen(stateMachine.currentState);
            for (size_t i = 0; i < len && i < 63; ++i) {
                aiStateBuffer[i] = static_cast<wchar_t>(stateMachine.currentState[i]);
            }
            aiStateBuffer[len < 63 ? len : 63] = L'\0';

            return aiStateBuffer;
        }

        ENGINE_API void __stdcall LoadScene(const wchar_t* sceneName)
        {
            if (!gEngine) {
                std::cerr << "[Engine DLL] Bridge not initialized!" << std::endl;
                return;
            }

            std::wstring wName(sceneName);
            std::string name(wName.begin(), wName.end());

            gEngine->getEnoEditor().selectScene(name);
        }
    }
}