/*============================================================================================
* @file         archetype_presets.hpp
* @author       Lee Zhe An (zhean.l@digipen.edu)
* @date         10/3/2025
* @brief        Declaration/definition file for archetype presets
* 
* All content � 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
*//*========================================================================================*/

#pragma once

// Engine Component(s)


#pragma once

// Engine Component(s)
#include "../component/component_registry.hpp"
#include "eno_data_types.hpp"
// Math
#include "../math/math.hpp"

// Utility
#include "../utility/eno_parser.hpp"

//stds
#include <tuple>
#include <unordered_map>
#include <any>
#include <utility>
#include <fstream>
#include <variant>

// To be removed
#include "../graphics/graphics.hpp"
#include "../component/components/collision_component.hpp"

namespace ECSComponents
{
	struct Preset
	{
		// Expansion with the PRESET x macro
		// defined in component_list.hpp
		// 
		// First param will always be the name of the file
		// Followed by a varied number of components which make up the preset
		// constructed as a tuple when expanded here!
		// Source: https://stackoverflow.com/questions/679979/how-to-make-a-variadic-macro-variable-number-of-arguments
#define PRESET(Name, ...) using Name = std::tuple<__VA_ARGS__>;
		PRESET_LIST
#undef PRESET

			// This is for automating the whole process
			// of passing in each preset individually into save()/load()
			//
			// Consequence of lacking this is the necessary maintenance
			// of calling save() on every single preset made
			//
			// Expanding the PRESET_LIST macro to call savePreset => save in the Prefab class
			template <typename Prefab>
		static void saveAll(Prefab& prefab)
		{
#define PRESET(Name, ...) savePreset<Prefab, Name>(prefab, #Name);
			PRESET_LIST
#undef PRESET
		}

		// Template arguments are essentially => prefab class | preset from PRESET_LIST
		// Using the prefab class to call the original save()
		// 
		// Fortunately, the type and the name of the file is exactly the same
		// so we can pass in Name as the Preset(type), as well as being the file name
		// through the savePreset call as savePreset<Prefab, Name>
		// 
		// Macro will decay the name string literal into a const char*
		template<typename Prefab, typename Preset>
		static void savePreset(Prefab& p, const char* name)
		{
			p.save(name, Preset{}); // Instantiate the preset
		}

		template <typename Prefab>
		static void loadAll(Prefab& prefab)
		{
#define PRESET(Name, ...) prefab.load(#Name);
			PRESET_LIST
#undef PRESET
		}
	};

	// This class will generate prefab data
	// based on the PRESET_LIST defined in component_list.hpp
	class EnoPrefabManager
	{
	public:

		EnoPrefabManager(Event::EnoEventBus& bus)
			:
			bus_{ bus }
		{}

		// Retrieving an instance of a saved prefab
		template <typename Preset>
		Preset retrieve()
		{
			auto it = prefabData_.find(typeid(Preset));
			if (it == prefabData_.end())
				throw std::runtime_error("Failed to find the specified preset!");

			// Construct a tuple to map out the prefab
			auto& map = it->second;
			Preset result;

			// A Preset represents a list of components
			// so apply is used here to expand the tuple into
			// separate arguments (components)
			// lambda(component A, component B, ...)
			//
			// For each element(component) in this pack, we get
			// the corresponding data from prefabData by matching
			// it with the same component in the map. Since the data
			// in the map is stored as std::any, we need to cast the component
			// back to its original type with any_cast. 
			// 
			// Since decltype(comps) will
			// result in a reference type (e.g VisibilityComponent&), we need
			// std::remove_reference_t to get the actual component type (e.g VisibilityComponent)
			// After which, the value of each component (field) is then assigned back to the tuple
			std::apply([&](auto&... comps)
				{
					((comps = std::any_cast<std::remove_reference_t<decltype(comps)>>(map[typeid(comps)])), ...);
				}, result);

			return result;
		}

