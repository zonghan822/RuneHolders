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
//After Loading, can use this to call
//prefabRegistry[prefab.name] = prefab;


void instantiatePrefab(const PrefabData& prefab, Engine::EnoEngine& engine) {

    ECS::Entity e = engine.getEntityManager().create<>();

    char name[50]{};
    size_t counter{};
    for (const char& a : prefab.name) {
        name[counter] = a;
        counter++;
    }

    e.add<ECSComponents::NameComponent>(static_cast<const char*>(name));
    //std::cout << "name of this entity: " << e.get<ECSComponents::NameComponent>().name << "\n";


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

    //add collider
    if (prefab.hasCollider) {
        e.add<Collision::CollisionComponent>(
            Collision::CollisionComponent(prefab.width, prefab.height)
        );
    }
}
