/*============================================================================================
* All content � 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         prefab.cpp
* @author       Yun Ji Koe (yunji.koe@digipen.edu)
* @date         01-10-2025
* @brief        Provides data structures and functions for saving, loading, and instantiating
*               prefabs. Prefabs are reusable templates that define attributes such as position,
*               size, color, shape, and collider information.
*===========================================================================================*/
#include "prefab.hpp"
#include "../engine/eno_engine.hpp"
#include "../editor/eno_scene_tree.hpp"
#include <filesystem>
#include <sstream>

std::unordered_map<std::string, PrefabData> prefabRegistry;
std::unordered_map<uint32_t, std::string> prefabInstanceMap;
std::unordered_map<std::string, std::set<uint32_t>> prefabToInstancesMap;



void savePrefab(const PrefabData& prefab) {
    std::string prefabDir = FileSystem::GetAssetsDir().string() + "/saves/prefabs/";

    // Create directory if it doesn't exist
    std::filesystem::create_directories(prefabDir);

    std::string filename = prefabDir + prefab.name + ".txt";
    std::ofstream file(filename);

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

    // Check if it's a JSON file (new format) or legacy format
    std::string firstLine;
    std::getline(file, firstLine);

    // Trim whitespace safely
    size_t start = firstLine.find_first_not_of(" \t\n\r");
    if (start == std::string::npos) {
        // Empty line, treat as legacy format
        file.seekg(0);
        file >> prefab.name;
        file >> prefab.x >> prefab.y;
        file >> prefab.width >> prefab.height;
        file >> prefab.color >> prefab.shape;
        file >> prefab.hasCollider;
        std::cout << "[Prefab] Loaded legacy prefab: " << prefab.name << "\n";
        return prefab;
    }

    size_t end = firstLine.find_last_not_of(" \t\n\r");
    firstLine = firstLine.substr(start, end - start + 1);

    if (firstLine == "{") {
        // New JSON format - read the entire file to get all data
        file.seekg(0);
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string fileContent = buffer.str();

        // Parse prefab name from the header section
        bool foundName = false;
        size_t namePos = fileContent.find("\"PrefabName\"");
        if (namePos != std::string::npos) {
            size_t startQuote = fileContent.find("\"", namePos + 13);
            size_t endQuote = fileContent.find("\"", startQuote + 1);
            if (startQuote != std::string::npos && endQuote != std::string::npos) {
                prefab.name = fileContent.substr(startQuote + 1, endQuote - startQuote - 1);
                foundName = true;
            }
        }

        // If PrefabName not found in JSON, extract from filename
        if (!foundName) {
            size_t lastSlash = filename.find_last_of("/\\");
            size_t lastDot = filename.find_last_of(".");
            if (lastSlash != std::string::npos && lastDot != std::string::npos && lastDot > lastSlash) {
                prefab.name = filename.substr(lastSlash + 1, lastDot - lastSlash - 1);
            } else {
                throw std::runtime_error("Could not parse prefab name from JSON file or filename!");
            }
        }

        // Look for "Entity Data" section and store it for later instantiation
        size_t entityDataPos = fileContent.find("\"Entity Data\"");
        if (entityDataPos != std::string::npos) {
            // Store everything from "Entity Data" onwards
            prefab.entityJsonData = fileContent.substr(entityDataPos);
            std::cout << "[Prefab] Loaded JSON prefab with entity data: " << prefab.name << "\n";
        } else {
            std::cout << "[Prefab] Loaded JSON prefab (no entity data): " << prefab.name << "\n";
        }

        return prefab;
    }
    else {
        // Legacy format - parse as before
        file.seekg(0); // Reset to beginning
        file >> prefab.name;
        file >> prefab.x >> prefab.y;
        file >> prefab.width >> prefab.height;
        file >> prefab.color >> prefab.shape;
        file >> prefab.hasCollider;
        std::cout << "[Prefab] Loaded legacy prefab: " << prefab.name << "\n";
        return prefab;
    }
}

