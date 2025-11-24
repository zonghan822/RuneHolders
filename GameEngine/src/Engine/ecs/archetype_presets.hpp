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


//stds
#include <tuple>


#pragma once

// Engine Component(s)
#include "../component/component_registry.hpp"
#include "eno_data_types.hpp"

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
			std::string filename = std::string(ASSETS_DIR_PATH) + "saves/prefabs/" + name;
			
			// Append preferred extension for save file format
			if (filename.find(".txt") == std::string::npos)
			{
				filename += ".txt";
			}

			std::ofstream save{ filename };
			if (!save)
			{
				throw std::runtime_error("Failed to create a save file for the scene tree!");
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
			std::filesystem::path filePath = std::filesystem::path(ASSETS_DIR_PATH) / "saves/prefabs" / name;

			if (filePath.extension() != ".txt")
				filePath += ".txt";

			std::ifstream file{ filePath };
			if (!file)
				throw std::runtime_error("Failed to open file at " + filePath.string());

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
				nextComp = content.find("struct ECSComponents::", start);
				if (nextComp == std::string::npos)
					nextComp = content.size();

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
					dataMap[name] = deduceValue(value, type);

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

	private:
		// Preset | <Component, Component Fields>
		std::unordered_map<std::type_index, std::unordered_map<std::type_index, std::any>> prefabData_;

		// Preset name | Preset
		std::unordered_map<std::string, std::type_index> presets_;

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

		// !!!!!!! Duplicate function in entity.cpp !!!!!!!
		// I already have a way to remove this but it will
		// take too much time! This is temporary!!!!!
		DataField deduceValue(const std::string& value, const std::string& type)
		{
			auto trim = [](const std::string& str) -> std::string
				{
					size_t start = str.find_first_not_of(" \t\n\r");
					if (start == std::string::npos) return "";

					size_t end = str.find_last_not_of(" \t\n\r");
					return str.substr(start, end - start + 1);
				};

			std::string tvalue = trim(value);
			std::string ttype = trim(type);

			// Convert to lowercase for consistency
			std::transform(std::begin(ttype), std::end(ttype), std::begin(ttype), ::tolower);

			// int
			if (ttype == "int")
				return std::stoi(tvalue);

			// uint32_t
			if (ttype == "uint32_t")
				return static_cast<uint32_t>(std::stoul(tvalue));

			// float
			if (ttype == "float")
				return std::stof(tvalue);

			// double
			if (ttype == "double")
				return std::stod(tvalue);

			// bool
			if (ttype == "bool")
			{
				if (tvalue == "true" || tvalue == "1") return true;
				if (tvalue == "false" || tvalue == "0") return false;

				throw std::runtime_error("Unknown string set as bool value in save!");
			}

			// char[32]
			if (ttype == "char32")
			{
				return tvalue;
			}

			// char[256]
			if (ttype == "char256")
			{
				return tvalue;
			}

			// char[64]
			if (ttype == "char64")
			{
				return tvalue;
			}

			// ---------------- Enums ----------------

			// Color
			if (ttype == "color")
			{
				const std::unordered_map<std::string, Color> colorMap =
				{
					{ "Green", Color::Green },
					{ "Red", Color::Red }
				};

				auto it = colorMap.find(tvalue);
				if (it != colorMap.end())
				{
					return it->second;
				}
			}

			// Collider Type
			if (ttype == "collidertype")
			{
				const std::unordered_map<std::string, ECSComponents::ColliderType> colliderMap =
				{
					{ "Circle",			ECSComponents::ColliderType::CIRCLE },
					{ "Line",			ECSComponents::ColliderType::LINE },
					{ "Point",			ECSComponents::ColliderType::POINT },
					{ "Rectangle",		ECSComponents::ColliderType::RECTANGLE },
				};

				auto it = colliderMap.find(tvalue);
				if (it != colliderMap.end())
				{
					return it->second;
				}
			}

			// ------------- Custom Math -------------
			// eVec2<float>
			if (ttype == "evec2(float)")
			{
				// First value
				float fv{}, sv{};
				size_t vstart = tvalue.find_first_of(':');
				size_t vend{};
				if (vstart != std::string::npos)
				{
					++vstart;

					// skip whitespaces to value
					while (vstart < tvalue.size() && isspace(tvalue[vstart])) ++vstart;

					vend = tvalue.find_first_of('|', vstart);
					if (vend != std::string::npos)
					{
						fv = std::stof(tvalue.substr(vstart, vend - vstart));
					}
				}

				// Second value
				vstart = tvalue.find(':', vstart);
				if (vstart != std::string::npos)
				{
					++vstart;

					// skip whitespaces to value
					while (vstart < tvalue.size() && isspace(tvalue[vstart])) ++vstart;

					vend = tvalue.find_first_of('|', vend + 1);
					if (vend != std::string::npos)
					{
						sv = std::stof(tvalue.substr(vstart, vend - vstart));
					}
					else if (vend == std::string::npos)
					{
						sv = std::stof(tvalue.substr(vstart));
					}
				}

				return Math::eVec2{ fv, sv };
			}

			// Graphics::Color (RGBA struct)
			if (ttype == "color(rgba)")
			{
				float r{}, g{}, b{}, a{};
				size_t vstart{}, vend{};

				// Parse r value
				vstart = tvalue.find_first_of(':');
				if (vstart != std::string::npos)
				{
					++vstart;
					while (vstart < tvalue.size() && isspace(tvalue[vstart])) ++vstart;
					vend = tvalue.find_first_of('|', vstart);
					if (vend != std::string::npos)
						r = std::stof(tvalue.substr(vstart, vend - vstart));
				}

				// Parse g value
				vstart = tvalue.find(':', vend);
				if (vstart != std::string::npos)
				{
					++vstart;
					while (vstart < tvalue.size() && isspace(tvalue[vstart])) ++vstart;
					vend = tvalue.find_first_of('|', vstart);
					if (vend != std::string::npos)
						g = std::stof(tvalue.substr(vstart, vend - vstart));
				}

				// Parse b value
				vstart = tvalue.find(':', vend);
				if (vstart != std::string::npos)
				{
					++vstart;
					while (vstart < tvalue.size() && isspace(tvalue[vstart])) ++vstart;
					vend = tvalue.find_first_of('|', vstart);
					if (vend != std::string::npos)
						b = std::stof(tvalue.substr(vstart, vend - vstart));
				}

				// Parse a value
				vstart = tvalue.find(':', vend);
				if (vstart != std::string::npos)
				{
					++vstart;
					while (vstart < tvalue.size() && isspace(tvalue[vstart])) ++vstart;
					vend = tvalue.find_first_of('|', vstart);
					if (vend != std::string::npos)
						a = std::stof(tvalue.substr(vstart, vend - vstart));
					else
						a = std::stof(tvalue.substr(vstart));
				}

				return Graphics::Color{ r, g, b, a };
			}

			// File path types
			if (ttype == "texchar240")
			{
				return std::make_shared<Graphics::EnoTexture>();
			}

			throw std::runtime_error("Unknown type { " + ttype + " } has been set in the save file!");
		}
	};
}