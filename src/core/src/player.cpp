
#include "openho_core.h"
#include "player.h"
#include "game.h"
#include "game_formulas.h"
#include <cmath>
#include <limits>
#include <stdexcept>

void Player::set_savings_rate_modifier(double value)
{
	if (!std::isfinite(value) || value < 0.0)
		throw std::invalid_argument("savings rate modifier must be finite and non-negative");
	savings_rate_modifier = value;
}

// ============================================================================
// Money Allocation Calculation Helpers
// ============================================================================

int64_t Player::calculate_savings_amount(const MoneyAllocation& money_alloc, int64_t player_income)
{
	double savings_amount = player_income * money_alloc.savings_fraction;
	return static_cast<int64_t>(std::round(savings_amount));
}

int64_t Player::calculate_research_amount(const MoneyAllocation& money_alloc, int64_t player_income)
{
	double research_amount = player_income * money_alloc.research_fraction;
	return static_cast<int64_t>(std::round(research_amount));
}

int64_t Player::calculate_planets_amount(double total_planets_fraction, int64_t player_income)
{
	double planets_amount = player_income * total_planets_fraction;
	return static_cast<int64_t>(std::round(planets_amount));
}

int64_t Player::calculate_research_stream_amount( const ResearchAllocation& player_research,
                                                  TechStream tech_type,
                                                  int64_t research_budget)
{
	double stream_fraction = 0.0;
	
	switch (tech_type)
	{
		case TECH_RANGE:
			stream_fraction = player_research.research_range_fraction;
			break;
		case TECH_SPEED:
			stream_fraction = player_research.research_speed_fraction;
			break;
		case TECH_WEAPONS:
			stream_fraction = player_research.research_weapons_fraction;
			break;
		case TECH_SHIELDS:
			stream_fraction = player_research.research_shields_fraction;
			break;
		case TECH_MINI:
			stream_fraction = player_research.research_mini_fraction;
			break;
		case TECH_RADICAL:
			stream_fraction = player_research.research_radical_fraction;
			break;
		default:
			return 0;
	}
	
	double tech_stream_amount = research_budget * stream_fraction;
	return static_cast<int64_t>(std::round(tech_stream_amount));
}


// ============================================================================
// Fleet Management
// ============================================================================

bool Player::validate_fleet(uint32_t design_id, uint32_t ship_count, const std::string& planet_name) const
{
	// Validate ship_count is positive
	if (ship_count == 0)
		{ return false; }
	
	// Validate design exists for this player
	const ShipDesign* design = get_ship_design(design_id);
	if (!design)
		{ return false; }
	
	// Validate planet exists through GameState
	const Planet* planet = game_state->get_planet(planet_name);
	if (!planet)
		{ return false; }
	
	return true;
}

uint32_t Player::build_fleet(uint32_t fleet_id, uint32_t design_id, uint32_t ship_count, const std::string& planet_name)
{
	// Get the design and planet pointers
	const ShipDesign* design = get_ship_design(design_id);
	if (!design) 
		{ return 0; }
	
	Planet* planet = game_state->get_planet(planet_name);
	if (!planet)
		{ return 0; }
	
	// Create fleet using private constructor (Player is a friend of Fleet)
	Fleet new_fleet(fleet_id, id, design_id, design, ship_count, planet->name);
	
	fleets.push_back(std::move(new_fleet));
	//
	return fleet_id;
}

uint32_t Player::create_fleet(uint32_t design_id, uint32_t ship_count, const std::string& planet_name)
{
	// Validate parameters
	if (!validate_fleet(design_id, ship_count, planet_name) )
		{ return 0; }
	
	// Allocate a globally unique fleet ID from GameState
	uint32_t fleet_id = game_state->allocate_fleet_id();
	
	// Build and add the fleet
	return build_fleet(fleet_id, design_id, ship_count, planet_name);
}

uint32_t Player::create_fleet(uint32_t design_id, uint32_t ship_count, uint32_t planet_id)
{
	const Planet* planet = game_state ? game_state->get_planet(planet_id) : nullptr;
	return planet ? create_fleet(design_id, ship_count, planet->name) : 0;
}

