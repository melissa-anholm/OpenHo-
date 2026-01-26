#include "game.h"
#include "game_setup.h"
#include "game_constants.h"
#include "game_formulas.h"
#include "text_assets.h"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <iostream>
#include <random>
#include <stdexcept>

// ============================================================================
// GameState Constructor
// ============================================================================

GameState::GameState(const class GameSetup& setup)
	: galaxy_params(setup.get_galaxy_params()),
	  player_setups(setup.get_player_setups())
{
	// Setup data has been copied and stored as members in initializer list
	
	// Initialize RNG with random seeds from std::random_device
	// This ensures each game has different random behavior
	std::random_device rd;
	uint64_t det_seed = rd();
	uint64_t ai_seed = det_seed + 1;  // Use seed + 1 for AI RNG
	rng = std::make_unique<DeterministicRNG>(det_seed, ai_seed);
	
	// Load text assets
	text_assets = std::make_unique<TextAssets>();
	text_assets->load_assets("TextAssets");
	
	// Initialize players with setup configuration
	players = initialize_players(player_setups);
	
	// Initialize galaxy with provided parameters
	// This also assigns planets to players internally
	galaxy = initialize_galaxy(galaxy_params);
	
	// Initialize KnowledgeGalaxy for each player
	initialize_player_knowledge();
	
	// Initialize research cost caches
	initialize_research_cost_caches();
	
	// Initialize the first turn
	start_first_turn();
}

// ============================================================================
// GameState Destructor
// ============================================================================
GameState::~GameState()
{ }

// ============================================================================
// Player Accessors
// ============================================================================
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


// Note: get_player_planets removed - use player->colonized_planets instead

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
	apply_money_interest();
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
// Research Cost Cache Management
// ============================================================================

void GameState::initialize_research_cost_caches()
{
	// Initialize all research cost vectors with starting tech levels
	// Players start with tech level 1 for most streams, so we initialize with 2 zeroes
	// research_cost_X[0] = 0 (cost to reach level 0, starting level)
	// research_cost_X[1] = 0 (cost to reach level 1, starting level)
	// research_cost_X[2] = actual cost to advance from level 1 to 2
	// etc.
	
	const int32_t INITIAL_TECH_LEVEL = 1;
	const int32_t INITIAL_CACHE_SIZE = INITIAL_TECH_LEVEL + 1;  // +1 for level 0
	const int32_t CACHE_EXTENSION_SIZE = 20;
	
	// Initialize each tech stream cache
	research_cost_range.assign(INITIAL_CACHE_SIZE + CACHE_EXTENSION_SIZE, 0);
	research_cost_speed.assign(INITIAL_CACHE_SIZE + CACHE_EXTENSION_SIZE, 0);
	research_cost_weapons.assign(INITIAL_CACHE_SIZE + CACHE_EXTENSION_SIZE, 0);
	research_cost_shields.assign(INITIAL_CACHE_SIZE + CACHE_EXTENSION_SIZE, 0);
	research_cost_mini.assign(INITIAL_CACHE_SIZE + CACHE_EXTENSION_SIZE, 0);
	research_cost_radical.assign(INITIAL_CACHE_SIZE + CACHE_EXTENSION_SIZE, 0);
	
	// Fill in the calculated costs starting from INITIAL_CACHE_SIZE
	for (int32_t level = INITIAL_CACHE_SIZE; level < INITIAL_CACHE_SIZE + CACHE_EXTENSION_SIZE; ++level)
	{
		research_cost_range[level] = GameFormulas::calculate_tech_range_advancement_cost(level - 1);
		research_cost_speed[level] = GameFormulas::calculate_tech_speed_advancement_cost(level - 1);
		research_cost_weapons[level] = GameFormulas::calculate_tech_weapons_advancement_cost(level - 1);
		research_cost_shields[level] = GameFormulas::calculate_tech_shields_advancement_cost(level - 1);
		research_cost_mini[level] = GameFormulas::calculate_tech_mini_advancement_cost(level - 1);
		research_cost_radical[level] = GameFormulas::calculate_tech_radical_advancement_cost(level - 1);
	}
}

