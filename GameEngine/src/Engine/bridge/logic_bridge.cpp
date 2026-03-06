/*============================================================================================
* @file         logic_bridge.cpp
* @brief        Bridge implementations for Logic System C# support:
*               entity queries, entity state, game pause, text, assets, combat, mouse.
*//*========================================================================================*/

#include "eno_bridge.hpp"
#include "../component/components/name.hpp"
#include "../component/components/visibility.hpp"
#include "../component/components/transform.hpp"
#include "../component/components/text_component.hpp"
#include "../component/components/combat_components.hpp"
#include "../component/components/camera_component.hpp"
#include "../component/components/buttons.hpp"
#include "../component/components/collision_component.hpp"
#include "../file/file_system.hpp"
#include "../graphics/graphics.hpp"
#include <GLFW/glfw3.h>

// gEngine is defined in eno_bridge.cpp
extern Engine::EnoEngine* gEngine;

namespace Bridge
{
    // -----------------------------------------------------------------------
    // Helper: find node by NameComponent in scene tree
    static Editor::Node* FindNodeByName(const std::string& name)
    {
        if (!gEngine) return nullptr;
        auto& sceneTree = gEngine->getEnoEditor().getSceneTree();
        auto& entityMng = gEngine->getEntityManager();
        for (auto* node : sceneTree.getAllNodes()) {
            if (!node) continue;
            if (node->entity_.has<ECSComponents::NameComponent>(entityMng)) {
                auto& nc = node->entity_.get<ECSComponents::NameComponent>(entityMng);
                if (std::string(nc.name) == name) {
                    return node;
                }
            }
        }
        return nullptr;
    }

    // -----------------------------------------------------------------------
    // Helper: resolve an ID to an ECS::Entity.
    // IDs returned by FindEntityByName are either sceneObjectIDs (for scene
    // tree entities) or raw ECS entity IDs (for dynamically-created entities).
    // We try the scene tree first; on failure we treat id as a raw entity ID.
    static ECS::Entity ResolveEntity(uint32_t id)
    {
        if (!gEngine) return ECS::Entity(UINT32_MAX);
        auto& sceneTree = gEngine->getEnoEditor().getSceneTree();
        Editor::Node* node = sceneTree.getNodeBySceneObjectID(id);
        if (node) return node->entity_;
        return ECS::Entity(id);  // dynamic entity: id IS the raw ECS entity ID
    }

