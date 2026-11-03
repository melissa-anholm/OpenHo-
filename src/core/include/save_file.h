#ifndef OPENHO_SAVE_FILE_H
#define OPENHO_SAVE_FILE_H

#include <cstdint>
#include <string>
#include <vector>

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
