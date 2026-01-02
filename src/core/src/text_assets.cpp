#include "text_assets.h"
#include "enums.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

// Simple JSON parsing without external dependencies
// This is a minimal parser for our specific JSON structure

TextAssets::TextAssets()
{
}

TextAssets::~TextAssets()
{
}

bool TextAssets::load_assets(const std::string& assets_dir)
{
	bool success = true;

	success &= load_planet_names(assets_dir);
	success &= load_player_names(assets_dir);
	success &= load_ship_names(assets_dir);
	success &= load_tech_level_names(assets_dir);
	success &= load_radical_techs(assets_dir);

	if (success)
	{
		std::cout << "Successfully loaded all text assets from " << assets_dir << std::endl;
	}
	else
	{
		std::cerr << "Warning: Some text assets failed to load from " << assets_dir << std::endl;
	}

	return success;
}

// ============================================================================
// Planet Names
// ============================================================================

bool TextAssets::load_planet_names(const std::string& assets_dir)
{
	std::string filepath = assets_dir + "/planet_names.json";
	std::ifstream file(filepath);

	if (!file.is_open())
	{
		std::cerr << "Error: Could not open " << filepath << std::endl;
		return false;
	}

	std::string line;
	bool in_array = false;

	while (std::getline(file, line))
	{
		// Look for array start
		if (line.find("\"planet_names\"") != std::string::npos)
		{
			in_array = true;
			continue;
		}

		if (in_array && line.find("]") != std::string::npos)
		{
			break;
		}

		if (in_array && line.find("\"") != std::string::npos)
		{
			// Extract string between quotes
			size_t first = line.find("\"");
			size_t last = line.rfind("\"");
			if (first != std::string::npos && last != std::string::npos && first != last)
			{
				std::string name = line.substr(first + 1, last - first - 1);
				if (!name.empty())
				{
					planet_names.push_back(name);
				}
			}
		}
	}

	file.close();
	return !planet_names.empty();
}

const std::vector<std::string>& TextAssets::get_planet_names() const
{
	return planet_names;
}

std::string TextAssets::get_random_planet_name(uint32_t seed) const
{
	if (planet_names.empty())
		return "";

	uint32_t index = seed % planet_names.size();
	return planet_names[index];
}

std::vector<std::string> TextAssets::get_planet_name_list() const
{
	return planet_names;
}

// ============================================================================
// Player Names
// ============================================================================

bool TextAssets::load_player_names(const std::string& assets_dir)
{
	std::string filepath = assets_dir + "/player_names.json";
	std::ifstream file(filepath);

	if (!file.is_open())
	{
		std::cerr << "Error: Could not open " << filepath << std::endl;
		return false;
	}

	std::string line;
	bool in_male_array = false;
	bool in_female_array = false;

	while (std::getline(file, line))
	{
		if (line.find("\"male_names\"") != std::string::npos)
		{
			in_male_array = true;
			in_female_array = false;
			continue;
		}

		if (line.find("\"female_names\"") != std::string::npos)
		{
			in_male_array = false;
			in_female_array = true;
			continue;
		}

		if ((in_male_array || in_female_array) && line.find("]") != std::string::npos)
		{
			in_male_array = false;
			in_female_array = false;
			continue;
		}

		if ((in_male_array || in_female_array) && line.find("\"") != std::string::npos)
		{
			size_t first = line.find("\"");
			size_t last = line.rfind("\"");
			if (first != std::string::npos && last != std::string::npos && first != last)
			{
				std::string name = line.substr(first + 1, last - first - 1);
				if (!name.empty())
				{
					if (in_male_array)
						male_player_names.push_back(name);
					else if (in_female_array)
						female_player_names.push_back(name);
				}
			}
		}
	}
	file.close();
	return !male_player_names.empty() && !female_player_names.empty();
}

const std::vector<std::string>& TextAssets::get_male_player_names() const
{
	return male_player_names;
}
const std::vector<std::string>& TextAssets::get_female_player_names() const
{
	return female_player_names;
}

