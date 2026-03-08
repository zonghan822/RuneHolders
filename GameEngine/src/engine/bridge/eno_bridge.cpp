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
        }

        ENGINE_API float __stdcall GetDeltaTime()
        {
            if (!gEngine) {
                std::cerr << "[Engine DLL] Bridge not initialized!" << std::endl;
                return 0.0f;
            }
            return static_cast<float>(gEngine->getDeltaTime());
        }

        ENGINE_API uint32_t __stdcall GetNode(const char* name)
        {
            Editor::Node* node = gEngine->getEnoEditor().getSceneTree().getNodeByName(name);
            if (!node)
                return UINT32_MAX;

            return node->sceneObjectID_;
        }

        ENGINE_API uint32_t __stdcall GetChild(uint32_t parentID, uint32_t index)
        {
            // Parent
            Editor::Node* parent = gEngine->getEnoEditor().getSceneTree().getNodeBySceneObjectID(parentID);
            if (!parent)
                return 0;

            // Child
            Editor::Node* child = gEngine->getEnoEditor().getSceneTree().getChild(parent, index);
            if (!child)
                return 0;

            return child->sceneObjectID_;
        }

        ENGINE_API uint32_t __stdcall GetNodeType(uint32_t id)
        {
            Editor::Node* node = gEngine->getEnoEditor().getSceneTree().getNodeBySceneObjectID(id);
            if (!node)
                return static_cast<uint32_t>(Editor::NodeType::EMPTY);

            return static_cast<uint32_t>(node->type_);
        }

        ENGINE_API uint32_t __stdcall GetZIndex(uint32_t nodeID)
        {
            if (!gEngine)
            {
                std::cerr << "[Engine DLL] Bridge not initialized!" << std::endl;
            }

            Editor::Node* node = gEngine->getEnoEditor().getSceneTree().getNodeByEntityID(nodeID);
            if (!node)
                return 0;

            if (node->entity_.has<ECSComponents::VisibilityComponent>(gEngine->getEntityManager()))
                return node->entity_.get<ECSComponents::VisibilityComponent>(gEngine->getEntityManager()).z_index;
            else
                throw std::runtime_error("No visibility component on the entity!");
        }

        ENGINE_API void __stdcall SetZIndex(uint32_t nodeID, uint32_t z)
        {
            if (!gEngine) 
            {
                std::cerr << "[Engine DLL] Bridge not initialized!" << std::endl;
            }

            Editor::Node* node = gEngine->getEnoEditor().getSceneTree().getNodeByEntityID(nodeID);
            if (!node)
                return;

            if (node->entity_.has<ECSComponents::VisibilityComponent>(gEngine->getEntityManager()))
            {
                auto& vis = node->entity_.get<ECSComponents::VisibilityComponent>(gEngine->getEntityManager());
                vis.z_index = z;
            }
            else
                throw std::runtime_error("No visibility component on the entity!");
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
    }
}