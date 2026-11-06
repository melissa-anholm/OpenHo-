#include "game_session.h"

#include "game.h"
#include "game_setup.h"
#include "save_file.h"
#include "ui_adapter.h"

#include <exception>
#include <utility>

namespace
{
void set_error(GameSessionError* output, GameSessionError error)
{
    if (output)
        *output = error;
}
}

const char* game_session_error_to_string(GameSessionError error)
{
    switch (error)
    {
        case GameSessionError::SUCCESS:
            return "Success";
        case GameSessionError::SESSION_CLOSED:
            return "Session is closed";
        case GameSessionError::INVALID_ARGUMENT:
            return "Invalid argument";
        case GameSessionError::GAME_CREATION_FAILED:
            return "Game creation failed";
        case GameSessionError::INVALID_SAVE_FILE:
            return "Invalid save file";
        case GameSessionError::INCOMPATIBLE_CHECKPOINT:
            return "Checkpoint is incompatible with the game definition";
        case GameSessionError::SAVE_FAILED:
            return "Save failed";
        case GameSessionError::GAME_NOT_READY:
            return "Game is not ready";
        case GameSessionError::TURN_PROCESSING_FAILED:
            return "Turn processing failed";
    }
    return "Unknown session error";
}

GameSession::GameSession(std::unique_ptr<GameState> state,
                         std::string save_path)
    : state_(std::move(state)),
      ui_(state_ ? std::make_unique<GameUiAdapter>(*state_) : nullptr),
      save_path_(std::move(save_path))
{
}

GameSession::~GameSession() = default;

GameSession::GameSession(GameSession&& other) noexcept
    : state_(std::move(other.state_)),
      ui_(state_ ? std::make_unique<GameUiAdapter>(*state_) : nullptr),
      save_path_(std::move(other.save_path_)),
      last_game_error_(other.last_game_error_)
{
    other.ui_.reset();
}

GameSession& GameSession::operator=(GameSession&& other) noexcept
{
    if (this == &other)
        return *this;

    close();
    state_ = std::move(other.state_);
    ui_ = state_ ? std::make_unique<GameUiAdapter>(*state_) : nullptr;
    save_path_ = std::move(other.save_path_);
    last_game_error_ = other.last_game_error_;
    other.ui_.reset();
    return *this;
}

std::unique_ptr<GameSession> GameSession::create_new(
    const GameSetup& setup,
    GameSessionError* error)
{
    set_error(error, GameSessionError::SUCCESS);
    try
    {
        auto state = std::make_unique<GameState>(setup);
        return std::unique_ptr<GameSession>(
            new GameSession(std::move(state)));
    }
    catch (const std::exception&)
    {
        set_error(error, GameSessionError::GAME_CREATION_FAILED);
        return nullptr;
    }
}

std::unique_ptr<GameSession> GameSession::load(
    const std::string& path,
    GameSessionError* error)
{
    set_error(error, GameSessionError::SUCCESS);
    if (path.empty())
    {
        set_error(error, GameSessionError::INVALID_ARGUMENT);
        return nullptr;
    }

    std::vector<uint8_t> definition_payload;
    SaveCheckpoint checkpoint;
    GameDefinition definition;
    if (!SaveFile::read_immutable_definition(path, definition_payload) ||
        !decode_game_definition(definition_payload, definition) ||
        !SaveFile::read_latest_checkpoint(path, checkpoint))
    {
        set_error(error, GameSessionError::INVALID_SAVE_FILE);
        return nullptr;
    }

    try
    {
        GameSetup setup(definition.galaxy_params, definition.player_setups);
        auto state = std::make_unique<GameState>(setup);
        if (!state->deserialize_state(checkpoint.payload))
        {
            set_error(error, GameSessionError::INCOMPATIBLE_CHECKPOINT);
            return nullptr;
        }
        return std::unique_ptr<GameSession>(
            new GameSession(std::move(state), path));
    }
    catch (const std::exception&)
    {
        set_error(error, GameSessionError::GAME_CREATION_FAILED);
        return nullptr;
    }
}

bool GameSession::is_open() const noexcept
{
    return state_ != nullptr && ui_ != nullptr;
}

GameState* GameSession::state() noexcept
{
    return state_.get();
}

const GameState* GameSession::state() const noexcept
{
    return state_.get();
}

GameUiAdapter* GameSession::ui() noexcept
{
    return ui_.get();
}

const GameUiAdapter* GameSession::ui() const noexcept
{
    return ui_.get();
}

GameSessionError GameSession::write_checkpoint(
    const std::string& path,
    SaveRecordPhase phase,
    bool replace_file)
{
    if (!is_open() || path.empty())
        return is_open() ? GameSessionError::INVALID_ARGUMENT
                         : GameSessionError::SESSION_CLOSED;

    const std::vector<uint8_t> definition = state_->serialize_immutable_definition();
    const std::vector<uint8_t> runtime = state_->serialize_state();
    if (definition.empty() || runtime.empty())
        return GameSessionError::SAVE_FAILED;

    if (replace_file)
    {
        if (!SaveFile::create(path, definition))
            return GameSessionError::SAVE_FAILED;
    }

    if (!SaveFile::append_checkpoint(path, state_->get_current_turn(), phase, runtime))
        return GameSessionError::SAVE_FAILED;
    return GameSessionError::SUCCESS;
}

GameSessionError GameSession::save_as(const std::string& path)
{
    const GameSessionError result = write_checkpoint(
        path, SaveRecordPhase::MANUAL_CHECKPOINT, true);
    if (result == GameSessionError::SUCCESS)
        save_path_ = path;
    return result;
}

GameSessionError GameSession::save()
{
    return write_checkpoint(save_path_, SaveRecordPhase::MANUAL_CHECKPOINT, false);
}

GameSessionError GameSession::autosave()
{
    return write_checkpoint(save_path_, SaveRecordPhase::TURN_CHECKPOINT, false);
}

GameSessionError GameSession::process_turn_and_autosave()
{
    if (!is_open())
        return GameSessionError::SESSION_CLOSED;

    last_game_error_ = ui_->process_turn();
    if (last_game_error_ != ErrorCode::SUCCESS)
    {
        return last_game_error_ == ErrorCode::GAME_NOT_READY
            ? GameSessionError::GAME_NOT_READY
            : GameSessionError::TURN_PROCESSING_FAILED;
    }
    return autosave();
}

void GameSession::close() noexcept
{
    ui_.reset();
    state_.reset();
    save_path_.clear();
    last_game_error_ = ErrorCode::SUCCESS;
}