std::string TextAssets::get_random_male_player_name(uint32_t seed) const
{
	if (male_player_names.empty())
		return "";

	uint32_t index = seed % male_player_names.size();
	return male_player_names[index];
}
std::string TextAssets::get_random_female_player_name(uint32_t seed) const
{
	if (female_player_names.empty())
		return "";

	uint32_t index = seed % female_player_names.size();
	return female_player_names[index];
}

std::vector<std::string> TextAssets::get_player_name_list(Gender gender) const
{
	if (gender == GENDER_F)
		return female_player_names;
	else if (gender == GENDER_M)
		return male_player_names;
	else
		return std::vector<std::string>();  // Return empty vector for invalid gender
}

// ============================================================================
// Ship Names
// ============================================================================

bool TextAssets::load_ship_names(const std::string& assets_dir)
{
	std::string filepath = assets_dir + "/ship_names.json";
	std::ifstream file(filepath);

	if (!file.is_open())
	{
		std::cerr << "Error: Could not open " << filepath << std::endl;
		return false;
	}

	std::string line;
	std::string current_ship_type;
	bool in_ship_type_array = false;

	while (std::getline(file, line))
	{
		// Look for ship type keys
		if (line.find("\"scout\"") != std::string::npos)
		{
			current_ship_type = "scout";
			in_ship_type_array = true;
			continue;
		}
		if (line.find("\"dreadnought\"") != std::string::npos)
		{
			current_ship_type = "dreadnought";
			in_ship_type_array = true;
			continue;
		}
		if (line.find("\"fighter\"") != std::string::npos)
		{
			current_ship_type = "fighter";
			in_ship_type_array = true;
			continue;
		}
		if (line.find("\"tanker\"") != std::string::npos)
		{
			current_ship_type = "tanker";
			in_ship_type_array = true;
			continue;
		}
		if (line.find("\"colony\"") != std::string::npos)
		{
			current_ship_type = "colony";
			in_ship_type_array = true;
			continue;
		}
		if (line.find("\"satellite\"") != std::string::npos)
		{
			current_ship_type = "satellite";
			in_ship_type_array = true;
			continue;
		}
		if (line.find("\"biological\"") != std::string::npos)
		{
			current_ship_type = "biological";
			in_ship_type_array = true;
			continue;
		}

		if (in_ship_type_array && line.find("]") != std::string::npos)
		{
			in_ship_type_array = false;
			continue;
		}

		if (in_ship_type_array && line.find("\"") != std::string::npos && current_ship_type != "ship_names")
		{
			size_t first = line.find("\"");
			size_t last = line.rfind("\"");
			if (first != std::string::npos && last != std::string::npos && first != last)
			{
				std::string name = line.substr(first + 1, last - first - 1);
				if (!name.empty())
				{
					ship_names[current_ship_type].push_back(name);
				}
			}
		}
	}
	file.close();
	return !ship_names.empty();
}

const std::vector<std::string>& TextAssets::get_ship_names(const std::string& ship_type) const
{
	static const std::vector<std::string> empty;
	auto it = ship_names.find(ship_type);
	if (it != ship_names.end())
		{ return it->second; }
	return empty;
}

std::string TextAssets::get_random_ship_name(const std::string& ship_type, uint32_t seed) const
{
	const std::vector<std::string>& names = get_ship_names(ship_type);
	if (names.empty())
		{ return ""; }

	uint32_t index = seed % names.size();
	return names[index];
}

// ============================================================================
// Technology Level Names
// ============================================================================