    extern "C" {

        // ============= Entity Queries =============

        ENGINE_API uint32_t __stdcall FindEntityByName(const wchar_t* name)
        {
            if (!gEngine) return 0;
            std::wstring wname(name);
            std::string sname(wname.begin(), wname.end());

            // 1) Try scene tree (returns sceneObjectID)
            Editor::Node* node = FindNodeByName(sname);
            if (node) return node->sceneObjectID_;

            // 2) Fall back to raw EntityManager (returns ECS entity ID)
            auto& entityMng = gEngine->getEntityManager();
            auto found = entityMng.findEntity<ECSComponents::NameComponent>(
                ECSComponents::NameComponent(sname.c_str()));
            if (!found.empty()) return found[0].id();

            return 0;
        }

        // ============= Entity State =============

        ENGINE_API bool __stdcall SetEntityVisible(uint32_t entityID, bool visible)
        {
            if (!gEngine) return false;
            auto& entityMng = gEngine->getEntityManager();
            ECS::Entity entity = ResolveEntity(entityID);
            if (!entity.has<ECSComponents::VisibilityComponent>(entityMng)) return false;
            entity.get<ECSComponents::VisibilityComponent>(entityMng).visible = visible;
            return true;
        }

        ENGINE_API bool __stdcall SetEntityScale(uint32_t entityID, float x, float y)
        {
            if (!gEngine) return false;
            auto& entityMng = gEngine->getEntityManager();
            ECS::Entity entity = ResolveEntity(entityID);
            if (!entity.has<ECSComponents::TransformComponent>(entityMng)) return false;
            auto& tf = entity.get<ECSComponents::TransformComponent>(entityMng);
            tf.scale.x = x;
            tf.scale.y = y;
            return true;
        }

        ENGINE_API bool __stdcall GetEntityScale(uint32_t entityID, float* outX, float* outY)
        {
            if (!gEngine || !outX || !outY) return false;
            auto& entityMng = gEngine->getEntityManager();
            ECS::Entity entity = ResolveEntity(entityID);
            if (!entity.has<ECSComponents::TransformComponent>(entityMng)) return false;
            auto& tf = entity.get<ECSComponents::TransformComponent>(entityMng);
            *outX = tf.scale.x;
            *outY = tf.scale.y;
            return true;
        }

        // ============= Game Pause =============

        ENGINE_API void __stdcall PauseGame()
        {
            if (!gEngine) return;
            gEngine->setPaused(true);
        }

        ENGINE_API void __stdcall ResumeGame()
        {
            if (!gEngine) return;
            gEngine->setPaused(false);
        }

        ENGINE_API bool __stdcall IsGamePaused()
        {
            if (!gEngine) return false;
            return gEngine->getIsPaused();
        }

        // ============= Text =============

        ENGINE_API bool __stdcall SetEntityText(uint32_t entityID, const wchar_t* text)
        {
            if (!gEngine || !text) return false;
            auto& entityMng = gEngine->getEntityManager();
            ECS::Entity entity = ResolveEntity(entityID);
            if (!entity.has<ECSComponents::TextComponent>(entityMng)) return false;
            auto& tc = entity.get<ECSComponents::TextComponent>(entityMng);
            std::wstring wtext(text);
            std::string stext(wtext.begin(), wtext.end());
            strncpy_s(tc.text, stext.c_str(), sizeof(tc.text) - 1);
            tc.text[sizeof(tc.text) - 1] = '\0';
            return true;
        }

        // ============= Assets =============

        ENGINE_API const wchar_t* __stdcall GetAssetsDir()
        {
            static std::wstring cachedPath;
            if (cachedPath.empty()) {
                std::string path = FileSystem::GetAssetsDir().string();
                cachedPath = std::wstring(path.begin(), path.end());
            }
            return cachedPath.c_str();
        }

        // ============= Combat =============

        ENGINE_API float __stdcall GetEntityHealth(uint32_t entityID)
        {
            if (!gEngine) return -1.0f;
            auto& entityMng = gEngine->getEntityManager();
            ECS::Entity entity = ResolveEntity(entityID);
            if (!entity.has<ECSComponents::HealthComponent>(entityMng)) return -1.0f;
            return entity.get<ECSComponents::HealthComponent>(entityMng).currentHealth;
        }

        ENGINE_API bool __stdcall SetEntityHealth(uint32_t entityID, float hp)
        {
            if (!gEngine) return false;
            auto& entityMng = gEngine->getEntityManager();
            ECS::Entity entity = ResolveEntity(entityID);
            if (!entity.has<ECSComponents::HealthComponent>(entityMng)) return false;
            auto& hc = entity.get<ECSComponents::HealthComponent>(entityMng);
            hc.currentHealth = (hp < 0.0f) ? 0.0f : (hp > hc.maxHealth ? hc.maxHealth : hp);
            hc.isDead = (hc.currentHealth <= 0.0f);
            return true;
        }

        ENGINE_API bool __stdcall AddEntityDamage(uint32_t entityID, float damage)
        {
            if (!gEngine) return false;
            auto& entityMng = gEngine->getEntityManager();
            ECS::Entity entity = ResolveEntity(entityID);
            if (!entity.has<ECSComponents::HealthComponent>(entityMng)) return false;
            auto& hc = entity.get<ECSComponents::HealthComponent>(entityMng);
            hc.takeDamage(damage);
            return true;
        }

        // ============= Mouse =============

        ENGINE_API bool __stdcall GetMouseWorldPosition(float* outX, float* outY)
        {
            if (!gEngine || !outX || !outY) return false;
            auto& entityMng = gEngine->getEntityManager();

            float cameraX = 0.0f;
            float cameraY = 0.0f;
            float viewportWidth  = 1920.0f;
            float viewportHeight = 1080.0f;

            auto cameraEntities = entityMng.findEntity<ECSComponents::CameraComponent>();
            if (!cameraEntities.empty()) {
                auto& cam = cameraEntities[0].get<ECSComponents::CameraComponent>(entityMng);
                cameraX = cam.position.x - cam.viewportWidth  * 0.5f;
                cameraY = cam.position.y - cam.viewportHeight * 0.5f;
                viewportWidth  = cam.viewportWidth;
                viewportHeight = cam.viewportHeight;
            }

            auto windowSize     = gEngine->getWindowManager().getWindowSize();
            auto mouseScreenPos = gEngine->getInputManager().getMousePosition();

            auto world = Graphics::screenToWorld(
                { static_cast<float>(mouseScreenPos.first), static_cast<float>(mouseScreenPos.second) },
                { 0.0f, 0.0f },
                { static_cast<float>(windowSize.first), static_cast<float>(windowSize.second) },
                viewportWidth,
                viewportHeight,
                cameraX,
                cameraY
            );

            *outX = world.x;
            *outY = world.y;
            return true;
        }

        ENGINE_API bool __stdcall IsMouseButtonDown(int button)
        {
            if (!gEngine) return false;
            return gEngine->getInputManager().isMousePressed(button);
        }

        // ============= Button =============

        ENGINE_API bool __stdcall IsButtonClicked(uint32_t entityID)
        {
            if (!gEngine) return false;

            // Only fire on the frame the mouse button goes down
            if (!gEngine->getInputManager().isMouseDown(GLFW_MOUSE_BUTTON_LEFT))
                return false;

            auto& entityMng = gEngine->getEntityManager();
            ECS::Entity entity = ResolveEntity(entityID);

            if (!entity.has<ECSComponents::ButtonComponent>(entityMng))   return false;
            if (!entity.has<ECSComponents::TransformComponent>(entityMng)) return false;
            if (!entity.has<ECSComponents::CollisionComponent>(entityMng)) return false;

            auto& btn       = entity.get<ECSComponents::ButtonComponent>(entityMng);
            auto& transform = entity.get<ECSComponents::TransformComponent>(entityMng);
            auto& collision = entity.get<ECSComponents::CollisionComponent>(entityMng);

            // Get mouse world position using the same logic as GetMouseWorldPosition
            float cameraX = 0.0f, cameraY = 0.0f;
            float viewportWidth = 1920.0f, viewportHeight = 1080.0f;
            auto cameraEntities = entityMng.findEntity<ECSComponents::CameraComponent>();
            if (!cameraEntities.empty()) {
                auto& cam = cameraEntities[0].get<ECSComponents::CameraComponent>(entityMng);
                cameraX       = cam.position.x - cam.viewportWidth  * 0.5f;
                cameraY       = cam.position.y - cam.viewportHeight * 0.5f;
                viewportWidth  = cam.viewportWidth;
                viewportHeight = cam.viewportHeight;
            }
            auto windowSize     = gEngine->getWindowManager().getWindowSize();
            auto mouseScreenPos = gEngine->getInputManager().getMousePosition();
            auto world = Graphics::screenToWorld(
                { static_cast<float>(mouseScreenPos.first), static_cast<float>(mouseScreenPos.second) },
                { 0.0f, 0.0f },
                { static_cast<float>(windowSize.first), static_cast<float>(windowSize.second) },
                viewportWidth, viewportHeight, cameraX, cameraY
            );

            // DrawInfoComponent is required by the signature but unused in hit-test logic
            ECSComponents::DrawInfoComponent dummyDraw{};
            const ECSComponents::DrawInfoComponent& drawRef =
                entity.has<ECSComponents::DrawInfoComponent>(entityMng)
                ? entity.get<ECSComponents::DrawInfoComponent>(entityMng)
                : dummyDraw;

            return btn.onClick({ world.x, world.y }, transform, drawRef, collision);
        }

        // ============= Scene Query =============

        ENGINE_API const wchar_t* __stdcall GetCurrentScene()
        {
            static std::wstring cachedScene;
            if (!gEngine) return L"";
            std::string name = gEngine->getEnoEditor().getCurrentScene().getSceneName();
            cachedScene = std::wstring(name.begin(), name.end());
            return cachedScene.c_str();
        }

        // ============= Application =============

        ENGINE_API void __stdcall QuitApplication()
        {
            if (!gEngine) return;
            gEngine->getWindowManager().setShouldClose(true);
        }

        // ============= Playing State =============

        ENGINE_API void __stdcall SetIsPlaying(bool playing)
        {
            if (!gEngine) return;
            gEngine->setIsPlaying(playing);
        }

        ENGINE_API bool __stdcall GetIsPlaying()
        {
            if (!gEngine) return false;
            return gEngine->getIsPlaying();
        }

        // ============= Script Invocation =============

        ENGINE_API void __stdcall RunScriptMethod(const wchar_t* className, const wchar_t* methodName)
        {
            if (!gEngine || !className || !methodName) return;
            gEngine->getCoreHost().runScriptMethod(std::wstring(className), methodName);
        }

    } // extern "C"
} // namespace Bridge