ECS::Entity instantiatePrefab(const PrefabData& prefab, ECS::EntityManager& entMng) {

    ECS::Entity e = entMng.create<>();

    // Create unique name by appending entity ID
    std::string uniqueName = prefab.name + " #" + std::to_string(e.id());

    e.add<ECSComponents::NameComponent>(entMng, uniqueName.c_str());
    //std::cout << "name of this entity: " << e.get<ECSComponents::NameComponent>().name << "\n;

    // Calculate CENTER position from DrawInfo's top-left
    float centerX = prefab.x + (prefab.width * 0.5f);
    float centerY = prefab.y + (prefab.height * 0.5f);

    ECSComponents::TransformComponent transform{
            {centerX, centerY},
            {0.0f, 0.0f},
            {1.0f, 1.0f}
    };

    e.add<ECSComponents::TransformComponent>(entMng, transform);
    // add draw info - RenderComponent only takes width, height, shape name, and color
    // Position comes from TransformComponent now
    ECSComponents::DrawInfoComponent DIC{
        prefab.width,
        prefab.height,
        prefab.shape == 0 ? "Circle" : "Rectangle",
        prefab.color
    };

    e.add<ECSComponents::DrawInfoComponent>(entMng, DIC);
    std::cout << e.get<ECSComponents::DrawInfoComponent>(entMng) << "\n";

    if (prefab.hasCollider) {

        e.add<ECSComponents::CollisionComponent>(
            entMng,
            ECSComponents::CollisionComponent(prefab.width, prefab.height)
        );
    }

    std::cout << "[Legacy] Prefab name :" << prefab.name << std::endl;
    // Button component
    if (prefab.name == "EXIT_BUTTON")
    {
        ECSComponents::VisibilityComponent visibility;
        visibility.visible = true;
        e.add<ECSComponents::VisibilityComponent>(entMng, visibility);

        e.add<ECSComponents::ButtonComponent>(entMng);
    }

    return e;
}

/*============================================================================================
 * NEW PREFAB SYSTEM FUNCTIONS
 *===========================================================================================*/

void registerPrefabInstance(uint32_t entityID, const std::string& prefabName)
{
    prefabInstanceMap[entityID] = prefabName;
    prefabToInstancesMap[prefabName].insert(entityID);
    std::cout << "[Prefab] Registered entity " << entityID << " as instance of " << prefabName << "\n";
}

void unregisterPrefabInstance(uint32_t entityID)
{
    auto it = prefabInstanceMap.find(entityID);
    if (it != prefabInstanceMap.end())
    {
        std::string prefabName = it->second;
        prefabInstanceMap.erase(it);

        auto instancesIt = prefabToInstancesMap.find(prefabName);
        if (instancesIt != prefabToInstancesMap.end())
        {
            instancesIt->second.erase(entityID);
        }

        std::cout << "[Prefab] Unregistered entity " << entityID << " from " << prefabName << "\n";
    }
}