		// Register list of presets under PRESET_LIST
		void registerPresets()
		{
#define PRESET(Name, ...) presets_.emplace(#Name, std::type_index(typeid(Name)));
			PRESET_LIST
#undef PRESET
		}

		// Save prefab data
		template <typename Tuple>
		void save(const std::string& name, Tuple&& tuple)
		{
			std::string filename = FileSystem::GetAssetsDir().string() + "/saves/prefabs/" + name;

			// Append preferred extension for save file format
			if (filename.find(".txt") == std::string::npos)
			{
				filename += ".txt";
			}

			std::filesystem::create_directories(FileSystem::GetAssetsDir() / "saves/prefabs");

			// Don't overwrite an existing prefab file — it may have been modified
			if (std::filesystem::exists(filename))
				return;

			std::ofstream save{ filename };
			if (!save)
			{
				std::cout << "[Prefab] Failed to create save file: " << filename << "\n";
				return;
			}

			// Opening brace
			save << "{\n";

			// Expand the tuple of components
			// retrieve each default constructed field name and value
			std::apply([&](auto&&... comp)
				{
					// Invoked lambda that will be
					// using the expanded component tuple
					// to write to the save file
					(([&](auto& c)
						{
							save << "\t\"" << std::type_index(typeid(c)).name() << "\": {\n";

							// Collect all the fields and output them after (to account for last line commas)
							std::vector<std::string> fields;
							c.reflect([&](const std::string& name, const auto& value, const std::string& type)
								{
									std::ostringstream line;
									line << "\"" << name << "\": {\n"
										<< "\t\t\t\"value\": \"" << value << "\",\n"
										<< "\t\t\t\"type\": \"" << type << "\"\n"
										<< "\t\t}";
									fields.push_back(line.str());
								});

							// Output
							for (size_t i = 0; i < fields.size(); i++)
							{
								save << "\t\t" << fields[i];
								if (i + 1 < fields.size())
									save << ",\n";
								else
									save << "\n";
							}

							save << "\t},\n";
						})(comp), ...);
				}, tuple);

			// Closing brace
			save << "}";
		}

