#include "fleet.h"
#include "ship_design.h"
#include "planet.h"
#include "knowledge_galaxy.h"
#include <algorithm>
#include <cmath>
#include <climits>
#include <memory>

// ============================================================================
// Fleet Constructor
// ============================================================================

Fleet::Fleet(uint32_t fleet_id, PlayerID player_id, uint32_t design_id,
             const ShipDesign* design,
             uint32_t ship_count, const std::string& planet_name)
		: id(fleet_id),
		  owner(player_id),
		  ship_stacks{{design_id, design, ship_count,
		               (design && design->type != SHIP_BIOLOGICAL) ? design->get_range() : 0,
		               design && design->type == SHIP_COLONY
			               ? std::vector<uint8_t>(ship_count, 10)
			               : std::vector<uint8_t>{},
			               design && design->type == SHIP_COLONY
			               ? std::vector<uint64_t>(ship_count)
			               : std::vector<uint64_t>{}}},
	  newly_built_this_turn(true),
	  location(FleetStationedLocation{planet_name})
{
	descriptor = "";
	static uint64_t next_colony_ship_id = 1;
	if (!ship_stacks.front().colony_ship_ids.empty())
		for (uint64_t& ship_id : ship_stacks.front().colony_ship_ids)
			ship_id = next_colony_ship_id++;
}

// ============================================================================
// Fleet Member Functions
// ============================================================================

void Fleet::refuel()
{
	for (auto& stack : ship_stacks)
		stack.fuel = stack.design && stack.design->type != SHIP_BIOLOGICAL
			? stack.design->get_range() : 0;
}

void Fleet::partial_refuel(int32_t amount)
{
	if (amount <= 0)
		return;
	for (auto& stack : ship_stacks)
	{
		const int32_t capacity = stack.design && stack.design->type != SHIP_BIOLOGICAL
			? stack.design->get_range() : 0;
		stack.fuel = std::min(capacity, stack.fuel + amount);
	}
}

FleetShipStack* Fleet::find_stack(uint32_t design_id)
{
	for (auto& stack : ship_stacks)
		if (stack.design_id == design_id)
			return &stack;
	return nullptr;
}

const FleetShipStack* Fleet::find_stack(uint32_t design_id) const
{
	for (const auto& stack : ship_stacks)
		if (stack.design_id == design_id)
			return &stack;
	return nullptr;
}

bool Fleet::remove_from_stack(uint32_t design_id, uint32_t requested_count,
	                            FleetShipStack& extracted)
{
	FleetShipStack* stack = find_stack(design_id);
	if (!stack || requested_count == 0 || requested_count > stack->ship_count)
		return false;
	extracted = *stack;
	extracted.ship_count = requested_count;
	if (!stack->colonists_on_board.empty())
	{
		extracted.colonists_on_board.assign(
			stack->colonists_on_board.end() - requested_count,
			stack->colonists_on_board.end());
		stack->colonists_on_board.resize(stack->colonists_on_board.size() - requested_count);
	}
	if (!stack->colony_ship_ids.empty())
	{
		extracted.colony_ship_ids.assign(
			stack->colony_ship_ids.end() - requested_count,
			stack->colony_ship_ids.end());
		stack->colony_ship_ids.resize(stack->colony_ship_ids.size() - requested_count);
	}
	stack->ship_count -= requested_count;
	if (stack->ship_count == 0)
		ship_stacks.erase(std::remove_if(ship_stacks.begin(), ship_stacks.end(),
			[design_id](const FleetShipStack& item) { return item.design_id == design_id; }),
			ship_stacks.end());
	return true;
}

void Fleet::add_to_stack(const FleetShipStack& incoming)
{
	if (incoming.ship_count == 0)
		return;
	FleetShipStack* existing = find_stack(incoming.design_id);
	if (!existing)
	{
		ship_stacks.push_back(incoming);
		return;
	}
	existing->ship_count += incoming.ship_count;
	existing->fuel = std::min(existing->fuel, incoming.fuel);
	existing->colonists_on_board.insert(existing->colonists_on_board.end(),
		incoming.colonists_on_board.begin(), incoming.colonists_on_board.end());
	existing->colony_ship_ids.insert(existing->colony_ship_ids.end(),
		incoming.colony_ship_ids.begin(), incoming.colony_ship_ids.end());
}

void Fleet::move_to(const std::string& destination_name,
                    KnowledgeGalaxy* knowledge_galaxy,
                    uint32_t current_turn)
{
	// Validate inputs
	if (destination_name.empty() || !knowledge_galaxy || is_in_transit())
		{ return; }
	const std::string& current_planet_name = stationed_planet_name();
	
	// If fleet is already at destination, do nothing
	if (current_planet_name == destination_name)
		{ return; }
	
	// Get distance from knowledge galaxy's distance matrix
	double distance = knowledge_galaxy->get_distance(current_planet_name, destination_name);
	
	// Travel duration is distance divided by effective speed, rounded up.
	uint32_t turns = 0;
	if (effective_speed() > 0)
	{
		turns = static_cast<uint32_t>(std::ceil(distance / effective_speed()));
	}
	if (turns == 0)
		{ turns = 1; }  // At least 1 turn to travel
	
	uint32_t arrival_turn = current_turn + turns;
	
	location = FleetInTransitLocation{
		current_planet_name, destination_name, current_turn, arrival_turn,
		distance, turns};
}

bool Fleet::is_in_transit() const
{
	return std::holds_alternative<FleetInTransitLocation>(location);
}

const std::string& Fleet::stationed_planet_name() const
{
	return std::get<FleetStationedLocation>(location).planet_name;
}

const FleetInTransitLocation* Fleet::transit_location() const
{
	return std::get_if<FleetInTransitLocation>(&location);
}

uint32_t Fleet::total_ship_count() const
{
	uint64_t total = 0;
	for (const auto& stack : ship_stacks)
		total += stack.ship_count;
	return total > UINT32_MAX ? UINT32_MAX : static_cast<uint32_t>(total);
}

int32_t Fleet::effective_fuel() const
{
	if (ship_stacks.empty())
		return 0;
	int32_t fuel = INT32_MAX;
	for (const auto& stack : ship_stacks)
		fuel = std::min(fuel, stack.fuel);
	return fuel == INT32_MAX ? 0 : fuel;
}

bool Fleet::consume_fuel(int32_t amount)
{
	if (amount < 0 || effective_fuel() < amount)
		return false;
	for (auto& stack : ship_stacks)
		stack.fuel -= amount;
	return true;
}

int32_t Fleet::effective_range() const
{
	if (ship_stacks.empty())
		return 0;
	int32_t range = INT32_MAX;
	for (const auto& stack : ship_stacks)
	{
		if (!stack.design)
			return 0;
		range = std::min(range, stack.design->get_range());
	}
	return range == INT32_MAX ? 0 : range;
}

int32_t Fleet::effective_speed() const
{
	if (ship_stacks.empty())
		return 0;
	int32_t speed = INT32_MAX;
	for (const auto& stack : ship_stacks)
	{
		if (!stack.design)
			return 0;
		speed = std::min(speed, stack.design->get_speed());
	}
	return speed == INT32_MAX ? 0 : speed;
}

bool Fleet::contains_colony_ship(uint64_t ship_id) const
{
	if (ship_id == 0)
		return false;
	for (const auto& stack : ship_stacks)
		if (std::find(stack.colony_ship_ids.begin(), stack.colony_ship_ids.end(), ship_id) !=
			stack.colony_ship_ids.end())
			return true;
	return false;
}

FleetCapabilities Fleet::capabilities() const
{
	return {effective_fuel(), effective_range(), effective_speed()};
}
