/*============================================================================================
* @file         eno_bridge_node.cpp
* @brief        Node management bridge — spawn, destroy, lookup.
*
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
*//*========================================================================================*/

#include "eno_bridge.hpp"

namespace Bridge
{
    extern "C"
    {
        ENGINE_API uint32_t __stdcall CreateNode(uint32_t type)
        {
            Editor::Node* node = gEngine->getEnoEditor().getSceneTree().create(Editor::NodeType(type));
            if (!node)
                return UINT32_MAX;

            return node->sceneObjectID_;
        }

        ENGINE_API void __stdcall FreeNode(uint32_t id)
        {
            if (!gEngine) return;

            auto& sceneTree = gEngine->getEnoEditor().getSceneTree();
            Editor::Node* node = sceneTree.getNodeBySceneObjectID(id);
            sceneTree.kill(id);
        }

        ENGINE_API uint32_t __stdcall GetNode(const char* name)
        {
            if (!gEngine) return UINT32_MAX;
            Editor::Node* node = gEngine->getEnoEditor().getSceneTree().getNodeByName(name);
            return node ? node->sceneObjectID_ : UINT32_MAX;
        }

        ENGINE_API uint32_t __stdcall GetChild(uint32_t parentID, uint32_t index)
        {
            if (!gEngine) return UINT32_MAX;
            auto& sceneTree = gEngine->getEnoEditor().getSceneTree();
            Editor::Node* parent = sceneTree.getNodeBySceneObjectID(parentID);
            if (!parent) return UINT32_MAX;
            Editor::Node* child = sceneTree.getChild(parent, index);
            return child ? child->sceneObjectID_ : UINT32_MAX;
        }

        ENGINE_API void __stdcall AddChild(uint32_t cid, uint32_t pid)
        {
            if (!gEngine) return;

            Editor::Node* pnode = gEngine->getEnoEditor().getSceneTree().getNodeBySceneObjectID(cid);
            Editor::Node* cnode = gEngine->getEnoEditor().getSceneTree().getNodeBySceneObjectID(pid);
            if (!cnode || !pnode)
                return;

            gEngine->getEnoEditor().getSceneTree().nest(cid, pid);
        }

        ENGINE_API void __stdcall Unparent(uint32_t id)
        {
            if (!gEngine) return;

            Editor::Node* cnode = gEngine->getEnoEditor().getSceneTree().getNodeBySceneObjectID(id);
            if (!cnode)
                return;

            gEngine->getEnoEditor().getSceneTree().unparent(id);
        }

        ENGINE_API void __stdcall Reparent(uint32_t cid, uint32_t pid)
        {
            if (!gEngine) return;

            Editor::Node* cnode = gEngine->getEnoEditor().getSceneTree().getNodeBySceneObjectID(cid);
            Editor::Node* pnode = gEngine->getEnoEditor().getSceneTree().getNodeBySceneObjectID(pid);
            if (!cnode || !pnode)
                return;

            gEngine->getEnoEditor().getSceneTree().reparent(cid, pid);
        }

        ENGINE_API uint32_t __stdcall GetParentNode(uint32_t id)
        {
            if (!gEngine) return UINT32_MAX;

            // Parent
            Editor::Node* node = gEngine->getEnoEditor().getSceneTree().getNodeBySceneObjectID(id);
            if (!node)
                return UINT32_MAX;

            if (node->parent_)
                return node->parent_->sceneObjectID_;
            else
                return UINT32_MAX;
        }

        ENGINE_API uint32_t __stdcall GetNodeType(uint32_t id)
        {
            if (!gEngine) return static_cast<uint32_t>(Editor::NodeType::EMPTY);
            Editor::Node* node = gEngine->getEnoEditor().getSceneTree().getNodeBySceneObjectID(id);
            if (!node) return static_cast<uint32_t>(Editor::NodeType::EMPTY);
            return static_cast<uint32_t>(node->type_);
        }

        // Fills outIDs with the sceneObjectIDs of all nodes whose NodeType matches nodeType.
        // Returns the total count written (capped at maxCount).
        ENGINE_API uint32_t __stdcall GetNodesByType(uint32_t nodeType, uint32_t* outIDs, uint32_t maxCount)
        {
            if (!gEngine || !outIDs) return 0;
            uint32_t count = 0;
            for (auto* node : gEngine->getEnoEditor().getSceneTree().getAllNodes())
            {
                if (node && static_cast<uint32_t>(node->type_) == nodeType)
                {
                    if (count < maxCount)
                        outIDs[count] = node->sceneObjectID_;
                    count++;
                }
            }
            return count;
        }

        ENGINE_API uint32_t __stdcall GetChildCount(uint32_t id)
        {
            if (!gEngine) return UINT32_MAX;

            Editor::Node* node = gEngine->getEnoEditor().getSceneTree().getNodeBySceneObjectID(id);
            if (!node)
                return static_cast<uint32_t>(Editor::NodeType::EMPTY);

            return static_cast<uint32_t>(node->children_.size());
        }

        ENGINE_API uint32_t __stdcall SpawnPrefab(const char* prefabName, float x, float y, uint32_t parentID)
        {
            if (!gEngine) return UINT32_MAX;

            auto& sceneTree = gEngine->getEnoEditor().getSceneTree();
            auto& entityMng = gEngine->getEntityManager();

            Editor::Node* parent = (parentID != UINT32_MAX)
                ? sceneTree.getNodeBySceneObjectID(parentID)
                : nullptr;

            Editor::Node* node = sceneTree.spawnFromPrefab(
                prefabName,
                parent,
                gEngine->getAnimationRegistry()
            );
            if (!node) return UINT32_MAX;

            // Place at the requested world position
            if (node->entity_.has<ECSComponents::TransformComponent>(entityMng))
            {
                auto& tc = node->entity_.get<ECSComponents::TransformComponent>(entityMng);
                tc.pos = { x, y };
            }

            // Call Begin() on the new script if the game is already running
            if (gEngine->getIsPlaying() && node->script_)
            {
                for (const auto& org : node->script_->origin())
                    gEngine->getCoreHost().begin(org, node->sceneObjectID_);
            }

            // Re-initialize animation controllers so the new node's textures and
            // animations are loaded on the next frame (spawnFromPrefab creates a
            // fresh empty controller that the dirty block hasn't seen yet).
            gEngine->markAnimationsDirty();

            return node->sceneObjectID_;
        }
    }
}