		// Load prefab data
		void load(const std::string& name)
		{
			// Ensure that the name for the file to be opened
			// has been appended with the appropriate extension
			//
			// This name passed will also serve as the name of the preset to look for
			std::filesystem::path filePath = FileSystem::GetAssetsDir() / "saves/prefabs" / name;

			if (filePath.extension() != ".txt")
				filePath += ".txt";

			std::ifstream file{ filePath };
			if (!file)
			{
				std::cout << "[Prefab] File not found, skipping: " << filePath.string() << "\n";
				return;
			}

			std::string presetName = filePath.stem().string();

			// Look up registered preset
			auto it = presets_.find(presetName);
			if (it == presets_.end())
				throw std::runtime_error("Failed to specify a registered preset!");

			std::type_index preset = it->second;

			// Store file contents
			std::stringstream buffer;
			buffer << file.rdbuf();
			std::string content = buffer.str();

			std::string line{};
			std::string compName{};

			size_t start{};
			size_t end{};
			size_t nextComp{};

			// To be used for consuming data fields
			auto findResult = [&](const std::string& open, const std::string& close, size_t& pos) -> std::string
				{
					size_t strStart = content.find(open, pos);
					if (strStart == std::string::npos)
						return "Missing field quote!";
					strStart += open.length();

					size_t strEnd = content.find(close, strStart);
					if (strEnd == std::string::npos)
						return "Missing field quote!";
					pos = strEnd + close.length();

					return content.substr(strStart, strEnd - strStart);
				};

			// Iterate through data file
			while (nextComp < content.size())
			{
				// Identify component
				if (nextComp > 0)
					start = nextComp;

				start = content.find("::", start);
				end = content.find("\"", start + 1);
				if (start == std::string::npos || end == std::string::npos)
					throw std::runtime_error("Unable to find closing quotes on component name!");

				compName = content.substr(start + 2, end - start - 2);

				// Component Data block start && end
				start = content.find("{", end);
				// MSVC returns "struct" or "class" depending on type
				size_t nextStruct = content.find("struct ECSComponents::", start);
				size_t nextClass = content.find("class ECSComponents::", start);
				nextComp = Math::minValue(
					nextStruct != std::string::npos ? nextStruct : content.size(),
					nextClass != std::string::npos ? nextClass : content.size()
				);

				std::unordered_map<std::string, DataField> dataMap;
				// Consume component data line
				while (start < nextComp && start != std::string::npos)
				{
					// Field name | value | type
					std::string name = findResult("\"", "\"", start);
					std::string value = findResult("\"value\": \"", "\"", start);
					std::string type = findResult("\"type\": \"", "\"", start);

					if (name == "Missing field quote!" ||
						value == "Missing field quote!" ||
						type == "Missing field quote!")
					{
						throw std::runtime_error("Prefab instantiation error!");
					}

					// Store
					dataMap[name] = Parser::deduceValue(value, type);

					// Check if there is another field after
					size_t endCheck = content.find("\"", start);
					start = content.find("},", start);
					if (endCheck == std::string::npos)
						break;
					start += 2;
				}

				EnoComponentRegistry::DeduceCompType(compName, [&]<typename T>()
				{
					T component;

					component.reflect([&](const std::string& fname,
						auto& fvalue,
						const std::string& ftype)
						{
							auto it = dataMap.find(fname);
							if (it == dataMap.end())
							{
								throw std::runtime_error("Unknown field found in the data map!");
							}

							// Get rid of the reference
							using ValueType = std::remove_reference_t<decltype(fvalue)>;

							try
							{
								if (fname == "Texture Path")
								{ /* empty by design */ }
								// char exception
								else if constexpr (std::is_array_v<ValueType>)
								{
									using Element = std::remove_extent_t<ValueType>;
									constexpr size_t N = std::extent_v<ValueType>;

									const std::string& str = std::get<std::string>(it->second);
									std::strncpy(fvalue, str.c_str(), N);
								}
								else
								{
									fvalue = std::get<ValueType>(it->second);
								}
							}
							catch (...)
							{
								throw std::runtime_error("Type mismatch for field " + fname);
							}
						});

					// Store data per prefab per component
					prefabData_[preset][std::type_index(typeid(T))] = component;
				});
			}
		}

		// Prints all prefabs 
		// and their default values
		void viewPrefabs()
		{
			for (auto& [pType, inner] : prefabData_)
			{
				std::cout << "Current Prefab Displaying: " << std::endl <<
					pType.name() << std::endl <<
					"____________________________________________" << std::endl;

				for (auto& [compType, compAny] : inner)
				{
					EnoComponentRegistry::DeduceCompType(compType, [&]<typename T>()
					{
						const T& c = std::any_cast<const T&>(compAny);
						viewComp(c);
					});
				}
			}
		}

		// View registered presets
		void viewPresets()
		{
			for (const auto& [ptname, ptype] : presets_)
			{
				std::cout << "Preset name: " << ptname << "has a type of " <<
					std::endl << ptype.name() << std::endl;
			}
		}

		// Returns all loaded preset names (sorted)
		std::vector<std::string> getPresetNames() const
		{
			std::vector<std::string> names;
			names.reserve(presets_.size());
			for (const auto& [name, _] : presets_)
				names.push_back(name);
			std::sort(names.begin(), names.end());
			return names;
		}

		// Returns a pointer to the component map for the named preset.
		// Returns nullptr if the preset name is not found.
		// Use EnoComponentRegistry::DeduceCompType to add each component to an entity.
		const std::unordered_map<std::type_index, std::any>* getPresetComponents(const std::string& name) const
		{
			auto presetIt = presets_.find(name);
			if (presetIt == presets_.end())
				return nullptr;

			auto prefabIt = prefabData_.find(presetIt->second);
			if (prefabIt == prefabData_.end())
				return nullptr;

			return &prefabIt->second;
		}