Fleet* Player::get_fleet(uint32_t fleet_id)
{
	for (auto& fleet : fleets)
	{
		if (fleet.id == fleet_id)
			{ return &fleet; }
	}
	return nullptr;
}
const Fleet* Player::get_fleet(uint32_t fleet_id) const
{
	for (const auto& fleet : fleets)
	{
		if (fleet.id == fleet_id)
			{ return &fleet; }
	}
	return nullptr;
}

bool Player::delete_fleet(uint32_t fleet_id)
{
	for (size_t i = 0; i < fleets.size(); ++i)
	{
		if (fleets[i].id == fleet_id)
		{
			fleets.erase(fleets.begin() + i);
			return true;
		}
	}
	return false;
}

ErrorCode Player::scrap_fleet(uint32_t fleet_id)
{
	Fleet* fleet = get_fleet(fleet_id);
	if (!fleet)
		return ErrorCode::FLEET_NOT_FOUND;
	if (fleet->is_in_transit())
		return ErrorCode::FLEET_IN_TRANSIT;

	// Validate the current location before changing any queued action. This
	// prevents an in-transit fleet from losing its movement order accidentally.
	const std::string planet_name = fleet->stationed_planet_name();
	if (!game_state->get_planet(planet_name))
		return ErrorCode::PLANET_NOT_FOUND;

	fleet_action_submission.moves.erase(
		std::remove_if(fleet_action_submission.moves.begin(),
			fleet_action_submission.moves.end(),
			[fleet_id](const FleetMoveAction& move) {
				return move.fleet_id == fleet_id;
			}), fleet_action_submission.moves.end());

	if (fleet->newly_built_this_turn)
	{
		const ErrorCode result = apply_scrap_fleet_now(fleet_id);
		if (result == ErrorCode::SUCCESS)
			game_state->mark_player_turn_unready(id);
		return result;
	}

	if (std::find(fleet_action_submission.scraps.begin(),
			fleet_action_submission.scraps.end(), fleet_id) ==
			fleet_action_submission.scraps.end())
		fleet_action_submission.scraps.push_back(fleet_id);
	fleet_action_submission.player_id = id;
	fleet_action_submission.target_turn = game_state->get_current_turn();
	game_state->mark_player_turn_unready(id);
	return ErrorCode::SUCCESS;
}

