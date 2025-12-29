#ifndef OPENHO_CORE_H
#define OPENHO_CORE_H

#include <cstdint>
#include <vector>
#include <string>

// ============================================================================
// Fundamental Types
// ============================================================================

// Unique identifiers for game entities
typedef uint32_t EntityID;
typedef uint32_t PlayerID;

// Coordinate types
typedef double GalaxyCoord;

// ============================================================================
// Enums
// ============================================================================

// Ship types
enum ShipType
{
	SHIP_SCOUT = 0,
	SHIP_FIGHTER = 1,
	SHIP_COLONY = 2,
	SHIP_SATELLITE = 3,
	SHIP_TANKER = 4,
	SHIP_DREADNOUGHT = 5,
	SHIP_BIOLOGICAL = 6
};

// Planet nova state
enum PlanetState
{
	PLANET_NORMAL = 0,
	PLANET_WARNING = 1,
	PLANET_DESTROYED = 2
};

// Player type (human or computer)
enum PlayerType
{
	PLAYER_HUMAN = 0,
	PLAYER_COMPUTER = 1
};

// Technology streams for research
enum TechStream
{
	TECH_RANGE = 0,
	TECH_SPEED = 1,
	TECH_WEAPONS = 2,
	TECH_SHIELDS = 3,
	TECH_MINIATURIZATION = 4,
	TECH_RADICAL = 5
};

// ============================================================================
// Data Structures
// ============================================================================

// Technology levels for a player
struct TechnologyLevels
{
	int32_t range;
	int32_t speed;
	int32_t weapons;
	int32_t shields;
	int32_t miniaturization;
	int32_t radical;
};

// Ship technological parameters (subset of TechnologyLevels)
struct ShipTechParameters
{
	int32_t range;
	int32_t speed;
	int32_t weapons;
	int32_t shields;
	int32_t miniaturization;
};

// Research allocation fractions for a player
// All fractions must sum to 1.0
struct ResearchAllocation
{
	double research_range_fraction;
	double research_speed_fraction;
	double research_weapons_fraction;
	double research_shields_fraction;
	double research_miniaturization_fraction;
	double research_radical_fraction;
};

// Income breakdown for a player (calculated each turn)
struct IncomeBreakdown
{
	int64_t planetary_income;      // Income from owned planets
	int64_t interest_income;       // Interest from savings (positive or negative)
	int64_t windfall_income;       // Windfall from rare events
	int64_t total_income;          // Sum of above three
};

// Forward declaration of ColonizedPlanet for use in Player
struct ColonizedPlanet;

// Planet information snapshot (what a player knows about a planet)
struct PlanetInfo
{
	double temperature;
	double gravity;
	int32_t metal;
	std::string name;
	PlayerID owner;
	int32_t population;
	int32_t income;
	uint32_t observation_turn;  // When this information was collected
};

// Planet structure (actual state on the host)
struct Planet
{
	uint32_t id;
	std::string name;
	GalaxyCoord x;
	GalaxyCoord y;
	
	// Immutable properties
	double gravity;
	
	// Mutable properties
	double temperature;
	int32_t metal;
	PlayerID owner;  // 0 if unowned
	