		// Returns a mutable pointer to the component map for the named preset.
		// Used by the prefab editor to modify default component values in-memory.
		std::unordered_map<std::type_index, std::any>* getPresetComponentsMutable(const std::string& name)
		{
			auto presetIt = presets_.find(name);
			if (presetIt == presets_.end())
				return nullptr;

			auto prefabIt = prefabData_.find(presetIt->second);
			if (prefabIt == prefabData_.end())
				return nullptr;

			return &prefabIt->second;
		}

		// Force-overwrites the prefab file on disk with the current in-memory data.
		void saveForcedToDisk(const std::string& name)
		{
			auto presetIt = presets_.find(name);
			if (presetIt == presets_.end())
				return;

			auto prefabIt = prefabData_.find(presetIt->second);
			if (prefabIt == prefabData_.end())
				return;

			std::string filename = FileSystem::GetAssetsDir().string() + "/saves/prefabs/" + name + ".txt";

			std::ofstream save{ filename, std::ios::trunc };
			if (!save)
			{
				std::cout << "[Prefab] Failed to open file for writing: " << filename << "\n";
				return;
			}

			save << "{\n";

			for (auto& [compTypeIdx, compAny] : prefabIt->second)
			{
				EnoComponentRegistry::DeduceCompType(compTypeIdx, [&]<typename T>()
				{
					T& comp = std::any_cast<T&>(compAny);

					save << "\t\"" << compTypeIdx.name() << "\": {\n";

					std::vector<std::string> fields;
					comp.reflect([&](const std::string& fname, const auto& fvalue, const std::string& ftype)
						{
							std::ostringstream line;
							line << "\"" << fname << "\": {\n"
								<< "\t\t\t\"value\": \"" << fvalue << "\",\n"
								<< "\t\t\t\"type\": \"" << ftype << "\"\n"
								<< "\t\t}";
							fields.push_back(line.str());
						});

					for (size_t i = 0; i < fields.size(); i++)
					{
						save << "\t\t" << fields[i];
						if (i + 1 < fields.size())
							save << ",\n";
						else
							save << "\n";
					}

					save << "\t},\n";
				});
			}

			save << "}";
			std::cout << "[Prefab] Saved prefab '" << name << "' to " << filename << "\n";
		}

	// ===================== User Prefab API =====================

		// Store or overwrite a named user prefab with the given component map, and save to disk.
		// Call this from ImGui after building the component map from the selected entity.
		void setUserPrefab(const std::string& name, std::unordered_map<std::type_index, std::any> comps)
		{
			userPrefabData_[name] = std::move(comps);
			saveUserPrefabToDisk(name);
		}

		// Write a user prefab to saves/prefabs/user_<name>.txt (same format as preset files).
		void saveUserPrefabToDisk(const std::string& name)
		{
			auto it = userPrefabData_.find(name);
			if (it == userPrefabData_.end()) return;

			std::filesystem::create_directories(FileSystem::GetAssetsDir() / "saves/prefabs");
			std::string filename = (FileSystem::GetAssetsDir() / "saves/prefabs" / ("user_" + name + ".txt")).string();

			std::ofstream save{ filename, std::ios::trunc };
			if (!save)
			{
				std::cout << "[UserPrefab] Failed to open for writing: " << filename << "\n";
				return;
			}

			save << "{\n";

			// Save node type so spawned instances get the correct type
			auto ntIt = userPrefabNodeTypes_.find(name);
			if (ntIt != userPrefabNodeTypes_.end())
			{
				save << "\t\"NodeType\": {\n";
				save << "\t\t\"value\": \"" << ntIt->second << "\",\n";
				save << "\t\t\"type\": \"uint32_t\"\n";
				save << "\t},\n";
			}
			for (auto& [compTypeIdx, compAny] : it->second)
			{
				EnoComponentRegistry::DeduceCompType(compTypeIdx, [&]<typename T>()
				{
					T& comp = std::any_cast<T&>(compAny);
					save << "\t\"" << compTypeIdx.name() << "\": {\n";

					std::vector<std::string> fields;
					comp.reflect([&](const std::string& fname, const auto& fvalue, const std::string& ftype)
					{
						std::ostringstream line;
						line << "\"" << fname << "\": {\n"
							 << "\t\t\t\"value\": \"" << fvalue << "\",\n"
							 << "\t\t\t\"type\": \"" << ftype << "\"\n"
							 << "\t\t}";
						fields.push_back(line.str());
					});

					for (size_t i = 0; i < fields.size(); i++)
					{
						save << "\t\t" << fields[i];
						save << (i + 1 < fields.size() ? ",\n" : "\n");
					}
					save << "\t},\n";
				});
			}
			save << "}";
			std::cout << "[UserPrefab] Saved '" << name << "' -> " << filename << "\n";
		}