ErrorCode Player::apply_scrap_fleet_now(uint32_t fleet_id)
{
	Fleet* fleet = get_fleet(fleet_id);
	if (!fleet)
		return ErrorCode::FLEET_NOT_FOUND;
	if (fleet->is_in_transit())
		return ErrorCode::FLEET_IN_TRANSIT;
	if (fleet->ship_stacks.empty())
		return ErrorCode::INVALID_FLEET_SIZE;

	const std::string planet_name = fleet->stationed_planet_name();
	Planet* planet = game_state->get_planet(planet_name);
	if (!planet)
		return ErrorCode::PLANET_NOT_FOUND;

	const bool same_turn_refund = fleet->newly_built_this_turn;
	__int128 construction_money = 0;
	__int128 construction_metal = 0;
	std::vector<uint32_t> prototype_rollbacks;
	for (const FleetShipStack& stack : fleet->ship_stacks)
	{
		if (!stack.design || stack.ship_count == 0 ||
			stack.design->build_cost < 0 || stack.design->metal_cost < 0)
			return ErrorCode::VALIDATION_FAILED;
		construction_money += static_cast<__int128>(stack.design->build_cost) * stack.ship_count;
		construction_metal += static_cast<__int128>(stack.design->metal_cost) * stack.ship_count;

		if (same_turn_refund && prototype_refund_candidates.count(stack.design_id) != 0)
		{
			bool design_remains_elsewhere = false;
			for (const Fleet& other : fleets)
			{
				if (other.id != fleet_id && other.find_stack(stack.design_id) != nullptr)
				{
					design_remains_elsewhere = true;
					break;
				}
			}
			if (!design_remains_elsewhere)
				prototype_rollbacks.push_back(stack.design_id);
		}
	}
	for (uint32_t design_id : prototype_rollbacks)
	{
		for (const auto& design : ship_designs)
		{
			if (design.id == design_id)
			{
				construction_money += design.prototype_cost;
				break;
			}
		}
	}

	if (construction_money > std::numeric_limits<int64_t>::max() ||
		construction_metal > std::numeric_limits<int64_t>::max())
		return ErrorCode::VALIDATION_FAILED;

	const int64_t full_metal_refund = static_cast<int64_t>(construction_metal);
	const int64_t metal_refund = same_turn_refund
		? full_metal_refund
		: static_cast<int64_t>((construction_metal * 15) / 100);
	const int64_t money_refund = same_turn_refund
		? static_cast<int64_t>(construction_money) : 0;

	if (same_turn_refund)
		money_savings += money_refund;

	Player* colony_owner = nullptr;
	if (planet->owner >= 0)
		colony_owner = game_state->get_player(static_cast<uint32_t>(planet->owner));
	if (colony_owner)
		colony_owner->metal_reserve += metal_refund;
	else
		planet->metal += metal_refund;

	for (uint32_t design_id : prototype_rollbacks)
	{
		for (auto& design : ship_designs)
		{
			if (design.id == design_id)
				design.has_been_prototyped = false;
		}
		prototype_refund_candidates.erase(design_id);
	}

	delete_fleet(fleet_id);
	refresh_prototype_refund_candidates();
	return ErrorCode::SUCCESS;
}

void Player::refresh_prototype_refund_candidates()
{
	for (auto iterator = prototype_refund_candidates.begin();
		 iterator != prototype_refund_candidates.end();)
	{
		const uint32_t design_id = *iterator;
		const bool still_newly_built = std::any_of(
			fleets.begin(), fleets.end(), [design_id](const Fleet& fleet) {
				return fleet.newly_built_this_turn && fleet.find_stack(design_id) != nullptr;
			});
		if (!still_newly_built)
			iterator = prototype_refund_candidates.erase(iterator);
		else
			++iterator;
	}
}

ErrorCode Player::merge_fleets(uint32_t source_fleet_id, uint32_t target_fleet_id)
{
	if (source_fleet_id == target_fleet_id)
		return ErrorCode::INVALID_PARAMETER;
	Fleet* source = get_fleet(source_fleet_id);
	Fleet* target = get_fleet(target_fleet_id);
	if (!source || !target)
		return ErrorCode::FLEET_NOT_FOUND;
	if (source->is_in_transit() || target->is_in_transit())
		return ErrorCode::FLEET_IN_TRANSIT;
	if (source->stationed_planet_name() != target->stationed_planet_name())
		return ErrorCode::INVALID_PARAMETER;
	if (source->ship_stacks.empty() || target->ship_stacks.empty())
		return ErrorCode::INVALID_FLEET_SIZE;

	const bool target_was_new = target->newly_built_this_turn;
	const bool source_was_new = source->newly_built_this_turn;
	const std::vector<FleetShipStack> source_stacks = source->ship_stacks;
	for (const auto& stack : source_stacks)
		target->add_to_stack(stack);
	target->newly_built_this_turn = target_was_new && source_was_new;
	delete_fleet(source_fleet_id);
	refresh_prototype_refund_candidates();
	game_state->mark_player_turn_unready(id);
	return ErrorCode::SUCCESS;
}