void GameState::ensure_research_costs_available(int32_t max_tech_level)
{
	// Check if we need to extend the cache for any tech stream
	const int32_t CACHE_EXTENSION_SIZE = 20;
	
	// Find the maximum current cache size
	size_t max_cache_size = std::max({
		research_cost_range.size(),
		research_cost_speed.size(),
		research_cost_weapons.size(),
		research_cost_shields.size(),
		research_cost_mini.size(),
		research_cost_radical.size()
	});
	
	// If max_tech_level + 1 is beyond current cache, extend all caches
	if (static_cast<size_t>(max_tech_level + 1) >= max_cache_size)
	{
		size_t new_size = max_tech_level + 1 + CACHE_EXTENSION_SIZE;
		
		// Extend each cache and fill in new values
		size_t old_size = research_cost_range.size();
		research_cost_range.resize(new_size);
		for (size_t level = old_size; level < new_size; ++level)
		{
			research_cost_range[level] = GameFormulas::calculate_tech_range_advancement_cost(level - 1);
		}
		
		old_size = research_cost_speed.size();
		research_cost_speed.resize(new_size);
		for (size_t level = old_size; level < new_size; ++level)
		{
			research_cost_speed[level] = GameFormulas::calculate_tech_speed_advancement_cost(level - 1);
		}
		
		old_size = research_cost_weapons.size();
		research_cost_weapons.resize(new_size);
		for (size_t level = old_size; level < new_size; ++level)
		{
			research_cost_weapons[level] = GameFormulas::calculate_tech_weapons_advancement_cost(level - 1);
		}
		
		old_size = research_cost_shields.size();
		research_cost_shields.resize(new_size);
		for (size_t level = old_size; level < new_size; ++level)
		{
			research_cost_shields[level] = GameFormulas::calculate_tech_shields_advancement_cost(level - 1);
		}
		
		old_size = research_cost_mini.size();
		research_cost_mini.resize(new_size);
		for (size_t level = old_size; level < new_size; ++level)
		{
			research_cost_mini[level] = GameFormulas::calculate_tech_mini_advancement_cost(level - 1);
		}
		
		old_size = research_cost_radical.size();
		research_cost_radical.resize(new_size);
		for (size_t level = old_size; level < new_size; ++level)
		{
			research_cost_radical[level] = GameFormulas::calculate_tech_radical_advancement_cost(level - 1);
		}
	}
}

// ============================================================================
// Private Helper Methods
// ============================================================================
std::vector<Player> GameState::initialize_players(const std::vector<PlayerSetup>& player_setups)
{
	// Initialize players based on setup configuration
	std::vector<Player> new_players;
	
	int32_t player_id = 1;  // Player IDs start at 1 (NOT_OWNED is reserved)
	
	for (const auto& setup : player_setups)
	{
		Player player(this);
		player.id = player_id++;
		player.type = setup.type;
		player.iq = setup.ai_iq;
		
		// Handle player name and gender
		if (setup.type == PLAYER_COMPUTER && setup.name.empty())
		{
			// Computer player with placeholder name: assign gender and name from TextAssets
			Gender selected_gender = (rng->nextInt32Range(0, 1) == 0) ? GENDER_F : GENDER_M;
			player.gender = selected_gender;
			
			// Get gender-appropriate names from TextAssets and select one randomly
			const std::vector<std::string>* names = nullptr;
			if (selected_gender == GENDER_F)
			{
				names = &text_assets->get_female_player_names();
			}
			else
			{
				names = &text_assets->get_male_player_names();
			}
			
			if (names && !names->empty())
			{
				uint32_t name_index = rng->nextUInt32Range(0, names->size() - 1);
				player.name = (*names)[name_index];
			}
			else
			{
				// Fallback if no names available
				player.name = (selected_gender == GENDER_F) ? "Computer_F" : "Computer_M";
			}
		}
		else
		{
			// Human player: use name and gender from setup
			player.name = setup.name;
			player.gender = setup.player_gender;
		}
		
		// Assign ideal_temperature from truncated Gaussian distribution
		// Centered on best_perceived_temperature_K with sigma = ideal_temp_range
		double min_ideal_temp = GameConstants::best_perceived_temperature_K - GameConstants::ideal_temp_range;
		double max_ideal_temp = GameConstants::best_perceived_temperature_K + GameConstants::ideal_temp_range;
		player.ideal_temperature = rng->nextNormalTruncated(
			GameConstants::best_perceived_temperature_K,
			GameConstants::ideal_temp_range,
			min_ideal_temp,
			max_ideal_temp
		);
		
		// ideal_gravity will be assigned in assign_planets_random() after homeworld is determined
		player.ideal_gravity = 0.0;  // Placeholder, will be set later
		
		new_players.push_back(std::move(player));
	}
	
	return new_players;
}

