#pragma once
#include <string>
#include <vector>
#include <stdexcept>
#include <unordered_map>
#include <iostream>
#include <fstream>


struct PrefabData {
    std::string name;
    float x, y;        // position
    float width, height;
    int color;
    int shape;
    bool hasCollider;


};

extern std::unordered_map<std::string, PrefabData> prefabRegistry;
void savePrefab(const PrefabData& prefab);
PrefabData loadPrefab(const std::string& filename);