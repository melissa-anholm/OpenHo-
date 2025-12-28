#include "game.h"
#include "game_constants.h"
#include "game_formulas.h"
#include "money_allocation_calculator.h"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <stdexcept>

// ============================================================================
// GameState Constructor
// ============================================================================

GameState::GameState(const GalaxyGenerationParams& params)
	: current_ai_rng_seed(0)
{
	// Initialize RNG with deterministic seed
	rng = std::make_unique<DeterministicRNG>(params.seed, params.seed);
	
	// Set up galaxy boundaries based on size
	double galaxyRadius = std::sqrt(params.size) * 10.0;  // Rough scaling
	galaxy.minX = -galaxyRadius;
	galaxy.maxX = galaxyRadius;
	galaxy.minY = -galaxyRadius;
	galaxy.maxY = galaxyRadius;
	
	galaxy.currentTurn = 0;
	
	// Initialize planets
	initialize_planets();
	
	// Initialize players (placeholder - will be set by game setup)
	initialize_players();
	
	// Build entity ID maps for quick lookup
	build_entity_maps();
}

// ============================================================================
// GameState Destructor
// ============================================================================

GameState::~GameState()
{
}

// ============================================================================
// Accessors
// ============================================================================

Galaxy& GameState::get_galaxy()
{
	return galaxy;
}

const Galaxy& GameState::get_galaxy() const
{
	return galaxy;
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
	if (it == player_id_to_index.end())
		return nullptr;
	return &galaxy.players[it->second];
}

const Player* GameState::get_player(uint32_t player_id) const
{
	auto it = player_id_to_index.find(player_id);
	if (it == player_id_to_index.end())
		return nullptr;
	return &galaxy.players[it->second];
}

Planet* GameState::get_planet(uint32_t planetID)
{
	auto it = planet_id_to_index.find(planetID);
	if (it == planet_id_to_index.end())
		return nullptr;
	return &galaxy.planets[it->second];
}

const Planet* GameState::get_planet(uint32_t planetID) const
{
	auto it = planet_id_to_index.find(planetID);
	if (it == planet_id_to_index.end())
		return nullptr;
	return &galaxy.planets[it->second];
}

Ship* GameState::get_ship(uint32_t shipID)
{
	auto it = ship_id_to_index.find(shipID);
	if (it == ship_id_to_index.end())
		return nullptr;
	return &galaxy.ships[it->second];
}

const Ship* GameState::get_ship(uint32_t shipID) const
{
	auto it = ship_id_to_index.find(shipID);
	if (it == ship_id_to_index.end())
		return nullptr;
	return &galaxy.ships[it->second];
}

const std::vector<size_t>& GameState::get_player_ships(uint32_t player_id) const
{
	static const std::vector<size_t> emptyVector;
	auto it = player_ships.find(player_id);
	if (it == player_ships.end())
		return emptyVector;
	return it->second;
}

const std::vector<size_t>& GameState::get_player_planets(uint32_t player_id) const
{
	static const std::vector<size_t> emptyVector;
	auto it = player_planets.find(player_id);
	if (it == player_planets.end())
		return emptyVector;
	return it->second;
}

// ============================================================================
// Money Allocation
// ============================================================================

void GameState::set_money_allocation(uint32_t player_id, const MoneyAllocation& alloc)
{
	Player* player = get_player(player_id);
	if (!player)
		throw std::runtime_error("Player not found");
	
	player->allocation = alloc;
}