		// Load one user prefab from saves/prefabs/user_<name>.txt into userPrefabData_.
		void loadUserPrefab(const std::string& name)
		{
			std::filesystem::path filePath = FileSystem::GetAssetsDir() / "saves/prefabs" / ("user_" + name + ".txt");
			std::ifstream file{ filePath };
			if (!file)
			{
				std::cout << "[UserPrefab] File not found: " << filePath.string() << "\n";
				return;
			}

			std::stringstream buffer;
			buffer << file.rdbuf();
			std::string content = buffer.str();

			// Read node type if present (written as "NodeType": { "value": "N", ... })
			{
				size_t ntPos = content.find("\"NodeType\":");
				if (ntPos != std::string::npos)
				{
					size_t valPos = content.find("\"value\": \"", ntPos);
					if (valPos != std::string::npos)
					{
						valPos += 10; // skip past '"value": "'
						size_t valEnd = content.find("\"", valPos);
						if (valEnd != std::string::npos)
						{
							try { userPrefabNodeTypes_[name] = static_cast<uint32_t>(std::stoul(content.substr(valPos, valEnd - valPos))); }
							catch (...) {}
						}
					}
				}
			}

			auto& compMap = userPrefabData_[name];
			compMap.clear();

			auto findResult = [&](const std::string& open, const std::string& close, size_t& pos) -> std::string
			{
				size_t s = content.find(open, pos);
				if (s == std::string::npos) return "Missing field quote!";
				s += open.length();
				size_t e = content.find(close, s);
				if (e == std::string::npos) return "Missing field quote!";
				pos = e + close.length();
				return content.substr(s, e - s);
			};

			std::string compName;
			size_t start{}, end{}, nextComp{};

			while (nextComp < content.size())
			{
				if (nextComp > 0) start = nextComp;
				start = content.find("::", start);
				end   = content.find("\"", start + 1);
				if (start == std::string::npos || end == std::string::npos) break;

				compName = content.substr(start + 2, end - start - 2);
				start = content.find("{", end);

				size_t nextStruct = content.find("struct ECSComponents::", start);
				size_t nextClass  = content.find("class ECSComponents::",  start);
				nextComp = Math::minValue(
					nextStruct != std::string::npos ? nextStruct : content.size(),
					nextClass  != std::string::npos ? nextClass  : content.size());

				std::unordered_map<std::string, DataField> dataMap;
				while (start < nextComp && start != std::string::npos)
				{
					std::string fname  = findResult("\"", "\"", start);
					std::string fvalue = findResult("\"value\": \"", "\"", start);
					std::string ftype  = findResult("\"type\": \"",  "\"", start);
					if (fname == "Missing field quote!" || fvalue == "Missing field quote!" || ftype == "Missing field quote!")
						break;

					dataMap[fname] = Parser::deduceValue(fvalue, ftype);
					size_t endCheck = content.find("\"", start);
					start = content.find("},", start);
					if (endCheck == std::string::npos) break;
					start += 2;
				}

				EnoComponentRegistry::DeduceCompType(compName, [&]<typename T>()
				{
					T component;
					component.reflect([&](const std::string& fname, auto& fvalue, const std::string&)
					{
						auto it = dataMap.find(fname);
						if (it == dataMap.end()) return;
						using ValueType = std::remove_reference_t<decltype(fvalue)>;
						try
						{
							if (fname == "Texture Path") { /* skip */ }
							else if constexpr (std::is_array_v<ValueType>)
							{
								const std::string& str = std::get<std::string>(it->second);
								std::strncpy(fvalue, str.c_str(), std::extent_v<ValueType>);
							}
							else
								fvalue = std::get<ValueType>(it->second);
						}
						catch (...) { /* skip on type mismatch */ }
					});
					compMap[std::type_index(typeid(T))] = component;
				});
			}
		}