ErrorCode Player::split_fleet_to_new_fleet(
	uint32_t source_fleet_id, uint32_t design_id, uint32_t ship_count,
	uint32_t& new_fleet_id)
{
	new_fleet_id = 0;
	Fleet* source = get_fleet(source_fleet_id);
	if (!source)
		return ErrorCode::FLEET_NOT_FOUND;
	if (source->is_in_transit())
		return ErrorCode::FLEET_IN_TRANSIT;
	FleetShipStack extracted;
	if (!source->remove_from_stack(design_id, ship_count, extracted))
		return ErrorCode::INVALID_FLEET_SIZE;

	const std::string planet_name = source->stationed_planet_name();
	const bool newly_built = source->newly_built_this_turn;
	const uint32_t allocated_id = game_state->allocate_fleet_id();
	Fleet new_fleet(allocated_id, id, design_id, extracted.design,
		ship_count, planet_name);
	new_fleet.ship_stacks.front() = extracted;
	new_fleet.newly_built_this_turn = newly_built;
	fleets.push_back(std::move(new_fleet));
	new_fleet_id = allocated_id;
	if (source->ship_stacks.empty())
		delete_fleet(source_fleet_id);
	game_state->mark_player_turn_unready(id);
	return ErrorCode::SUCCESS;
}

ErrorCode Player::move_stack_to_fleet(
	uint32_t source_fleet_id, uint32_t target_fleet_id,
	uint32_t design_id, uint32_t ship_count)
{
	if (source_fleet_id == target_fleet_id)
		return ErrorCode::INVALID_PARAMETER;
	Fleet* source = get_fleet(source_fleet_id);
	Fleet* target = get_fleet(target_fleet_id);
	if (!source || !target)
		return ErrorCode::FLEET_NOT_FOUND;
	if (source->is_in_transit() || target->is_in_transit())
		return ErrorCode::FLEET_IN_TRANSIT;
	if (source->stationed_planet_name() != target->stationed_planet_name())
		return ErrorCode::INVALID_PARAMETER;
	FleetShipStack extracted;
	if (!source->remove_from_stack(design_id, ship_count, extracted))
		return ErrorCode::INVALID_FLEET_SIZE;
	target->add_to_stack(extracted);
	target->newly_built_this_turn =
		target->newly_built_this_turn && source->newly_built_this_turn;
	const bool source_empty = source->ship_stacks.empty();
	if (source_empty)
		delete_fleet(source_fleet_id);
	refresh_prototype_refund_candidates();
	game_state->mark_player_turn_unready(id);
	return ErrorCode::SUCCESS;
}

void Player::move_fleet(uint32_t fleet_id, const std::string& destination_planet_name)
{
	if (!game_state || game_state->check_player_move_fleet(
		id, fleet_id, destination_planet_name) != ErrorCode::SUCCESS)
		return;

	fleet_action_submission.player_id = id;
	fleet_action_submission.target_turn = game_state->get_current_turn();
	auto existing = std::find_if(
		fleet_action_submission.moves.begin(), fleet_action_submission.moves.end(),
		[fleet_id](const FleetMoveAction& move) { return move.fleet_id == fleet_id; });
	if (existing != fleet_action_submission.moves.end())
		existing->destination_planet_name = destination_planet_name;
	else
		fleet_action_submission.moves.push_back({fleet_id, destination_planet_name});
	game_state->mark_player_turn_unready(id);
}

void Player::move_fleet(uint32_t fleet_id, uint32_t destination_planet_id)
{
	const Planet* destination = game_state ? game_state->get_planet(destination_planet_id) : nullptr;
	if (destination)
		move_fleet(fleet_id, destination->name);
}

std::vector<Fleet*> Player::get_fleets_in_transit()
{
	std::vector<Fleet*> result;
	for (auto& fleet : fleets)
		if (fleet.is_in_transit())
			result.push_back(&fleet);
	return result;
}

const std::vector<const Fleet*> Player::get_fleets_in_transit() const
{
	std::vector<const Fleet*> result;
	for (const auto& fleet : fleets)
		if (fleet.is_in_transit())
			result.push_back(&fleet);
	return result;
}

std::vector<const Fleet*> Player::get_fleets_stationed_at(
	const std::string& planet_name) const
{
	std::vector<const Fleet*> result;
	for (const auto& fleet : fleets)
	{
		if (!fleet.is_in_transit() && fleet.total_ship_count() > 0 &&
			fleet.stationed_planet_name() == planet_name)
			result.push_back(&fleet);
	}
	return result;
}


