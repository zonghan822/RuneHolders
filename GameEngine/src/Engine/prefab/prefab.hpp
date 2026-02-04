/*============================================================================================
* All content � 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         prefab.hpp
* @author       Yun Ji Koe (yunji.koe@digipen.edu)
* @date         01-10-2025
* @brief        Provides data structures and functions for saving, loading, and instantiating
*               prefabs. Prefabs are reusable templates that define attributes such as position,
*               size, color, shape, and collider information.
*===========================================================================================*/

#pragma once
#include <string>
#include <vector>
#include <stdexcept>
#include <unordered_map>
#include <set>
#include <iostream>
#include <fstream>

#include "../engine/eno_engine.hpp"
#include "../editor/undo/entity_snapshot.hpp"

// Forward declare EnoEngine its usable in function signatures
namespace Engine {
    class EnoEngine;
}

// Forward declare EnoSceneTree
namespace Editor {
    class EnoSceneTree;
}

struct PrefabData {
    std::string name;
    float x, y;        // position (legacy, kept for backward compatibility)
    float width, height; // legacy
    int color;         // legacy
    int shape;         // legacy
    bool hasCollider;  // legacy

    // New component-based data
    Editor::EntitySnapshot entityData;  // Full entity snapshot with all components (in-memory only)
    std::string entityJsonData;         // Serialized entity data to load from disk

    PrefabData() : name{ "Empty" }, x{ 250.0f }, y{ 250.0f }, width{ 30.f }, height{ 30.f }, color{ 0 }, shape{ 1 }, hasCollider{ 0 } {}  // shape: 0=Circle, 1=Rectangle

};

// Registry of all loaded prefabs
extern std::unordered_map<std::string, PrefabData> prefabRegistry;

// Instance tracking: maps entity IDs to their prefab name
extern std::unordered_map<uint32_t, std::string> prefabInstanceMap;

// Reverse mapping: prefab name to set of entity IDs
extern std::unordered_map<std::string, std::set<uint32_t>> prefabToInstancesMap;

// Prefabs
/*============================================================================================
 * \brief       Instantiates a prefab into the engine world (legacy version).
 * \param[in]   PrefabData const& prefab
 * \param[in]   Engine::EnoEngine& engine
 * \return      void
 *===========================================================================================*/
ECS::Entity instantiatePrefab(const PrefabData& prefab, ECS::EntityManager& entMng);

/*============================================================================================
 * \brief       Saves a prefab to persistent storage (legacy version).
 * \param[in]   PrefabData const& prefab
 * \return      void
 *===========================================================================================*/
void savePrefab(const PrefabData& prefab);

/*============================================================================================
 * \brief       Loads a prefab definition from persistent storage (legacy version).
 * \param[in]   std::string const& filename
 * \return      PrefabData
 *===========================================================================================*/
PrefabData loadPrefab(const std::string& filename);

/*============================================================================================
 * \brief       Creates a prefab from an existing entity.
 * \param[in]   ECS::Entity entity - The entity to create prefab from
 * \param[in]   std::string const& prefabName - Name for the prefab
 * \param[in]   ECS::EntityManager& entityManager
 * \param[in]   Editor::EnoSceneTree& sceneTree
 * \return      void
 *===========================================================================================*/
void createPrefabFromEntity(
    ECS::Entity entity,
    const std::string& prefabName,
    ECS::EntityManager& entityManager,
    Editor::EnoSceneTree& sceneTree
);

/*============================================================================================
 * \brief       Instantiates a prefab from entity snapshot data.
 * \param[in]   std::string const& prefabName - Name of the prefab to instantiate
 * \param[in]   ECS::EntityManager& entityManager
 * \param[in]   Editor::EnoSceneTree& sceneTree
 * \return      ECS::Entity - The created entity
 *===========================================================================================*/
ECS::Entity instantiatePrefabFromSnapshot(
    const std::string& prefabName,
    ECS::EntityManager& entityManager
);

/*============================================================================================
 * \brief       Updates all instances of a prefab with new component data.
 * \param[in]   std::string const& prefabName - Name of the prefab
 * \param[in]   ECS::Entity sourceEntity - Entity with the updated data
 * \param[in]   ECS::EntityManager& entityManager
 * \param[in]   Editor::EnoSceneTree& sceneTree
 * \return      int - Number of instances updated
 *===========================================================================================*/
int updateAllPrefabInstances(
    const std::string& prefabName,
    ECS::Entity sourceEntity,
    ECS::EntityManager& entityManager,
    Editor::EnoSceneTree& sceneTree
);

/*============================================================================================
 * \brief       Registers an entity as an instance of a prefab.
 * \param[in]   uint32_t entityID
 * \param[in]   std::string const& prefabName
 *===========================================================================================*/
void registerPrefabInstance(uint32_t entityID, const std::string& prefabName);

/*============================================================================================
 * \brief       Unregisters an entity from prefab instance tracking.
 * \param[in]   uint32_t entityID
 *===========================================================================================*/
void unregisterPrefabInstance(uint32_t entityID);




