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
#include "../component/components/drawInfo.hpp"
#include "../graphics/animation/animation.hpp"
#include "../file/file_system.hpp"
#include "../graphics/graphics.hpp"
#include <vector>
#include <cmath>

// gEngine is defined in eno_bridge.cpp
extern Engine::EnoEngine* gEngine;

// -----------------------------------------------------------------------
// Line segment queue: C# fills this each frame via AddRuneLine(),
// game_logic.cpp drains it via DrawQueuedRuneLines() during render.
// -----------------------------------------------------------------------
static std::vector<RuneLineSegment> g_runeLines;

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

        // ============= Mouse Buttons =============

        ENGINE_API bool __stdcall IsMouseHeld(int button)
        {
            if (!gEngine) return false;
            return gEngine->getInputManager().isMousePressed(button);
        }

        ENGINE_API bool __stdcall IsMousePressed(int button)
        {
            if (!gEngine) return false;
            return gEngine->getInputManager().isMouseDown(button);
        }

        ENGINE_API bool __stdcall IsMouseReleased(int button)
        {
            if (!gEngine) return false;
            return gEngine->getInputManager().isMouseUp(button);
        }

        // ============= Rune Line Rendering =============

        ENGINE_API void __stdcall ClearRuneLines()
        {
            g_runeLines.clear();
        }

        ENGINE_API void __stdcall AddRuneLine(float x1, float y1, float x2, float y2,
                                              float r, float g, float b, float a, float width)
        {
            g_runeLines.push_back({ x1, y1, x2, y2, r, g, b, a, width });
        }

        ENGINE_API void __stdcall DrawQueuedRuneLines()
        {
            if (!gEngine || g_runeLines.empty()) return;
            auto& graphics = gEngine->getGraphicsManager();

            for (const auto& seg : g_runeLines) {
                float dx = seg.x2 - seg.x1;
                float dy = seg.y2 - seg.y1;
                float length = std::sqrt(dx * dx + dy * dy);
                if (length < 0.1f) continue;

                float angle = std::atan2(dy, dx);
                Math::eVec2<float> topLeft = {
                    (seg.x1 + seg.x2) * 0.5f - length * 0.5f,
                    (seg.y1 + seg.y2) * 0.5f - seg.width * 0.5f
                };

                Graphics::Color color(seg.r, seg.g, seg.b, seg.a);
                graphics.drawRect(topLeft, { length, seg.width }, color, true, angle);
            }
        }

        // ============= Entity Draw Color =============

        ENGINE_API bool __stdcall SetEntityDrawColor(uint32_t entityID, float r, float g, float b, float a)
        {
            if (!gEngine) return false;
            auto& sceneTree = gEngine->getEnoEditor().getSceneTree();
            auto& entityMng = gEngine->getEntityManager();
            Editor::Node* node = sceneTree.getNodeBySceneObjectID(entityID);
            if (!node) return false;
            if (!node->entity_.has<ECSComponents::DrawInfoComponent>(entityMng)) return false;
            auto& draw = node->entity_.get<ECSComponents::DrawInfoComponent>(entityMng);
            draw.color = Graphics::Color(r, g, b, a);
            return true;
        }

        // ============= Entity Queries (name substring) =============

        ENGINE_API int __stdcall FindAllEntitiesContainingName(const wchar_t* nameSubstr, uint32_t* outIDs, int maxCount)
        {
            if (!gEngine || !nameSubstr || !outIDs || maxCount <= 0) return 0;
            std::wstring wsubstr(nameSubstr);
            std::string  ssubstr(wsubstr.begin(), wsubstr.end());

            auto& sceneTree = gEngine->getEnoEditor().getSceneTree();
            auto& entityMng = gEngine->getEntityManager();

            int count = 0;
            for (auto* node : sceneTree.getAllNodes()) {
                if (!node || count >= maxCount) break;
                if (node->entity_.has<ECSComponents::NameComponent>(entityMng)) {
                    auto& nc = node->entity_.get<ECSComponents::NameComponent>(entityMng);
                    if (std::string(nc.name).find(ssubstr) != std::string::npos) {
                        outIDs[count++] = node->sceneObjectID_;
                    }
                }
            }
            return count;
        }

        // ============= Spell Spawning =============
        // spellType: 1=ICE, 2=FIRE, 3=LIGHT
        // Returns sceneObjectID of spawned entity so C# can track it; 0 on failure.

        ENGINE_API uint32_t __stdcall SpawnRuptureMark(float x, float y, int spellType)
        {
            if (!gEngine || spellType < 1 || spellType > 3) return 0;

            const char* spawnSheetName   = "";
            const char* windupSheetName  = "";
            const char* explodeSheetName = "";

            switch (spellType) {
            case 1: // ICE
                spawnSheetName   = "RuptureMark-Ice-Spawn-Sheet";
                windupSheetName  = "RuptureMark-Ice-Windup-Sheet";
                explodeSheetName = "RuptureMark-Ice-Explosion-Sheet";
                break;
            case 2: // FIRE
                spawnSheetName   = "RuptureMark-Fire-Spawn-Sheet";
                windupSheetName  = "RuptureMark-Fire-Windup-Sheet";
                explodeSheetName = "RuptureMark-Fire-Explosion-Sheet";
                break;
            case 3: // LIGHT
                spawnSheetName   = "RuptureMark-Light-Spawn-Sheet";
                windupSheetName  = "RuptureMark-Light-Windup-Sheet";
                explodeSheetName = "RuptureMark-Light-Explosion-Sheet";
                break;
            }

            auto& entityMng      = gEngine->getEntityManager();
            auto& resourceLoader = gEngine->getResourceLoader();
            auto& sceneTree      = gEngine->getEnoEditor().getSceneTree();

            auto texture = resourceLoader.getTexture(spawnSheetName);

            ECS::Entity mark = entityMng.create();
            mark.add<ECSComponents::NameComponent>(entityMng, ECSComponents::NameComponent("RuptureMark"));
            mark.add<ECSComponents::TransformComponent>(entityMng);
            mark.add<ECSComponents::DrawInfoComponent>(entityMng);
            mark.add<ECSComponents::AnimationControllerComponent>(entityMng);

            auto& tf = mark.get<ECSComponents::TransformComponent>(entityMng);
            tf.pos   = { x, y };
            tf.scale = { 1.0f, 1.0f };

            auto& draw = mark.get<ECSComponents::DrawInfoComponent>(entityMng);
            draw.width   = 128.0f;
            draw.height  = 128.0f;
            draw.color   = Graphics::Color::White;
            strcpy_s(draw.shape, "Rectangle");
            draw.texture = texture;

            auto& animComp = mark.get<ECSComponents::AnimationControllerComponent>(entityMng);
            animComp.setAnimationData("spawn",     spawnSheetName,   128, 128, 1, 8, 0.1f, false);
            animComp.setAnimationData("windup",    windupSheetName,  128, 128, 1, 5, 0.1f, false);
            animComp.setAnimationData("explosion", explodeSheetName, 128, 128, 1, 4, 0.1f, false);
            strcpy_s(animComp.currentAnimationName, "spawn");

            // Register in scene tree so bridge functions (FindEntityByName, SetEntityAnimation, etc.) can find it
            Editor::Node* node = sceneTree.createFromEntity(mark);
            if (!node) return 0;

            return node->sceneObjectID_;
        }

    } // extern "C"
} // namespace Bridge
