#include "entity.hpp"
#include "../resources/eno_loader.hpp"


namespace ECS
{
	void EntityManager::saveJson(
		std::ofstream& file, 
		const Entity& entity, 
		uint32_t& jsonLevel, 
		std::function<void(std::ofstream&, const uint32_t&)> appendIndent)
	{
		// Get components
		auto it = entityMap_.find(entity.id());
		if (it == entityMap_.end())
		{
			throw std::runtime_error("Entity has not been mapped onto the manager!");
		}

		std::vector<std::type_index> types = it->second.archetype->types_;

		// Opening brace (for component data)
		appendIndent(file, jsonLevel);
		file << "\"Entity Data\"" << std::endl;
		appendIndent(file, jsonLevel);
		file << "{\n";

		bool firstIter{ true };
		for (size_t i = 0; i < types.size(); i++)
		{
			if (i < types.size() && !firstIter)
				file << '\n';

			// Refer to DeduceCompType's definition for the reason as to why
			// <typename T> is used! But long story short, it will help
			// create "context" of the component type being used in this lambda!
			// 
			// mutable is used here to modify level which is a local, copied variable
			// used to control indentations on the save file!
			ECSComponents::EnoComponentRegistry::DeduceCompType(types[i], [&, level = jsonLevel + 1]<typename T>() mutable
				{
					// Retrieve the value of the component in a { Name, Value } pair
					auto value = it->second.archetype->chunks_[it->second.chunkIndex]->retrieve<T>(
						it->second.archetype->findComponentIndex<T>(),
						it->second.entityIndexInChunk);

					// Component Name
					appendIndent(file, level);
					file << '[' << types[i].name() << ']' << '\n';

					appendIndent(file, level++);
					file << "{\n";

					// Every component will have a reflect function defined!
					// Use the reflect function in each component to print
					// 
					// (further reflect-related documentation can be found in components with
					// reflect defined e.g the visibility component in visibility.hpp)
					value.reflect([&](const std::string& name, const auto& value, const std::string& type)
						{
							// Component field name + value writes
							appendIndent(file, level);

							file << "\"" << name << "\": <" << type << "> { " << value << " }\n";
						});

					appendIndent(file, --level);
					file << "}\n";
				});

			firstIter = false;
		}

		// Closing brace (for component data)
		appendIndent(file, jsonLevel);
		file << "}\n";
	}

