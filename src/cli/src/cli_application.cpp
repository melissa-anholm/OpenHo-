#include "cli_application.h"

#include "game.h"
#include "game_session.h"
#include "game_setup.h"
#include "text_assets.h"
#include "ui_adapter.h"

#include <array>
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
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

std::string owner_text(int32_t owner, const std::vector<UiPlayerSnapshot>& players)
{
    if (owner == NOT_OWNED)
        return "unowned";
    const auto player = std::find_if(
        players.begin(), players.end(),
        [owner](const UiPlayerSnapshot& value) {
            return value.id == static_cast<uint32_t>(owner);
        });
    return player == players.end() ? "unknown" : player->name;
}

std::vector<std::string> split_tokens(const std::string& value)
{
    std::istringstream input(value);
    std::vector<std::string> tokens;
    std::string token;
    while (input >> token)
        tokens.push_back(token);
    return tokens;
}

size_t research_stream_index(const std::string& name)
{
    static const std::array<std::string, 6> names = {
        "range", "speed", "weapons", "shields", "mini", "radical"};
    const auto match = std::find(names.begin(), names.end(), name);
    return match == names.end()
        ? names.size()
        : static_cast<size_t>(std::distance(names.begin(), match));
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
    TextAssets text_assets;
    if (!text_assets.load_assets(assets_directory_))
    {
        print_error("load player names", "unable to load text assets");
        return false;
    }

    const std::string female_name = text_assets.get_random_female_player_name(0);
    const std::string male_name = text_assets.get_random_male_player_name(0);
    std::vector<PlayerSetup> player_setups = {
        {female_name.empty() ? "Player One" : female_name,
         GENDER_F, PLAYER_HUMAN, 100, START_NORMAL},
        {male_name.empty() ? "Player Two" : male_name,
         GENDER_M, PLAYER_HUMAN, 100, START_NORMAL}
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
    return synchronize_pending_submission();
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

bool CliApplication::synchronize_pending_submission()
{
    if (!session_ || !session_->state() || !session_->ui())
        return false;

    const UiGameSnapshot snapshot = session_->ui()->snapshot(local_player_id_);
    const auto player = std::find_if(
        snapshot.players.begin(), snapshot.players.end(),
        [this](const UiPlayerSnapshot& value) { return value.id == local_player_id_; });
    if (player == snapshot.players.end())
        return false;

    pending_submission_.savings_fraction = player->savings_fraction;
    pending_submission_.research_fraction = player->research_fraction;
    pending_submission_.planets_fraction = player->planets_fraction;
    pending_submission_.research_fractions = player->research_fractions;
    pending_submission_.dip_amount = player->dip_amount;
    pending_submission_.dip_persistent = player->dip_persistent;
    pending_submission_.colony_allocations.clear();
    for (const UiColonySnapshot& colony : player->colonies)
    {
        pending_submission_.colony_allocations.push_back({
            colony.planet_id,
            colony.funding_fraction,
            colony.mining_fraction,
            colony.terraforming_fraction});
    }
    double funding_total = 0.0;
    for (const EconomicColonyAllocation& colony : pending_submission_.colony_allocations)
        funding_total += colony.funding_fraction;
    if (funding_total > std::numeric_limits<double>::epsilon())
    {
        for (EconomicColonyAllocation& colony : pending_submission_.colony_allocations)
            colony.funding_fraction /= funding_total;
    }
    pending_submission_.planets_to_abandon.clear();
    submission_dirty_ = false;
    return true;
}

bool CliApplication::submit_pending_submission()
{
    if (!session_ || !session_->ui())
        return false;

    const ErrorCode result = session_->ui()->submit_economic_actions(
        local_player_id_, pending_submission_);
    if (result != ErrorCode::SUCCESS)
    {
        print_error("submit", error_code_to_string(result));
        return false;
    }
    submission_dirty_ = false;
    std::cout << "Economic submission accepted for the current turn.\n";
    return true;
}

bool CliApplication::adjust_savings_fraction(double value, std::string& error)
{
    return EconomicAllocation::adjust_fraction_proportionally(
        value, pending_submission_.savings_fraction,
        {&pending_submission_.research_fraction, &pending_submission_.planets_fraction},
        error);
}

bool CliApplication::adjust_research_budget_fraction(double value, std::string& error)
{
    return EconomicAllocation::adjust_fraction_proportionally(
        value, pending_submission_.research_fraction,
        {&pending_submission_.savings_fraction, &pending_submission_.planets_fraction},
        error);
}

bool CliApplication::set_planets_fraction(double value, std::string& error)
{
    return EconomicAllocation::adjust_fraction_proportionally(
        value, pending_submission_.planets_fraction,
        {&pending_submission_.savings_fraction, &pending_submission_.research_fraction},
        error);
}

bool CliApplication::adjust_planet_funding_fraction(uint32_t planet_id,
                                                    double value,
                                                    std::string& error)
{
    if (!std::isfinite(value) || value < 0.0 || value > 1.0)
    {
        error = "fraction must be finite and between 0 and 1";
        return false;
    }

    auto selected = std::find_if(
        pending_submission_.colony_allocations.begin(),
        pending_submission_.colony_allocations.end(),
        [planet_id](const EconomicColonyAllocation& allocation) {
            return allocation.planet_id == planet_id;
        });
    if (selected == pending_submission_.colony_allocations.end())
    {
        error = "planet is not one of the local colonies";
        return false;
    }

    std::vector<double> overall_planet_shares;
    overall_planet_shares.reserve(pending_submission_.colony_allocations.size());
    for (const EconomicColonyAllocation& allocation : pending_submission_.colony_allocations)
        overall_planet_shares.push_back(
            pending_submission_.planets_fraction * allocation.funding_fraction);

    const size_t selected_index = static_cast<size_t>(
        std::distance(pending_submission_.colony_allocations.begin(), selected));
    std::vector<double*> other_shares;
    other_shares.reserve(overall_planet_shares.size() + 2);
    other_shares.push_back(&pending_submission_.savings_fraction);
    other_shares.push_back(&pending_submission_.research_fraction);
    for (size_t index = 0; index < overall_planet_shares.size(); ++index)
        if (index != selected_index)
            other_shares.push_back(&overall_planet_shares[index]);

    if (!EconomicAllocation::adjust_fraction_proportionally(
            value, overall_planet_shares[selected_index], other_shares, error))
        return false;

    double new_planets_fraction = 0.0;
    for (double share : overall_planet_shares)
        new_planets_fraction += share;
    pending_submission_.planets_fraction = new_planets_fraction;

    if (new_planets_fraction > std::numeric_limits<double>::epsilon())
    {
        for (size_t index = 0; index < overall_planet_shares.size(); ++index)
            pending_submission_.colony_allocations[index].funding_fraction =
                overall_planet_shares[index] / new_planets_fraction;
    }
    return true;
}

bool CliApplication::adjust_planet_development_fraction(uint32_t planet_id,
                                                        bool mining,
                                                        double value,
                                                        std::string& error)
{
    auto selected = std::find_if(
        pending_submission_.colony_allocations.begin(),
        pending_submission_.colony_allocations.end(),
        [planet_id](const EconomicColonyAllocation& allocation) {
            return allocation.planet_id == planet_id;
        });
    if (selected == pending_submission_.colony_allocations.end())
    {
        error = "planet is not one of the local colonies";
        return false;
    }

    double& selected_fraction = mining
        ? selected->mining_fraction
        : selected->terraforming_fraction;
    double& peer_fraction = mining
        ? selected->terraforming_fraction
        : selected->mining_fraction;
    return EconomicAllocation::adjust_fraction_proportionally(
        value, selected_fraction, {&peer_fraction}, error);
}

bool CliApplication::adjust_research_stream_fraction(size_t stream_index,
                                                     double value,
                                                     std::string& error)
{
    if (stream_index >= pending_submission_.research_fractions.size())
    {
        error = "research stream index must be between 0 and 5";
        return false;
    }

    std::vector<double*> other_fractions;
    other_fractions.reserve(pending_submission_.research_fractions.size() - 1);
    for (size_t index = 0; index < pending_submission_.research_fractions.size(); ++index)
        if (index != stream_index)
            other_fractions.push_back(&pending_submission_.research_fractions[index]);
    return EconomicAllocation::adjust_fraction_proportionally(
        value, pending_submission_.research_fractions[stream_index], other_fractions, error);
}

bool CliApplication::resolve_planet_name(const std::string& name,
                                         uint32_t& planet_id,
                                         std::string& error) const
{
    if (!session_ || !session_->ui())
    {
        error = "no active game session";
        return false;
    }

    const std::string requested_name = trim(name);
    const UiGameSnapshot snapshot = session_->ui()->snapshot(local_player_id_);
    const UiPlanetSnapshot* match = nullptr;
    for (const UiPlanetSnapshot& planet : snapshot.planets)
    {
        if (planet.name != requested_name)
            continue;
        if (match != nullptr)
        {
            error = "planet name is ambiguous: " + requested_name;
            return false;
        }
        match = &planet;
    }

    if (match == nullptr)
    {
        error = "unknown planet name: " + requested_name;
        return false;
    }
    planet_id = match->id;
    return true;
}

void CliApplication::print_help() const
{
    std::cout << "Commands:\n"
              << "  help                         Show this help.\n"
              << "  status                       Show turn, year, and save state.\n"
              << "  players                      Show players and readiness.\n"
              << "  planets                      Show the local player's planet views.\n"
              << "  economy                      Show local economic state and pending choices.\n"
              << "  set_spending S R P           Set savings/research/planets fractions.\n"
              << "  set_savings VALUE            Adjust savings and proportionally rebalance the rest.\n"
              << "  set_research_budget VALUE    Adjust research and proportionally rebalance the rest.\n"
              << "  set_planets VALUE            Adjust planetary development and rebalance the rest.\n"
              << "  set_planet_budget NAME VALUE Set one planet's total-budget share.\n"
              << "  set_research A B C D E F     Set six research-stream fractions.\n"
              << "  set_research_stream NAME VALUE Adjust one research stream only.\n"
              << "  set_planet NAME F M T        Set funding/mining/terraforming fractions.\n"
              << "  set_mining NAME VALUE        Adjust mining and rebalance terraforming.\n"
              << "  set_terraforming NAME VALUE Adjust terraforming and rebalance mining.\n"
              << "  dip AMOUNT [once|persistent] Set pending Dip Into Savings.\n"
              << "  abandon NAME                 Queue a colony abandonment.\n"
              << "  cancel_abandon NAME          Cancel a queued abandonment.\n"
              << "  ready                        Submit choices and mark player 1 ready.\n"
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
        std::cout << " owner=" << owner_text(planet.apparent_owner, snapshot.players)
                  << " population=" << planet.apparent_population
                  << " observed=" << planet.observation_year << "\n";
    }
}

void CliApplication::print_economy() const
{
    if (!session_ || !session_->ui())
        return;

    const UiGameSnapshot snapshot = session_->ui()->snapshot(local_player_id_);
    const auto player = std::find_if(
        snapshot.players.begin(), snapshot.players.end(),
        [this](const UiPlayerSnapshot& value) { return value.id == local_player_id_; });
    if (player == snapshot.players.end())
        return;

    std::cout << "Player " << local_player_id_
              << ": money=" << player->money
              << ", money income=" << player->money_income
              << ", effective income=" << player->effective_income
              << ", metal=" << player->metal_reserve
              << ", metal income=" << player->metal_income << "\n";
    std::cout << "  Pending spending: savings=" << pending_submission_.savings_fraction
              << ", research=" << pending_submission_.research_fraction
              << ", planets=" << pending_submission_.planets_fraction
              << ", submitted=" << (submission_dirty_ ? "no" : "yes") << "\n";
    std::cout << "  Pending dip: amount=" << pending_submission_.dip_amount
              << ", mode=" << (pending_submission_.dip_persistent ? "persistent" : "once")
              << "\n";
    for (const EconomicColonyAllocation& colony : pending_submission_.colony_allocations)
    {
        std::cout << "  Planet " << colony.planet_id
                  << ": funding=" << colony.funding_fraction
                  << ", mining=" << colony.mining_fraction
                  << ", terraforming=" << colony.terraforming_fraction << "\n";
    }
    if (!pending_submission_.planets_to_abandon.empty())
    {
        std::cout << "  Abandon queued:";
        for (uint32_t planet_id : pending_submission_.planets_to_abandon)
            std::cout << " " << planet_id;
        std::cout << "\n";
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
    else if (command == "economy")
    {
        print_economy();
    }
    else if (command == "set_spending")
    {
        try
        {
            input >> pending_submission_.savings_fraction
                  >> pending_submission_.research_fraction
                  >> pending_submission_.planets_fraction;
            if (!input)
                throw std::invalid_argument("expected three spending fractions");
            submission_dirty_ = true;
            std::cout << "Pending spending allocation updated.\n";
        }
        catch (const std::exception& error)
        {
            print_error(command, error.what());
        }
    }
    else if (command == "set_savings" || command == "set_research_budget" ||
             command == "set_planets")
    {
        try
        {
            double value = 0.0;
            input >> value;
            if (!input)
                throw std::invalid_argument("expected one fraction");

            std::string adjustment_error;
            bool adjusted = false;
            if (command == "set_savings")
                adjusted = adjust_savings_fraction(value, adjustment_error);
            else if (command == "set_research_budget")
                adjusted = adjust_research_budget_fraction(value, adjustment_error);
            else
                adjusted = set_planets_fraction(value, adjustment_error);
            if (!adjusted)
                throw std::invalid_argument(adjustment_error);

            submission_dirty_ = true;
            std::cout << "Pending spending allocation updated and normalized.\n";
        }
        catch (const std::exception& error)
        {
            print_error(command, error.what());
        }
    }
    else if (command == "set_planet_budget")
    {
        try
        {
            const std::vector<std::string> tokens =
                split_tokens(trim(line.substr(command.size())));
            if (tokens.size() < 2)
                throw std::invalid_argument("expected planet name and one fraction");

            std::string planet_name = tokens.front();
            for (size_t index = 1; index + 1 < tokens.size(); ++index)
                planet_name += " " + tokens[index];

            uint32_t planet_id = 0;
            std::string resolution_error;
            if (!resolve_planet_name(planet_name, planet_id, resolution_error))
                throw std::invalid_argument(resolution_error);

            const double value = std::stod(tokens.back());
            std::string adjustment_error;
            if (!adjust_planet_funding_fraction(planet_id, value, adjustment_error))
                throw std::invalid_argument(adjustment_error);

            submission_dirty_ = true;
            std::cout << "Pending planetary allocation updated and normalized.\n";
        }
        catch (const std::exception& error)
        {
            print_error(command, error.what());
        }
    }
    else if (command == "set_research")
    {
        try
        {
            for (double& fraction : pending_submission_.research_fractions)
                input >> fraction;
            if (!input)
                throw std::invalid_argument("expected six research fractions");
            submission_dirty_ = true;
            std::cout << "Pending research allocation updated.\n";
        }
        catch (const std::exception& error)
        {
            print_error(command, error.what());
        }
    }
    else if (command == "set_research_stream")
    {
        try
        {
            std::string stream_name;
            double value = 0.0;
            input >> stream_name >> value;
            if (!input)
                throw std::invalid_argument("expected stream name and one fraction");

            const size_t stream_index = research_stream_index(stream_name);
            std::string adjustment_error;
            if (!adjust_research_stream_fraction(stream_index, value, adjustment_error))
                throw std::invalid_argument(adjustment_error);

            submission_dirty_ = true;
            std::cout << "Pending research-stream allocation updated and normalized.\n";
        }
        catch (const std::exception& error)
        {
            print_error(command, error.what());
        }
    }
    else if (command == "set_mining" || command == "set_terraforming")
    {
        try
        {
            const std::vector<std::string> tokens =
                split_tokens(trim(line.substr(command.size())));
            if (tokens.size() < 2)
                throw std::invalid_argument("expected planet name and one fraction");

            std::string planet_name = tokens.front();
            for (size_t index = 1; index + 1 < tokens.size(); ++index)
                planet_name += " " + tokens[index];

            uint32_t planet_id = 0;
            std::string resolution_error;
            if (!resolve_planet_name(planet_name, planet_id, resolution_error))
                throw std::invalid_argument(resolution_error);

            const double value = std::stod(tokens.back());
            std::string adjustment_error;
            const bool mining = command == "set_mining";
            if (!adjust_planet_development_fraction(
                    planet_id, mining, value, adjustment_error))
                throw std::invalid_argument(adjustment_error);

            submission_dirty_ = true;
            std::cout << "Pending planet development allocation updated and normalized.\n";
        }
        catch (const std::exception& error)
        {
            print_error(command, error.what());
        }
    }
    else if (command == "set_planet")
    {
        try
        {
            const std::vector<std::string> tokens = split_tokens(trim(line.substr(command.size())));
            if (tokens.size() < 4)
                throw std::invalid_argument("expected planet name and three fractions");

            const size_t fraction_start = tokens.size() - 3;
            std::string planet_name = tokens.front();
            for (size_t index = 1; index < fraction_start; ++index)
                planet_name += " " + tokens[index];

            uint32_t planet_id = 0;
            std::string resolution_error;
            if (!resolve_planet_name(planet_name, planet_id, resolution_error))
                throw std::invalid_argument(resolution_error);

            EconomicColonyAllocation allocation;
            allocation.planet_id = planet_id;
            allocation.funding_fraction = std::stod(tokens[fraction_start]);
            allocation.mining_fraction = std::stod(tokens[fraction_start + 1]);
            allocation.terraforming_fraction = std::stod(tokens[fraction_start + 2]);
            const auto existing = std::find_if(
                pending_submission_.colony_allocations.begin(),
                pending_submission_.colony_allocations.end(),
                [planet_id](const EconomicColonyAllocation& value) {
                    return value.planet_id == planet_id;
                });
            if (existing == pending_submission_.colony_allocations.end())
                throw std::invalid_argument("planet is not one of the local colonies");
            *existing = allocation;
            submission_dirty_ = true;
            std::cout << "Pending allocation for planet " << planet_name << " updated.\n";
        }
        catch (const std::exception& error)
        {
            print_error(command, error.what());
        }
    }
    else if (command == "dip")
    {
        try
        {
            input >> pending_submission_.dip_amount;
            if (!input || pending_submission_.dip_amount < 0)
                throw std::invalid_argument("expected a non-negative dip amount");
            std::string mode;
            input >> mode;
            if (!mode.empty() && mode != "once" && mode != "persistent")
                throw std::invalid_argument("mode must be once or persistent");
            pending_submission_.dip_persistent = mode == "persistent";
            submission_dirty_ = true;
            std::cout << "Pending Dip Into Savings updated.\n";
        }
        catch (const std::exception& error)
        {
            print_error("dip", error.what());
        }
    }
    else if (command == "abandon" || command == "cancel_abandon")
    {
        uint32_t planet_id = 0;
        std::string resolution_error;
        if (!resolve_planet_name(trim(line.substr(command.size())), planet_id, resolution_error))
        {
            print_error(command, resolution_error);
        }
        else if (command == "abandon")
        {
            if (std::find(pending_submission_.planets_to_abandon.begin(),
                          pending_submission_.planets_to_abandon.end(),
                          planet_id) == pending_submission_.planets_to_abandon.end())
            {
                pending_submission_.planets_to_abandon.push_back(planet_id);
            }
            submission_dirty_ = true;
            std::cout << "Pending abandonment for planet " << planet_id << " queued.\n";
        }
        else
        {
            const auto existing = std::remove(
                pending_submission_.planets_to_abandon.begin(),
                pending_submission_.planets_to_abandon.end(), planet_id);
            pending_submission_.planets_to_abandon.erase(
                existing, pending_submission_.planets_to_abandon.end());
            submission_dirty_ = true;
            std::cout << "Pending abandonment for planet " << planet_id << " cancelled.\n";
        }
    }
    else if (command == "ready")
    {
        if (!submit_pending_submission())
            return true;
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
            if (submission_dirty_)
            {
                print_error("process", "call ready to submit pending economic choices first");
                return true;
            }
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
                if (!synchronize_pending_submission())
                    return false;
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
            if (!synchronize_pending_submission())
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
