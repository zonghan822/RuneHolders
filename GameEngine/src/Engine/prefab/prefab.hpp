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


};

extern std::unordered_map<std::string, PrefabData> prefabRegistry;
// Prefabs
void instantiatePrefab(const PrefabData& prefab, Engine::EnoEngine& engine);
void savePrefab(const PrefabData& prefab);
PrefabData loadPrefab(const std::string& filename);