	// Loading entity data in Json
	void EntityManager::loadJson(
		EntityManager& entMng,
		const std::string& data, Entity& entity,
		const size_t& start, const size_t& end,
		std::function<size_t(const std::string& data, const size_t& openBrace)> findMatchingBrace)
	{
		// Verify that there is an Entity data block
		size_t dblock = data.find("Entity Data", start);
		if (dblock == std::string::npos)
		{
			std::cerr << "No Entity Data block found in save file data!" << std::endl;
			return;
		}

		// Find and identify component structs
		size_t pos = data.find('[', start);

		entity = entMng.create();
		// Parsing entity data blocks
		while (pos != std::string::npos && pos < end)
		{
			// Component Name
			size_t compEnd = data.find(']', pos);
			if (compEnd == std::string::npos) break;
			std::string compName = data.substr(pos + 1, compEnd - pos - 1);

			// *can be revisited for further improvement
			const std::string prefix = "struct ECSComponents::";
			if (compName.rfind(prefix, 0) == 0)
			{
				compName = compName.substr(prefix.size());
			}

			// Component Data scope
			size_t compScopeStart = data.find('{', compEnd);
			if (compScopeStart == std::string::npos) break;
			size_t compScopeEnd = findMatchingBrace(data, compScopeStart);

			// Component Data
			std::string compData = data.substr(compScopeStart, compScopeEnd - compScopeStart - 1);

			auto findResult = [&](char open, char close, size_t& pos) -> std::string
				{
					size_t strStart = compData.find(open, pos);
					size_t strEnd = compData.find(close, strStart + 1);
					if (strStart == std::string::npos || strEnd == std::string::npos)
						return "NPOS";

					pos = strEnd + 1;
					return compData.substr(strStart + 1, strEnd - strStart - 1);
				};

			// Parse and map fields of a component
			std::unordered_map<std::string, ECSComponents::DataField> dataMap;
			size_t fieldPos{};
			while (fieldPos < compData.size())
			{
				// Field name
				std::string fname = findResult('"', '"', fieldPos);
				if (fname == "NPOS") break;

				// Field type
				std::string ftype = findResult('<', '>', fieldPos);
				if (ftype == "NPOS") break;

				// Field value
				std::string fvalue = findResult('{', '}', fieldPos);
				if (fvalue == "NPOS") break;

				// Map
				dataMap[fname] = Parser::deduceValue(fvalue, ftype);
			}

			// Deduce component found
			ECSComponents::EnoComponentRegistry::DeduceCompType(compName, [&]<typename T>()
			{
				entity.add<T>(entMng);

				// Packaging the data.
				// Call reflect() ONCE so fields are visited in declaration order.
				// This is critical for components whose reflect() contains loops
				// driven by a previously-declared counter (e.g. StateMachineComponent
				// loops over states[0..numStates-1]): iterating dataMap per-entry and
				// calling reflect() each time could visit state fields before numStates
				// is set, silently dropping them.
				T component{};
				component.reflect([&](const std::string& name, auto& value, auto)
					{
						auto it = dataMap.find(name);
						if (it == dataMap.end()) return;
						const auto& fvalue = it->second;

						using ValueType = std::remove_reference_t<decltype(value)>;

						// Char types are true pain, must make exception...
						// Check if value is a char array
						if constexpr (std::is_array_v<ValueType> && std::is_same_v<std::remove_extent_t<ValueType>, char>)
						{
							// Variant must hold a string type
							std::visit([&](auto&& val) {
								using T = std::decay_t<decltype(val)>;

								if constexpr (std::is_same_v<T, std::string>)
								{
									std::strncpy(value, val.c_str(), sizeof(value));
									value[sizeof(value) - 1] = '\0';
								}
								else if constexpr (std::is_same_v<T, const char*>)
								{
									std::strncpy(value, val, sizeof(value));
									value[sizeof(value) - 1] = '\0';
								}
								else
								{
									// empty by design
									// silently skip non-string variant values
								}
								}, fvalue);
						}
						else
						{
							// normal assignment for other types (particularly arithmetic types)
							std::visit([&](auto&& val) {
								using V = std::decay_t<decltype(val)>;
								using F = std::decay_t<decltype(value)>;

								if constexpr (std::is_same_v<F, V>)
								{
									value = val;
								}
								else if constexpr (std::is_arithmetic_v<F> && std::is_arithmetic_v<V>)
								{
									value = static_cast<F>(val);
								}
								else
								{
									std::cerr << "Skipping assignment: variant type " << typeid(V).name()
										<< " does not match field type " << typeid(F).name() << "\n";
								}
								}, fvalue);
						}
					});

				entity.set<T>(entMng, component);
			});

			pos = data.find('[', compScopeEnd + 1);
			if (pos == std::string::npos)
				break;
		}

		// Ensuring that entity id will be incremented safely
		uint32_t highestID{};
		for (auto& [entityID, _] : entityMap_)
		{
			highestID = Math::maxValue(highestID, entityID);
		}
		id = highestID + 1;

		// After loading all components, load textures if needed
		if (entity.has<ECSComponents::RenderComponent>(entMng))
		{
			auto& renderComp = entity.get<ECSComponents::RenderComponent>(entMng);

			std::cout << "[DEBUG TEXTURE LOAD] Entity " << entity.id()
					  << " - texturePath: '" << renderComp.texturePath << "'"
					  << " - texture ptr null: " << (renderComp.texture == nullptr ? "YES" : "NO") << "\n";

			// If texturePath is set but texture is null, resolve it by name from the loader
			if (renderComp.texturePath[0] != '\0' && !renderComp.texture)
			{
				std::string texName = std::string(renderComp.texturePath);
				std::shared_ptr<Graphics::EnoTexture> tex;

				// Look up by name in the already-loaded resource cache
				if (loader_)
					tex = loader_->getTexture(texName);

				if (tex)
				{
					renderComp.texture = tex;
					bus_.publish<Graphics::TextureMapping>(Graphics::TextureMapping{ texName, tex });
					std::cout << "[EntityManager] Resolved texture from loader: " << texName << "\n";
				}
				else
				{
					std::cerr << "[EntityManager] FAILED to find texture in loader: " << texName << "\n";
				}
			}
			else if (renderComp.texturePath[0] == '\0')
			{
				std::cout << "[DEBUG TEXTURE LOAD] texturePath is empty, skipping\n";
			}
			else if (renderComp.texture != nullptr)
			{
				std::cout << "[DEBUG TEXTURE LOAD] texture already set, skipping\n";
			}
		}
		else
		{
			std::cout << "[DEBUG TEXTURE LOAD] Entity " << entity.id() << " has no RenderComponent\n";
		}
	}

