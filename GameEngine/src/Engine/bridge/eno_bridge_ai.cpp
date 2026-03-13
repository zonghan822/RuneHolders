/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         eno_bridge_ai.cpp
* @author       Muhammad Raziq Bin Mohd Rafig (muhammadraziq.b@digipen.edu)   50%
* @co-author    ZongHan Law (zonghan.law@digipen.edu)   50%
* @brief        AI bridge — exposes AIComponent fields and pathfinding to C# scripts.
*//*========================================================================================*/



#include "eno_bridge.hpp"
#include "../component/components/ai_component.hpp"
#include "../ai/pathfinding/ai_path_agent.hpp"

namespace Bridge
{
    static ECSComponents::AIComponent* getAI(uint32_t sceneObjectID)
    {
        if (!gEngine) return nullptr;
        Editor::Node* node = gEngine->getEnoEditor().getSceneTree().getNodeBySceneObjectID(sceneObjectID);
        if (!node) return nullptr;
        if (!node->entity_.has<ECSComponents::AIComponent>(gEngine->getEntityManager())) return nullptr;
        return &node->entity_.get<ECSComponents::AIComponent>(gEngine->getEntityManager());
    }

    static AI::AIPathAgent* getPathAgent(uint32_t sceneObjectID)
    {
        if (!gEngine) return nullptr;
        Editor::Node* node = gEngine->getEnoEditor().getSceneTree().getNodeBySceneObjectID(sceneObjectID);
        if (!node) return nullptr;
        try {
            return &gEngine->getAIManager().getPathAgent(node->entity_);
        } catch (...) {
            return nullptr;
        }
    }

    extern "C"
    {
        // ============= AIComponent fields =============

        ENGINE_API float __stdcall GetAwarenessRadius(uint32_t sceneObjectID)
        {
            auto* ai = getAI(sceneObjectID);
            return ai ? ai->awarenessRadius : 0.f;
        }

        ENGINE_API void __stdcall SetAwarenessRadius(uint32_t sceneObjectID, float value)
        {
            if (auto* ai = getAI(sceneObjectID))
                ai->awarenessRadius = value;
        }

        ENGINE_API float __stdcall GetChaseSpeed(uint32_t sceneObjectID)
        {
            auto* ai = getAI(sceneObjectID);
            return ai ? ai->chaseSpeed : 0.f;
        }

        ENGINE_API void __stdcall SetChaseSpeed(uint32_t sceneObjectID, float value)
        {
            if (auto* ai = getAI(sceneObjectID))
                ai->chaseSpeed = value;
        }

        ENGINE_API bool __stdcall GetAllowDiagonal(uint32_t sceneObjectID)
        {
            auto* ai = getAI(sceneObjectID);
            return ai ? ai->allowDiagonal : false;
        }

        ENGINE_API void __stdcall SetAllowDiagonal(uint32_t sceneObjectID, bool value)
        {
            if (auto* ai = getAI(sceneObjectID))
                ai->allowDiagonal = value;
        }

        // ============= Pathfinding =============

        // Compute a path from a world-space start to a world-space goal.
        // allowDiagonal is read automatically from the entity's AIComponent.
        ENGINE_API void __stdcall AI_ComputePath(
            uint32_t sceneObjectID,
            float startX, float startY,
            float goalX,  float goalY)
        {
            auto* agent = getPathAgent(sceneObjectID);
            if (!agent) return;

            auto* ai = getAI(sceneObjectID);
            bool diag = ai ? ai->allowDiagonal : false;

            auto& pf = gEngine->getPathfinder();
            auto start = pf.worldToTile({ startX, startY });
            auto goal  = pf.worldToTile({ goalX,  goalY  });
            agent->computePath(start, goal, diag);
        }

        // Clear the stored path.
        ENGINE_API void __stdcall AI_ClearPath(uint32_t sceneObjectID)
        {
            if (auto* agent = getPathAgent(sceneObjectID))
                agent->clear();
        }

        // Returns true when there is at least one unvisited tile remaining.
        ENGINE_API bool __stdcall AI_HasPath(uint32_t sceneObjectID)
        {
            auto* agent = getPathAgent(sceneObjectID);
            return agent && agent->hasPath();
        }

        // Step to the next tile in the path.
        ENGINE_API void __stdcall AI_Advance(uint32_t sceneObjectID)
        {
            if (auto* agent = getPathAgent(sceneObjectID))
                agent->advance();
        }

        // World-space X of the current waypoint tile centre.
        ENGINE_API float __stdcall AI_GetNextWaypointX(uint32_t sceneObjectID)
        {
            auto* agent = getPathAgent(sceneObjectID);
            if (!agent || !agent->hasPath()) return 0.f;
            return gEngine->getPathfinder().tileToWorld(agent->getCurrentTile()).x;
        }

        // World-space Y of the current waypoint tile centre.
        ENGINE_API float __stdcall AI_GetNextWaypointY(uint32_t sceneObjectID)
        {
            auto* agent = getPathAgent(sceneObjectID);
            if (!agent || !agent->hasPath()) return 0.f;
            return gEngine->getPathfinder().tileToWorld(agent->getCurrentTile()).y;
        }

        // Convert a world-space position to its tile column (X).
        ENGINE_API int __stdcall AI_WorldToTileX(float worldX, float worldY)
        {
            if (!gEngine) return -1;
            return gEngine->getPathfinder().worldToTile({ worldX, worldY }).x;
        }

        // Convert a world-space position to its tile row (Y).
        ENGINE_API int __stdcall AI_WorldToTileY(float worldX, float worldY)
        {
            if (!gEngine) return -1;
            return gEngine->getPathfinder().worldToTile({ worldX, worldY }).y;
        }

        // Convert a tile column/row to the world-space centre X.
        ENGINE_API float __stdcall AI_TileToWorldX(int tileX, int tileY)
        {
            if (!gEngine) return 0.f;
            return gEngine->getPathfinder().tileToWorld({ tileX, tileY }).x;
        }

        // Convert a tile column/row to the world-space centre Y.
        ENGINE_API float __stdcall AI_TileToWorldY(int tileX, int tileY)
        {
            if (!gEngine) return 0.f;
            return gEngine->getPathfinder().tileToWorld({ tileX, tileY }).y;
        }
    }
}
