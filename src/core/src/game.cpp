#include "game.h"
#include "game_setup.h"
#include "game_constants.h"
#include "game_formulas.h"
#include "text_assets.h"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <iostream>
#include <stdexcept>

// ============================================================================
// GameState Constructor
// ============================================================================

GameState::GameState(const class GameSetup& setup)
	: current_ai_rng_seed(0),
	  galaxy_params(setup.get_galaxy_params()),
	  player_setups(setup.get_player_setups())
{
	// Setup data has been copied and stored as members in initializer list
	
	// Initialize RNG with default seed
	rng = std::make_unique<DeterministicRNG>(0, 0);
	
	// Load text assets
	text_assets = std::make_unique<TextAssets>();
	text_assets->load_assets("TextAssets");
	
	// Initialize players with setup configuration
	players = initialize_players(player_setups);
	
	// Initialize galaxy with provided parameters
	initialize_galaxy(galaxy_params);
	
	// Initialize the first turn
	start_first_turn();
}

// ============================================================================
// GameState Destructor
// ============================================================================

GameState::~GameState()
{ }

// ============================================================================
// Accessors
// ============================================================================

Galaxy& GameState::get_galaxy()
{
	return *galaxy;
}
const Galaxy& GameState::get_galaxy() const
{
	return *galaxy;
}

DeterministicRNG& GameState::get_rng()
{
	return *rng;
}
const DeterministicRNG& GameState::get_rng() const
{
	return *rng;
}

Player* GameState::get_player(uint32_t player_id)
{
	auto it = player_id_to_index.find(player_id);
	if (it == player_id_to_index.end() )
		{ return nullptr; }
	return &players[it->second];
}
const Player* GameState::get_player(uint32_t player_id) const
{
	auto it = player_id_to_index.find(player_id);
	if (it == player_id_to_index.end())
		{ return nullptr; }
	return &players[it->second];
}

Player* GameState::get_player_by_name(const std::string& name)
{
	for (Player& player : players)
	{
		if (player.name == name)
			{ return &player; }
	}
	return nullptr;
}
const Player* GameState::get_player_by_name(const std::string& name) const
{
	for (const Player& player : players)
	{
		if (player.name == name)
			{ return &player; }
	}
	return nullptr;
}

int64_t GameState::get_player_money_income(uint32_t player_id) const
{
	const Player* player = get_player(player_id);
	return player ? player->money_income : 0;
}
int64_t GameState::get_player_metal_income(uint32_t player_id) const
{
	const Player* player = get_player(player_id);
	return player ? player->metal_income : 0;
}

int64_t GameState::get_player_money(uint32_t player_id) const
{
	const Player* player = get_player(player_id);
	return player ? player->money_savings : 0;
}
int64_t GameState::get_player_metal_reserve(uint32_t player_id) const
{
	const Player* player = get_player(player_id);
	return player ? player->metal_reserve : 0;
}

double GameState::get_player_ideal_temperature(uint32_t player_id) const
{
	const Player* player = get_player(player_id);
	return player ? player->ideal_temperature : 0.0;
}
double GameState::get_player_ideal_gravity(uint32_t player_id) const
{
	const Player* player = get_player(player_id);
	return player ? player->ideal_gravity : 0.0;
}

Planet* GameState::get_planet(uint32_t planetID)
{
	auto it = planet_id_to_index.find(planetID);
	if (it == planet_id_to_index.end())
		{ return nullptr; }
	return &galaxy->planets[it->second];
}
const Planet* GameState::get_planet(uint32_t planetID) const
{
	auto it = planet_id_to_index.find(planetID);
	if (it == planet_id_to_index.end())
		{ return nullptr; }
	return &galaxy->planets[it->second];
}

