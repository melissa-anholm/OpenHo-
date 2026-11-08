#include "game.h"
#include "game_setup.h"
#include "game_constants.h"
#include "game_formulas.h"
#include "save_file.h"
#include "text_assets.h"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <iostream>
#include <map>
#include <random>
#include <stdexcept>
#include <unordered_set>

// ============================================================================
// GameState Constructor
// ============================================================================

GameState::GameState(const class GameSetup& setup)
	: galaxy_params(setup.get_galaxy_params()),
	  player_setups(setup.get_player_setups())
{
	// Setup data has been copied and stored as members in initializer list
	
		// Use an explicit setup seed for reproducible games. A zero seed requests
		// a fresh random seed for normal interactive game creation.
		std::random_device rd;
		uint64_t det_seed = galaxy_params.seed;
		if (det_seed == 0)
			det_seed = rd();
		uint64_t ai_seed = det_seed + 1;  // Use seed + 1 for AI RNG
	rng = std::make_unique<DeterministicRNG>(det_seed, ai_seed);
	
		// Load text assets
		text_assets = std::make_unique<TextAssets>();
		text_assets->load_assets(setup.get_assets_directory());
	
		// Initialize players with setup configuration
		players = initialize_players(player_setups);
		for (const auto& player : players)
		{
			turn_ready[player.id] = false;
		}
	
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

const std::vector<PlayerPublicInfo>& GameState::get_full_player_info_history(uint32_t player_id) const
{
	static const std::vector<PlayerPublicInfo> empty_history;
	auto it = player_info_history.find(player_id);
	return it == player_info_history.end() ? empty_history : it->second;
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
	return galaxy ? galaxy->get_planet(planet_name) : nullptr;
}
const Planet* GameState::get_planet(const std::string& planet_name) const
{
	return galaxy ? galaxy->get_planet(planet_name) : nullptr;
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
	turn_ready[player_id] = false;
	capture_economic_action_submissions();
}

void GameState::set_planet_allocation(
	uint32_t player_id, uint32_t planet_id,
	double mining_fraction, double terraforming_fraction)
{
	Player* player = get_player(player_id);
	if (!player)
		throw std::runtime_error("Player not found");

	for (auto& colony : player->colonized_planets)
	{
		if (colony.get_id() != planet_id)
			continue;
		colony.set_budget_split(mining_fraction, terraforming_fraction);
		turn_ready[player_id] = false;
		capture_economic_action_submissions();
		return;
	}
	throw std::runtime_error("Planet is not owned by player");
}

const Player::MoneyAllocation& GameState::get_money_allocation(uint32_t player_id) const
{
	const Player* player = get_player(player_id);
	if (!player)
		throw std::runtime_error("Player not found");
	
	return player->allocation;
}

void GameState::set_dip_amount(uint32_t player_id, int64_t dip_amount)
{
	Player* player = get_player(player_id);
	if (!player)
		throw std::runtime_error("Player not found");
	if (dip_amount < 0)
		throw std::invalid_argument("Dip amount must not be negative");

	const int64_t savings_before_pending = player->money_savings +
		player->pending_dip_amount;
	const int64_t available_dip = GameFormulas::calculate_available_build_money(
		savings_before_pending,
		calculate_summed_planetary_income(*player),
		static_cast<uint32_t>(player->colonized_planets.size()));
	const int64_t permitted_dip = std::min(dip_amount, available_dip);

	player->money_savings += player->pending_dip_amount - permitted_dip;
	player->pending_dip_amount = permitted_dip;
	player->pending_dip_persistent = permitted_dip > 0;
	turn_ready[player_id] = false;
	capture_economic_action_submissions();
}

void GameState::set_one_time_dip_amount(uint32_t player_id, int64_t dip_amount)
{
	Player* player = get_player(player_id);
	if (!player)
		throw std::runtime_error("Player not found");
	if (dip_amount < 0)
		throw std::invalid_argument("Dip amount must not be negative");

	const int64_t savings_before_pending = player->money_savings +
		player->pending_dip_amount;
	const int64_t available_dip = GameFormulas::calculate_available_build_money(
		savings_before_pending,
		calculate_summed_planetary_income(*player),
		static_cast<uint32_t>(player->colonized_planets.size()));
	const int64_t permitted_dip = std::min(dip_amount, available_dip);

	player->money_savings += player->pending_dip_amount - permitted_dip;
	player->pending_dip_amount = permitted_dip;
	player->pending_dip_persistent = false;
	turn_ready[player_id] = false;
	capture_economic_action_submissions();
}

void GameState::clear_dip_amount(uint32_t player_id)
{
	Player* player = get_player(player_id);
	if (!player)
		throw std::runtime_error("Player not found");

	player->money_savings += player->pending_dip_amount;
	player->pending_dip_amount = 0;
	player->pending_dip_persistent = false;
	turn_ready[player_id] = false;
	capture_economic_action_submissions();
}

int64_t GameState::get_dip_amount(uint32_t player_id) const
{
	const Player* player = get_player(player_id);
	if (!player)
		throw std::runtime_error("Player not found");

	return player->pending_dip_amount;
}

bool GameState::is_dip_persistent(uint32_t player_id) const
{
	const Player* player = get_player(player_id);
	if (!player)
		throw std::runtime_error("Player not found");

	return player->pending_dip_persistent;
}

const EconomicActionSubmission& GameState::get_economic_action_submission(uint32_t player_id) const
{
	const Player* player = get_player(player_id);
	if (!player)
		throw std::runtime_error("Player not found");
	return player->get_economic_submission();
}

const FleetActionSubmission& GameState::get_fleet_action_submission(uint32_t player_id) const
{
	const Player* player = get_player(player_id);
	if (!player)
		throw std::runtime_error("Player not found");
	return player->get_fleet_action_submission();
}

const EconomicAfterState& GameState::get_economic_afterstate(uint32_t player_id) const
{
	const Player* player = get_player(player_id);
	if (!player)
		throw std::runtime_error("Player not found");
	return player->get_economic_afterstate();
}

ErrorCode GameState::submit_economic_action(
	uint32_t player_id, const EconomicActionSubmission& submission)
{
	Player* player = get_player(player_id);
	if (!player || submission.player_id != player_id)
		return ErrorCode::INVALID_PLAYER_ID;
	if (player->is_eliminated())
		return ErrorCode::INVALID_TURN_STATE;
	turn_ready[player_id] = false;
	if (submission.target_turn != current_turn)
		return ErrorCode::INVALID_TURN_STATE;

	if (check_player_set_spending_allocation(
			player_id, submission.savings_fraction, submission.research_fraction,
			submission.planets_fraction) != ErrorCode::SUCCESS)
		return ErrorCode::INVALID_ALLOCATION;
	double research_total = 0.0;
	for (double fraction : submission.research_fractions)
	{
		if (!std::isfinite(fraction) || fraction < 0.0)
			return ErrorCode::INVALID_ALLOCATION;
		research_total += fraction;
	}
	if (std::abs(research_total - 1.0) > GameConstants::Allocation_Sum_Tolerance)
		return ErrorCode::INVALID_ALLOCATION;
		if (submission.dip_amount < 0)
			return ErrorCode::INVALID_PARAMETER;
		std::unordered_set<std::string> abandonment_names;
		for (const std::string& planet_name : submission.planets_to_abandon)
		{
			if (!abandonment_names.insert(planet_name).second ||
				std::none_of(player->colonized_planets.begin(), player->colonized_planets.end(),
					[&planet_name](const ColonizedPlanet& colony) {
						return colony.get_name() == planet_name;
					}))
				return ErrorCode::PLANET_NOT_OWNED;
		}

		std::unordered_map<std::string, const EconomicColonyAllocation*> allocations;
		double funding_total = 0.0;
		for (const auto& allocation : submission.colony_allocations)
		{
			const Planet* allocation_planet = get_planet(allocation.planet_name);
			if (!allocation_planet || !allocations.emplace(allocation.planet_name, &allocation).second ||
				!std::isfinite(allocation.funding_fraction) || allocation.funding_fraction < 0.0 ||
				check_player_set_planet_allocation(
					player_id, allocation_planet->id, allocation.mining_fraction,
					allocation.terraforming_fraction) != ErrorCode::SUCCESS)
				return ErrorCode::INVALID_ALLOCATION;
			funding_total += allocation.funding_fraction;
		}
	if (allocations.size() != player->colonized_planets.size())
		return ErrorCode::INVALID_ALLOCATION;
	if (std::abs(funding_total - 1.0) > GameConstants::Allocation_Sum_Tolerance)
		return ErrorCode::INVALID_ALLOCATION;
		for (const auto& colony : player->colonized_planets)
			if (allocations.find(colony.get_name()) == allocations.end())
				return ErrorCode::INVALID_ALLOCATION;

	Player::MoneyAllocation money_allocation{
		submission.savings_fraction, submission.research_fraction,
		submission.planets_fraction,
		{submission.research_fractions[0], submission.research_fractions[1],
		 submission.research_fractions[2], submission.research_fractions[3],
		 submission.research_fractions[4], submission.research_fractions[5]}};
	player->allocation = money_allocation;
		player->economic_submission.planets_to_abandon = submission.planets_to_abandon;
		for (auto& colony : player->colonized_planets)
		{
			const auto& allocation = *allocations.at(colony.get_name());
		colony.set_funding_fraction(allocation.funding_fraction);
		colony.set_budget_split(allocation.mining_fraction, allocation.terraforming_fraction);
	}
	if (submission.dip_amount == 0)
		clear_dip_amount(player_id);
	else if (submission.dip_persistent)
		set_dip_amount(player_id, submission.dip_amount);
	else
		set_one_time_dip_amount(player_id, submission.dip_amount);
	capture_economic_action_submissions();
	return ErrorCode::SUCCESS;
}

ErrorCode GameState::submit_fleet_actions(
	uint32_t player_id, const FleetActionSubmission& submission)
{
	Player* player = get_player(player_id);
	if (!player || submission.player_id != player_id)
		return ErrorCode::INVALID_PLAYER_ID;
	if (player->is_eliminated())
		return ErrorCode::INVALID_TURN_STATE;
	if (submission.target_turn != current_turn)
		return ErrorCode::INVALID_TURN_STATE;

	const ErrorCode validation = check_fleet_action_submission(player_id, submission);
	if (validation != ErrorCode::SUCCESS)
		return validation;

	player->fleet_action_submission = submission;
	turn_ready[player_id] = false;
	return ErrorCode::SUCCESS;
}

// ============================================================================
// Turn Processing
// ============================================================================

ErrorCode GameState::mark_player_turn_ready(uint32_t player_id)
{
			const Player* player = get_player(player_id);
			if (!player)
				return ErrorCode::INVALID_PLAYER_ID;
			if (player->is_eliminated())
				return ErrorCode::INVALID_TURN_STATE;
			if (player->economic_submission.player_id != player_id ||
					player->economic_submission.target_turn != current_turn ||
					player->fleet_action_submission.player_id != player_id ||
					player->fleet_action_submission.target_turn != current_turn)
					return ErrorCode::INVALID_TURN_STATE;

			turn_ready[player_id] = true;
			return ErrorCode::SUCCESS;
}

ErrorCode GameState::mark_player_turn_unready(uint32_t player_id)
{
	if (!get_player(player_id))
		return ErrorCode::INVALID_PLAYER_ID;
	turn_ready[player_id] = false;
	return ErrorCode::SUCCESS;
}

int32_t GameState::get_player_turn_ready_status(uint32_t player_id) const
{
		if (!get_player(player_id))
			return -1;

		auto it = turn_ready.find(player_id);
		return (it != turn_ready.end() && it->second) ? 1 : 0;
}

bool GameState::all_players_ready() const
{
		if (players.empty())
			return false;

			bool required_player_exists = false;
			for (const auto& player : players)
			{
				if (player.is_eliminated())
					continue;
				required_player_exists = true;
				if (player.economic_submission.player_id != player.id ||
					player.economic_submission.target_turn != current_turn ||
					player.fleet_action_submission.player_id != player.id ||
					player.fleet_action_submission.target_turn != current_turn ||
					check_fleet_action_submission(player.id, player.fleet_action_submission) != ErrorCode::SUCCESS)
					return false;
				auto it = turn_ready.find(player.id);
				if (it == turn_ready.end() || !it->second)
					return false;
			}

			return required_player_exists;
}

ErrorCode GameState::colonize_planet(uint32_t player_id, uint32_t planet_id)
{
	Player* player = get_player(player_id);
	Planet* planet = get_planet(planet_id);
	if (!player)
		return ErrorCode::INVALID_PLAYER_ID;
	if (player->is_eliminated())
		return ErrorCode::INVALID_TURN_STATE;
	if (!planet)
		return ErrorCode::INVALID_PLANET_ID;
	if (planet->owner != NOT_OWNED)
		return ErrorCode::PLANET_ALREADY_OWNED;

	planet->owner = static_cast<PlayerID>(player_id);
	planet->population = GameConstants::Colonization_Initial_Population;
	player->colonized_planets.emplace_back(planet, player);
	player->colonized_planets.back().update_desirability();
	const double funding_share =
		1.0 / static_cast<double>(player->colonized_planets.size());
	for (auto& colony : player->colonized_planets)
		colony.set_funding_fraction(funding_share);
	capture_economic_action_submissions();
	return ErrorCode::SUCCESS;
}

ErrorCode GameState::queue_planet_abandonment(uint32_t player_id, uint32_t planet_id)
{
	Player* player = get_player(player_id);
	if (!player)
		return ErrorCode::INVALID_PLAYER_ID;
	if (player->is_eliminated())
		return ErrorCode::INVALID_TURN_STATE;
	if (std::none_of(player->colonized_planets.begin(), player->colonized_planets.end(),
		[planet_id](const ColonizedPlanet& colony) { return colony.get_id() == planet_id; }))
		return ErrorCode::PLANET_NOT_OWNED;
	const Planet* planet = get_planet(planet_id);
	if (!planet)
		return ErrorCode::INVALID_PLANET_ID;
	if (std::find(player->economic_submission.planets_to_abandon.begin(),
		player->economic_submission.planets_to_abandon.end(), planet->name) !=
		player->economic_submission.planets_to_abandon.end())
		return ErrorCode::INVALID_TURN_STATE;

	player->economic_submission.planets_to_abandon.push_back(planet->name);
	turn_ready[player_id] = false;
	capture_economic_action_submissions();
	return ErrorCode::SUCCESS;
}

ErrorCode GameState::cancel_planet_abandonment(uint32_t player_id, uint32_t planet_id)
{
	Player* player = get_player(player_id);
	if (!player)
		return ErrorCode::INVALID_PLAYER_ID;
	auto& abandonments = player->economic_submission.planets_to_abandon;
	const Planet* planet = get_planet(planet_id);
	if (!planet)
		return ErrorCode::INVALID_PLANET_ID;
	auto it = std::find(abandonments.begin(), abandonments.end(), planet->name);
	if (it == abandonments.end())
		return ErrorCode::INVALID_TURN_STATE;
	abandonments.erase(it);
	turn_ready[player_id] = false;
	capture_economic_action_submissions();
	return ErrorCode::SUCCESS;
}

void GameState::start_first_turn()
{
	// Initialize the first turn with initial game state
	// This is called during game initialization before any process_turn() calls
	capture_and_distribute_player_public_info();
	capture_economic_action_submissions();
	capture_fleet_action_submissions();
}

	void GameState::process_turn()
	{
		if (!all_players_ready())
			return;

		expire_newly_built_fleet_flags();

		// Phase 1 economic order:
	// 1. Capture beginning-of-turn savings and public information.
	// 2. Apply population growth.
	// 3. Calculate planetary income using the new population.
	// 4. Add interest calculated from beginning-of-turn savings.
	// 5. Apply the resulting effective income to allocations and economic work.
	// Later fleet and nova phases remain provisional or unimplemented.
	process_queued_abandonments();
	lock_pending_dips();
	for (auto& player : players)
		player.turn_opening_savings = player.money_savings;

	capture_and_distribute_player_public_info();
	
	process_population_growth();
	update_planet_incomes();
	calculate_player_incomes();
	apply_money_interest();
	apply_dip_into_savings();
			process_money_allocation();
				process_research();
					process_planets();
					process_queued_fleet_actions();
					refuel_stationed_fleets();
					replenish_colony_ship_crews();
					process_ships();
				process_automatic_colonization();
			process_novae();
		// Refresh views once, after queued fleet effects and colonization
			// processing have settled and before the next player-action phase.
			refresh_player_planet_views();

	check_bankruptcy_limits();
	capture_economic_afterstates();
	increment_turn();
	increment_year();
		renew_persistent_dips();
		capture_economic_action_submissions();
		capture_fleet_action_submissions();
		for (auto& ready : turn_ready)
			ready.second = false;
	}

	void GameState::expire_newly_built_fleet_flags()
	{
		for (auto& player : players)
		{
			for (auto& fleet : player.fleets)
				fleet.newly_built_this_turn = false;
			player.prototype_refund_candidates.clear();
		}
	}

	void GameState::capture_economic_action_submissions()
{
	for (auto& player : players)
	{
		EconomicActionSubmission& submission = player.economic_submission;
		submission.player_id = player.id;
			submission.target_turn = current_turn;
		submission.savings_fraction = player.allocation.savings_fraction;
		submission.research_fraction = player.allocation.research_fraction;
		submission.planets_fraction = player.allocation.planets_fraction;
		submission.research_fractions = {
			player.allocation.research.research_range_fraction,
			player.allocation.research.research_speed_fraction,
			player.allocation.research.research_weapons_fraction,
			player.allocation.research.research_shields_fraction,
			player.allocation.research.research_mini_fraction,
			player.allocation.research.research_radical_fraction};
		submission.dip_amount = player.pending_dip_amount;
		submission.dip_persistent = player.pending_dip_persistent;
			submission.colony_allocations.clear();
			for (const auto& colony : player.colonized_planets)
				submission.colony_allocations.push_back({
					colony.get_name(), colony.get_funding_fraction(),
					colony.get_mining_fraction(), colony.get_terraforming_fraction()});
	}
}

void GameState::capture_fleet_action_submissions()
{
	for (auto& player : players)
	{
		FleetActionSubmission& submission = player.fleet_action_submission;
		submission.player_id = player.id;
		submission.target_turn = current_turn;
		submission.scraps.clear();
	}
}

void GameState::capture_economic_afterstates()
{
	for (auto& player : players)
	{
		EconomicAfterState& after = player.economic_afterstate;
		after.turn = current_turn == 0 ? 0 : current_turn - 1;
		after.planetary_income = player.current_turn_income.planetary_income;
		after.interest_income = player.current_turn_income.interest_income;
		after.net_income = player.money_income;
		after.effective_income = player.effective_income;
		after.savings_after_processing = player.money_savings;
		after.metal_reserve_after_processing = player.metal_reserve;
		after.colonies.clear();
		for (const auto& colony : player.colonized_planets)
		{
				const Planet* planet = get_planet(colony.get_name());
				if (planet)
					after.colonies.push_back({planet->name, planet->population, colony.get_income()});
		}
	}
}

// ==========================================================================
// Serialization
// ============================================================================
std::vector<uint8_t> GameState::serialize_immutable_definition() const
{
	GameDefinition definition;
	definition.galaxy_params = galaxy_params;
	definition.galaxy_size = galaxy->gal_size;
	definition.player_setups = player_setups;
	definition.player_ideal_temperatures.reserve(players.size());
	definition.player_ideal_gravities.reserve(players.size());
	for (const Player& player : players)
	{
		definition.player_ideal_temperatures.push_back(player.ideal_temperature);
		definition.player_ideal_gravities.push_back(player.ideal_gravity);
	}
	definition.planets.reserve(galaxy->planets.size());
	for (const Planet& planet : galaxy->planets)
	{
		definition.planets.push_back({
			planet.id,
			planet.name,
			planet.x,
			planet.y,
			planet.true_gravity
			});
		}
		definition.distance_matrix = galaxy->distance_matrix;
		definition.destinations_by_distance = galaxy->destinations_by_distance;
		definition.home_planet_ids.reserve(galaxy->home_planet_indices.size());
	for (size_t index : galaxy->home_planet_indices)
	{
		if (index >= galaxy->planets.size())
			return {};
		definition.home_planet_ids.push_back(galaxy->planets[index].id);
	}
	return encode_game_definition(definition);
}

std::vector<uint8_t> GameState::serialize_state() const
{
	RuntimeCheckpointPayload checkpoint;
	checkpoint.turn = current_turn;
	checkpoint.year = current_year;
	checkpoint.next_fleet_id = next_fleet_id;
	checkpoint.deterministic_seed = rng->getDeterministicSeed();
	checkpoint.ai_seed = rng->getAISeed();
	checkpoint.deterministic_rng_state = rng->serialize_deterministic_rng_state();
	checkpoint.ai_rng_state = rng->serialize_ai_rng_state();
	checkpoint.planets.reserve(galaxy->planets.size());
	for (const Planet& planet : galaxy->planets)
	{
		RuntimePlanetState state;
		state.id = planet.id;
		state.owner = planet.owner;
		state.true_temperature = planet.true_temperature;
		state.metal = planet.metal;
		state.population = planet.population;
		state.nova_state = static_cast<int32_t>(planet.nova_state);
		state.views.reserve(planet.Views.size());
		for (const auto& [observer_id, view] : planet.Views)
			{
				RuntimePlanetState::View view_state;
				view_state.observer_id = static_cast<uint32_t>(observer_id);
				view_state.apparent_temperature = view.apparent_temperature;
				view_state.has_observed_gravity = view.has_observed_gravity;
				view_state.metal = view.metal;
				view_state.apparent_owner = view.apparent_owner;
				view_state.apparent_population = view.apparent_population;
				view_state.observation_year = view.observation_year;
				view_state.income = view.income;
				view_state.can_be_profitable = view.can_be_profitable;
				view_state.perceived_value = view.perceived_value;
				state.views.push_back(std::move(view_state));
			}
			checkpoint.planets.push_back(state);
	}

	checkpoint.players.reserve(players.size());
	for (const Player& player : players)
	{
		RuntimePlayerState state;
		state.id = player.id;
		state.economic_submission_target_turn = player.economic_submission.target_turn;
		state.eliminated = player.is_eliminated();
		state.money_savings = player.money_savings;
		state.metal_reserve = player.metal_reserve;
		state.metal_income = player.metal_income;
		state.savings_rate_modifier = player.savings_rate_modifier;
		state.pending_dip_amount = player.pending_dip_amount;
		state.pending_dip_persistent = player.pending_dip_persistent;
		state.locked_dip_amount = player.locked_dip_amount;
		state.locked_dip_persistent = player.locked_dip_persistent;
		state.savings_fraction = player.allocation.savings_fraction;
		state.research_fraction = player.allocation.research_fraction;
		state.planets_fraction = player.allocation.planets_fraction;
		state.research_fractions = {
			player.allocation.research.research_range_fraction,
			player.allocation.research.research_speed_fraction,
			player.allocation.research.research_weapons_fraction,
			player.allocation.research.research_shields_fraction,
			player.allocation.research.research_mini_fraction,
			player.allocation.research.research_radical_fraction};
		state.technology_levels = {player.tech.range, player.tech.speed, player.tech.weapons,
			player.tech.shields, player.tech.mini, player.tech.radical};
			state.partial_research = {player.partial_research.research_points_range,
				player.partial_research.research_points_speed,
				player.partial_research.research_points_weapons,
				player.partial_research.research_points_shields,
				player.partial_research.research_points_mini,
				player.partial_research.research_points_radical};
			for (const ShipDesign& design : player.ship_designs)
				state.ship_designs.push_back({design.id, design.name,
					static_cast<uint8_t>(design.type), design.get_range(), design.get_speed(),
					design.get_weapons(), design.get_shields(), design.get_mini(),
					design.build_cost, design.prototype_cost, design.metal_cost,
					design.has_been_prototyped});
			for (const Fleet& fleet : player.fleets)
			{
				RuntimeFleetState fleet_state;
				fleet_state.id = fleet.id;
				fleet_state.owner = fleet.owner;
				fleet_state.newly_built_this_turn = fleet.newly_built_this_turn;
				fleet_state.descriptor = fleet.descriptor;
					for (const FleetShipStack& stack : fleet.ship_stacks)
						fleet_state.stacks.push_back({stack.design_id, stack.ship_count, stack.fuel,
								stack.colonists_on_board, stack.colony_ship_ids});
					fleet_state.post_colonization_ship_id = fleet.post_colonization_ship_id;
					fleet_state.post_colonization_planet_name = fleet.post_colonization_planet_name;
				if (const auto* stationed = std::get_if<FleetStationedLocation>(&fleet.location))
					fleet_state.planet_name = stationed->planet_name;
				else
				{
					const auto& transit = std::get<FleetInTransitLocation>(fleet.location);
					fleet_state.in_transit = true;
					fleet_state.origin_planet_name = transit.origin_planet_name;
					fleet_state.destination_planet_name = transit.destination_planet_name;
					fleet_state.departure_turn = transit.departure_turn;
					fleet_state.arrival_turn = transit.arrival_turn;
					fleet_state.distance = transit.distance;
					fleet_state.turns_to_travel = transit.turns_to_travel;
				}
				state.fleets.push_back(std::move(fleet_state));
			}
			state.prototype_refund_candidates.assign(
				player.prototype_refund_candidates.begin(), player.prototype_refund_candidates.end());
			state.fleet_action_target_turn = player.fleet_action_submission.target_turn;
			for (const FleetMoveAction& move : player.fleet_action_submission.moves)
				state.fleet_moves.push_back({move.fleet_id, move.destination_planet_name});
			state.fleet_scraps = player.fleet_action_submission.scraps;
			for (const ColonizedPlanet& colony : player.colonized_planets)
		{
			RuntimeColonyState colony_state;
			colony_state.planet_id = colony.get_id();
			colony_state.income = colony.get_income();
			colony_state.funding_fraction = colony.get_funding_fraction();
			colony_state.mining_fraction = colony.get_mining_fraction();
			colony_state.terraforming_fraction = colony.get_terraforming_fraction();
				state.colonies.push_back(colony_state);
			}
			state.planets_to_abandon.clear();
			for (const std::string& planet_name : player.economic_submission.planets_to_abandon)
			{
				const Planet* planet = get_planet(planet_name);
				if (planet)
					state.planets_to_abandon.push_back(planet->id);
			}
			state.economic_afterstate = player.economic_afterstate;
		checkpoint.players.push_back(state);
	}
	return encode_runtime_checkpoint(checkpoint);
}

bool GameState::deserialize_state(const std::vector<uint8_t>& data)
{
	RuntimeCheckpointPayload checkpoint;
	if (!decode_runtime_checkpoint(data, checkpoint) ||
		checkpoint.planets.size() != galaxy->planets.size() ||
		checkpoint.players.size() != players.size() ||
		checkpoint.deterministic_rng_state.empty() ||
		checkpoint.ai_rng_state.empty() ||
		checkpoint.deterministic_seed != rng->getDeterministicSeed() ||
		checkpoint.ai_seed != rng->getAISeed())
		return false;

	std::unordered_map<uint32_t, const RuntimePlanetState*> saved_planets;
	for (const auto& state : checkpoint.planets)
		if (!saved_planets.emplace(state.id, &state).second ||
			planet_id_to_index.find(state.id) == planet_id_to_index.end())
			return false;

	std::unordered_map<uint32_t, const RuntimePlayerState*> saved_players;
	for (const auto& state : checkpoint.players)
		if (!saved_players.emplace(state.id, &state).second ||
				player_id_to_index.find(state.id) == player_id_to_index.end())
				return false;

		std::unordered_map<uint32_t, size_t> owned_planet_counts;
		for (const auto& state : checkpoint.planets)
		{
			Planet* planet = get_planet(state.id);
			if (!planet || (state.owner != NOT_OWNED && saved_players.find(state.owner) == saved_players.end()))
				return false;
			if (state.owner != NOT_OWNED)
				++owned_planet_counts[static_cast<uint32_t>(state.owner)];

			std::unordered_set<uint32_t> observer_ids;
			for (const auto& view : state.views)
			{
				if (!observer_ids.insert(view.observer_id).second ||
					saved_players.find(view.observer_id) == saved_players.end())
					return false;
			}
		}
	for (const auto& player_state : checkpoint.players)
	{
		if (player_state.technology_levels[0] < 0 || player_state.technology_levels[1] < 0 ||
			player_state.technology_levels[2] < 0 || player_state.technology_levels[3] < 0 ||
			player_state.technology_levels[4] < 0 || player_state.technology_levels[5] < 0 ||
			player_state.colonies.size() != owned_planet_counts[player_state.id])
			return false;

		std::unordered_set<uint32_t> colony_ids;
		for (const auto& colony_state : player_state.colonies)
		{
			if (!colony_ids.insert(colony_state.planet_id).second)
				return false;
			const auto planet_it = saved_planets.find(colony_state.planet_id);
				if (planet_it == saved_planets.end() ||
					planet_it->second->owner != static_cast<int32_t>(player_state.id))
					return false;
				}
			for (uint32_t planet_id : player_state.planets_to_abandon)
				if (colony_ids.find(planet_id) == colony_ids.end())
					return false;
		}
		std::unordered_set<uint32_t> checkpoint_fleet_ids;
		uint32_t maximum_fleet_id = 0;
		for (const auto& player_state : checkpoint.players)
		{
			std::unordered_set<uint32_t> design_ids;
			for (const auto& design : player_state.ship_designs)
				if (!design_ids.insert(design.id).second)
					return false;
			for (const auto& fleet : player_state.fleets)
			{
				if (fleet.owner != static_cast<int32_t>(player_state.id) ||
					fleet.stacks.empty() || !checkpoint_fleet_ids.insert(fleet.id).second ||
					fleet.id == 0)
					return false;
				maximum_fleet_id = std::max(maximum_fleet_id, fleet.id);
				if (fleet.in_transit)
				{
					if (!get_planet(fleet.origin_planet_name) ||
						!get_planet(fleet.destination_planet_name) ||
						fleet.arrival_turn < fleet.departure_turn)
						return false;
				}
				else if (!get_planet(fleet.planet_name))
					return false;
				for (const auto& stack : fleet.stacks)
					if (stack.ship_count == 0 || design_ids.count(stack.design_id) == 0)
						return false;
			}
			for (uint32_t design_id : player_state.prototype_refund_candidates)
				if (design_ids.count(design_id) == 0)
					return false;
				std::unordered_set<uint32_t> submitted_fleets;
				for (const auto& move : player_state.fleet_moves)
					if (checkpoint_fleet_ids.count(move.fleet_id) == 0 ||
						!submitted_fleets.insert(move.fleet_id).second ||
						!get_planet(move.destination_planet_name))
						return false;
				for (uint32_t fleet_id : player_state.fleet_scraps)
					if (checkpoint_fleet_ids.count(fleet_id) == 0 ||
						!submitted_fleets.insert(fleet_id).second)
					return false;
		}
		if (checkpoint.next_fleet_id == 0 || checkpoint.next_fleet_id <= maximum_fleet_id)
			return false;

	for (auto& planet : galaxy->planets)
	{
		const RuntimePlanetState& state = *saved_planets.at(planet.id);
		planet.owner = state.owner;
		planet.true_temperature = state.true_temperature;
		planet.metal = state.metal;
		planet.population = state.population;
		planet.nova_state = static_cast<PlanetNovaState>(state.nova_state);
			planet.Views.clear();
			const RuntimePlanetState& saved_state = state;
			for (const auto& saved_view : saved_state.views)
			{
				PlanetView& view = planet.get_or_create_view(
					static_cast<PlayerID>(saved_view.observer_id));
				view.apparent_temperature = saved_view.apparent_temperature;
				view.has_observed_gravity = saved_view.has_observed_gravity;
				view.metal = saved_view.metal;
				view.apparent_owner = saved_view.apparent_owner;
					view.apparent_population = saved_view.apparent_population;
					view.observation_year = saved_view.observation_year;
					view.income = saved_view.income;
					view.can_be_profitable = saved_view.can_be_profitable;
				view.perceived_value = saved_view.perceived_value;
			}
		}

		for (auto& player : players)
		{
				const RuntimePlayerState& state = *saved_players.at(player.id);
				player.ship_designs.clear();
				uint32_t maximum_design_id = 0;
				for (const auto& saved_design : state.ship_designs)
				{
					ShipDesign design;
					design.id = saved_design.id;
					design.name = saved_design.name;
					design.type = static_cast<ShipType>(saved_design.type);
					design.build_cost = saved_design.build_cost;
					design.prototype_cost = saved_design.prototype_cost;
					design.metal_cost = saved_design.metal_cost;
					design.has_been_prototyped = saved_design.has_been_prototyped;
					set_ship_design_tech(design, saved_design.tech_range, saved_design.tech_speed,
						saved_design.tech_weapons, saved_design.tech_shields, saved_design.tech_mini);
					player.ship_designs.push_back(std::move(design));
					maximum_design_id = std::max(maximum_design_id, saved_design.id);
				}
				player.next_ship_design_id = maximum_design_id + 1;
				player.fleets.clear();
				for (const auto& saved_fleet : state.fleets)
				{
					const auto& first_stack = saved_fleet.stacks.front();
					const ShipDesign* first_design = player.get_ship_design(first_stack.design_id);
					if (!first_design)
						return false;
					const std::string initial_planet = saved_fleet.in_transit
						? saved_fleet.origin_planet_name : saved_fleet.planet_name;
					Fleet restored(saved_fleet.id, player.id, first_stack.design_id,
						first_design, first_stack.ship_count, initial_planet);
						restored.descriptor = saved_fleet.descriptor;
						restored.newly_built_this_turn = saved_fleet.newly_built_this_turn;
						restored.post_colonization_ship_id = saved_fleet.post_colonization_ship_id;
						restored.post_colonization_planet_name = saved_fleet.post_colonization_planet_name;
					restored.ship_stacks.clear();
					for (const auto& saved_stack : saved_fleet.stacks)
					{
						const ShipDesign* design = player.get_ship_design(saved_stack.design_id);
							std::vector<uint8_t> colonists = saved_stack.colonists_on_board;
							if (design && design->type == SHIP_COLONY && colonists.empty())
								colonists.assign(saved_stack.ship_count, 10);
							restored.ship_stacks.push_back({saved_stack.design_id, design,
								saved_stack.ship_count, saved_stack.fuel, std::move(colonists),
								saved_stack.colony_ship_ids});
					}
					if (saved_fleet.in_transit)
						restored.location = FleetInTransitLocation{
							saved_fleet.origin_planet_name, saved_fleet.destination_planet_name,
							saved_fleet.departure_turn, saved_fleet.arrival_turn,
							saved_fleet.distance, saved_fleet.turns_to_travel};
					else
						restored.location = FleetStationedLocation{saved_fleet.planet_name};
					player.fleets.push_back(std::move(restored));
				}
				player.prototype_refund_candidates.clear();
				player.prototype_refund_candidates.insert(
					state.prototype_refund_candidates.begin(), state.prototype_refund_candidates.end());
				player.fleet_action_submission.player_id = player.id;
				player.fleet_action_submission.target_turn = state.fleet_action_target_turn;
				player.fleet_action_submission.moves.clear();
				for (const auto& move : state.fleet_moves)
					player.fleet_action_submission.moves.push_back({move.fleet_id, move.destination_planet_name});
				player.fleet_action_submission.scraps = state.fleet_scraps;
				player.economic_submission.player_id = player.id;
			player.economic_submission.target_turn = state.economic_submission_target_turn;
			player.economic_afterstate = state.economic_afterstate;
		player.set_eliminated(state.eliminated);
		player.money_savings = state.money_savings;
		player.metal_reserve = state.metal_reserve;
		player.set_savings_rate_modifier(state.savings_rate_modifier);
		player.pending_dip_amount = state.pending_dip_amount;
		player.pending_dip_persistent = state.pending_dip_persistent;
		player.locked_dip_amount = state.locked_dip_amount;
		player.locked_dip_persistent = state.locked_dip_persistent;
		player.allocation.savings_fraction = state.savings_fraction;
		player.allocation.research_fraction = state.research_fraction;
		player.allocation.planets_fraction = state.planets_fraction;
		player.allocation.research = {state.research_fractions[0], state.research_fractions[1],
			state.research_fractions[2], state.research_fractions[3],
			state.research_fractions[4], state.research_fractions[5]};
		player.tech = {state.technology_levels[0], state.technology_levels[1],
			state.technology_levels[2], state.technology_levels[3],
			state.technology_levels[4], state.technology_levels[5]};
		player.partial_research = {state.partial_research[0], state.partial_research[1],
			state.partial_research[2], state.partial_research[3],
			state.partial_research[4], state.partial_research[5]};
		player.colonized_planets.clear();
		for (const RuntimeColonyState& colony_state : state.colonies)
		{
			Planet* planet = get_planet(colony_state.planet_id);
			if (!planet || planet->owner != static_cast<PlayerID>(player.id))
				return false;
			player.colonized_planets.emplace_back(
				planet, &player, colony_state.income,
				colony_state.funding_fraction,
				ColonizedPlanet::PlanetaryBudgetSplit(
					colony_state.mining_fraction, colony_state.terraforming_fraction));
			}
			player.economic_submission.colony_allocations.clear();
			for (const auto& colony : player.colonized_planets)
					player.economic_submission.colony_allocations.push_back({
							colony.get_name(), colony.get_funding_fraction(),
							colony.get_mining_fraction(), colony.get_terraforming_fraction()});
				player.economic_submission.planets_to_abandon.clear();
				for (uint32_t planet_id : state.planets_to_abandon)
				{
					const Planet* planet = get_planet(planet_id);
					if (planet)
						player.economic_submission.planets_to_abandon.push_back(planet->name);
				}
			player.economic_submission.savings_fraction = player.allocation.savings_fraction;
			player.economic_submission.research_fraction = player.allocation.research_fraction;
			player.economic_submission.planets_fraction = player.allocation.planets_fraction;
			player.economic_submission.research_fractions = state.research_fractions;
			player.economic_submission.dip_amount = player.pending_dip_amount;
			player.economic_submission.dip_persistent = player.pending_dip_persistent;
			player.turn_opening_savings = player.money_savings;
		player.metal_income = state.metal_income;
	}
	calculate_player_incomes(false);
	apply_money_interest();
	apply_dip_into_savings();
	int32_t maximum_technology_level = 0;
	for (const Player& player : players)
	{
		maximum_technology_level = std::max({maximum_technology_level,
			player.tech.range, player.tech.speed, player.tech.weapons,
			player.tech.shields, player.tech.mini, player.tech.radical});
	}
	ensure_research_costs_available(maximum_technology_level);
	rng->deserialize_deterministic_rng_state(checkpoint.deterministic_rng_state);
	rng->deserialize_ai_rng_state(checkpoint.ai_rng_state);
	current_turn = checkpoint.turn;
	current_year = checkpoint.year;
	next_fleet_id = checkpoint.next_fleet_id;
	for (auto& ready : turn_ready)
		ready.second = false;
	for (auto& bankruptcy : bankruptcy_limit_reached)
		bankruptcy.second = false;
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
	build_entity_maps(*new_galaxy);
	
	return new_galaxy;
}

void GameState::build_entity_maps(const Galaxy& galaxy_ref)
{
	// Build planet ID to index map and planet name to index map
	for (size_t i = 0; i < galaxy_ref.planets.size(); ++i)
	{
		planet_id_to_index[galaxy_ref.planets[i].id] = i;
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

void GameState::process_population_growth()
{
	for (auto& player : players)
	{
		for (auto& colonized : player.colonized_planets)
		{
			Planet* planet = get_planet(colonized.get_id());
			if (!planet || planet->owner != static_cast<PlayerID>(player.id))
				continue;

			const int64_t population_growth = GameFormulas::calculate_population_growth(
				planet->population,
				planet->true_temperature,
				planet->true_gravity,
				player.ideal_temperature,
				player.ideal_gravity
			);
			const int64_t new_population = static_cast<int64_t>(planet->population) + population_growth;
			planet->population = static_cast<int32_t>(new_population);
		}
	}
}

int64_t GameState::calculate_summed_planetary_income(const Player& player) const
{
	int64_t summed_income = 0;
	for (const auto& colonized : player.colonized_planets)
		summed_income += colonized.get_income();
	return summed_income;
}

void GameState::check_bankruptcy_limits()
{
	for (const auto& player : players)
	{
		const bool reached = player.money_savings <= -
			GameFormulas::calculate_bankruptcy_debt_limit(
				calculate_summed_planetary_income(player),
				static_cast<uint32_t>(player.colonized_planets.size()));
		const bool previously_reached = bankruptcy_limit_reached[player.id];
		bankruptcy_limit_reached[player.id] = reached;
		if (reached && !previously_reached)
		{
			std::cout << "Player " << player.name
			          << " has reached the provisional bankruptcy limit.\n";
		}
	}
}

void GameState::calculate_player_incomes(bool reset_metal_income)
{
	// For each player, calculate total income from all owned planets
	for (auto& player : players)
	{
		player.money_income = 0;
		if (reset_metal_income)
			player.metal_income = 0;
		player.current_turn_income = {0, 0, 0, 0};
		
		for (const auto& colonized : player.colonized_planets)
		{
			player.money_income += colonized.get_income();
			// Metal income calculation would go here
		}
		player.current_turn_income.planetary_income = player.money_income;
		player.current_turn_income.windfall_income =
			GameFormulas::calculate_windfall_income(player.id, this);
		player.current_turn_income.total_income =
			player.current_turn_income.planetary_income +
			player.current_turn_income.windfall_income;
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
			
			colonized.set_income(static_cast<int32_t>(planet->population * happiness * 10));
			
			}
		}
}

void GameState::process_money_allocation()
{
	// Distribute the committed effective income according to the player's
	// previous-turn allocation settings. Negative income is charged directly
	// to savings and does not fund any allocation category.
	for (auto& player : players)
	{
		if (player.effective_income > 0)
		{
			const int64_t savings_amount = Player::calculate_savings_amount(
				player.allocation, player.effective_income);
			player.money_savings += savings_amount;
		}
		else if (player.effective_income < 0)
		{
			player.money_savings += player.effective_income;
		}
	}
}

void GameState::apply_money_interest()
{
	// Add current-turn interest to the income pool. Interest is calculated from
	// the savings snapshot taken before any current-turn economic changes.
	for (auto& player : players)
		{
				const int64_t interest = GameFormulas::calculate_money_interest(
					player.turn_opening_savings, player.savings_rate_modifier);
			player.money_income += player.current_turn_income.windfall_income + interest;
			player.effective_income = player.money_income;
		player.current_turn_income.interest_income = interest;
		player.current_turn_income.total_income = player.money_income;
	}
}

void GameState::apply_dip_into_savings()
{
	for (auto& player : players)
	{
		// The locked amount was already removed from savings when it was
		// selected during the preceding player-action phase.
		player.effective_income = player.money_income + player.locked_dip_amount;
	}
}

void GameState::lock_pending_dips()
{
	for (auto& player : players)
	{
		player.locked_dip_amount = player.pending_dip_amount;
		player.locked_dip_persistent = player.pending_dip_persistent;
		player.pending_dip_amount = 0;
		player.pending_dip_persistent = false;
	}
}

void GameState::renew_persistent_dips()
{
	for (auto& player : players)
	{
		if (player.locked_dip_persistent && player.locked_dip_amount > 0)
		{
			const int64_t available_dip = GameFormulas::calculate_available_build_money(
				player.money_savings,
				calculate_summed_planetary_income(player),
				static_cast<uint32_t>(player.colonized_planets.size()));
			const int64_t renewed_dip = std::min(
				player.locked_dip_amount, available_dip);
			player.pending_dip_amount = renewed_dip;
			player.pending_dip_persistent = renewed_dip > 0;
			player.money_savings -= renewed_dip;
		}

		player.locked_dip_amount = 0;
		player.locked_dip_persistent = false;
	}
}

void GameState::process_queued_fleet_actions()
{
	for (auto& player : players)
	{
		std::vector<FleetMoveAction> pending_moves;
		for (const FleetMoveAction& move : player.fleet_action_submission.moves)
		{
			Fleet* fleet = player.get_fleet(move.fleet_id);
			if (!fleet || fleet->is_in_transit())
				continue;
			if (check_player_move_fleet(player.id, move.fleet_id,
				move.destination_planet_name) != ErrorCode::SUCCESS)
			{
				pending_moves.push_back(move);
				continue;
			}
			const Planet* destination = get_planet(move.destination_planet_name);
			const bool restricted = fleet->post_colonization_ship_id != 0;
			const bool friendly_established = destination &&
				destination->owner == static_cast<PlayerID>(player.id);
			if (restricted && !friendly_established)
			{
				pending_moves.push_back(move);
				continue;
			}
			KnowledgeGalaxy* knowledge = player.get_knowledge_galaxy();
			if (!knowledge)
			{
				pending_moves.push_back(move);
				continue;
			}
			const double distance = knowledge->get_distance(
				fleet->stationed_planet_name(), move.destination_planet_name);
			const int32_t fuel_cost = GameFormulas::calculate_fuel_consumption(distance);
			if (!fleet->consume_fuel(fuel_cost))
			{
				pending_moves.push_back(move);
				continue;
			}
			fleet->move_to(move.destination_planet_name,
				player.get_knowledge_galaxy(), current_turn);
		}
		player.fleet_action_submission.moves = std::move(pending_moves);
		for (uint32_t fleet_id : player.fleet_action_submission.scraps)
			player.apply_scrap_fleet_now(fleet_id);
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
			const FleetInTransitLocation* transit = fleet.transit_location();
			if (transit && transit->arrival_turn <= current_turn)
			{
				// Fleet has arrived at destination
					const std::string destination_name = transit->destination_planet_name;
					Planet* destination = get_planet(destination_name);
				
				if (destination)
				{
					const bool established_colony = destination->owner != NOT_OWNED;
					// Move fleet from space to destination
					fleet.location = FleetStationedLocation{destination->name};
					if (established_colony)
						fleet.refuel();
				}
			}
		}
	}
}

void GameState::process_automatic_colonization()
{
	struct Candidate
	{
		uint32_t player_id = 0;
		uint32_t fleet_id = 0;
		size_t stack_index = 0;
		size_t ship_index = 0;
		uint64_t ship_id = 0;
		int32_t speed = 0;
	};

	std::map<std::string, std::vector<Candidate>> candidates_by_planet;
	for (auto& player : players)
	{
		for (auto& fleet : player.fleets)
		{
			if (fleet.is_in_transit())
				continue;

			const std::string planet_name = fleet.stationed_planet_name();
			const Planet* planet = get_planet(planet_name);
			if (!planet || planet->owner != NOT_OWNED)
				continue;

			for (size_t stack_index = 0; stack_index < fleet.ship_stacks.size(); ++stack_index)
			{
				FleetShipStack& stack = fleet.ship_stacks[stack_index];
				if (!stack.design || stack.design->type != SHIP_COLONY)
					continue;
				if (stack.colonists_on_board.size() < stack.ship_count)
					stack.colonists_on_board.resize(stack.ship_count, 10);
				for (size_t ship_index = 0; ship_index < stack.colonists_on_board.size(); ++ship_index)
				{
					if (stack.colonists_on_board[ship_index] == 10)
						candidates_by_planet[planet_name].push_back({
							player.id, fleet.id, stack_index, ship_index,
							stack.colony_ship_ids.size() > ship_index ? stack.colony_ship_ids[ship_index] : 0,
							stack.design->get_speed()});
				}
			}
		}
	}

	for (auto& [planet_name, candidates] : candidates_by_planet)
	{
		if (candidates.empty())
			continue;
		Planet* planet = get_planet(planet_name);
		if (!planet || planet->owner != NOT_OWNED)
			continue;

		std::unordered_set<uint32_t> candidate_players;
		for (const Candidate& candidate : candidates)
			candidate_players.insert(candidate.player_id);
		const bool multiple_players = candidate_players.size() > 1;
		int32_t selected_speed = candidates.front().speed;
		for (const Candidate& candidate : candidates)
			selected_speed = multiple_players
				? std::max(selected_speed, candidate.speed)
				: std::min(selected_speed, candidate.speed);

		std::vector<size_t> tied_indices;
		for (size_t i = 0; i < candidates.size(); ++i)
			if (candidates[i].speed == selected_speed)
				tied_indices.push_back(i);
		const size_t selected_index = tied_indices.size() == 1
			? tied_indices.front()
			: static_cast<size_t>(rng->nextInt32Range(
				0, static_cast<int32_t>(tied_indices.size() - 1)));
		const Candidate& selected = candidates[tied_indices[selected_index]];
		if (colonize_planet(selected.player_id, planet->id) != ErrorCode::SUCCESS)
			continue;
		Fleet* selected_fleet = get_fleet(selected.player_id, selected.fleet_id);
		if (selected_fleet && selected.stack_index < selected_fleet->ship_stacks.size())
		{
			FleetShipStack& stack = selected_fleet->ship_stacks[selected.stack_index];
			if (selected.ship_index < stack.colonists_on_board.size())
				stack.colonists_on_board[selected.ship_index] = 0;
			selected_fleet->post_colonization_ship_id = selected.ship_id;
			selected_fleet->post_colonization_planet_name = planet->name;
		}
		}
	}

void GameState::replenish_colony_ship_crews()
{
	for (auto& player : players)
	{
		for (auto& fleet : player.fleets)
		{
			if (fleet.is_in_transit())
				continue;
			const Planet* planet = get_planet(fleet.stationed_planet_name());
			if (!planet || planet->owner == NOT_OWNED)
				continue;
			for (auto& stack : fleet.ship_stacks)
			{
				if (!stack.design || stack.design->type != SHIP_COLONY)
					continue;
				if (stack.colonists_on_board.size() < stack.ship_count)
					stack.colonists_on_board.resize(stack.ship_count, 10);
				std::fill(stack.colonists_on_board.begin(),
					stack.colonists_on_board.end(), 10);
			}
		}
	}
}

void GameState::refuel_stationed_fleets()
{
	for (auto& player : players)
	{
		for (auto& fleet : player.fleets)
		{
			if (fleet.is_in_transit())
				continue;
			const Planet* planet = get_planet(fleet.stationed_planet_name());
			if (planet && planet->owner != NOT_OWNED)
				fleet.refuel();
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

void GameState::process_queued_abandonments()
{
	for (auto& player : players)
	{
		for (const std::string& planet_name : player.economic_submission.planets_to_abandon)
		{
			auto colony_it = std::find_if(
				player.colonized_planets.begin(), player.colonized_planets.end(),
				[&planet_name](const ColonizedPlanet& colony) {
					return colony.get_name() == planet_name;
				});
			if (colony_it == player.colonized_planets.end())
				continue;

			Planet* planet = get_planet(planet_name);
				if (planet && planet->owner == static_cast<PlayerID>(player.id))
				{
					planet->owner = NOT_OWNED;
					planet->population = 0;
				}
				for (auto& fleet : player.fleets)
				{
					if (fleet.post_colonization_planet_name != planet_name)
						continue;
					for (auto& stack : fleet.ship_stacks)
						for (size_t i = 0; i < stack.colony_ship_ids.size(); ++i)
							if (stack.colony_ship_ids[i] == fleet.post_colonization_ship_id &&
								i < stack.colonists_on_board.size())
								stack.colonists_on_board[i] = 10;
					fleet.post_colonization_ship_id = 0;
					fleet.post_colonization_planet_name.clear();
				}
				player.colonized_planets.erase(colony_it);
		}
		player.economic_submission.planets_to_abandon.clear();
	}
	capture_economic_action_submissions();
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
		if (player.effective_income <= 0)
			continue;

		// Calculate research budget for this player
		int64_t research_budget = Player::calculate_research_amount(
				player.allocation, player.effective_income);
		
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
		if (player.effective_income <= 0)
			continue;

		// Calculate total money available for planet development this turn
		int64_t total_planet_development_budget = static_cast<int64_t>(
			player.effective_income * player.allocation.planets_fraction );
			
			// Iterate through each colonized planet
			for (ColonizedPlanet& colonized : player.colonized_planets)
			{
				// Get the planet object
				Planet* planet = get_planet(colonized.get_id() );
				if (!planet || planet->owner != static_cast<PlayerID>(player.id))
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
			
			// Process terraforming for this planet
			process_planet_terraforming(player, planet, terraforming_budget);
			
			// Process mining for this planet
			process_planet_mining(player, planet, mining_budget);
		}
	}
}

// ============================================================================
// Planet Processing Helper Functions
// ============================================================================
void GameState::process_planet_terraforming(Player& player, Planet* planet, int64_t terraforming_budget)
{
	// Skip if no budget allocated to terraforming
	if (terraforming_budget <= 0)
		{ return; }
	
	// Skip if planet is already at ideal temperature
	if (planet->true_temperature == player.ideal_temperature)
		{ return; }
	
	// Calculate the maximum temperature change possible with this budget
	double temperature_magnitude = GameFormulas::calculate_temperature_change(terraforming_budget);
	double temperature_change = 0.0;
	int64_t actual_terraforming_cost = terraforming_budget;
	
	// Determine direction and check for overshoot
	if (planet->true_temperature < player.ideal_temperature)
	{
		// Moving towards higher temperature (heating up)
		double distance_to_ideal = player.ideal_temperature - planet->true_temperature;
		if (temperature_magnitude > distance_to_ideal)
		{
			// Would overshoot - use only what's needed to reach ideal temperature
			temperature_change = distance_to_ideal;
			actual_terraforming_cost = GameFormulas::calculate_money_to_terraform(distance_to_ideal);
		}
		else
		{
			// Safe to apply full temperature change
			temperature_change = temperature_magnitude;
		}
	}
	else if (planet->true_temperature > player.ideal_temperature)
	{
		// Moving towards lower temperature (cooling down)
		double distance_to_ideal = planet->true_temperature - player.ideal_temperature;
		if (temperature_magnitude > distance_to_ideal)
		{
			// Would overshoot - use only what's needed to reach ideal temperature
			temperature_change = -distance_to_ideal;
			actual_terraforming_cost = GameFormulas::calculate_money_to_terraform(distance_to_ideal);
		}
		else
		{
			// Safe to apply full temperature change
			temperature_change = -temperature_magnitude;
		}
	}
	
	// Refund excess terraforming money to player savings
	int64_t excess_terraforming = terraforming_budget - actual_terraforming_cost;
	if (excess_terraforming > 0)
	{
		player.money_savings += excess_terraforming;
	}
	
	// Apply the temperature change to the planet
	planet->true_temperature += temperature_change;
}

void GameState::process_planet_mining(Player& player, Planet* planet, int64_t mining_budget)
{
	// Skip if no budget allocated to mining
	if (mining_budget <= 0)
		{ return; }
	
	// Calculate metal extraction with the allocated budget
	int64_t metal_extracted = GameFormulas::calculate_metal_mined(mining_budget);
	int64_t planet_metal_available = static_cast<int64_t>(planet->metal);
	
	// Check if there's insufficient metal on the planet
	if (metal_extracted > planet_metal_available)
	{
		// Calculate the actual cost to mine all remaining metal
		int64_t actual_cost = GameFormulas::calculate_money_to_mine(planet_metal_available);
		
		// Extract all available metal
		metal_extracted = planet_metal_available;
		
		// Refund excess money to player savings
		int64_t excess_money = mining_budget - actual_cost;
		if (excess_money > 0)
		{
			player.money_savings += excess_money;
		}
	}
	
	// Update planet and player resources
	planet->metal -= metal_extracted;
	player.metal_reserve += metal_extracted;
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

std::vector<const Fleet*> GameState::get_player_fleets_stationed_at(
	uint32_t player_id, const std::string& planet_name) const
{
	const Player* player = get_player(player_id);
	if (!player)
		return {};
	return player->get_fleets_stationed_at(planet_name);
}

bool GameState::delete_fleet(uint32_t player_id, uint32_t fleet_id)
{
	Player* player = get_player(player_id);
	if (!player)
		{ return false; }
	
	// Delegate to Player
	return player->delete_fleet(fleet_id);
}

ErrorCode GameState::scrap_fleet(uint32_t player_id, uint32_t fleet_id)
{
	Player* player = get_player(player_id);

	return player ? player->scrap_fleet(fleet_id)
		           : ErrorCode::INVALID_PLAYER_ID;
}

ErrorCode GameState::merge_fleets(
	uint32_t player_id, uint32_t source_fleet_id, uint32_t target_fleet_id)
{
	Player* player = get_player(player_id);
	return player ? player->merge_fleets(source_fleet_id, target_fleet_id)
		           : ErrorCode::INVALID_PLAYER_ID;
}

ErrorCode GameState::split_fleet_to_new_fleet(
	uint32_t player_id, uint32_t source_fleet_id, uint32_t design_id,
	uint32_t ship_count, uint32_t& new_fleet_id)
{
	new_fleet_id = 0;
	Player* player = get_player(player_id);
	return player ? player->split_fleet_to_new_fleet(
		source_fleet_id, design_id, ship_count, new_fleet_id)
		           : ErrorCode::INVALID_PLAYER_ID;
}

ErrorCode GameState::move_stack_to_fleet(
	uint32_t player_id, uint32_t source_fleet_id, uint32_t target_fleet_id,
	uint32_t design_id, uint32_t ship_count)
{
	Player* player = get_player(player_id);
	return player ? player->move_stack_to_fleet(
		source_fleet_id, target_fleet_id, design_id, ship_count)
		           : ErrorCode::INVALID_PLAYER_ID;
}

void GameState::move_fleet(uint32_t player_id, uint32_t fleet_id, uint32_t destination_planet_id)
{
	const Planet* destination = get_planet(destination_planet_id);
	if (destination)
		move_fleet(player_id, fleet_id, destination->name);
}

void GameState::move_fleet(uint32_t player_id,
                           uint32_t fleet_id,
                           const std::string& destination_planet_name)
{
	Player* player = get_player(player_id);
	if (!player)
		return;

	FleetActionSubmission submission = player->fleet_action_submission;
	auto existing = std::find_if(
		submission.moves.begin(), submission.moves.end(),
		[fleet_id](const FleetMoveAction& move) { return move.fleet_id == fleet_id; });
	if (existing != submission.moves.end())
		existing->destination_planet_name = destination_planet_name;
	else
		submission.moves.push_back({fleet_id, destination_planet_name});
	submit_fleet_actions(player_id, submission);
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
		
			setup_home_planet(player, *planet, setup);
		
		
		// Log assignment
		std::cout << "Player " << player.name << " assigned starting planet: " 
		          << planet->name << " (gravity: " << planet->true_gravity
		          << ", ideal_gravity: " << player.ideal_gravity << ")\n";
	}
	
	std::cout << "Successfully assigned starting planets to all " << players.size() << " players.\n";
}

void GameState::setup_home_planet(
	Player& player, Planet& planet, const PlayerSetup& setup)
{
	planet.owner = player.id;

	double ideal_gravity;
	if (setup.starting_colony_quality == START_OUTPOST)
	{
		double min_ideal = std::max(
			planet.true_gravity - 0.20, GameConstants::Starting_Planet_Min_Gravity);
		double max_ideal = std::min(
			planet.true_gravity + 0.20, GameConstants::Starting_Planet_Max_Gravity);
		ideal_gravity = min_ideal + rng->nextDouble() * (max_ideal - min_ideal);
		ideal_gravity = std::round(ideal_gravity * 100.0) / 100.0;
	}
	else
	{
		ideal_gravity = planet.true_gravity;
	}

	player.ideal_gravity = ideal_gravity;
	planet.true_temperature = player.ideal_temperature;
	planet.population = GameConstants::Starting_Colony_Population[setup.starting_colony_quality];
	planet.metal = GameConstants::Starting_Colony_Metal[setup.starting_colony_quality];
	player.colonized_planets.emplace_back(
		&planet, &player,
		GameConstants::Starting_Colony_Income[setup.starting_colony_quality]);
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
	if (ship_count > 1000)
		{ return ErrorCode::INVALID_FLEET_SIZE; }

	if (design->build_cost < 0 || design->prototype_cost < 0 || design->metal_cost < 0)
		{ return ErrorCode::VALIDATION_FAILED; }
	const uint64_t count = static_cast<uint64_t>(ship_count);
	if (count > static_cast<uint64_t>(INT64_MAX) /
			static_cast<uint64_t>(design->build_cost == 0 ? 1 : design->build_cost) ||
		count > static_cast<uint64_t>(INT64_MAX) /
			static_cast<uint64_t>(design->metal_cost == 0 ? 1 : design->metal_cost))
		{ return ErrorCode::VALIDATION_FAILED; }

	const int64_t construction_cost = design->build_cost * static_cast<int64_t>(ship_count);
	const int64_t prototype_cost = design->has_been_prototyped ? 0 : design->prototype_cost;
	if (construction_cost > INT64_MAX - prototype_cost)
		{ return ErrorCode::VALIDATION_FAILED; }
	const int64_t total_money_cost = construction_cost + prototype_cost;
	const int64_t total_metal_cost = design->metal_cost * static_cast<int64_t>(ship_count);
	const int64_t available_money = GameFormulas::calculate_available_build_money(
		player->get_money(),
		calculate_summed_planetary_income(*player),
		static_cast<uint32_t>(player->get_colonized_planets().size()));
	if (total_money_cost > available_money)
		{ return ErrorCode::INSUFFICIENT_MONEY; }
	if (player->get_metal() < total_metal_cost)
		{ return ErrorCode::INSUFFICIENT_METAL; }
	
	return ErrorCode::SUCCESS;
}

ErrorCode GameState::check_player_design_ship(
	uint32_t player_id, const std::string& name, ShipType type,
	int32_t requested_range_level, int32_t requested_speed_level,
	int32_t requested_weapons_level, int32_t requested_shields_level,
	int32_t requested_miniaturization_level) const
{
	// Check player exists
	const Player* player = get_player(player_id);
	if (!player)
		{ return ErrorCode::INVALID_PLAYER_ID; }
	
		// Check design name is not empty
		if (name.empty())
			{ return ErrorCode::INVALID_PARAMETER; }
		if (type < SHIP_SCOUT || type > SHIP_BIOLOGICAL)
			{ return ErrorCode::INVALID_SHIP_TYPE; }
		if (type == SHIP_BIOLOGICAL)
			{ return ErrorCode::INVALID_SHIP_TYPE; }
		if (!GameFormulas::check_ship_design_levels(
			type, requested_range_level, requested_speed_level,
			requested_weapons_level, requested_shields_level,
			requested_miniaturization_level))
			{ return ErrorCode::TECH_LEVEL_NOT_AVAILABLE; }
	
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
	const int32_t player_range_level = tech.range;
	const int32_t player_speed_level = tech.speed;
	const int32_t player_weapons_level = tech.weapons;
	const int32_t player_shields_level = tech.shields;
	const int32_t player_miniaturization_level = tech.mini;
	if (type == SHIP_SATELLITE && requested_range_level != 0)
		return ErrorCode::TECH_LEVEL_NOT_AVAILABLE;
	if (requested_range_level > GameFormulas::ship_design_technology_ceiling(
		type, TECH_RANGE, player_range_level))
		return ErrorCode::TECH_LEVEL_NOT_AVAILABLE;
	if (requested_speed_level > GameFormulas::ship_design_technology_ceiling(
		type, TECH_SPEED, player_speed_level))
		return ErrorCode::TECH_LEVEL_NOT_AVAILABLE;
	if (requested_weapons_level > GameFormulas::ship_design_technology_ceiling(
		type, TECH_WEAPONS, player_weapons_level))
		return ErrorCode::TECH_LEVEL_NOT_AVAILABLE;
	if (requested_shields_level > GameFormulas::ship_design_technology_ceiling(
		type, TECH_SHIELDS, player_shields_level))
		return ErrorCode::TECH_LEVEL_NOT_AVAILABLE;
	if (requested_miniaturization_level >
		GameFormulas::ship_design_technology_ceiling(
			type, TECH_MINI, player_miniaturization_level))
		return ErrorCode::TECH_LEVEL_NOT_AVAILABLE;
	
	return ErrorCode::SUCCESS;
}

ErrorCode GameState::build_ships_from_design(uint32_t player_id, uint32_t design_id, uint32_t ship_count, uint32_t planet_id)
{
	const ErrorCode validation = check_player_build_fleet(
		player_id, design_id, ship_count, planet_id);
	if (validation != ErrorCode::SUCCESS)
		return validation;

	Player* player = get_player(player_id);
	if (!player)
		return ErrorCode::INVALID_PLAYER_ID;
	if (ship_count == 0)
		return ErrorCode::INVALID_FLEET_SIZE;
	if (ship_count > 1000)
		return ErrorCode::INVALID_FLEET_SIZE;

	ShipDesign* design = nullptr;
	for (auto& candidate : player->ship_designs)
	{
		if (candidate.id == design_id)
		{
			design = &candidate;
			break;
		}
	}
	if (!design)
		return ErrorCode::SHIP_DESIGN_NOT_FOUND;

	Planet* planet = get_planet(planet_id);
	if (!planet)
		return ErrorCode::INVALID_PLANET_ID;
	if (planet->owner != static_cast<PlayerID>(player_id))
		return ErrorCode::PLANET_NOT_OWNED;

	if (design->build_cost < 0 || design->prototype_cost < 0 || design->metal_cost < 0)
		return ErrorCode::VALIDATION_FAILED;
	if (ship_count > static_cast<uint64_t>(INT64_MAX) /
			static_cast<uint64_t>(design->build_cost == 0 ? 1 : design->build_cost))
		return ErrorCode::VALIDATION_FAILED;
	if (ship_count > static_cast<uint64_t>(INT64_MAX) /
			static_cast<uint64_t>(design->metal_cost == 0 ? 1 : design->metal_cost))
		return ErrorCode::VALIDATION_FAILED;

	const int64_t construction_cost = design->build_cost * static_cast<int64_t>(ship_count);
	const int64_t prototype_cost = design->has_been_prototyped ? 0 : design->prototype_cost;
	if (construction_cost > INT64_MAX - prototype_cost)
		return ErrorCode::VALIDATION_FAILED;
	const int64_t total_money_cost = construction_cost + prototype_cost;
	const int64_t total_metal_cost = design->metal_cost * static_cast<int64_t>(ship_count);

	const int64_t available_money = GameFormulas::calculate_available_build_money(
		player->money_savings,
		calculate_summed_planetary_income(*player),
		static_cast<uint32_t>(player->colonized_planets.size()));
	if (total_money_cost > available_money)
		return ErrorCode::INSUFFICIENT_MONEY;
	if (player->metal_reserve < total_metal_cost)
		return ErrorCode::INSUFFICIENT_METAL;
	const bool first_prototype_this_build = !design->has_been_prototyped;

	const uint32_t fleet_id = player->build_fleet(
		allocate_fleet_id(), design_id, ship_count, planet->name);
	if (fleet_id == 0)
		return ErrorCode::UNKNOWN_ERROR;

	player->money_savings -= total_money_cost;
	player->metal_reserve -= total_metal_cost;
	design->has_been_prototyped = true;
	if (first_prototype_this_build)
		player->prototype_refund_candidates.insert(design_id);
	return ErrorCode::SUCCESS;
}

ErrorCode GameState::check_player_set_spending_allocation(uint32_t player_id, double savings_frac, double research_frac, double planets_frac) const
{
	// Check player exists
	if (!get_player(player_id))
		return ErrorCode::INVALID_PLAYER_ID;
	
	// Check fractions are valid (all >= 0.0)
	if (!std::isfinite(savings_frac) || !std::isfinite(research_frac) ||
		!std::isfinite(planets_frac) || savings_frac < 0.0 ||
		research_frac < 0.0 || planets_frac < 0.0)
		return ErrorCode::INVALID_ALLOCATION;
	
	// Check fractions sum to 1.0 within one double machine epsilon.
	double total = savings_frac + research_frac + planets_frac;
	if (std::abs(total - 1.0) > GameConstants::Allocation_Sum_Tolerance)
		return ErrorCode::INVALID_ALLOCATION;
	
	return ErrorCode::SUCCESS;
}

ErrorCode GameState::check_player_move_fleet(uint32_t player_id, uint32_t fleet_id, uint32_t destination_planet_id) const
{
	const Planet* destination = get_planet(destination_planet_id);
	return destination
		? check_player_move_fleet(player_id, fleet_id, destination->name)
		: ErrorCode::INVALID_PLANET_ID;
}

ErrorCode GameState::check_player_move_fleet(uint32_t player_id,
		uint32_t fleet_id,
		const std::string& destination_planet_name) const
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
	const Planet* dest_planet = get_planet(destination_planet_name);
	if (!dest_planet)
		return ErrorCode::INVALID_PLANET_ID;
	
	// Check fleet is not already in transit
		if (fleet->is_in_transit())
			return ErrorCode::FLEET_IN_TRANSIT;
		if (fleet->stationed_planet_name() == destination_planet_name)
			return ErrorCode::INVALID_TURN_STATE;
		if (fleet->effective_speed() <= 0)
			return ErrorCode::FLEET_NO_SPEED;

		const KnowledgeGalaxy* knowledge = player->get_knowledge_galaxy();
		if (!knowledge)
			return ErrorCode::INVALID_TURN_STATE;
		const double distance = knowledge->get_distance(
			fleet->stationed_planet_name(), destination_planet_name);
		if (!std::isfinite(distance) || distance < 0.0)
			return ErrorCode::INVALID_TURN_STATE;
			if (distance > static_cast<double>(fleet->effective_range()))
				return ErrorCode::FLEET_OUT_OF_RANGE;
			if (distance > static_cast<double>(fleet->effective_fuel()))
			{
				const Planet* origin = get_planet(fleet->stationed_planet_name());
				if (!origin || origin->owner == NOT_OWNED)
					return ErrorCode::INSUFFICIENT_FUEL;
			}
			return ErrorCode::SUCCESS;
}

ErrorCode GameState::check_fleet_action_submission(
	uint32_t player_id, const FleetActionSubmission& submission) const
{
	const Player* player = get_player(player_id);
	if (!player)
		return ErrorCode::INVALID_PLAYER_ID;
	if (submission.player_id != player_id || submission.target_turn != current_turn)
		return ErrorCode::INVALID_TURN_STATE;

	std::unordered_set<uint32_t> submitted_fleets;
	for (const FleetMoveAction& move : submission.moves)
	{
		if (!submitted_fleets.insert(move.fleet_id).second)
			return ErrorCode::INVALID_TURN_STATE;
		const ErrorCode result = check_player_move_fleet(
			player_id, move.fleet_id, move.destination_planet_name);
		if (result != ErrorCode::SUCCESS)
			return result;
		const Fleet* fleet = player->get_fleet(move.fleet_id);
		if (!fleet || fleet->stationed_planet_name() == move.destination_planet_name)
			return ErrorCode::INVALID_TURN_STATE;
		}
		for (uint32_t fleet_id : submission.scraps)
		{
			if (!submitted_fleets.insert(fleet_id).second)
				return ErrorCode::INVALID_TURN_STATE;
			const Fleet* fleet = player->get_fleet(fleet_id);
			if (!fleet)
				return ErrorCode::FLEET_NOT_FOUND;
			if (fleet->is_in_transit())
				return ErrorCode::FLEET_IN_TRANSIT;
			if (fleet->newly_built_this_turn ||
				get_planet(fleet->stationed_planet_name()) == nullptr)
				return ErrorCode::INVALID_TURN_STATE;
		}
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
	if (!std::isfinite(mining_frac) || !std::isfinite(terraforming_frac) ||
		mining_frac < 0.0 || terraforming_frac < 0.0)
		return ErrorCode::INVALID_ALLOCATION;
	
	// Check fractions sum to 1.0 within one double machine epsilon.
	double total = mining_frac + terraforming_frac;
	if (std::abs(total - 1.0) > GameConstants::Allocation_Sum_Tolerance)
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
		
		}
		
	refresh_player_planet_views();
	std::cout << "Successfully initialized KnowledgeGalaxy for all " << players.size() << " players.\n";
}

bool GameState::set_obs_lost_battle_quickly(uint32_t player_id,
                                            uint32_t planet_id,
                                            PlayerID apparent_owner)
{
	Player* player = get_player(player_id);
	Planet* planet = get_planet(planet_id);
	if (!player || !planet)
		return false;
	planet->get_or_create_view(static_cast<PlayerID>(player_id))
		.set_obs_lost_battle_quickly(apparent_owner, static_cast<int32_t>(current_year));
	return true;
}

bool GameState::set_obs_lost_battle_eventually(uint32_t player_id, uint32_t planet_id)
{
	Player* player = get_player(player_id);
	Planet* planet = get_planet(planet_id);
	if (!player || !planet)
		return false;
	planet->get_or_create_view(static_cast<PlayerID>(player_id))
		.set_obs_lost_battle_eventually(*planet, static_cast<int32_t>(current_year));
	return true;
}


bool GameState::set_obs_friendly_ship_visit(uint32_t player_id, uint32_t planet_id)

{
	Player* player = get_player(player_id);
	Planet* planet = get_planet(planet_id);
	if (!player || !planet)
		return false;
	planet->get_or_create_view(static_cast<PlayerID>(player_id))
		.set_obs_friendly_ship_visit(*planet, static_cast<int32_t>(current_year));
	return true;
}

bool GameState::set_obs_fullcolony(uint32_t player_id, uint32_t planet_id)
{
	Player* player = get_player(player_id);
	Planet* planet = get_planet(planet_id);
	if (!player || !planet || planet->owner != static_cast<PlayerID>(player_id))
		return false;

	for (const ColonizedPlanet& colony : player->colonized_planets)
	{
		if (colony.get_id() == planet_id)
		{
			planet->get_or_create_view(static_cast<PlayerID>(player_id))
				.set_obs_fullcolony(*planet, colony, static_cast<int32_t>(current_year));
			return true;
		}
	}

	return false;
}

void GameState::refresh_player_planet_views()
{
	for (auto& player : players)
	{
		for (auto& planet : galaxy->planets)
		{
			if (set_obs_fullcolony(player.id, planet.id))
				continue;

			bool stationed_fleet = false;
			for (const Fleet& fleet : player.fleets)
			{
					if (!fleet.is_in_transit() && fleet.total_ship_count() > 0 &&
							fleet.stationed_planet_name() == planet.name)
				{
					stationed_fleet = true;
					break;
				}
			}

			if (stationed_fleet)
				set_obs_friendly_ship_visit(player.id, planet.id);
		}
	}
}