		// Scan saves/prefabs/ for user_*.txt files and load all of them.
		void loadAllUserPrefabs()
		{
			auto prefabDir = FileSystem::GetAssetsDir() / "saves/prefabs";
			if (!std::filesystem::exists(prefabDir)) return;

			for (auto& entry : std::filesystem::directory_iterator(prefabDir))
			{
				if (entry.path().extension() != ".txt") continue;
				std::string stem = entry.path().stem().string();
				if (stem.rfind("user_", 0) == 0)
					loadUserPrefab(stem.substr(5));
			}
		}

		// Returns sorted names of all loaded user prefabs.
		std::vector<std::string> getUserPrefabNames() const
		{
			std::vector<std::string> names;
			names.reserve(userPrefabData_.size());
			for (const auto& [n, _] : userPrefabData_)
				names.push_back(n);
			std::sort(names.begin(), names.end());
			return names;
		}

		bool userPrefabExists(const std::string& name) const
		{
			return userPrefabData_.find(name) != userPrefabData_.end();
		}

		const std::unordered_map<std::type_index, std::any>* getUserPrefabComponents(const std::string& name) const
		{
			auto it = userPrefabData_.find(name);
			return it != userPrefabData_.end() ? &it->second : nullptr;
		}

		// Store the original node type (as uint32_t to avoid circular includes) for a user prefab.
		void setUserPrefabNodeType(const std::string& name, uint32_t nodeType)
		{
			userPrefabNodeTypes_[name] = nodeType;
			saveUserPrefabToDisk(name);
		}

		// Returns the stored node type for a user prefab, or 1 (CUSTOM) if not set.
		uint32_t getUserPrefabNodeType(const std::string& name) const
		{
			auto it = userPrefabNodeTypes_.find(name);
			return it != userPrefabNodeTypes_.end() ? it->second : 1u; // 1 = CUSTOM
		}

		// Remove a user prefab from memory and delete its file from disk.
		void deleteUserPrefab(const std::string& name)
		{
			userPrefabData_.erase(name);
			userPrefabNodeTypes_.erase(name);
			auto filePath = FileSystem::GetAssetsDir() / "saves/prefabs" / ("user_" + name + ".txt");
			std::filesystem::remove(filePath);
			std::cout << "[UserPrefab] Deleted '" << name << "'\n";
		}

	private:
		// Preset | <Component, Component Fields>
		std::unordered_map<std::type_index, std::unordered_map<std::type_index, std::any>> prefabData_;

		// Preset name | Preset
		std::unordered_map<std::string, std::type_index> presets_;

		// User-defined prefabs: name -> component map
		std::unordered_map<std::string, std::unordered_map<std::type_index, std::any>> userPrefabData_;

		// User-defined prefab node types: name -> NodeType as uint32_t
		std::unordered_map<std::string, uint32_t> userPrefabNodeTypes_;

		// !!!!!!!!!! SHOULD BE REMOVED !!!!!!!!!!
		Event::EnoEventBus& bus_;

		// Iterate through each reflected component field
		// prints the name of each field, as well as their value and type
		template <typename T>
		void viewComp(const T& component)
		{
			std::cout << "Component: " << typeid(component).name() << std::endl;
			component.reflect([&](const std::string& name, const auto& value, const std::string& type)
				{
					std::cout << "Field " << name << " <" << type << ">: " << value << std::endl;
				});

			std::cout << "___________________________________" << std::endl;
		}
	};
}