/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
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
#include <iostream>
#include <fstream>

#include "../engine/eno_engine.hpp"

// Forward declare EnoEngine its usable in function signatures
namespace Engine {
    class EnoEngine;
}


struct PrefabData {
    std::string name;
    float x, y;        // position
    float width, height;
    int color;
    int shape;
    bool hasCollider;

    PrefabData() : name{ "Empty" }, x{ 250.0f }, y{ 250.0f }, width{ 30.f }, height{ 30.f }, color{ 0 }, shape{ 0 }, hasCollider{ 0 } {}

};

extern std::unordered_map<std::string, PrefabData> prefabRegistry;

// Prefabs
/*============================================================================================
 * \brief       Instantiates a prefab into the engine world.
 * \param[in]   PrefabData const& prefab
 * \param[in]   Engine::EnoEngine& engine
 * \return      void
 *===========================================================================================*/
ECS::Entity instantiatePrefab(const PrefabData& prefab, ECS::EntityManager& entMng);

/*============================================================================================
 * \brief       Saves a prefab to persistent storage.
 * \param[in]   PrefabData const& prefab  
 * \return      void
 *===========================================================================================*/
void savePrefab(const PrefabData& prefab);

/*============================================================================================
 * \brief       Loads a prefab definition from persistent storage.
 * \param[in]   std::string const& filename   
 * \return      PrefabData   
 *===========================================================================================*/
PrefabData loadPrefab(const std::string& filename);




/*============================================================================================
 * \brief       Loads a prefab definition from persistent storage.
 * \param[in]   std::string const& filename 
 * \return      PrefabData   
 *===========================================================================================*/
//PrefabData getPrefabList(const std::string& filename);