Planet* GameState::get_planet(const std::string& planet_name)
{
	auto it = planet_name_to_index.find(planet_name);
	if (it == planet_name_to_index.end())
		{ return nullptr; }
	return &galaxy->planets[it->second];
}
const Planet* GameState::get_planet(const std::string& planet_name) const
{
	auto it = planet_name_to_index.find(planet_name);
	if (it == planet_name_to_index.end())
		{ return nullptr; }
	return &galaxy->planets[it->second];
}


const std::vector<size_t>& GameState::get_player_planets(uint32_t player_id) const
{
	static const std::vector<size_t> emptyVector;
	auto it = player_planets.find(player_id);
	if (it == player_planets.end() )
		{ return emptyVector; } 
	return it->second;
}

uint32_t GameState::get_num_players() const
{
	return static_cast<uint32_t>(players.size());
}

const std::vector<Player>& GameState::get_players() const
{
	return players;
}

std::vector<Player>& GameState::get_players()
{
	return players;
}

void GameState::increment_year()
{
	current_year += 10;
}
void GameState::increment_turn()
{
	current_turn++;
}

uint32_t GameState::get_current_year() const
{
	return current_year;
}
uint32_t GameState::get_current_turn() const
{
	return current_turn;
}

uint32_t GameState::allocate_fleet_id()
{
	return next_fleet_id++;
}

// ============================================================================
// Money Allocation
// ============================================================================

void GameState::set_money_allocation(uint32_t player_id, const Player::MoneyAllocation& money_alloc)
{
	Player* player = get_player(player_id);
	if (!player) 
		{ throw std::runtime_error("Player not found"); }
	
	player->allocation = money_alloc;
}

const Player::MoneyAllocation& GameState::get_money_allocation(uint32_t player_id) const
{
	const Player* player = get_player(player_id);
	if (!player)
		throw std::runtime_error("Player not found");
	
	return player->allocation;
}

// ============================================================================
// AI RNG Seed Management
// ============================================================================
uint64_t GameState::get_ai_rng_seed() const
{
	return current_ai_rng_seed;
}

void GameState::set_ai_rng_seed(uint64_t seed)
{
	current_ai_rng_seed = seed;
	rng->setAISeed(seed);
}

// ============================================================================
// Turn Processing
// ============================================================================
void GameState::start_first_turn()
{
	// Initialize the first turn with initial game state
	// This is called during game initialization before any process_turn() calls
	capture_and_distribute_player_public_info();
}

void GameState::process_turn()
{
	// Process turn in order:
	// 0. Capture and distribute public player information from previous turn
	// 1. Calculate player incomes
	// 2. Update planet incomes
	// 3. Process money allocation (research, terraforming, mining)
	// 4. Apply money interest
	// 5. Process research
	// 6. Process terraforming
	// 7. Process mining
	// 8. Process ships
	// 9. Process novae
	
	capture_and_distribute_player_public_info();
	
	calculate_player_incomes();
	update_planet_incomes();
	process_money_allocation();
	process_research();
	process_planets();
	process_ships();
	process_novae();
	
	increment_turn();
	increment_year();
}

// ============================================================================
// Serialization
// ============================================================================

std::vector<uint8_t> GameState::serialize_state() const
{
	// Placeholder for serialization
	// This will be implemented in serialization.cpp
	std::vector<uint8_t> buffer;
	return buffer;
}

bool GameState::deserialize_state(const std::vector<uint8_t>& data)
{
	// Placeholder for deserialization
	// This will be implemented in serialization.cpp
	return true;
}

// ============================================================================
// Private Helper Methods
// ============================================================================

std::vector<Player> GameState::initialize_players(const std::vector<PlayerSetup>& player_setups)
{
	// Initialize players based on setup configuration
	std::vector<Player> new_players;
	
	// TODO: Populate players with initial state based on player_setups
	// For now, create default players
	// In the future, this will use player_setups to configure:
	// - Player names
	// - Player types (human vs AI)
	// - AI difficulty levels
	
	return new_players;
}