	// Saving entity data in binary
	void EntityManager::saveBin(const std::string& file)
	{
		std::ofstream out(file, std::ios::binary);
		if (!out)
		{
			throw std::runtime_error("Failed to open the file to write entity save data!");
		}

		// Saving data by each stored archetype
		// 
		// Writing to the file the following data respectively:
		// 
		// (char[8])  Header					: 8 bytes
		// (char[8])  Version					: 8 bytes
		// (uint32_t) Type Count				: 4 bytes
		// (uint32_t) Registered Type IDs		: 4 bytes
		//											.
		//											.
		// (uint32_t) Chunk Count				: 4 bytes
		// (uint32_t) Entity Count				: 4 bytes
		// Component Data						: Indeterminate
		//											.
		//											.

		// Header
		const char header[8] = "EnoECSS";
		out.write(header, sizeof(header));

		// Version
		out.write(Config::engine_version,
			sizeof(Config::engine_version));

		// Number of archetypes to write
		uint32_t archetypeCount = static_cast<uint32_t>(archetypeMng_.getArchetypes().size());
		out.write(reinterpret_cast<const char*>(&archetypeCount), sizeof(archetypeCount));

		for (auto& [archetypeKey, upArchetype] : archetypeMng_.getArchetypes())
		{
			Archetype& archetype = *upArchetype;

			// Retrieve and write number of types from each archetype
			uint32_t typeCount = static_cast<uint32_t>(archetype.typeSizes_.size());
			out.write(reinterpret_cast<const char*>(&typeCount), sizeof(typeCount));

			// For each component associated with an archetype, 
			// write their registered id onto the output
			for (size_t i = 0; i < typeCount; i++)
			{
				uint32_t typeID = ECSComponents::EnoComponentRegistry().GetComponentID(archetype.types_[i]);
				out.write(reinterpret_cast<const char*>(&typeID), sizeof(typeID));
			}

			// Retrieve and write number of chunks from each archetype
			uint32_t chunkCount = static_cast<uint32_t>(archetype.chunks_.size());
			out.write(reinterpret_cast<const char*>(&chunkCount), sizeof(chunkCount));

			// Write the data from the chunk
			for (auto& pChunk : archetype.chunks_)
			{
				Chunk& chunk = *pChunk;

				// Write the number of entities currently residing in the chunk
				uint32_t entityCount = static_cast<uint32_t>(chunk.current_);
				out.write(
					reinterpret_cast<const char*>(&entityCount),
					sizeof(entityCount));

				// Write the entity ID mapping
				out.write(
					reinterpret_cast<const char*>(chunk.entities_),
					sizeof(uint32_t) * entityCount);

				// Write in each component data by making use
				// of the continguous nature of each component
				for (size_t i = 0; i < typeCount; i++)
				{
#ifdef ARENA_ENABLED
					size_t stride = archetype.typeSizes_[i];
					size_t bytes = stride * static_cast<size_t>(entityCount);

					if (!bytes) continue;

					const std::byte* src = chunk.data_[i];
					out.write(reinterpret_cast<const char*>(src), bytes);
#else
					size_t componentSize = archetype.typeSizes_[i] * entityCount;
					out.write(reinterpret_cast<const char*>(chunk.pComponent_[i]), componentSize);
#endif
				}
			}
		}
	}