void createPrefabFromEntity(
    ECS::Entity entity,
    const std::string& prefabName,
    ECS::EntityManager& entityManager,
    Editor::EnoSceneTree& sceneTree)
{
    std::cout << "[Prefab] Creating prefab '" << prefabName << "' from entity " << entity.id() << "\n";

    // Create a snapshot of the entity
    Editor::EntitySnapshot snapshot = Editor::EntitySerializer::captureEntity(
        entity,
        entityManager,
        sceneTree
    );

    // Create or update the prefab data
    PrefabData& prefab = prefabRegistry[prefabName];
    prefab.name = prefabName;
    prefab.entityData = snapshot;

    // Also populate legacy fields for backward compatibility if DrawInfo exists
    for (const auto& compSnapshot : snapshot.components)
    {
        if (compSnapshot.type == typeid(ECSComponents::DrawInfoComponent))
        {
            auto& drawInfo = compSnapshot.get<ECSComponents::DrawInfoComponent>();
            prefab.width = drawInfo.width;
            prefab.height = drawInfo.height;
            prefab.shape = (strcmp(drawInfo.shape, "Circle") == 0) ? 0 : 1;
            // Color is stored as RGBA, but legacy uses int enum
            if (drawInfo.color.r == 0.0f && drawInfo.color.g == 1.0f)
                prefab.color = 0; // Green
            else if (drawInfo.color.r == 1.0f && drawInfo.color.g == 0.0f)
                prefab.color = 1; // Red
        }
        else if (compSnapshot.type == typeid(ECSComponents::TransformComponent))
        {
            auto& transform = compSnapshot.get<ECSComponents::TransformComponent>();
            prefab.x = transform.pos.x;
            prefab.y = transform.pos.y;
        }
        else if (compSnapshot.type == typeid(ECSComponents::CollisionComponent))
        {
            prefab.hasCollider = true;
        }
    }

    // Save to file using proper component serialization
    std::string prefabDir = FileSystem::GetAssetsDir().string() + "/saves/prefabs/";

    // Create directory if it doesn't exist
    std::filesystem::create_directories(prefabDir);

    std::string filename = prefabDir + prefabName + ".txt";
    std::ofstream file(filename);

    if (!file)
    {
        throw std::runtime_error("[Prefab] Failed to open file for writing: " + filename);
    }

    // Write prefab header
    file << "{\n";
    file << "\t\"PrefabName\": \"" << prefabName << "\",\n";
    file << "\t\"ComponentCount\": " << snapshot.components.size() << "\n";
    file << "}\n\n";

    // Write entity data in the same format as EntityManager::saveJson()
    file << "\"Entity Data\"\n";
    file << "{\n";

    bool firstComponent = true;
    for (const auto& compSnapshot : snapshot.components)
    {
        if (!firstComponent)
            file << "\n";
        firstComponent = false;

        // Use DeduceCompType to get the actual component type and serialize using reflect()
        ECSComponents::EnoComponentRegistry::DeduceCompType(compSnapshot.type, [&]<typename T>()
        {
            const T& component = compSnapshot.get<T>();

            // Write component header
            file << "\t[" << compSnapshot.type.name() << "]\n";
            file << "\t{\n";

            // Use reflect to write all fields
            component.reflect([&](const std::string& name, const auto& value, const std::string& type)
            {
                file << "\t\t\"" << name << "\": <" << type << "> { " << value << " }\n";
            });

            file << "\t}\n";
        });
    }

    file << "}\n";
    file.close();

    std::cout << "[Prefab] Saved prefab '" << prefabName << "' to " << filename << "\n";
    std::cout << "[Prefab] Captured " << snapshot.components.size() << " components\n";
}