void GameState::initialize_galaxy(const GalaxyGenerationParams& params)
{
	// Initialize galaxy with user-provided parameters
	// Retry if we don't have enough suitable home planets for all players
	
	uint32_t attempt = 0;
	bool success = false;
	
	while (attempt < GameConstants::Galaxy_Gen_Retry_Count && !success)
	{
		// Create galaxy with current parameters
		GalaxyGenerationParams attempt_params = params;
		attempt_params.seed = params.seed + attempt;
		
		galaxy = std::make_unique<Galaxy>(attempt_params, this);
		
		// Check if we have enough suitable home planets
		std::vector<Planet*> suitable_planets = find_suitable_home_planets();
		
		if (suitable_planets.size() >= players.size())
		{
			success = true;
			if (attempt > 0)
			{
				std::cout << "Galaxy generated successfully on attempt " << (attempt + 1) 
				          << " with " << suitable_planets.size() << " suitable home planets.\n";
			}
		}
		else
		{
			std::cerr << "Attempt " << (attempt + 1) << "/" << GameConstants::Galaxy_Gen_Retry_Count 
			          << ": Only " << suitable_planets.size() << " suitable planets found, need " 
			          << players.size() << ". Retrying...\n";
			attempt++;
		}
	}
	
	if (!success)
	{
		throw std::runtime_error("Failed to generate galaxy with sufficient home planets after " + 
		                         std::to_string(GameConstants::Galaxy_Gen_Retry_Count) + " attempts.");
	}
	
	// Build entity ID maps for quick lookup
	build_entity_maps();
	
	// Initialize the first turn
	start_first_turn();
}

void GameState::build_entity_maps()
{
	// Build planet ID to index map, planet name to index map, and player planet ownership map
	for (size_t i = 0; i < galaxy->planets.size(); ++i)
	{
		planet_id_to_index[galaxy->planets[i].id] = i;
		planet_name_to_index[galaxy->planets[i].name] = i;
		if (galaxy->planets[i].owner != 0)
		{
			player_planets[galaxy->planets[i].owner].push_back(i);
		}
	}
	
	// Build player ID to index map and player name to index map
	for (size_t i = 0; i < players.size(); ++i)
	{
		player_id_to_index[players[i].id] = i;
		player_name_to_index[players[i].name] = i;
	}
	
	// Build per-player fleet mapping
	for (size_t player_idx = 0; player_idx < players.size(); ++player_idx)
	{
		const auto& player = players[player_idx];
		const auto& fleets = player.get_fleets();
		for (size_t fleet_idx = 0; fleet_idx < fleets.size(); ++fleet_idx)
		{
			player_fleets[player.id].push_back(fleet_idx);
		}
	}
}

void GameState::calculate_player_incomes()
{
	// For each player, calculate total income from all owned planets
	for (auto& player : players)
	{
		player.money_income = 0;
		player.metal_income = 0;
		
		for (const auto& colonized : player.colonized_planets)
		{
			player.money_income += colonized.get_income();
			// Metal income calculation would go here
		}
	}
}

void GameState::update_planet_incomes()
{
	// For each player's colonized planet, calculate income based on population, temperature, gravity, and owner's ideals
	for (auto& player : players)
	{
		for (auto& colonized : player.colonized_planets)
		{
			// Get the actual planet object from the galaxy
			Planet* planet = get_planet(colonized.get_id());
			if (!planet)
			{
				colonized.set_income(0);
				continue;
			}
			
			// Income formula (placeholder):
			// Base income from population * happiness factor
			// Happiness factor depends on how close the planet is to the owner's ideals
			
			double tempDiff = std::abs(planet->true_temperature - player.ideal_temperature);
			double gravDiff = std::abs(planet->true_gravity - player.ideal_gravity);
			
			// Simple happiness calculation (0.0 to 1.0)
			double tempHappiness = std::max(0.0, 1.0 - tempDiff / 100.0);
			double gravHappiness = std::max(0.0, 1.0 - gravDiff / 2.0);
			double happiness = (tempHappiness + gravHappiness) / 2.0;
			
			colonized.set_income(static_cast<int32_t>(colonized.get_population() * happiness * 10));
		}
	}
}

