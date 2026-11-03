#ifndef OPENHO_SAVE_FILE_H
#define OPENHO_SAVE_FILE_H

#include "game_setup.h"
#include "galaxy.h"
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
    static constexpr uint32_t CURRENT_SCHEMA_VERSION = 1;

    uint32_t schema_version = CURRENT_SCHEMA_VERSION;
    GalaxyGenerationParams galaxy_params;
    double galaxy_size = 0.0;
    std::vector<ImmutablePlanetDefinition> planets;
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