std::unique_ptr<Galaxy> GameState::initialize_galaxy(const GalaxyGenerationParams& params)
{
	// Initialize galaxy with user-provided parameters
	// Galaxy constructor handles home planet selection with proper gravity constraints
	// Failures are caught immediately during coordinate generation
	
	std::vector<Planet*> home_planets;
	std::unique_ptr<Galaxy> new_galaxy = std::make_unique<Galaxy>(params, this);
	
	// Convert home planet indices to Planet* pointers for assignment
	for (size_t index : new_galaxy->home_planet_indices)
	{
		home_planets.push_back(&new_galaxy->planets[index]);
	}
	
	// Verify we have enough home planets (should always be true if Galaxy constructor succeeded)
	if (home_planets.size() < players.size())
	{
		throw std::runtime_error("Galaxy has insufficient home planets (" + std::to_string(home_planets.size()) + 
		     ") for " + std::to_string(players.size()) + " players.");
	}
	
	// Assign home planets to players
	assign_planets_random(home_planets);
	
	// Build entity ID maps for quick lookup
	build_entity_maps();
	
	return new_galaxy;
}

void GameState::build_entity_maps()
{
	// Build planet ID to index map and planet name to index map
	for (size_t i = 0; i < galaxy->planets.size(); ++i)
	{
		planet_id_to_index[galaxy->planets[i].id] = i;
		planet_name_to_index[galaxy->planets[i].name] = i;
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
			
			// Apply population growth
			int64_t population_growth = GameFormulas::calculate_population_growth(
				colonized.get_population(),
				planet->true_temperature,
				planet->true_gravity,
				player.ideal_temperature,
				player.ideal_gravity
			);
			
			int64_t new_population = colonized.get_population() + population_growth;
			colonized.set_population(static_cast<int32_t>(new_population));
			planet->population = static_cast<int32_t>(new_population);
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
		
		// TODO: Add windfall income (special events, bonuses, etc.)
		int64_t windfall = GameFormulas::calculate_windfall_income(player.id, this);
		player.money_savings += windfall;
		
		// Allocate money according to allocation settings
		// (This will be implemented in money_allocation.cpp)
	}
}

void GameState::apply_money_interest()
{
	// Apply interest on savings to player income
	// Positive savings earn interest, negative savings (debt) incur interest costs
	// TODO: Handle negative income (when debt interest exceeds income)
	// For now, interest is simply added to income. In the future, we may need to
	// implement mechanics for players with negative total income.
	
	for (auto& player : players)
	{
		int64_t interest = GameFormulas::calculate_money_interest(player.money_savings);
		player.money_income += interest;
	}
}

void GameState::process_ships()
{
	// Process fleet arrivals at their destinations
	// Iterate through all players and check their in-transit fleets
	for (auto& player : players)
	{
		// Check each fleet to see if it has arrived
		for (auto& fleet : player.fleets)
		{
			// Check if fleet is in transit and has arrived
			if (fleet.transit && fleet.transit->arrival_turn <= current_turn)
			{
				// Fleet has arrived at destination
				uint32_t dest_id = fleet.transit->destination_planet_id;
				Planet* destination = nullptr;
				
				// Find the destination planet
				if (dest_id < galaxy->planets.size())
				{
					destination = &galaxy->planets[dest_id];
				}
				
				if (destination)
				{
					// Move fleet from space to destination
					fleet.current_planet = destination;
					
					// Clear transit info
					fleet.transit.reset();
					
					// Update old transit fields for compatibility
					fleet.in_transit = false;
					fleet.origin_planet = destination;
					fleet.destination_planet = nullptr;
					fleet.distance_to_destination = 0;
					fleet.turns_to_destination = 0;
				}
			}
		}
	}
}