// ============================================================================
// Ship Design Management
// ============================================================================
uint32_t Player::create_ship_design(const std::string& name, ShipType type, int32_t tech_range, int32_t tech_speed, int32_t tech_weapons, int32_t tech_shields, int32_t tech_mini)
{
	if (!game_state || game_state->check_player_design_ship(
		id, name, type, tech_range, tech_speed, tech_weapons, tech_shields, tech_mini) != ErrorCode::SUCCESS)
		return 0;
	
	uint32_t design_id = next_ship_design_id++;
	ShipDesign design;
	design.id = design_id;
	design.name = name;
	design.type = type;
	
		// Set tech levels using the friend function
		set_ship_design_tech(design, tech_range, tech_speed, tech_weapons, tech_shields, tech_mini);
		design.build_cost = GameFormulas::calculate_ship_design_build_cost(
			type, tech_range, tech_speed, tech_weapons, tech_shields, tech_mini);
		design.prototype_cost = GameFormulas::calculate_ship_design_prototype_cost(
			type, tech_range, tech_speed, tech_weapons, tech_shields, tech_mini);
		design.metal_cost = GameFormulas::calculate_ship_design_metal_cost(
			type, tech_range, tech_speed, tech_weapons, tech_shields, tech_mini);
	
	ship_designs.push_back(design);
	return design_id;
}

const ShipDesign* Player::get_ship_design(uint32_t design_id) const
{
	for (const auto& design : ship_designs)
	{
		if (design.id == design_id)
			{ return &design; }
	}
	return nullptr;
}

bool Player::delete_ship_design(uint32_t design_id)
{
	// TODO: Check if any fleets use this design before deleting
	
	for (size_t i = 0; i < ship_designs.size(); ++i)
	{
		if (ship_designs[i].id == design_id)
		{
			ship_designs.erase(ship_designs.begin() + i);
			return true;
		}
	}
	return false;
}

// ============================================================================
// Player Info History Management
// ============================================================================

void Player::receive_player_public_info(uint32_t source_player_id, const PlayerPublicInfo& info)
{
	// Append the new info to the history for this player
	player_info_history[source_player_id].push_back(info);
}

const std::vector<PlayerPublicInfo>* Player::get_player_info_history(uint32_t player_id) const
{
	auto it = player_info_history.find(player_id);
	if (it != player_info_history.end())
	return &it->second;
	
	return nullptr;
}

bool Player::validate_player_info_history(uint32_t player_id) const
{
	auto it = player_info_history.find(player_id);
	if (it == player_info_history.end())
		{ return false; }  // No history for this player

	const auto& history = it->second;
	if (history.empty())
		{ return false; }  // Empty history
	
	// Check for consistency: each entry should have sequential years
	for (size_t i = 1; i < history.size(); ++i)
	{
		// Year should be sequential
		if (history[i].year != history[i-1].year + 1)
			{ return false; }
		
		// Player ID should be consistent
		if (history[i].player_id != history[i-1].player_id)
			{ return false; }
	}
	return true;
}

void Player::request_full_player_info_resync(uint32_t player_id)
{
	// This is a placeholder - in a real implementation, this would
	// communicate with GameState to request a full resync
	// For now, we just clear the history for this player
	player_info_history[player_id].clear();
}


// ============================================================================
// Technology Accessors
// ============================================================================

int32_t Player::get_tech_level(uint32_t tech_type) const
{
	switch (tech_type)
	{
		case 0:  // TECH_RANGE
			return tech.range;
		case 1:  // TECH_SPEED
			return tech.speed;
		case 2:  // TECH_WEAPONS
			return tech.weapons;
		case 3:  // TECH_SHIELDS
			return tech.shields;
		case 4:  // TECH_MINI
			return tech.mini;
		case 5:  // TECH_RADICAL
			return tech.radical;
		default:
			return -1;  // Invalid tech type
	}
}