bool TextAssets::load_tech_level_names(const std::string& assets_dir)
{
	std::string filepath = assets_dir + "/tech_level_names.json";
	std::ifstream file(filepath);

	if (!file.is_open())
	{
		std::cerr << "Error: Could not open " << filepath << std::endl;
		return false;
	}

	std::string line;
	std::string current_tech_stream;
	bool in_tech_array = false;

	while (std::getline(file, line))
	{
		// Look for tech stream keys
		if (line.find("\"range\"") != std::string::npos)
		{
			current_tech_stream = "range";
			in_tech_array = true;
			continue;
		}
		if (line.find("\"speed\"") != std::string::npos)
		{
			current_tech_stream = "speed";
			in_tech_array = true;
			continue;
		}
		if (line.find("\"weapons\"") != std::string::npos)
		{
			current_tech_stream = "weapons";
			in_tech_array = true;
			continue;
		}
		if (line.find("\"shields\"") != std::string::npos)
		{
			current_tech_stream = "shields";
			in_tech_array = true;
			continue;
		}
		if (line.find("\"mini\"") != std::string::npos)
		{
			current_tech_stream = "mini";
			in_tech_array = true;
			continue;
		}

		if (in_tech_array && line.find("]") != std::string::npos)
		{
			in_tech_array = false;
			continue;
		}

		// Parse level-name pairs: { "level": N, "name": "..." }
		if (in_tech_array && line.find("\"level\"") != std::string::npos && line.find("\"name\"") != std::string::npos)
		{
			// Extract level number
			size_t level_pos = line.find("\"level\"");
			size_t colon_pos = line.find(":", level_pos);
			size_t comma_pos = line.find(",", colon_pos);
			if (level_pos != std::string::npos && colon_pos != std::string::npos && comma_pos != std::string::npos)
			{
				std::string level_str = line.substr(colon_pos + 1, comma_pos - colon_pos - 1);
				// Remove whitespace
				level_str.erase(0, level_str.find_first_not_of(" \t"));
				level_str.erase(level_str.find_last_not_of(" \t") + 1);
				uint32_t level = std::stoul(level_str);

				// Extract name
				size_t name_pos = line.find("\"name\"");
				size_t name_colon = line.find(":", name_pos);
				size_t name_first = line.find("\"", name_colon);
				size_t name_last = line.rfind("\"");
				if (name_first != std::string::npos && name_last != std::string::npos && name_first != name_last)
				{
					std::string name = line.substr(name_first + 1, name_last - name_first - 1);
					if (!name.empty())
					{
						tech_level_names[current_tech_stream][level] = name;
					}
				}
			}
		}
	}

	file.close();
	return !tech_level_names.empty();
}

const std::map<uint32_t, std::string>& TextAssets::get_tech_level_names(const std::string& tech_stream) const
{
	static const std::map<uint32_t, std::string> empty;
	auto it = tech_level_names.find(tech_stream);
	if (it != tech_level_names.end())
		return it->second;
	return empty;
}

std::string TextAssets::get_tech_level_name(const std::string& tech_stream, uint32_t level) const
{
	try
	{
		return tech_level_names.at(tech_stream).at(level);
	}
	catch (const std::out_of_range&)
	{
		return "";
	}
}

// ============================================================================
// Radical Technology Descriptions
// ============================================================================

bool TextAssets::load_radical_techs(const std::string& assets_dir)
{
	std::string filepath = assets_dir + "/radical_tech.json";
	std::ifstream file(filepath);

	if (!file.is_open())
	{
		std::cerr << "Error: Could not open " << filepath << std::endl;
		return false;
	}

	std::string line;
	bool in_array = false;

	while (std::getline(file, line))
	{
		if (line.find("\"radical_techs\"") != std::string::npos)
		{
			in_array = true;
			continue;
		}

		if (in_array && line.find("]") != std::string::npos)
		{
			break;
		}

		if (in_array && line.find("\"") != std::string::npos)
		{
			size_t first = line.find("\"");
			size_t last = line.rfind("\"");
			if (first != std::string::npos && last != std::string::npos && first != last)
			{
				std::string description = line.substr(first + 1, last - first - 1);
				if (!description.empty())
				{
					radical_techs.push_back(description);
				}
			}
		}
	}

	file.close();
	return !radical_techs.empty();
}

const std::vector<std::string>& TextAssets::get_radical_techs() const
{
	return radical_techs;
}

std::string TextAssets::get_radical_tech_description(uint32_t index) const
{
	if (index >= radical_techs.size())
		return "";

	return radical_techs[index];
}