void GameState::process_novae()
{
	// TODO: Process nova events (rare event - low priority)
	// Placeholder: Currently does nothing
	// Real implementation will:
	//   - Check for planets entering nova state
	//   - Handle nova warning periods
	//   - Destroy planets in nova state
	//   - Update player knowledge of destroyed planets
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
// Ship design management methods removed - use Player methods directly

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
	
	// Ensure we have cached costs up to the player's current tech level
	ensure_research_costs_available(*tech_level);
	
	// Check if we can advance the technology level
	while (true)
	{
		// Get advancement cost from cache
		int64_t advancement_cost = 0;
		int32_t next_level = *tech_level + 1;
		
		switch (stream)
		{
			case TECH_RANGE:
				if (static_cast<size_t>(next_level) >= research_cost_range.size())
					ensure_research_costs_available(next_level);
				advancement_cost = research_cost_range[next_level];
				break;
			case TECH_SPEED:
				if (static_cast<size_t>(next_level) >= research_cost_speed.size())
					ensure_research_costs_available(next_level);
				advancement_cost = research_cost_speed[next_level];
				break;
			case TECH_WEAPONS:
				if (static_cast<size_t>(next_level) >= research_cost_weapons.size())
					ensure_research_costs_available(next_level);
				advancement_cost = research_cost_weapons[next_level];
				break;
			case TECH_SHIELDS:
				if (static_cast<size_t>(next_level) >= research_cost_shields.size())
					ensure_research_costs_available(next_level);
				advancement_cost = research_cost_shields[next_level];
				break;
			case TECH_MINI:
				if (static_cast<size_t>(next_level) >= research_cost_mini.size())
					ensure_research_costs_available(next_level);
				advancement_cost = research_cost_mini[next_level];
				break;
			case TECH_RADICAL:
				if (static_cast<size_t>(next_level) >= research_cost_radical.size())
					ensure_research_costs_available(next_level);
				advancement_cost = research_cost_radical[next_level];
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
			
			// Update planet desirability
			colonized.update_desirability();
			
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
	// Delegate to Player to handle validation and initiate movement
	Player* player = get_player(player_id);
	if (!player)
		return;  // Player not found
	
	// Player::move_fleet handles all validation and calls Fleet::move_to()
	player->move_fleet(fleet_id, destination_planet_id);
}

void GameState::refuel_fleet(uint32_t player_id, uint32_t fleet_id)
{
	Fleet* fleet = get_fleet(player_id, fleet_id);
	if (!fleet)
		{ return; }
	
	fleet->refuel();
}

void GameState::assign_planets_random(const std::vector<Planet*>& suitable_planets)
{
	// Create a mutable copy of suitable_planets for shuffling
	std::vector<Planet*> shuffled_planets = suitable_planets;
	
	// Randomly shuffle suitable planets using RNG
	for (size_t i = shuffled_planets.size() - 1; i > 0; --i)
	{
		size_t j = rng->nextInt32Range(0, i);
		std::swap(shuffled_planets[i], shuffled_planets[j]);
	}
	
	// Assign one planet to each player
	for (size_t player_idx = 0; player_idx < players.size(); ++player_idx)
	{
		Player& player = players[player_idx];
		Planet* planet = shuffled_planets[player_idx];
		const PlayerSetup& setup = player_setups[player_idx];
		
		// Set planet ownership
		planet->owner = player.id;
		
		// Determine ideal_gravity based on starting colony quality
		double ideal_gravity;
		if (setup.starting_colony_quality == START_OUTPOST)
		{
			// For START_OUTPOST, randomize ideal_gravity within +/-0.20 of true_gravity
			// But constrain to [Starting_Planet_Min_Gravity, Starting_Planet_Max_Gravity]
			double min_ideal = std::max( planet->true_gravity - 0.20,
				GameConstants::Starting_Planet_Min_Gravity );
			double max_ideal = std::min( planet->true_gravity + 0.20,
				GameConstants::Starting_Planet_Max_Gravity );
			
			// Generate random value in range [min_ideal, max_ideal]
			double random_offset = rng->nextDouble() * (max_ideal - min_ideal);
			ideal_gravity = min_ideal + random_offset;
			
			// Round to 2 decimal places
			ideal_gravity = std::round(ideal_gravity * 100.0) / 100.0;
		}
		else
		{
			// For all other qualities, set ideal_gravity to match planet's true_gravity
			ideal_gravity = planet->true_gravity;
		}
		
		player.ideal_gravity = ideal_gravity;
		
		// Adjust homeworld temperature to match player's ideal_temperature
		planet->true_temperature = player.ideal_temperature;
		
		// Set homeworld population based on starting colony quality
		planet->population = GameConstants::Starting_Colony_Population[setup.starting_colony_quality];
		
		// Create ColonizedPlanet entry for this player with quality-based values
		ColonizedPlanet colonized_planet(
		planet, &player,
		GameConstants::Starting_Colony_Population[setup.starting_colony_quality],
		GameConstants::Starting_Colony_Income[setup.starting_colony_quality] );
		
		// TODO: Set metal based on quality (add to ColonizedPlanet if needed)
		
		// Add to player's colonized planets
		player.colonized_planets.push_back(colonized_planet);
		
		
		// Log assignment
		std::cout << "Player " << player.name << " assigned starting planet: " 
		          << planet->name << " (gravity: " << planet->true_gravity 
		          << ", ideal_gravity: " << ideal_gravity << ")\n";
	}
	
	std::cout << "Successfully assigned starting planets to all " << players.size() << " players.\n";
}


// ============================================================================
// Validation Methods (check_* pattern)
// ============================================================================
ErrorCode GameState::check_player_build_fleet(uint32_t player_id, uint32_t design_id, uint32_t ship_count, uint32_t planet_id) const
{
	// Check player exists
	const Player* player = get_player(player_id);
	if (!player)
		{ return ErrorCode::INVALID_PLAYER_ID; }
	
	// Check design exists and belongs to player
	const ShipDesign* design = player->get_ship_design(design_id);
	if (!design)
		{ return ErrorCode::SHIP_DESIGN_NOT_FOUND; }
	
	// Check planet exists
	const Planet* planet = get_planet(planet_id);
	if (!planet)
		{ return ErrorCode::INVALID_PLANET_ID; }
	
	// Check player owns the planet
	bool owns_planet = false;
	for (const auto& colonized : player->get_colonized_planets())
	{
		if (colonized.get_id() == planet_id)
		{
			owns_planet = true;
			break;
		}
	}
	if (!owns_planet)
		{ return ErrorCode::PLANET_NOT_OWNED; }
	
	// Check ship count is valid
	if (ship_count == 0)
		{ return ErrorCode::INVALID_FLEET_SIZE; } 
	
	// Check player has sufficient money/metal for fleet construction
	int64_t total_cost = design->metal_cost * ship_count;
	if (player->get_metal() < total_cost)
		{ return ErrorCode::INSUFFICIENT_METAL; }
	
	return ErrorCode::SUCCESS;
}

ErrorCode GameState::check_player_design_ship(uint32_t player_id, const std::string& name, ShipType type, int32_t tech_range, int32_t tech_speed, int32_t tech_weapons, int32_t tech_shields, int32_t tech_mini) const
{
	// Check player exists
	const Player* player = get_player(player_id);
	if (!player)
		{ return ErrorCode::INVALID_PLAYER_ID; }
	
	// Check design name is not empty
	if (name.empty())
		{ return ErrorCode::INVALID_PARAMETER; } 
	
	// Check design name is not duplicate
	for (const auto& design : player->get_ship_designs())
	{
		if (design.name == name)
			{ return ErrorCode::DESIGN_NAME_DUPLICATE; }
	}
	
	// Check design limit not reached (max 100)
	if (player->get_ship_designs().size() >= 100)
		return ErrorCode::SHIP_DESIGN_LIMIT_REACHED;
	
	// Check tech levels are available to player
	const auto& tech = player->get_tech_levels();
	if (type==SHIP_SATELLITE && tech_range != 0)
		return ErrorCode::TECH_LEVEL_NOT_AVAILABLE;
	if (tech_range > tech.range || (tech_range < 1 && type!=SHIP_SATELLITE) )
		return ErrorCode::TECH_LEVEL_NOT_AVAILABLE;
	if (tech_speed > tech.speed || tech_speed < 2)
		return ErrorCode::TECH_LEVEL_NOT_AVAILABLE;
	if (tech_weapons > tech.weapons || tech_weapons < 1)
		return ErrorCode::TECH_LEVEL_NOT_AVAILABLE;
	if (tech_shields > tech.shields || tech_shields < 1)
		return ErrorCode::TECH_LEVEL_NOT_AVAILABLE;
	if (tech_mini > tech.mini || tech_mini < 0)
		return ErrorCode::TECH_LEVEL_NOT_AVAILABLE;
	
	return ErrorCode::SUCCESS;
}

ErrorCode GameState::check_player_set_spending_allocation(uint32_t player_id, double savings_frac, double research_frac, double planets_frac) const
{
	// Check player exists
	if (!get_player(player_id))
		return ErrorCode::INVALID_PLAYER_ID;
	
	// Check fractions are valid (all >= 0.0)
	if (savings_frac < 0.0 || research_frac < 0.0 || planets_frac < 0.0)
		return ErrorCode::INVALID_ALLOCATION;
	
	// Check fractions sum to approximately 1.0 (allow small floating point error)
	double total = savings_frac + research_frac + planets_frac;
	if (std::abs(total - 1.0) > 0.001)  // Allow 0.1% error
		return ErrorCode::INVALID_ALLOCATION;
	
	return ErrorCode::SUCCESS;
}

ErrorCode GameState::check_player_move_fleet(uint32_t player_id, uint32_t fleet_id, uint32_t destination_planet_id) const
{
	// Check player exists
	const Player* player = get_player(player_id);
	if (!player)
		return ErrorCode::INVALID_PLAYER_ID;
	
	// Check fleet exists and belongs to player
	const Fleet* fleet = get_fleet(player_id, fleet_id);
	if (!fleet)
		return ErrorCode::FLEET_NOT_FOUND;
	
	// Check destination planet exists
	const Planet* dest_planet = get_planet(destination_planet_id);
	if (!dest_planet)
		return ErrorCode::INVALID_PLANET_ID;
	
	// Check fleet is not already in transit
	if (fleet->in_transit)
		return ErrorCode::FLEET_IN_TRANSIT;
	
	return ErrorCode::SUCCESS;
}

ErrorCode GameState::check_player_set_planet_allocation(uint32_t player_id, uint32_t planet_id, double mining_frac, double terraforming_frac) const
{
	// Check player exists
	const Player* player = get_player(player_id);
	if (!player)
		return ErrorCode::INVALID_PLAYER_ID;
	
	// Check planet exists
	const Planet* planet = get_planet(planet_id);
	if (!planet)
		return ErrorCode::INVALID_PLANET_ID;
	
	// Check player owns the planet
	bool owns_planet = false;
	for (const auto& colonized : player->get_colonized_planets())
	{
		if (colonized.get_id() == planet_id)
		{
			owns_planet = true;
			break;
		}
	}
	if (!owns_planet)
		return ErrorCode::PLANET_NOT_OWNED;
	
	// Check fractions are valid (all >= 0.0)
	if (mining_frac < 0.0 || terraforming_frac < 0.0)
		return ErrorCode::INVALID_ALLOCATION;
	
	// Check fractions sum to approximately 1.0 (allow small floating point error)
	double total = mining_frac + terraforming_frac;
	if (std::abs(total - 1.0) > 0.001)  // Allow 0.1% error
		return ErrorCode::INVALID_ALLOCATION;
	
	return ErrorCode::SUCCESS;
}


// ============================================================================
// Player Knowledge Initialization
// ============================================================================
void GameState::initialize_player_knowledge()
{
	// Initialize KnowledgeGalaxy for each player
	for (auto& player : players)
	{
		// Create KnowledgeGalaxy for this player
		player.knowledge_galaxy = new KnowledgeGalaxy(*galaxy, player.id);
		
		// Find this player's homeworld
		// The homeworld is the first (and only) colonized planet at game start
		if (!player.colonized_planets.empty())
		{
			const ColonizedPlanet& homeworld_colonized = player.colonized_planets[0];
			Planet* homeworld = get_planet(homeworld_colonized.get_id());
			
			// Observe the homeworld to populate full knowledge
			// This updates all observable fields in the KnowledgePlanet
			player.knowledge_galaxy->observe_planet(homeworld->id, *homeworld, &player, current_year);
			
			// Get the KnowledgePlanet for the homeworld
			if (homeworld)
			{
				KnowledgePlanet* homeworld_knowledge = player.knowledge_galaxy->get_planet(homeworld->id);
				if (homeworld_knowledge)
				{
					// Create a ColonizedPlanet in the KnowledgePlanet
					// This represents the player's knowledge of their own colonized homeworld
					homeworld_knowledge->colonization = std::make_unique<ColonizedPlanet>(
						homeworld,
						&player,
						homeworld_colonized.get_population(),
						homeworld_colonized.get_income()
					);
				}
			}
		}
	}
	
	std::cout << "Successfully initialized KnowledgeGalaxy for all " << players.size() << " players.\n";
}
