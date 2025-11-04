/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         prefab.cpp
* @author       Yun Ji Koe (yunji.koe@digipen.edu)
* @date         01-10-2025
* @brief        Provides data structures and functions for saving, loading, and instantiating
*               prefabs. Prefabs are reusable templates that define attributes such as position,
*               size, color, shape, and collider information.
*===========================================================================================*/
#include "prefab.hpp"
#include "../engine/eno_engine.hpp"

std::unordered_map<std::string, PrefabData> prefabRegistry;



void savePrefab(const PrefabData& prefab) {
    std::ofstream file( "../../../assets/saves/" + prefab.name + ".txt");
    if (!file){
        throw std::runtime_error("file no open for write!!!");
    }

    file << prefab.name << "\n";
    file << prefab.x << " " << prefab.y << "\n";
    file << prefab.width << " " << prefab.height << "\n";
    file << prefab.color << " " << prefab.shape << "\n";
    file << prefab.hasCollider << "\n";


    std::cout << prefab.name;
}

PrefabData loadPrefab(const std::string& filename) {
    PrefabData prefab;
    std::ifstream file(filename);
    if (!file) {
        throw std::runtime_error("file no open for read!!!");
    }

    file >> prefab.name;
    file >> prefab.x >> prefab.y;
    file >> prefab.width >> prefab.height;
    file >> prefab.color >> prefab.shape;
    file >> prefab.hasCollider;
    return prefab;
}

ECS::Entity instantiatePrefab(const PrefabData& prefab, Engine::EnoEngine& engine) {

    ECS::Entity e = engine.getEntityManager().create<>();

    char name[50]{};
    size_t counter{};
    for (const char& a : prefab.name) {
        name[counter] = a;
        counter++;
    }

    //e.add<ECSComponents::NameComponent>(static_cast<char*>(name));
    e.add<ECSComponents::NameComponent>(prefab.name.c_str());

    // Calculate CENTER position from DrawInfo's top-left
    float centerX = prefab.x + (prefab.width * 0.5f);
    float centerY = prefab.y + (prefab.height * 0.5f);

    ECSComponents::TransformComponent transform{
            {centerX, centerY},
            {0.0f, 0.0f},
            {1.0f, 1.0f}
    };

    e.add<ECSComponents::TransformComponent>(transform);
    // add draw info
    ECSComponents::DrawInfoComponent DIC{
        prefab.x, 
        prefab.y, 
        prefab.width, 
        prefab.height,
        prefab.shape == 0 ? "Circle" : "Rectangle",
        prefab.color
    };

    e.add<ECSComponents::DrawInfoComponent>(DIC);
    std::cout << e.get<ECSComponents::DrawInfoComponent>() << "\n";

    if (prefab.hasCollider) {

        e.add<ECSComponents::CollisionComponent>(
            ECSComponents::CollisionComponent(prefab.width, prefab.height)
        );
    }
    return e;
}
