#include "cli_application.h"

#include "game.h"
#include "game_session.h"
#include "game_setup.h"
#include "ui_adapter.h"

#include <array>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <utility>

namespace
{
constexpr uint32_t kDefaultPlanetCount = 10;
constexpr uint32_t kDefaultPlayerCount = 2;
constexpr double kDefaultDensity = 0.5;
constexpr uint64_t kDefaultSeed = 42;

std::string trim(const std::string& value)
{
    const size_t first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos)
        return {};
    const size_t last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

std::string owner_text(int32_t owner)
{
    if (owner == NOT_OWNED)
        return "unowned";
    return std::to_string(owner);
}
}

CliApplication::CliApplication(std::string assets_directory)
    : assets_directory_(std::move(assets_directory))
{
    if (assets_directory_.empty())
        assets_directory_ = "TextAssets";
    initialize_new_session();
}

CliApplication::~CliApplication() = default;

bool CliApplication::initialize_new_session()
{
    std::vector<PlayerSetup> player_setups = {
        {"Player One", GENDER_F, PLAYER_HUMAN, 100, START_NORMAL},
        {"Player Two", GENDER_M, PLAYER_HUMAN, 100, START_NORMAL}
    };
    GameSetup setup(
        GalaxyGenerationParams(
            kDefaultPlanetCount,
            kDefaultPlayerCount,
            kDefaultDensity,
            GALAXY_RANDOM,
            kDefaultSeed),
        std::move(player_setups),
        assets_directory_);

    GameSessionError error = GameSessionError::SUCCESS;
    session_ = GameSession::create_new(setup, &error);
    if (!session_)
    {
        print_error("create game", game_session_error_to_string(error));
        return false;
    }

    const GameSessionError save_result = session_->save_as(default_save_path_);
    if (save_result != GameSessionError::SUCCESS)
    {
        print_error("create save", game_session_error_to_string(save_result));
        session_.reset();
        return false;
    }

    if (!mark_non_local_players_ready())
        return false;
    return true;
}

bool CliApplication::mark_non_local_players_ready()
{
    if (!session_ || !session_->state() || !session_->ui())
        return false;

    for (const Player& player : session_->state()->get_players())
    {
        if (player.id == local_player_id_)
            continue;
        const ErrorCode result = session_->ui()->mark_ready(player.id);
        if (result != ErrorCode::SUCCESS && result != ErrorCode::PLAYER_ALREADY_READY)
        {
            print_error("ready player " + std::to_string(player.id),
                        error_code_to_string(result));
            return false;
        }
    }
    return true;
}

void CliApplication::print_help() const
{
    std::cout << "Commands:\n"
              << "  help                         Show this help.\n"
              << "  status                       Show turn, year, and save state.\n"
              << "  players                      Show players and readiness.\n"
              << "  planets                      Show the local player's planet views.\n"
              << "  ready                        Mark player 1 ready.\n"
              << "  unready                      Mark player 1 unready.\n"
              << "  process                      Process the turn and autosave.\n"
              << "  save <path>                  Save a manual checkpoint.\n"
              << "  load <path>                  Load the latest checkpoint.\n"
              << "  quit                         Exit the CLI.\n";
}

void CliApplication::print_status() const
{
    if (!session_ || !session_->state() || !session_->ui())
    {
        std::cout << "No active game session.\n";
        return;
    }
    const UiGameSnapshot snapshot = session_->ui()->snapshot(local_player_id_);
    std::cout << "Turn " << snapshot.current_turn
              << ", year " << snapshot.current_year
              << ", all players ready: " << (snapshot.all_players_ready ? "yes" : "no")
              << ", save: "
              << (session_->save_path().empty() ? "none" : session_->save_path())
              << "\n";
}

void CliApplication::print_players() const
{
    if (!session_ || !session_->ui())
        return;

    const UiGameSnapshot snapshot = session_->ui()->snapshot(local_player_id_);
    for (const UiPlayerSnapshot& player : snapshot.players)
    {
        std::cout << "Player " << player.id << ": " << player.name
                  << ", ready=" << (player.ready ? "yes" : "no")
                  << ", eliminated=" << (player.eliminated ? "yes" : "no");
        if (player.id == local_player_id_)
        {
            std::cout << ", money=" << player.money
                      << ", income=" << player.money_income
                      << ", metal=" << player.metal_reserve
                      << ", colonies=" << player.colonies.size();
        }
        std::cout << "\n";
    }
}

