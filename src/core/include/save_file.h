#ifndef OPENHO_SAVE_FILE_H
#define OPENHO_SAVE_FILE_H

#include "game_setup.h"
#include "galaxy.h"
#include "economic_state.h"
#include <array>
#include <cstdint>
#include <string>
#include <vector>

/// Static planet information that defines a generated galaxy. Planetary
/// temperature is deliberately excluded: even the game's true temperature is
/// mutable and belongs in a runtime checkpoint, along with ownership,
/// population, metal, and nova state.
struct ImmutablePlanetDefinition
{
    uint32_t id = 0;
    std::string name;
    double x = 0.0;
    double y = 0.0;
    double true_gravity = 0.0;
};

/// The immutable portion of one game. This is a logical payload, not a dump of
/// C++ object memory or pointers.
struct GameDefinition
{
    static constexpr uint32_t CURRENT_SCHEMA_VERSION = 2;

    uint32_t schema_version = CURRENT_SCHEMA_VERSION;
    GalaxyGenerationParams galaxy_params;
    double galaxy_size = 0.0;
    std::vector<ImmutablePlanetDefinition> planets;
    // Both structures are indexed by one-based planet ID; row zero is unused.
    std::vector<std::vector<double>> distance_matrix;
    std::vector<std::vector<PlanetDistance>> destinations_by_distance;
    std::vector<uint32_t> home_planet_ids;
    std::vector<PlayerSetup> player_setups;
    // Indexed in the same order as player_setups. These are finalized during
    // game initialization and remain unchanged for the life of the game.
    std::vector<double> player_ideal_temperatures;
    std::vector<double> player_ideal_gravities;
};

/// Encode/decode the immutable definition payload stored at the beginning of
/// an OpenHo save file. The codec validates sizes, enums, finite floating-point
/// values, and the payload magic/version.
[[nodiscard]] std::vector<uint8_t> encode_game_definition(const GameDefinition& definition);
[[nodiscard]] bool decode_game_definition(const std::vector<uint8_t>& payload,
                                          GameDefinition& definition);

struct RuntimePlanetState
{
    uint32_t id = 0;
    int32_t owner = 0;
    double true_temperature = 0.0;
    int32_t metal = 0;
    int32_t population = 0;
    int32_t nova_state = 0;
    struct View
    {
        uint32_t observer_id = 0;
        double apparent_temperature = 0.0;
        bool has_observed_gravity = false;
        int32_t metal = 0;
        int32_t apparent_owner = 0;
        int32_t apparent_population = 0;
        int32_t observation_year = 0;
        int32_t income = 0;
        int32_t can_be_profitable = 0;
        int32_t perceived_value = 0;
    };

    std::vector<View> views;
};

struct RuntimeColonyState
{
    uint32_t planet_id = 0;
    int32_t income = 0;
    double funding_fraction = 0.0;
    double mining_fraction = 0.0;
    double terraforming_fraction = 0.0;
};

struct RuntimePlayerState
{
    uint32_t id = 0;
    uint32_t economic_submission_target_turn = 0;
    bool eliminated = false;
    int64_t money_savings = 0;
    int64_t metal_reserve = 0;
    int64_t metal_income = 0;
    double savings_rate_modifier = 1.0;
    int64_t pending_dip_amount = 0;
    bool pending_dip_persistent = false;
    int64_t locked_dip_amount = 0;
    bool locked_dip_persistent = false;
    double savings_fraction = 0.0;
    double research_fraction = 0.0;
    double planets_fraction = 0.0;
    std::array<double, 6> research_fractions{};
    std::array<int32_t, 6> technology_levels{};
    std::array<int64_t, 6> partial_research{};
    std::vector<RuntimeColonyState> colonies;
    std::vector<uint32_t> planets_to_abandon;
    EconomicAfterState economic_afterstate;
};

struct RuntimeCheckpointPayload
{
    static constexpr uint32_t CURRENT_SCHEMA_VERSION = 9;

    uint32_t schema_version = CURRENT_SCHEMA_VERSION;
    uint32_t turn = 0;
    uint32_t year = 2000;
    uint64_t deterministic_seed = 0;
    uint64_t ai_seed = 0;
    std::vector<uint8_t> deterministic_rng_state;
    std::vector<uint8_t> ai_rng_state;
    std::vector<RuntimePlanetState> planets;
    std::vector<RuntimePlayerState> players;
};

[[nodiscard]] std::vector<uint8_t> encode_runtime_checkpoint(
    const RuntimeCheckpointPayload& checkpoint);
[[nodiscard]] bool decode_runtime_checkpoint(
    const std::vector<uint8_t>& payload,
    RuntimeCheckpointPayload& checkpoint);

/// Logical phase at which a save record was created.
enum class SaveRecordPhase : uint8_t
{
    TURN_CHECKPOINT = 1,
    MANUAL_CHECKPOINT = 2,
};

/// A validated runtime checkpoint read from an OpenHo save file.
struct SaveCheckpoint
{
    uint32_t turn = 0;
    SaveRecordPhase phase = SaveRecordPhase::TURN_CHECKPOINT;
    std::vector<uint8_t> payload;
};

/// Append-only container for one immutable game definition and checkpoint records.
///
/// The container deliberately treats the GameState payload as opaque. GameState
/// owns the logical snapshot schema; this class owns file framing, checksums,
/// append/recovery behavior, and compaction.
class SaveFile
{
public:
    /// Create or replace a save file containing an immutable definition.
    static bool create(const std::string& path,
                       const std::vector<uint8_t>& immutable_definition);

    /// Append a complete runtime checkpoint to an existing save file.
    static bool append_checkpoint(const std::string& path,
                                  uint32_t turn,
                                  SaveRecordPhase phase,
                                  const std::vector<uint8_t>& payload);

    /// Read the newest complete, checksum-valid checkpoint. Incomplete trailing
    /// bytes are ignored so an interrupted append preserves the prior checkpoint.
    static bool read_latest_checkpoint(const std::string& path,
                                       SaveCheckpoint& checkpoint);

    /// Read the immutable definition section from a save file.
    static bool read_immutable_definition(const std::string& path,
                                          std::vector<uint8_t>& definition);

    /// Compact the file to its immutable definition and newest valid checkpoint.
    static bool compact(const std::string& path);
};

#endif // OPENHO_SAVE_FILE_H