void GameState::process_money_allocation()
{
	// Apply money allocation from each player
	// This distributes the player's money to various purposes
	for (auto& player : players)
	{
		// Add income to money
		player.money_savings += player.money_income;
		
		// Allocate money according to allocation settings
		// (This will be implemented in money_allocation.cpp)
	}
}

void GameState::process_ships()
{
	// Process ship movement and actions
	// (This will be implemented in a separate file)
}

void GameState::process_novae()
{
	// Process nova events
	// (This will be implemented in planet_mechanics.cpp)
}

// ============================================================================
// Player Public Information
// ============================================================================

void GameState::capture_and_distribute_player_public_info()
{
	// Capture public information for all players at the current turn
	for (const auto& player : players)
	{
		PlayerPublicInfo info;
		info.player_id = player.id;
		info.year = current_year;
		info.turn = current_turn;
		
		// Technology levels (subset - no Radical)
		info.tech_range = player.tech.range;
		info.tech_speed = player.tech.speed;
		info.tech_weapons = player.tech.weapons;
		info.tech_shields = player.tech.shields;
		info.tech_mini = player.tech.mini;
		
		// Resources
		info.money_income = player.money_income;
		info.money_savings = player.money_savings;
		info.metal_savings = player.metal_reserve;
		
		// Territory
		info.planets_owned = static_cast<uint32_t>(player.colonized_planets.size());
		
		// Calculated metrics
		info.ship_power = GameFormulas::calculate_player_fleet_power(player.id, this);
		info.victory_points = GameFormulas::calculate_player_victory_points(player.id, this);
		
		// Store in history
		player_info_history[player.id].push_back(info);
	}
}



// ============================================================================
// Ship Design Management
// ============================================================================
uint32_t GameState::create_ship_design(uint32_t player_id, const std::string& name, ShipType type, 
                                      int32_t tech_range, int32_t tech_speed, int32_t tech_weapons, 
                                      int32_t tech_shields, int32_t tech_mini)
{
	Player* player = get_player(player_id);
	if (!player) 
		{ return 0; }
	
	// Delegate to Player
	return player->create_ship_design(name, type, tech_range, tech_speed, tech_weapons, tech_shields, tech_mini);
}

const ShipDesign* GameState::get_ship_design(uint32_t player_id, uint32_t design_id) const
{
	const Player* player = get_player(player_id);
	if (!player) 
		{ return nullptr; }
	
	// Delegate to Player
	return player->get_ship_design(design_id);
}

const std::vector<ShipDesign>& GameState::get_player_ship_designs(uint32_t player_id) const
{
	static const std::vector<ShipDesign> emptyVector;
	
	const Player* player = get_player(player_id);
	if (!player)
		{ return emptyVector; }
	
	// Delegate to Player
	return player->get_ship_designs();
}

bool GameState::delete_ship_design(uint32_t player_id, uint32_t design_id)
{
	Player* player = get_player(player_id);
	if (!player)
		{ return false; }
	
	// Delegate to Player
	return player->delete_ship_design(design_id);
}

void GameState::build_ship_from_design(uint32_t player_id, uint32_t design_id)
{
	Player* player = get_player(player_id);
	if (!player)
		{ return; }
	
	// Find the design
	const ShipDesign* design = get_ship_design(player_id, design_id);
	if (!design)
		{ return; }
	
	// Building from the design
	// (Implementation for actually creating the ship will go here)
	// This will deduct costs from player->money_savings and player->metalReserve
}

