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

// Development allocation for a specific planet
struct PlanetDevelopmentAllocation
{
	uint32_t planet_id;
	double development_fraction;
	double mining_fraction;
	double terraforming_fraction;
};

// Money allocation for a player (fractions-based)
struct MoneyAllocation
{
	double savings_fraction;
	double research_fraction;
	ResearchAllocation research;
	std::vector<PlanetDevelopmentAllocation> planet_allocations;
};

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
	uint32_t observationTurn;  // When this information was collected
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
	int32_t population;
	
	// Nova state
	PlanetState state;
	int32_t turnsUntilNova;  // If state == PLANET_WARNING
	
	// Calculated property
	int32_t income;  // Calculated based on population, temperature, gravity, and owner's ideals
};

// Ship structure
struct Ship
{
	uint32_t id;
	ShipType type;
	PlayerID owner;
	ShipTechParameters techParams;
	
	// Position: either at a planet or in transit
	uint32_t currentPlanetID;  // If not in transit
	uint32_t destinationPlanetID;  // If in transit
	int32_t etaTurns;  // Turns until arrival (if in transit)
	
	// Fleet information
	uint32_t fleetID;  // Every ship is always in a fleet; a lone ship is a fleet of one
};

// Ship design pattern (immutable once created)
struct ShipDesign
{
	uint32_t id;                    // Unique ID within player's design library
	std::string name;
	ShipType type;
	
	// Technology levels associated with this design
	int32_t techRange;
	int32_t techSpeed;
	int32_t techWeapons;
	int32_t techShields;
	int32_t techMiniaturization;
	
	// Costs (calculated at design creation time, immutable)
	int64_t buildCost;              // Cost to build each ship of this design
	int64_t prototypeCost;          // Cost for the first ship of this design
	int64_t metalCost;              // Metal cost per ship
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
	int64_t metalReserve;
	
	// Technology levels
	TechnologyLevels tech;
	
	// Ideal planetary conditions (hidden from player)
	double idealTemperature;
	double idealGravity;
	
	// Calculated properties
	int64_t moneyIncome;  // Per turn
	int64_t metalIncome;  // Per turn
	
	// Current money allocation
	MoneyAllocation allocation;
	
	// Fog of war: player's knowledge of planets
	std::vector<PlanetInfo> planetKnowledge;
	
	// Ship designs
	std::vector<ShipDesign> shipDesigns;      // All designs, ordered by creation (max 100)
	uint32_t nextShipDesignID;                // Counter for unique design IDs (never resets)
	
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
	uint32_t playerID;
	uint32_t turn;
	
	// Technology levels (subset - no Radical)
	int32_t techRange;
	int32_t techSpeed;
	int32_t techWeapons;
	int32_t techShields;
	int32_t techMiniaturization;
	
	// Resources
	int64_t moneyIncome;
	int64_t moneySavings;
	int64_t metalSavings;
	
	// Calculated metrics
	int64_t fleetPower;
	int32_t victoryPoints;
};

// Galaxy structure
struct Galaxy
{
	// Boundaries
	GalaxyCoord minX;
	GalaxyCoord maxX;
	GalaxyCoord minY;
	GalaxyCoord maxY;
	
	// Immutable planet list
	std::vector<Planet> planets;
	
	// Mutable ship list
	std::vector<Ship> ships;
	
	// Players
	std::vector<Player> players;
	
	// Current turn number
	uint32_t currentTurn;
	
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

// Game state// Game initialization & management
void* game_create();
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
void game_set_money_allocation(void* game, uint32_t player_id, const MoneyAllocation* alloc);
void game_get_money_allocation(void* game, uint32_t player_id, MoneyAllocation* out);

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