ECS::Entity instantiatePrefabFromSnapshot(
    const std::string& prefabName,
    ECS::EntityManager& entityManager)
{
    std::cout << "[Prefab] Instantiating prefab '" << prefabName << "' from snapshot\n";

    // Find the prefab in the registry
    auto it = prefabRegistry.find(prefabName);
    if (it == prefabRegistry.end())
    {
        std::cerr << "[Prefab] Prefab '" << prefabName << "' not found in registry!\n";
        throw std::runtime_error("Prefab not found: " + prefabName);
    }

    const PrefabData& prefab = it->second;

    // If the prefab has entity snapshot data, use it
    if (!prefab.entityData.components.empty())
    {
        std::cout << "[Prefab] Using new snapshot-based instantiation\n";

        // Create new entity
        ECS::Entity newEntity = entityManager.create<>();

        // Restore all components from snapshot
        for (const auto& compSnapshot : prefab.entityData.components)
        {
            // Skip PrefabInstanceComponent if it exists in the snapshot
            if (compSnapshot.type == typeid(ECSComponents::PrefabInstanceComponent))
                continue;

            Editor::EntitySerializer::restoreComponent(newEntity, compSnapshot, entityManager);
        }

        // After restoring components, load textures if needed
        if (newEntity.has<ECSComponents::RenderComponent>(entityManager))
        {
            auto& renderComp = newEntity.get<ECSComponents::RenderComponent>(entityManager);

            // If texturePath is set but texture is null, load the texture
            if (renderComp.texturePath[0] != '\0' && !renderComp.texture)
            {
                auto tex = std::make_shared<Graphics::EnoTexture>();
                std::string fullpath = FileSystem::GetAssetsDir().string() + "/art/" + std::string(renderComp.texturePath) + ".png";

                if (tex->loadFromFile(fullpath))
                {
                    renderComp.texture = tex;
                    std::cout << "[Prefab] Loaded texture: " << renderComp.texturePath << "\n";
                }
                else
                {
                    std::cerr << "[Prefab] Failed to load texture: " << fullpath << "\n";
                }
            }
        }

        // Add PrefabInstanceComponent to track which prefab this came from
        newEntity.add<ECSComponents::PrefabInstanceComponent>(entityManager, prefabName.c_str());

        // Ensure the entity has a NameComponent with a unique name
        std::string uniqueName = prefabName + " #" + std::to_string(newEntity.id());
        if (newEntity.has<ECSComponents::NameComponent>(entityManager))
        {
            // Update existing NameComponent
            auto& nameComp = newEntity.get<ECSComponents::NameComponent>(entityManager);
            std::strncpy(nameComp.name, uniqueName.c_str(), sizeof(nameComp.name) - 1);
            nameComp.name[sizeof(nameComp.name) - 1] = '\0';
        }
        else
        {
            // Add NameComponent if it doesn't exist
            newEntity.add<ECSComponents::NameComponent>(entityManager, uniqueName.c_str());
        }

        // Register this entity as an instance of the prefab
        registerPrefabInstance(newEntity.id(), prefabName);

        std::cout << "[Prefab] Created entity " << newEntity.id() << " from prefab '" << prefabName << "'\n";
        return newEntity;
    }
    else if (!prefab.entityJsonData.empty())
    {
        // Use persisted JSON data (loaded from disk)
        std::cout << "[Prefab] Using persisted JSON data for instantiation\n";

        // Helper function to find matching brace
        auto findMatchingBrace = [](const std::string& data, const size_t& openBrace) -> size_t
        {
            int depth = 1;
            for (size_t i = openBrace + 1; i < data.size(); ++i)
            {
                if (data[i] == '{') depth++;
                else if (data[i] == '}') depth--;
                if (depth == 0) return i + 1;
            }
            return std::string::npos;
        };

        // Create entity using loadJson
        ECS::Entity newEntity;
        entityManager.loadJson(
            entityManager,
            prefab.entityJsonData,
            newEntity,
            0,
            prefab.entityJsonData.size(),
            findMatchingBrace
        );

        // Add PrefabInstanceComponent to track which prefab this came from
        if (!newEntity.has<ECSComponents::PrefabInstanceComponent>(entityManager))
        {
            newEntity.add<ECSComponents::PrefabInstanceComponent>(entityManager, prefabName.c_str());
        }

        // Ensure the entity has a NameComponent with a unique name
        std::string uniqueName = prefabName + " #" + std::to_string(newEntity.id());
        if (newEntity.has<ECSComponents::NameComponent>(entityManager))
        {
            auto& nameComp = newEntity.get<ECSComponents::NameComponent>(entityManager);
            std::strncpy(nameComp.name, uniqueName.c_str(), sizeof(nameComp.name) - 1);
            nameComp.name[sizeof(nameComp.name) - 1] = '\0';
        }
        else
        {
            newEntity.add<ECSComponents::NameComponent>(entityManager, uniqueName.c_str());
        }

        // Register this entity as an instance of the prefab
        registerPrefabInstance(newEntity.id(), prefabName);

        std::cout << "[Prefab] Created entity " << newEntity.id() << " from prefab JSON data '" << prefabName << "'\n";
        return newEntity;
    }
    else
    {
        std::cout << "[Prefab] Falling back to legacy instantiation\n";
        // Fall back to legacy instantiation
        ECS::Entity entity = instantiatePrefab(prefab, entityManager);

        // Add PrefabInstanceComponent
        entity.add<ECSComponents::PrefabInstanceComponent>(entityManager, prefabName.c_str());

        // Register instance
        registerPrefabInstance(entity.id(), prefabName);

        return entity;
    }
}

