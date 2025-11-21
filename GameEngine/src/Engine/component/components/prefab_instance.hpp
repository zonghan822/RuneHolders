/*============================================================================================
* All content � 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         prefab_instance.hpp
* @brief        Component for tracking which prefab an entity was instantiated from
*===========================================================================================*/

#pragma once
#include <string>
#include <cstring>

namespace ECSComponents
{
    /*============================================================================================
     * \struct   PrefabInstanceComponent
     * \brief    Tracks which prefab an entity was instantiated from, enabling bulk updates
     *===========================================================================================*/
    struct PrefabInstanceComponent
    {
        char prefabName[64];  // Name of the prefab this entity is an instance of

        PrefabInstanceComponent() : prefabName{} {}

        PrefabInstanceComponent(const char* name)
        {
            std::strncpy(prefabName, name, sizeof(prefabName) - 1);
            prefabName[sizeof(prefabName) - 1] = '\0';
        }

        // Reflection for serialization (const version for saving)
        template <typename Visitor>
        void reflect(Visitor&& l) const
        {
            l("Prefab Name", prefabName, "char64");
        }

        // Reflection for deserialization (non-const version for loading)
        template <typename Visitor>
        void reflect(Visitor&& l)
        {
            l("Prefab Name", prefabName, "char64");
        }
    };
}