// Research Processing!
void GameState::process_research()
{
	// Process research for each player
	for (Player& player : players)
	{
		// Calculate research budget for this player
		int64_t research_budget = Player::calculate_research_amount(
			player.allocation, player.money_income);
		
		// Process each research stream
		process_research_stream(player, TECH_RANGE,   research_budget);
		process_research_stream(player, TECH_SPEED,   research_budget);
		process_research_stream(player, TECH_WEAPONS, research_budget);
		process_research_stream(player, TECH_SHIELDS, research_budget);
		process_research_stream(player, TECH_MINI,    research_budget);
		process_research_stream(player, TECH_RADICAL, research_budget);
	}
}

void GameState::process_research_stream(Player& player, TechStream stream, int64_t research_budget)
{
	// Calculate the budget for this specific research stream
	int64_t stream_budget = Player::calculate_research_stream_amount(
		player.allocation.research, stream, research_budget);
	
	// Convert money to research points using the conversion formula
	int64_t research_points_gained = GameFormulas::convert_money_to_research_points(stream_budget);
	
	// Get the current research points and tech level for this stream
	int64_t* research_points = nullptr;
	int32_t* tech_level = nullptr;

	switch (stream)
	{
		case TECH_RANGE:
			research_points = &player.partial_research.research_points_range;
			tech_level = &player.tech.range;
			break;
		case TECH_SPEED:
			research_points = &player.partial_research.research_points_speed;
			tech_level = &player.tech.speed;
			break;
		case TECH_WEAPONS:
			research_points = &player.partial_research.research_points_weapons;
			tech_level = &player.tech.weapons;
			break;
		case TECH_SHIELDS:
			research_points = &player.partial_research.research_points_shields;
			tech_level = &player.tech.shields;
			break;
		case TECH_MINI:
			research_points = &player.partial_research.research_points_mini;
			tech_level = &player.tech.mini;
			break;
		case TECH_RADICAL:
			research_points = &player.partial_research.research_points_radical;
			tech_level = &player.tech.radical;
			break;
		default:
			return;
	}
	// Add the converted research points to the player's research points
	*research_points += research_points_gained;
	
	// Check if we can advance the technology level
	while (true)
	{
		int64_t advancement_cost = 10;
		
		switch (stream)
		{
			case TECH_RANGE:
				advancement_cost = GameFormulas::calculate_tech_range_advancement_cost(*tech_level);
				break;
			case TECH_SPEED:
				advancement_cost = GameFormulas::calculate_tech_speed_advancement_cost(*tech_level);
				break;
			case TECH_WEAPONS:
				advancement_cost = GameFormulas::calculate_tech_weapons_advancement_cost(*tech_level);
				break;
			case TECH_SHIELDS:
				advancement_cost = GameFormulas::calculate_tech_shields_advancement_cost(*tech_level);
				break;
			case TECH_MINI:
				advancement_cost = GameFormulas::calculate_tech_mini_advancement_cost(*tech_level);
				break;
			case TECH_RADICAL:
				advancement_cost = GameFormulas::calculate_tech_radical_advancement_cost(*tech_level);
				break;
			default:
				return;
		}
		
		// If we have enough research points to advance, do so
		if (*research_points >= advancement_cost)
		{
			*research_points -= advancement_cost;
			(*tech_level)++;
		}
		else
		{
			// Not enough points, stop checking
			break;
		}
	}
}


