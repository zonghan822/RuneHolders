#include "prefab.hpp"
#include "../engine/eno_engine.hpp"

std::unordered_map<std::string, PrefabData> prefabRegistry;

void savePrefab(const PrefabData& prefab) {
    std::ofstream file(prefab.name + ".txt");
    if (!file){
        throw std::runtime_error("file no open!!!");
    }

    file << prefab.name << "\n";
    file << prefab.x << " " << prefab.y << "\n";
    file << prefab.width << " " << prefab.height << "\n";
    file << prefab.color << " " << prefab.shape << "\n";
    file << prefab.hasCollider << "\n";
}

PrefabData loadPrefab(const std::string& filename) {
    PrefabData prefab;
    std::ifstream file(filename);
    file >> prefab.name;
    file >> prefab.x >> prefab.y;
    file >> prefab.width >> prefab.height;
    file >> prefab.color >> prefab.shape;
    file >> prefab.hasCollider;
    return prefab;
}
//After Loading, can use this to call
//prefabRegistry[prefab.name] = prefab;


void instantiatePrefab(const PrefabData& prefab, Engine::EnoEngine& engine) {

    ECS::Entity e = engine.getEntityManager().create<>();

    // add transform
    ECSComponents::TransformComponent t{
        {prefab.x, prefab.y, 0.0f},             //x,y,z
        {0,0,0},                                //rotation
        {1,1,1}                                 //scale
    };
    e.add<ECSComponents::TransformComponent>(t);

    // add draw info
    ECSComponents::DrawInfoComponent DIC{
        prefab.x, prefab.y, 
        prefab.width, prefab.height,
        prefab.shape == 0 ? "Circle" : "Rectangle",
        prefab.color
    };
    e.add<ECSComponents::DrawInfoComponent>(DIC);

    if (prefab.hasCollider) {
        e.add<Collision::CollisionComponent>(
            Collision::CollisionComponent(prefab.width, prefab.height)
        );
    }
}