	// Nova state
	PlanetState state;
	int32_t turns_until_nova;  // If state == PLANET_WARNING
	};
	
	// Colonized planet - extends Planet with player-specific allocation information
	struct ColonizedPlanet : public Planet
	{
		// Budget split for this planet (how to divide its budget between mining and terraforming)
		struct BudgetSplit
		{
		private:
			double mining_fraction;
			double terraforming_fraction;
			
		public:
			// Constructor
			BudgetSplit() : mining_fraction(0.5), terraforming_fraction(0.5) {}
			
			// Getters
			double get_mining_fraction() const
			{
				return mining_fraction;
			}
			
			double get_terraforming_fraction() const
			{
				return terraforming_fraction;
			}
			
			// Setters
			void set_mining_fraction(double fraction)
			{
				mining_fraction = fraction;
				normalize();
			}
			
			void set_terraforming_fraction(double fraction)
			{
				terraforming_fraction = fraction;
				normalize();
			}
			
			// Normalize the fractions so they sum to 1.0
			void normalize()
			{
				double total = mining_fraction + terraforming_fraction;
				if (total > 0.0)
				{
					mining_fraction /= total;
					terraforming_fraction /= total;
				}
				else
				{
					// If both are zero, split evenly
					mining_fraction = 0.5;
					terraforming_fraction = 0.5;
				}
			}
		};
		
	// Member variables
	double planet_funding_fraction;      // What fraction of total planet budget this planet gets
	BudgetSplit budget_split;            // How to split this planet's budget between mining/terraforming
	
	// Player-specific properties (only for colonized planets)
	int32_t population;                  // Population on this planet
	int32_t income;                      // Calculated based on population, temperature, gravity, and owner's ideals
	};

// Ship structure
struct Ship
{
	uint32_t id;
	ShipType type;
	PlayerID owner;
	ShipTechParameters techParams;
	
	// Position: either at a planet or in transit
	uint32_t current_planet_id;  // If not in transit
	uint32_t destination_planet_id;  // If in transit
	int32_t eta_turns;  // Turns until arrival (if in transit)
	
	// Fleet information
	uint32_t fleet_id;  // Every ship is always in a fleet; a lone ship is a fleet of one
};

// Ship design pattern (immutable once created)
struct ShipDesign
{
	uint32_t id;                    // Unique ID within player's design library
	std::string name;
	ShipType type;
	
	// Technology levels associated with this design
	int32_t tech_range;
	int32_t tech_speed;
	int32_t tech_weapons;
	int32_t tech_shields;
	int32_t tech_miniaturization;
	
	// Costs (calculated at design creation time, immutable)
	int64_t build_cost;              // Cost to build each ship of this design
	int64_t prototype_cost;          // Cost for the first ship of this design
	int64_t metal_cost;              // Metal cost per ship
};

// Player structure
struct Player
{
	uint32_t id;
	std::string name;
	PlayerType type;
	int32_t iq;  // For computer players
	
	// Resources
	int64_t money;  // Savings account
	int64_t metal_reserve;
	
	// Technology levels
	TechnologyLevels tech;
	
	// Ideal planetary conditions (hidden from player)
	double ideal_temperature;
	double ideal_gravity;
	
	// Calculated properties
	int64_t money_income;  // Per turn
	int64_t metal_income;  // Per turn
	
	// Income breakdown for current turn
	IncomeBreakdown current_turn_income;
	
	// Money allocation for this player (fractions-based)
	struct MoneyAllocation
	{
		double savings_fraction;
		double research_fraction;
		double planets_fraction;  // Fraction of income allocated to planet development
		ResearchAllocation research;
	};
	
	// Current money allocation
	MoneyAllocation allocation;
	
	// Colonized planets (owned by this player with allocation information)
	std::vector<ColonizedPlanet> colonized_planets;
	
	// Fog of war: player's knowledge of planets
	std::vector<PlanetInfo> planetKnowledge;
	
	// Ship designs
	std::vector<ShipDesign> ship_designs;      // All designs, ordered by creation (max 100)
	uint32_t next_ship_design_id;                // Counter for unique design IDs (never resets)
	
	// Research progress (accumulated points per research stream)
	int64_t research_points_range;
	int64_t research_points_speed;
	int64_t research_points_weapons;
	int64_t research_points_shields;
	int64_t research_points_miniaturization;
	int64_t research_points_radical;
};

// Galaxy generation parameters
struct GalaxyGenerationParams
{
	uint32_t size;      // Number of planets (5-500)
	double density;     // Planet distribution density (TBD)
	std::string shape;  // "random", "spiral", "circle", "ring", "cluster", "grid"
	uint64_t seed;      // Random seed for generation
};

// Public information about a player (visible to all other players)
struct PlayerPublicInfo
{
	uint32_t player_id;
	uint32_t turn;
	