void GameState::process_planets()
{
	// Process planets for each player
	for (Player& player : players)
	{
		// Calculate total money available for planet development this turn
		int64_t total_planet_development_budget = static_cast<int64_t>(
			player.money_income * player.allocation.planets_fraction );
		
		// Iterate through each colonized planet
		for (ColonizedPlanet& colonized : player.colonized_planets)
		{
			// Get the planet object
			Planet* planet = get_planet(colonized.get_id() );
			if (!planet || planet->owner != player.id)
				{ continue; }  // Skip if planet doesn't exist or isn't owned by this player 
			
			// Calculate money allocated to this specific planet
			int64_t planet_budget = static_cast<int64_t>(
				total_planet_development_budget * colonized.get_funding_fraction() );
			
			// Split the planet budget between mining and terraforming
			int64_t terraforming_budget = static_cast<int64_t>(
				planet_budget * colonized.get_terraforming_fraction() );
			int64_t mining_budget = static_cast<int64_t>(
				planet_budget * colonized.get_mining_fraction() );
			
			// TERRAFORMING: Calculate and apply temperature change
			double temperature_change = GameFormulas::calculate_temperature_change(
				terraforming_budget, planet->true_temperature, player.ideal_temperature );
				
			planet->true_temperature += temperature_change;
			
			// MINING: Calculate metal extraction and update reserves
			int64_t metal_extracted = GameFormulas::calculate_metal_mined(
				mining_budget, planet->metal );
			planet->metal -= metal_extracted;
			player.metal_reserve += metal_extracted;
		}
	}
}


// ============================================================================
// Fleet Management Methods
// ============================================================================
uint32_t GameState::create_fleet(uint32_t player_id, uint32_t design_id, 
                                 uint32_t ship_count, uint32_t planet_id)
{
	Player* player = get_player(player_id);
	if (!player)
		{ return 0; }
	
	// Validate and get the ship design
	const ShipDesign* design = player->get_ship_design(design_id);
	if (!design)
		{ return 0; }  // Design not found for this player
	
	// Validate and get the planet
	Planet* planet = get_planet(planet_id);
	if (!planet)
		{ return 0; } // Planet not found
	
	// Allocate a globally unique fleet ID
	uint32_t fleet_id = allocate_fleet_id();
	
	// Delegate to Player with validated pointers
	return player->create_fleet(fleet_id, design, ship_count, planet);
}

Fleet* GameState::get_fleet(uint32_t player_id, uint32_t fleet_id)
{
	Player* player = get_player(player_id);
	if (!player)
		{ return nullptr; }
	
	// Delegate to Player
	return player->get_fleet(fleet_id);
}
const Fleet* GameState::get_fleet(uint32_t player_id, uint32_t fleet_id) const
{
	const Player* player = get_player(player_id);
	if (!player) 
		{ return nullptr; }
	
	// Delegate to Player
	return player->get_fleet(fleet_id);
}

const std::vector<Fleet>& GameState::get_player_fleets(uint32_t player_id) const
{
	static const std::vector<Fleet> emptyVector;
	
	const Player* player = get_player(player_id);
	if (!player)
		{ return emptyVector; }
	
	return player->fleets;
}

bool GameState::delete_fleet(uint32_t player_id, uint32_t fleet_id)
{
	Player* player = get_player(player_id);
	if (!player)
		{ return false; }
	
	// Delegate to Player
	return player->delete_fleet(fleet_id);
}

void GameState::move_fleet(uint32_t player_id, uint32_t fleet_id, uint32_t destination_planet_id)
{
	Fleet* fleet = get_fleet(player_id, fleet_id);
	if (!fleet)
		{ return; }
	
	Planet* destination = get_planet(destination_planet_id);
	if (!destination)
		{ return; }
	
	// If fleet is already at destination, do nothing
	if (fleet->current_planet && fleet->current_planet->id == destination_planet_id)
		{ return; }
	
	// Set up movement
	fleet->origin_planet = fleet->current_planet;
	fleet->destination_planet = destination;
	fleet->current_planet = nullptr;
	fleet->in_transit = true;
	
	// Calculate distance and turns (placeholder - will be refined with actual pathfinding)
	if (fleet->origin_planet && fleet->destination_planet)
	{
		double dx = fleet->destination_planet->x - fleet->origin_planet->x;
		double dy = fleet->destination_planet->y - fleet->origin_planet->y;
		fleet->distance_to_destination = std::sqrt(dx * dx + dy * dy);
		
		// Calculate turns based on fleet speed
		if (fleet->ship_design && fleet->ship_design->get_speed() > 0)
		{
			fleet->turns_to_destination = static_cast<uint32_t>(
				std::ceil(fleet->distance_to_destination / fleet->ship_design->get_speed() ) );
		}
		else
		{
			fleet->turns_to_destination = 0;
		}
	}
}