void CliApplication::print_planets() const
{
    if (!session_ || !session_->ui())
        return;

    const UiGameSnapshot snapshot = session_->ui()->snapshot(local_player_id_);
    std::cout << "Planets visible to player " << local_player_id_ << ":\n";
    for (const UiPlanetSnapshot& planet : snapshot.planets)
    {
        std::cout << "  " << planet.id << ": " << planet.name
                  << " (" << std::fixed << std::setprecision(1)
                  << planet.x << ", " << planet.y << ")";
        if (!planet.has_observation)
        {
            std::cout << " [unobserved]\n";
            continue;
        }
        std::cout << " owner=" << owner_text(planet.apparent_owner)
                  << " population=" << planet.apparent_population
                  << " observed=" << planet.observation_year << "\n";
    }
}

void CliApplication::print_error(const std::string& operation,
                                  const std::string& detail) const
{
    std::cerr << "Error: " << operation << ": " << detail << "\n";
}

bool CliApplication::handle_command(const std::string& raw_line)
{
    const std::string line = trim(raw_line);
    if (line.empty())
        return true;

    std::istringstream input(line);
    std::string command;
    input >> command;

    if (command == "help")
    {
        print_help();
    }
    else if (command == "status")
    {
        print_status();
    }
    else if (command == "players")
    {
        print_players();
    }
    else if (command == "planets")
    {
        print_planets();
    }
    else if (command == "ready")
    {
        const ErrorCode result = session_ && session_->ui()
            ? session_->ui()->mark_ready(local_player_id_)
            : ErrorCode::INVALID_TURN_STATE;
        if (result != ErrorCode::SUCCESS)
            print_error("ready", error_code_to_string(result));
        else
            std::cout << "Player " << local_player_id_ << " is ready.\n";
    }
    else if (command == "unready")
    {
        const ErrorCode result = session_ && session_->ui()
            ? session_->ui()->mark_unready(local_player_id_)
            : ErrorCode::INVALID_TURN_STATE;
        if (result != ErrorCode::SUCCESS)
            print_error("unready", error_code_to_string(result));
        else
            std::cout << "Player " << local_player_id_ << " is unready.\n";
    }
    else if (command == "process")
    {
        if (!session_)
        {
            print_error("process", "no active game session");
        }
        else
        {
            if (!mark_non_local_players_ready())
                return false;
            const GameSessionError result = session_->process_turn_and_autosave();
            if (result != GameSessionError::SUCCESS)
            {
                const std::string detail = result == GameSessionError::GAME_NOT_READY
                    ? error_code_to_string(session_->last_game_error())
                    : game_session_error_to_string(result);
                print_error("process", detail);
            }
            else
            {
                std::cout << "Turn processed and autosaved.\n";
            }
        }
    }
    else if (command == "save")
    {
        std::string path;
        input >> path;
        if (path.empty())
        {
            print_error("save", "a path is required");
        }
        else if (!session_ || session_->save_as(path) != GameSessionError::SUCCESS)
        {
            print_error("save", "unable to write checkpoint");
        }
        else
        {
            std::cout << "Saved to " << path << ".\n";
        }
    }
    else if (command == "load")
    {
        std::string path;
        input >> path;
        GameSessionError error = GameSessionError::SUCCESS;
        auto loaded = GameSession::load(path, &error, assets_directory_);
        if (!loaded)
        {
            print_error("load", game_session_error_to_string(error));
        }
        else
        {
            session_ = std::move(loaded);
            if (!mark_non_local_players_ready())
                return false;
            std::cout << "Loaded " << path << ".\n";
        }
    }
    else if (command == "quit" || command == "exit")
    {
        if (session_)
            session_->close();
        return false;
    }
    else
    {
        print_error("command", "unknown command; try 'help'");
    }
    return true;
}

int CliApplication::run()
{
    if (!session_)
        return 1;

    std::cout << "OpenHo CLI skeleton. Player 1 is local; player 2 is automatically ready.\n";
    std::cout << "Type 'help' for commands.\n";
    print_status();

    std::string line;
    while (std::cout << "> " && std::getline(std::cin, line))
    {
        if (!handle_command(line))
            break;
    }
    return 0;
}