	// Technology levels (subset - no Radical)
	int32_t tech_range;
	int32_t tech_speed;
	int32_t tech_weapons;
	int32_t tech_shields;
	int32_t tech_miniaturization;
	
	// Resources
	int64_t money_income;
	int64_t money_savings;
	int64_t metal_savings;
	int64_t income;  // Total income this turn (for historical tracking)
	
	// Calculated metrics
	int64_t fleet_power;
	int32_t victory_points;
};

// Galaxy structure
struct Galaxy
{
	// Boundaries
	GalaxyCoord min_x;
	GalaxyCoord max_x;
	GalaxyCoord min_y;
	GalaxyCoord max_y;
	
	// Immutable planet list
	std::vector<Planet> planets;
	
	// Mutable ship list
	std::vector<Ship> ships;
	
	// Players
	std::vector<Player> players;
	
	// Current turn number
	uint32_t current_turn;
	
	// Two separate RNG engines
	// (Implementation details in rng.h)
};

// ============================================================================
// C API for Objective-C++ Bridging
// ============================================================================

#ifdef __cplusplus
extern "C"
{
#endif

// Game initialization and management
void* game_create(const GalaxyGenerationParams* params);
void game_destroy(void* game);

// Game state queries
uint32_t game_get_current_turn(void* game);
uint32_t game_get_num_players(void* game);
uint32_t game_get_num_planets(void* game);
uint32_t game_get_num_ships(void* game);

// Player queries
void game_get_player(void* game, uint32_t player_id, Player* out);
int64_t game_get_player_money_income(void* game, uint32_t player_id);
int64_t game_get_player_metal_income(void* game, uint32_t player_id);
int64_t game_get_player_money_reserve(void* game, uint32_t player_id);
int64_t game_get_player_metal_reserve(void* game, uint32_t player_id);

// Planet queries
void game_get_planet(void* game, uint32_t planet_id, Planet* out);
void game_get_planet_perceived_values(void* game, uint32_t planet_id, uint32_t player_id, double* out_temp, double* out_grav);

// Ship queries
void game_get_ship(void* game, uint32_t ship_id, Ship* out);

// Fleet queries
uint32_t game_get_num_fleets(void* game);

// Player public information queries
void game_get_player_public_info(void* game, uint32_t player_id, uint32_t turn, PlayerPublicInfo* out);
void game_get_player_public_info_current(void* game, uint32_t player_id, PlayerPublicInfo* out);
uint32_t game_get_player_info_history_size(void* game, uint32_t player_id);

// Ship design queries and management
uint32_t game_create_ship_design(void* game, uint32_t player_id, const char* name, ShipType type, int32_t tech_range, int32_t tech_speed, int32_t tech_weapons, int32_t tech_shields, int32_t tech_miniaturization);
void game_get_ship_design(void* game, uint32_t player_id, uint32_t design_id, ShipDesign* out);
void game_get_player_ship_designs(void* game, uint32_t player_id, ShipDesign* out, uint32_t max_count, uint32_t* out_count);
uint32_t game_get_num_player_ship_designs(void* game, uint32_t player_id);
void game_delete_ship_design(void* game, uint32_t player_id, uint32_t design_id);
void game_build_ship_from_design(void* game, uint32_t player_id, uint32_t design_id);

// Money allocation
void game_set_money_allocation(void* game, uint32_t player_id, const Player::MoneyAllocation* alloc);
void game_get_money_allocation(void* game, uint32_t player_id, Player::MoneyAllocation* out);

// AI RNG seed management
uint64_t game_get_ai_rng_seed(void* game);
void game_set_ai_rng_seed(void* game, uint64_t seed);

// Turn processing
void game_process_turn(void* game);

// Serialization
int game_serialize_state(void* game, void* buffer, int buffer_size);
int game_deserialize_state(void* game, const void* buffer, int buffer_size);
int game_get_serialized_state_size(void* game);

#ifdef __cplusplus
}
#endif

#endif // OPENHO_CORE_H