	// Loading entity data
	void EntityManager::loadBin(const std::string& file)
	{
		std::ifstream in(file, std::ios::binary);
		if (!in)
		{
			throw std::runtime_error("Failed to open the ECS data file!");
		}

		// Verify header
		char header[8]{};
		in.read(header, sizeof(header));
		if (strncmp(header, "EnoECSS", 8) != 0)
		{
			throw std::runtime_error("Opened file is not an ECS data file for the EnoEngine!");
		}

		// Verify engine version
		char version[8]{};
		in.read(version, sizeof(version));
		if (strncmp(version, Config::engine_version, 8) != 0)
		{
			throw std::runtime_error("Opened file is not an ECS data file for the EnoEngine!");
		}

		// Read number of archetypes
		uint32_t archetypeCount{};
		in.read(reinterpret_cast<char*>(&archetypeCount), sizeof(archetypeCount));

		// Handled per archetype
		for (uint32_t a = 0; a < archetypeCount; ++a)
		{
			// Read for type count
			uint32_t typeCount{};
			in.read(reinterpret_cast<char*>(&typeCount), sizeof(typeCount));

			// Retrieve the types and associated sizes
			// to recreate archetypes
			std::vector<std::type_index> types;
			std::vector<size_t> sizes;
			types.reserve(typeCount);
			sizes.reserve(typeCount);

			for (size_t i = 0; i < typeCount; i++)
			{
				// Retrieve type IDs
				uint32_t typeID{};
				in.read(reinterpret_cast<char*>(&typeID), sizeof(typeID));

				std::type_index typeIdx = ECSComponents::EnoComponentRegistry::FindComponent(typeID);

				// Use registered type ID
				// to retrieve the type and size of a component
				types.push_back(typeIdx);
				sizes.push_back(ECSComponents::EnoComponentRegistry::GetComponentSize(typeIdx));
			}

			Archetype* archetype = archetypeMng_.getOrCreateArch(arena_, types, sizes);

			// Read for chunk count
			uint32_t chunkCount{};
			in.read(reinterpret_cast<char*>(&chunkCount), sizeof(chunkCount));

#if ARENA_ENABLED
			for (uint32_t c = 0; c < chunkCount; ++c)
			{
				// Read in entity count for each chunk
				uint32_t entityCount{};
				in.read(reinterpret_cast<char*>(&entityCount), sizeof(entityCount));

				Chunk* chunk = archetype->createChunk(entityCount, types, sizes);
				size_t chunkIndex = archetype->chunks_.size() - 1;

				// Read entity IDs
				in.read(
					reinterpret_cast<char*>(chunk->entities_),
					sizeof(uint32_t) * entityCount
				);

				// Read component data into allocated arrays
				size_t componentCount = archetype->types_.size();
				for (size_t i = 0; i < componentCount; i++)
				{
					in.read(
						reinterpret_cast<char*>(chunk->data_[i]),
						sizes[i] * entityCount
					);
				}

				// Entity map housekeeping
				for (size_t e = 0; e < entityCount; e++)
				{
					uint32_t entityID = chunk->entities_[e];
					entityMap_[entityID] = EntityLoc{ archetype, chunkIndex, e };
				}
			}

			// Ensuring that entity id will be incremented safely
			uint32_t highestID{};
			for (auto& [entityID, _] : entityMap_)
			{
				highestID = Math::maxValue(highestID, entityID);
			}
			id = highestID + 1;
#else
			for (uint32_t c = 0; c < chunkCount; ++c)
			{
				// Read in entity count for each chunk
				uint32_t entityCount{};
				in.read(reinterpret_cast<char*>(&entityCount), sizeof(entityCount));

				// Allocate a chunk to hold the entities
				//archetype->createChunk(arena_, entityCount, sizes);
				Chunk& chunk = *archetype->chunks_.back();
				size_t chunkIndex = archetype->chunks_.size() - 1;

				// Read in component data
				for (size_t d = 0; d < typeCount; d++)
				{
					size_t bytes = sizes[d] * entityCount;
					in.read(reinterpret_cast<char*>(chunk.pComponent_[d]), bytes);
				}

				// Entity map housekeeping
				for (size_t e = 0; e < entityCount; e++)
				{
					uint32_t entityID = assignID();
					entityMap_[entityID] = EntityLoc{ archetype, chunkIndex, e };
				}

				chunk.current_ = entityCount;
			}
#endif
		}
	}
}