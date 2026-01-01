#ifndef OPENHO_TEXT_ASSETS_H
#define OPENHO_TEXT_ASSETS_H

#include <vector>
#include <map>
#include <string>
#include "enums.h"
#include <map>
#include <cstdint>

// ============================================================================
// TextAssets Class
// ============================================================================

/**
 * Manages loading and access to text assets (names, descriptions, etc.)
 * loaded from JSON files in the TextAssets directory.
 */
class TextAssets
{
public:
	TextAssets();
	~TextAssets();

	// Load all text assets from JSON files
	bool load_assets(const std::string& assets_dir);

	// Planet names
	const std::vector<std::string>& get_planet_names() const;
	std::string get_random_planet_name(uint32_t seed) const;
	std::vector<std::string> get_planet_name_list() const;  // Returns a copy of all planet names

	// Player names
	const std::vector<std::string>& get_male_player_names() const;
	const std::vector<std::string>& get_female_player_names() const;
	std::string get_random_male_player_name(uint32_t seed) const;
	std::string get_random_female_player_name(uint32_t seed) const;
	std::vector<std::string> get_player_name_list(Gender gender) const;  // Returns a copy of player names for the given gender

	// Ship names by type
	const std::vector<std::string>& get_ship_names(const std::string& ship_type) const;
	std::string get_random_ship_name(const std::string& ship_type, uint32_t seed) const;

	// Technology level names
	const std::map<uint32_t, std::string>& get_tech_level_names(const std::string& tech_stream) const;
	std::string get_tech_level_name(const std::string& tech_stream, uint32_t level) const;

	// Radical technology descriptions
	const std::vector<std::string>& get_radical_techs() const;
	std::string get_radical_tech_description(uint32_t index) const;

private:
	// Asset storage
	std::vector<std::string> planet_names;
	std::vector<std::string> male_player_names;
	std::vector<std::string> female_player_names;
	std::map<std::string, std::vector<std::string>> ship_names;  // ship_type -> names
	std::map<std::string, std::map<uint32_t, std::string>> tech_level_names;  // tech_stream -> (level -> name)
	std::vector<std::string> radical_techs;

	// Helper methods
	bool load_planet_names(const std::string& assets_dir);
	bool load_player_names(const std::string& assets_dir);
	bool load_ship_names(const std::string& assets_dir);
	bool load_tech_level_names(const std::string& assets_dir);
	bool load_radical_techs(const std::string& assets_dir);
};

#endif  // OPENHO_TEXT_ASSETS_H
