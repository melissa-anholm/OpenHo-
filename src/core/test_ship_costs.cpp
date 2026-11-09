#include "game_formulas.h"

#include <cassert>
#include <cstdint>
#include <iostream>

namespace
{
	void expect_equal(int64_t actual, int64_t expected)
	{
		assert(actual == expected);
	}
}

int main()
{
	using namespace GameFormulas;

	// Base costs at all-zero ordinary technology and miniaturization.
	expect_equal(calculate_ship_design_build_cost(
		SHIP_FIGHTER, 0, 0, 0, 0, 0), 6000);
	expect_equal(calculate_ship_design_metal_cost(
		SHIP_FIGHTER, 0, 0, 0, 0, 0), 600);
	expect_equal(calculate_ship_design_prototype_cost(
		SHIP_FIGHTER, 0, 0, 0, 0, 0), 24000);
	expect_equal(calculate_ship_design_prototype_cost(
		SHIP_SATELLITE, 0, 0, 1, 0, 0), 4160);
	expect_equal(calculate_ship_design_prototype_cost(
		SHIP_DREADNOUGHT, 2, 0, 0, 0, 0), 308000);

	// Colony ships and tankers use the same heavy-ship 2x multiplier.
	expect_equal(calculate_ship_design_prototype_cost(
		SHIP_COLONY, 0, 0, 0, 0, 0),
		2 * calculate_ship_design_build_cost(SHIP_COLONY, 0, 0, 0, 0, 0));
	expect_equal(calculate_ship_design_prototype_cost(
		SHIP_TANKER, 0, 0, 0, 0, 0),
		2 * calculate_ship_design_build_cost(SHIP_TANKER, 0, 0, 0, 0, 0));

	// Ordinary technology increments use the ship-type multipliers.
	expect_equal(calculate_ship_design_build_cost(
		SHIP_FIGHTER, 1, 0, 0, 0, 0), 6100);
	expect_equal(calculate_ship_design_metal_cost(
		SHIP_FIGHTER, 1, 0, 0, 0, 0), 610);
	expect_equal(calculate_ship_design_build_cost(
		SHIP_DREADNOUGHT, 2, 0, 0, 0, 0), 154000);
	expect_equal(calculate_ship_design_metal_cost(
		SHIP_DREADNOUGHT, 2, 0, 0, 0, 0), 15400);

	// Miniaturization is applied after the complete ordinary cost.
	expect_equal(calculate_ship_design_build_cost(
		SHIP_FIGHTER, 0, 0, 0, 0, 1), 18000);
	expect_equal(calculate_ship_design_metal_cost(
		SHIP_FIGHTER, 0, 0, 0, 0, 1), 540);
	expect_equal(calculate_ship_design_build_cost(
		SHIP_FIGHTER, 0, 0, 0, 0, 2), 36000);
	expect_equal(calculate_ship_design_metal_cost(
		SHIP_FIGHTER, 0, 0, 0, 0, 2), 486);

	// Satellite speed is free, while range is structurally locked at zero.
	expect_equal(calculate_ship_design_build_cost(
		SHIP_SATELLITE, 0, 5, 0, 0, 0), 1000);
	expect_equal(calculate_ship_design_metal_cost(
		SHIP_SATELLITE, 0, 5, 0, 0, 0), 100);
	expect_equal(calculate_ship_design_build_cost(
		SHIP_SATELLITE, 0, 0, 1, 0, 0), 1040);
	expect_equal(calculate_ship_design_metal_cost(
		SHIP_SATELLITE, 0, 0, 1, 0, 0), 104);

	// A fractional final metal cost must round upward.
	expect_equal(calculate_ship_design_metal_cost(
		SHIP_FIGHTER, 0, 0, 0, 0, 3), 438);

	// Structural validation is separate from the calculations.
	assert(check_ship_design_levels(SHIP_FIGHTER, 0, 1, 1, 1, 0));
	assert(check_ship_design_levels(SHIP_SATELLITE, 0, 4, 1, 1, 10));
	assert(!check_ship_design_levels(SHIP_SATELLITE, 1, 0, 0, 0, 0));
	assert(!check_ship_design_levels(SHIP_FIGHTER, -1, 0, 0, 0, 0));
	assert(!check_ship_design_levels(SHIP_BIOLOGICAL, 0, 0, 0, 0, 0));

	// Intentional debt is provisionally limited to ten turns of repayment from
	// summed planetary income, including 15% debt interest each turn.
	const int64_t planetary_income = 10000;
	const int64_t intentional_limit = calculate_intentional_debt_limit(
		planetary_income, 2);
	assert(intentional_limit > 50000 && intentional_limit < 51000);
	expect_equal(calculate_available_build_money(0, planetary_income, 2),
		intentional_limit);
	expect_equal(calculate_available_build_money(-25000, planetary_income, 2),
		intentional_limit - 25000);
	expect_equal(calculate_intentional_debt_limit(0, 2), 0);
	expect_equal(calculate_available_build_money(-100000, 0, 2), 0);
	// Bankruptcy uses five times the debt where interest equals
	// max(planetary income, 50,000). For income of 10,000, the floor is
	// 1,666,666; income of 100,000 raises the limit to 3,333,333.
	assert(!has_reached_bankruptcy_limit(-1666665, planetary_income, 2));
	assert(has_reached_bankruptcy_limit(-1666666, planetary_income, 2));
	assert(!has_reached_bankruptcy_limit(-3333332, 100000, 2));
	assert(has_reached_bankruptcy_limit(-3333333, 100000, 2));
	assert(has_reached_bankruptcy_limit(-3000000, 0, 2));

	std::cout << "Ship cost tests passed.\n";
	return 0;
}
