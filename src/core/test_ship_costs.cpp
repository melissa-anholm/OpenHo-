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
		SHIP_FIGHTER, 0, 0, 0, 0, 0), 60000);

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
	assert(check_ship_design_levels(SHIP_FIGHTER, 0, 0, 0, 0, 0));
	assert(check_ship_design_levels(SHIP_SATELLITE, 0, 4, 0, 0, 10));
	assert(!check_ship_design_levels(SHIP_SATELLITE, 1, 0, 0, 0, 0));
	assert(!check_ship_design_levels(SHIP_FIGHTER, -1, 0, 0, 0, 0));
	assert(!check_ship_design_levels(SHIP_BIOLOGICAL, 0, 0, 0, 0, 0));

	// Provisional debt boundaries are shared by intentional actions such as
	// ship building and Dip Into Savings, while bankruptcy uses a larger limit.
	expect_equal(calculate_available_build_money(0), 100000);
	expect_equal(calculate_available_build_money(-25000), 75000);
	expect_equal(calculate_available_build_money(-100000), 0);
	expect_equal(calculate_available_build_money(-125000), 0);
	assert(!has_reached_bankruptcy_limit(-249999));
	assert(has_reached_bankruptcy_limit(-250000));
	assert(has_reached_bankruptcy_limit(-300000));

	std::cout << "Ship cost tests passed.\n";
	return 0;
}
