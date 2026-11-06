#ifndef OPENHO_GAME_SESSION_H
#define OPENHO_GAME_SESSION_H

#include "error_codes.h"
#include <cstdint>
#include <memory>
#include <string>

class GameSetup;
class GameState;
class GameUiAdapter;
enum class SaveRecordPhase : uint8_t;

/// Errors produced by the frontend-independent game-session lifecycle layer.
enum class GameSessionError : uint32_t
{
    SUCCESS = 0,
    SESSION_CLOSED,
    INVALID_ARGUMENT,
    GAME_CREATION_FAILED,
    INVALID_SAVE_FILE,
    INCOMPATIBLE_CHECKPOINT,
    SAVE_FAILED,
    GAME_NOT_READY,
    TURN_PROCESSING_FAILED,
};

[[nodiscard]] const char* game_session_error_to_string(GameSessionError error);

/// Owns one GameState and the UI adapter that references it.
///
/// GameSession is the lifecycle boundary for frontends. It creates or loads a
/// game, rebuilds the adapter whenever state is replaced, manages the active
/// save path, and provides manual and turn-boundary checkpoint operations.
class GameSession
{
public:
    /// Create a new session from an already-collected setup description.
    /// The setup is not queried interactively here.
    [[nodiscard]] static std::unique_ptr<GameSession> create_new(
        const GameSetup& setup,
        GameSessionError* error = nullptr);

    /// Load the immutable definition and newest checkpoint from a save file.
    [[nodiscard]] static std::unique_ptr<GameSession> load(
        const std::string& path,
        GameSessionError* error = nullptr,
        const std::string& assets_directory = "TextAssets");

    ~GameSession();

    GameSession(const GameSession&) = delete;
    GameSession& operator=(const GameSession&) = delete;
    GameSession(GameSession&&) noexcept;
    GameSession& operator=(GameSession&&) noexcept;

    [[nodiscard]] bool is_open() const noexcept;
    [[nodiscard]] GameState* state() noexcept;
    [[nodiscard]] const GameState* state() const noexcept;
    [[nodiscard]] GameUiAdapter* ui() noexcept;
    [[nodiscard]] const GameUiAdapter* ui() const noexcept;

    /// Save to a new or existing path and make it the active session path.
    [[nodiscard]] GameSessionError save_as(const std::string& path);

    /// Save to the active path as a manual checkpoint.
    [[nodiscard]] GameSessionError save();

    /// Append a turn checkpoint to the active path.
    [[nodiscard]] GameSessionError autosave();

    /// Process the turn through the UI boundary and autosave on success.
    [[nodiscard]] GameSessionError process_turn_and_autosave();

    /// Close the session. References returned by state() and ui() become null.
    void close() noexcept;

    [[nodiscard]] const std::string& save_path() const noexcept
    {
        return save_path_;
    }

    /// If the most recent session operation delegated to GameUiAdapter and
    /// failed, this returns its underlying core error code.
    [[nodiscard]] ErrorCode last_game_error() const noexcept
    {
        return last_game_error_;
    }

private:
    explicit GameSession(std::unique_ptr<GameState> state,
                         std::string save_path = {});

    [[nodiscard]] GameSessionError write_checkpoint(
        const std::string& path,
        SaveRecordPhase phase,
        bool replace_file);

    std::unique_ptr<GameState> state_;
    std::unique_ptr<GameUiAdapter> ui_;
    std::string save_path_;
    ErrorCode last_game_error_ = ErrorCode::SUCCESS;
};

#endif // OPENHO_GAME_SESSION_H