void GameState::refuel_fleet(uint32_t player_id, uint32_t fleet_id)
{
	Fleet* fleet = get_fleet(player_id, fleet_id);
	if (!fleet)
		{ return; }
	
	fleet->refuel();
}



// ============================================================================
// Starting Planet Assignment (for random galaxies)
// ============================================================================

std::vector<Planet*> GameState::find_suitable_home_planets() const
{
	std::vector<Planet*> suitable_planets;
	
	// Filter planets by gravity range
	for (Planet& planet : galaxy->planets)
	{
		// Check if gravity is within suitable range
		if (planet.true_gravity >= GameConstants::Starting_Planet_Min_Gravity &&
		    planet.true_gravity <= GameConstants::Starting_Planet_Max_Gravity)
		{
			// Check if planet is not already owned
			if (planet.owner == NOT_OWNED)
			{
				suitable_planets.push_back(&planet);
			}
		}
	}
	
	return suitable_planets;
}

std::vector<size_t> GameState::generate_suitable_home_planets() const
{
	std::vector<size_t> suitable_planets;
	
	// Filter planets by gravity range
	for (size_t i = 0; i < galaxy->planets.size(); ++i)
	{
		const Planet& planet = galaxy->planets[i];
		
		// Check if gravity is within suitable range
		if (planet.true_gravity >= GameConstants::Starting_Planet_Min_Gravity &&
		    planet.true_gravity <= GameConstants::Starting_Planet_Max_Gravity)
		{
			// Check if planet is not already owned
			if (planet.owner == NOT_OWNED)
			{
				suitable_planets.push_back(i);
			}
		}
	}
	
	return suitable_planets;
}

void GameState::assign_planets_random(StartingColonyQuality quality)
{
	// Get list of suitable home planets (already validated in initialize_galaxy)
	std::vector<Planet*> suitable_planets = find_suitable_home_planets();
	
	// Validate that we have enough suitable planets
	if (suitable_planets.size() < players.size())
	{
		std::cerr << "ERROR: Not enough suitable home planets (" << suitable_planets.size() 
		          << ") for " << players.size() << " players.\n";
		// TODO: Implement error handling strategy
		return;
	}
	
	// Randomly shuffle suitable planets using RNG
	for (size_t i = suitable_planets.size() - 1; i > 0; --i)
	{
		size_t j = rng->nextInt32Range(0, i);
		std::swap(suitable_planets[i], suitable_planets[j]);
	}
	
	// Assign one planet to each player
	for (size_t player_idx = 0; player_idx < players.size(); ++player_idx)
	{
		Player& player = players[player_idx];
		Planet* planet = suitable_planets[player_idx];
		
		// Set planet ownership
		planet->owner = player.id;
		
		// Set player's ideal gravity to match starting planet
		player.ideal_gravity = planet->true_gravity;
		
		// Create ColonizedPlanet entry for this player with quality-based values
		ColonizedPlanet colonized_planet(
			planet,
			&player,
			GameConstants::Starting_Colony_Population[quality],
			GameConstants::Starting_Colony_Income[quality]
		);
		
		// TODO: Set metal based on quality (add to ColonizedPlanet if needed)
		
		// Add to player's colonized planets
		player.colonized_planets.push_back(colonized_planet);
		
		// Update player_planets mapping
		auto it = planet_id_to_index.find(planet->id);
		if (it != planet_id_to_index.end())
		{
			player_planets[player.id].push_back(it->second);
		}
		
		// Log assignment
		std::cout << "Player " << player.name << " assigned starting planet: " 
		          << planet->name << " (gravity: " << planet->true_gravity << ")\n";
	}
	
	std::cout << "Successfully assigned starting planets to all " << players.size() << " players.\n";
}