int updateAllPrefabInstances(
    const std::string& prefabName,
    ECS::Entity sourceEntity,
    ECS::EntityManager& entityManager,
    Editor::EnoSceneTree& sceneTree)
{
    std::cout << "[Prefab] Updating all instances of prefab '" << prefabName << "'\n";
    std::cout << "[Prefab] DEBUG: prefabToInstancesMap size = " << prefabToInstancesMap.size() << "\n";
    std::cout << "[Prefab] DEBUG: prefabInstanceMap size = " << prefabInstanceMap.size() << "\n";

    // Debug: print all tracked prefabs
    std::cout << "[Prefab] DEBUG: Tracked prefabs in prefabToInstancesMap:\n";
    for (const auto& [name, instances] : prefabToInstancesMap)
    {
        std::cout << "  - " << name << ": " << instances.size() << " instances\n";
    }

    // First, update the prefab template itself
    createPrefabFromEntity(sourceEntity, prefabName, entityManager, sceneTree);

    // Find all instances of this prefab
    auto it = prefabToInstancesMap.find(prefabName);
    if (it == prefabToInstancesMap.end() || it->second.empty())
    {
        std::cout << "[Prefab] No instances found for prefab '" << prefabName << "'\n";
        std::cout << "[Prefab] DEBUG: Checking if any entities have PrefabInstanceComponent with this name...\n";

        // Scan all entities to find unreg istered instances
        int foundUnregistered = 0;
        for (auto& entity : entityManager.findEntity<ECSComponents::PrefabInstanceComponent>())
        {
            auto& comp = entity.get<ECSComponents::PrefabInstanceComponent>(entityManager);
            std::cout << "[Prefab] DEBUG: Found entity " << entity.id() << " with prefab name: '" << comp.prefabName << "'\n";
            if (std::string(comp.prefabName) == prefabName)
            {
                foundUnregistered++;
                std::cout << "[Prefab] DEBUG: Found unregistered instance! Registering entity " << entity.id() << "\n";
                registerPrefabInstance(entity.id(), prefabName);
            }
        }

        if (foundUnregistered > 0)
        {
            std::cout << "[Prefab] Found and registered " << foundUnregistered << " unregistered instances. Retrying update...\n";
            // Retry the lookup
            it = prefabToInstancesMap.find(prefabName);
            if (it == prefabToInstancesMap.end() || it->second.empty())
            {
                std::cout << "[Prefab] Still no instances after registration!\n";
                return 0;
            }
        }
        else
        {
            return 0;
        }
    }

    // Get the updated prefab data
    const PrefabData& updatedPrefab = prefabRegistry[prefabName];

    int updateCount = 0;
    std::vector<uint32_t> instancesToUpdate(it->second.begin(), it->second.end());

    for (uint32_t instanceID : instancesToUpdate)
    {
        // Skip the source entity
        if (instanceID == sourceEntity.id())
            continue;

        // Check if entity still exists in the entity map
        auto& entityMap = entityManager.getMap();
        if (entityMap.find(instanceID) == entityMap.end())
        {
            std::cout << "[Prefab] Entity " << instanceID << " no longer valid, skipping\n";
            unregisterPrefabInstance(instanceID);
            continue;
        }

        ECS::Entity instanceEntity(instanceID);

        std::cout << "[Prefab] Updating entity " << instanceID << "\n";

        // Store the current transform position to preserve instance-specific placement
        Math::eVec2<float> preservedPosition{};
        bool hasTransform = instanceEntity.has<ECSComponents::TransformComponent>(entityManager);
        if (hasTransform)
        {
            auto& transform = instanceEntity.get<ECSComponents::TransformComponent>(entityManager);
            preservedPosition = transform.pos;
        }

        // Update all components from the prefab snapshot
        for (const auto& compSnapshot : updatedPrefab.entityData.components)
        {
            // Skip name and prefab instance components
            if (compSnapshot.type == typeid(ECSComponents::NameComponent) ||
                compSnapshot.type == typeid(ECSComponents::PrefabInstanceComponent))
                continue;

            // Update existing components or add new ones
            // We need to check if the entity has the component first
            if (compSnapshot.type == typeid(ECSComponents::TransformComponent))
            {
                auto& comp = compSnapshot.get<ECSComponents::TransformComponent>();
                if (instanceEntity.has<ECSComponents::TransformComponent>(entityManager))
                    instanceEntity.set<ECSComponents::TransformComponent>(entityManager, comp);
                else
                    instanceEntity.add<ECSComponents::TransformComponent>(entityManager, comp);
            }
            else if (compSnapshot.type == typeid(ECSComponents::DrawInfoComponent))
            {
                auto& comp = compSnapshot.get<ECSComponents::DrawInfoComponent>();
                if (instanceEntity.has<ECSComponents::DrawInfoComponent>(entityManager))
                    instanceEntity.set<ECSComponents::DrawInfoComponent>(entityManager, comp);
                else
                    instanceEntity.add<ECSComponents::DrawInfoComponent>(entityManager, comp);
            }
            else if (compSnapshot.type == typeid(ECSComponents::CollisionComponent))
            {
                auto& comp = compSnapshot.get<ECSComponents::CollisionComponent>();
                if (instanceEntity.has<ECSComponents::CollisionComponent>(entityManager))
                    instanceEntity.set<ECSComponents::CollisionComponent>(entityManager, comp);
                else
                    instanceEntity.add<ECSComponents::CollisionComponent>(entityManager, comp);
            }
            else if (compSnapshot.type == typeid(ECSComponents::PhysicsComponent))
            {
                auto& comp = compSnapshot.get<ECSComponents::PhysicsComponent>();
                if (instanceEntity.has<ECSComponents::PhysicsComponent>(entityManager))
                    instanceEntity.set<ECSComponents::PhysicsComponent>(entityManager, comp);
                else
                    instanceEntity.add<ECSComponents::PhysicsComponent>(entityManager, comp);
            }
            else if (compSnapshot.type == typeid(ECSComponents::VisibilityComponent))
            {
                auto& comp = compSnapshot.get<ECSComponents::VisibilityComponent>();
                if (instanceEntity.has<ECSComponents::VisibilityComponent>(entityManager))
                    instanceEntity.set<ECSComponents::VisibilityComponent>(entityManager, comp);
                else
                    instanceEntity.add<ECSComponents::VisibilityComponent>(entityManager, comp);
            }
            else if (compSnapshot.type == typeid(ECSComponents::LayerComponent))
            {
                auto& comp = compSnapshot.get<ECSComponents::LayerComponent>();
                if (instanceEntity.has<ECSComponents::LayerComponent>(entityManager))
                    instanceEntity.set<ECSComponents::LayerComponent>(entityManager, comp);
                else
                    instanceEntity.add<ECSComponents::LayerComponent>(entityManager, comp);
            }
            else if (compSnapshot.type == typeid(ECSComponents::ButtonComponent))
            {
                auto& comp = compSnapshot.get<ECSComponents::ButtonComponent>();
                if (instanceEntity.has<ECSComponents::ButtonComponent>(entityManager))
                    instanceEntity.set<ECSComponents::ButtonComponent>(entityManager, comp);
                else
                    instanceEntity.add<ECSComponents::ButtonComponent>(entityManager, comp);
            }
            else if (compSnapshot.type == typeid(ECSComponents::AudioComponent))
            {
                auto& comp = compSnapshot.get<ECSComponents::AudioComponent>();
                if (instanceEntity.has<ECSComponents::AudioComponent>(entityManager))
                    instanceEntity.set<ECSComponents::AudioComponent>(entityManager, comp);
                else
                    instanceEntity.add<ECSComponents::AudioComponent>(entityManager, comp);
            }
            else if (compSnapshot.type == typeid(ECSComponents::AIComponent))
            {
                auto& comp = compSnapshot.get<ECSComponents::AIComponent>();
                if (instanceEntity.has<ECSComponents::AIComponent>(entityManager))
                    instanceEntity.set<ECSComponents::AIComponent>(entityManager, comp);
                else
                    instanceEntity.add<ECSComponents::AIComponent>(entityManager, comp);
            }
            else if (compSnapshot.type == typeid(ECSComponents::PlayerComponent))
            {
                auto& comp = compSnapshot.get<ECSComponents::PlayerComponent>();
                if (instanceEntity.has<ECSComponents::PlayerComponent>(entityManager))
                    instanceEntity.set<ECSComponents::PlayerComponent>(entityManager, comp);
                else
                    instanceEntity.add<ECSComponents::PlayerComponent>(entityManager, comp);
            }
            else if (compSnapshot.type == typeid(ECSComponents::BoxComponent))
            {
                auto& comp = compSnapshot.get<ECSComponents::BoxComponent>();
                if (instanceEntity.has<ECSComponents::BoxComponent>(entityManager))
                    instanceEntity.set<ECSComponents::BoxComponent>(entityManager, comp);
                else
                    instanceEntity.add<ECSComponents::BoxComponent>(entityManager, comp);
            }
        }

        // Restore the preserved position
        if (hasTransform && instanceEntity.has<ECSComponents::TransformComponent>(entityManager))
        {
            auto& transform = instanceEntity.get<ECSComponents::TransformComponent>(entityManager);
            transform.pos = preservedPosition;
        }

        updateCount++;
    }

    std::cout << "[Prefab] Updated " << updateCount << " instances of prefab '" << prefabName << "'\n";
    return updateCount;
}