const MoneyAllocation& GameState::get_money_allocation(uint32_t player_id) const
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

	void GameState::process_turn()
{
	// Process turn in order:
	// 1. Calculate player incomes
	// 2. Update planet incomes
	// 3. Process money allocation (research, terraforming, mining)
	// 4. Apply money interest
	// 5. Process research
	// 6. Process terraforming
	// 7. Process mining
	// 8. Process ships
	// 9. Process novae
	// 10. Capture public player information for all players
	
	calculate_player_incomes();
	update_planet_incomes();
	process_money_allocation();
	apply_money_interest();
	process_research();
	process_terraforming();
	process_mining();
	process_ships();
	process_novae();
	capture_player_public_info();
	
	galaxy.currentTurn++;
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

void GameState::initialize_planets()
{
	// Generate planets based on galaxy parameters
	// This is a placeholder implementation
	
	// For now, create a small test galaxy
	for (uint32_t i = 0; i < 10; ++i)
	{
		Planet planet;
		planet.id = i + 1;
		planet.name = "Planet_" + std::to_string(i + 1);
		
		// Random position within galaxy bounds
		planet.x = galaxy.minX + rng->nextDouble() * (galaxy.maxX - galaxy.minX);
		planet.y = galaxy.minY + rng->nextDouble() * (galaxy.maxY - galaxy.minY);
		
		// Random properties
		planet.gravity = 0.5 + rng->nextDouble() * 1.5;  // 0.5 to 2.0
		planet.temperature = -50.0 + rng->nextDouble() * 200.0;  // -50 to 150
		planet.metal = 100 + rng->nextInt32Range(0, 500);
		
		planet.owner = 0;  // Unowned
		planet.population = 0;
		planet.state = PLANET_NORMAL;
		planet.turnsUntilNova = 0;
		planet.income = 0;
		
		galaxy.planets.push_back(planet);
	}
}

void GameState::initialize_players()
{
	// Placeholder: Initialize players
	// This will be called during game setup
}

void GameState::build_entity_maps()
{
	// Build planet ID to index map and player planet ownership map
	for (size_t i = 0; i < galaxy.planets.size(); ++i)
	{
		planet_id_to_index[galaxy.planets[i].id] = i;
		if (galaxy.planets[i].owner != 0)
		{
			player_planets[galaxy.planets[i].owner].push_back(i);
		}
	}
	
	// Build ship ID to index map and player ship ownership map
	for (size_t i = 0; i < galaxy.ships.size(); ++i)
	{
		ship_id_to_index[galaxy.ships[i].id] = i;
		player_ships[galaxy.ships[i].owner].push_back(i);
	}
	
	// Build player ID to index map
	for (size_t i = 0; i < galaxy.players.size(); ++i)
	{
		player_id_to_index[galaxy.players[i].id] = i;
	}
}

void GameState::calculate_player_incomes()
{
	// For each player, calculate total income from all owned planets
	for (auto& player : galaxy.players)
	{
		player.moneyIncome = 0;
		player.metalIncome = 0;
		
		for (const auto& planet : galaxy.planets)
		{
			if (planet.owner == player.id)
			{
				player.moneyIncome += planet.income;
				// Metal income calculation would go here
			}
		}
	}
}

void GameState::update_planet_incomes()
{
	// For each planet, calculate income based on population, temperature, gravity, and owner's ideals
	for (auto& planet : galaxy.planets)
	{
		if (planet.owner == 0)
		{
			planet.income = 0;
			continue;
		}
		
		// Get the owner's ideal conditions
		const Player* owner = get_player(planet.owner);
		if (!owner)
		{
			planet.income = 0;
			continue;
		}
		
		// Income formula (placeholder):
		// Base income from population * happiness factor
		// Happiness factor depends on how close the planet is to the owner's ideals
		
		double tempDiff = std::abs(planet.temperature - owner->idealTemperature);
		double gravDiff = std::abs(planet.gravity - owner->idealGravity);
		
		// Simple happiness calculation (0.0 to 1.0)
		double tempHappiness = std::max(0.0, 1.0 - tempDiff / 100.0);
		double gravHappiness = std::max(0.0, 1.0 - gravDiff / 2.0);
		double happiness = (tempHappiness + gravHappiness) / 2.0;
		
		planet.income = static_cast<int32_t>(planet.population * happiness * 10);
	}
}

void GameState::process_money_allocation()
{
	// Apply money allocation from each player
	// This distributes the player's money to various purposes
	for (auto& player : galaxy.players)
	{
		// Add income to money
		player.money += player.moneyIncome;
		
		// Allocate money according to allocation settings
		// (This will be implemented in money_allocation.cpp)
	}
}

void GameState::process_research()
{
	// Process research progress for each player
	// (This will be implemented in a separate file)
}

void GameState::process_terraforming()
{
	// Process terraforming on planets
	// (This will be implemented in planet_mechanics.cpp)
}

void GameState::process_mining()
{
	// Process mining on planets
	// (This will be implemented in planet_mechanics.cpp)
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

void GameState::capture_player_public_info()
{
	// Capture public information for all players at the current turn
	for (const auto& player : galaxy.players)
	{
		PlayerPublicInfo info;
		info.player_id = player.id;
		info.turn = galaxy.currentTurn;
		
		// Technology levels (subset - no Radical)
		info.tech_range = player.tech.range;
		info.tech_speed = player.tech.speed;
		info.tech_weapons = player.tech.weapons;
		info.tech_shields = player.tech.shields;
		info.tech_miniaturization = player.tech.miniaturization;
		
		// Resources
		info.moneyIncome = player.moneyIncome;
		info.moneySavings = player.money;
		info.metalSavings = player.metalReserve;
		
		// Calculated metrics
		info.fleetPower = GameFormulas::calculatePlayerFleetPower(player.id, this);
		info.victoryPoints = GameFormulas::calculatePlayerVictoryPoints(player.id, this);
		
		// Store in history
		player_info_history[player.id].push_back(info);
	}
}

PlayerPublicInfo GameState::get_playerPublicInfo(uint32_t player_id, uint32_t turn) const
{
	PlayerPublicInfo info;
	std::memset(&info, 0, sizeof(PlayerPublicInfo));
	
	auto it = player_info_history.find(player_id);
	if (it == player_info_history.end())
		return info;
	
	const auto& history = it->second;
	
	// Find the entry for the requested turn
	// Since we store one entry per turn, we can use turn as an index
	// But we need to account for the fact that history might not start at turn 0
	if (history.empty())
		return info;
	
	// If turn is within range, retrieve it
	if (turn < history.size())
	{
		info = history[turn];
	}
	
	return info;
}

PlayerPublicInfo GameState::get_playerPublicInfoCurrent(uint32_t player_id) const
{
	PlayerPublicInfo info;
	std::memset(&info, 0, sizeof(PlayerPublicInfo));
	
	auto it = player_info_history.find(player_id);
	if (it == player_info_history.end())
		return info;
	
	const auto& history = it->second;
	if (history.empty())
		return info;
	
	// Return the most recent entry
	return history.back();
}

uint32_t GameState::get_player_info_historySize(uint32_t player_id) const
{
	auto it = player_info_history.find(player_id);
	if (it == player_info_history.end())
		return 0;
	
	return static_cast<uint32_t>(it->second.size());
}

// ============================================================================
// Ship Design Management
// ============================================================================

uint32_t GameState::create_ship_design(uint32_t player_id, const std::string& name, ShipType type, 
                                      int32_t tech_range, int32_t tech_speed, int32_t tech_weapons, 
                                      int32_t tech_shields, int32_t tech_miniaturization)
{
	Player* player = get_player(player_id);
	if (!player)
		return 0;
	
	// Check if player has reached the design limit
	if (player->shipDesigns.size() >= GameConstants::MAX_SHIP_DESIGNS_PER_PLAYER)
		return 0;
	
	// Create the new design
	ShipDesign design;
	design.id = player->nextShipDesignID++;  // Use monotonic counter for unique IDs
	design.name = name;
	design.type = type;
	design.tech_range = tech_range;
	design.tech_speed = tech_speed;
	design.tech_weapons = tech_weapons;
	design.tech_shields = tech_shields;
	design.tech_miniaturization = tech_miniaturization;
	
	// Calculate costs using formulas
	design.buildCost = GameFormulas::calculateShipDesignBuildCost(tech_range, tech_speed, tech_weapons, tech_shields, tech_miniaturization);
	design.prototypeCost = GameFormulas::calculateShipDesignPrototypeCost(tech_range, tech_speed, tech_weapons, tech_shields, tech_miniaturization);
	design.metalCost = GameFormulas::calculateShipDesignMetalCost(tech_range, tech_speed, tech_weapons, tech_shields, tech_miniaturization);
	
	// Add to permanent designs
	player->shipDesigns.push_back(design);
	
	return design.id;
}

const ShipDesign* GameState::get_shipDesign(uint32_t player_id, uint32_t design_id) const
{
	const Player* player = get_player(player_id);
	if (!player)
		return nullptr;
	
	// Search in permanent designs
	for (const auto& design : player->shipDesigns)
	{
		if (design.id == design_id)
			return &design;
	}
	
	return nullptr;
}

const std::vector<ShipDesign>& GameState::get_playerShipDesigns(uint32_t player_id) const
{
	static const std::vector<ShipDesign> emptyVector;
	
	const Player* player = get_player(player_id);
	if (!player)
		return emptyVector;
	
	return player->shipDesigns;
}

bool GameState::delete_ship_design(uint32_t player_id, uint32_t design_id)
{
	Player* player = get_player(player_id);
	if (!player)
		return false;
	
	// Check if any ships use this design
	for (const auto& ship : galaxy.ships)
	{
		if (ship.owner == player_id)
		{
			// Need to check if ship uses this design
			// (This requires adding design_id to Ship struct - TBD)
			return false;
		}
	}
	
	// Remove from vector
	auto it = std::find_if(player->shipDesigns.begin(), player->shipDesigns.end(),
	                      [design_id](const ShipDesign& d) { return d.id == design_id; });
	
	if (it != player->shipDesigns.end())
	{
		player->shipDesigns.erase(it);
		return true;
	}
	
	return false;
}

void GameState::build_ship_from_design(uint32_t player_id, uint32_t design_id)
{
	Player* player = get_player(player_id);
	if (!player)
		return;
	
	// Find the design
	const ShipDesign* design = getShipDesign(player_id, design_id);
	if (!design)
		return;
	
	// Building from the design
	// (Implementation for actually creating the ship will go here)
	// This will deduct costs from player->money and player->metalReserve
}



void GameState::apply_money_interest()
{
	// Apply interest to each player's money savings
	// Positive savings earn interest, negative savings (debt) incur interest
	for (auto& player : galaxy.players)
	{
		int64_t interest = GameFormulas::calculate_money_interest(player.money);
		player.money += interest;
	}
}


void GameState::process_research()
{
	// Process research for each player
	for (Player& player : galaxy.players)
	{
		// Calculate total planets budget to get planets_fraction
		double total_planets_fraction = 0.0;
		for (const PlanetDevelopmentAllocation& planet_alloc : player.allocation.planet_allocations)
		{
			total_planets_fraction += planet_alloc.development_fraction;
		}
		
		// Calculate research budget for this player
		int64_t research_budget = MoneyAllocationCalculator::calculate_research_amount(
			player.allocation, player.moneyIncome);
		
		// Process each research stream
		process_research_stream(player, TECH_RANGE, research_budget);
		process_research_stream(player, TECH_SPEED, research_budget);
		process_research_stream(player, TECH_WEAPONS, research_budget);
		process_research_stream(player, TECH_SHIELDS, research_budget);
		process_research_stream(player, TECH_MINIATURIZATION, research_budget);
		process_research_stream(player, TECH_RADICAL, research_budget);
	}
}

	void GameState::process_research_stream(Player& player, TechStream stream, int64_t research_budget)
	{
		// Calculate the budget for this specific research stream
		int64_t stream_budget = MoneyAllocationCalculator::calculate_research_stream_amount(
			player.allocation.research, stream, research_budget);
		
		// Convert money to research points using the conversion formula
		int64_t research_points_gained = GameFormulas::convert_money_to_research_points(stream_budget);
		
		// Get the current research points and tech level for this stream
	int64_t* research_points = nullptr;
	int32_t* tech_level = nullptr;
	
	switch (stream)
	{
		case TECH_RANGE:
			research_points = &player.research_points_range;
			tech_level = &player.tech.range;
			break;
		case TECH_SPEED:
			research_points = &player.research_points_speed;
			tech_level = &player.tech.speed;
			break;
		case TECH_WEAPONS:
			research_points = &player.research_points_weapons;
			tech_level = &player.tech.weapons;
			break;
		case TECH_SHIELDS:
			research_points = &player.research_points_shields;
			tech_level = &player.tech.shields;
			break;
		case TECH_MINIATURIZATION:
			research_points = &player.research_points_miniaturization;
			tech_level = &player.tech.miniaturization;
			break;
		case TECH_RADICAL:
			research_points = &player.research_points_radical;
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
		int64_t advancement_cost = 0;
		
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
			case TECH_MINIATURIZATION:
				advancement_cost = GameFormulas::calculate_tech_miniaturization_advancement_cost(*tech_level);
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
