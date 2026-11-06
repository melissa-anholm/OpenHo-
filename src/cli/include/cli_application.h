#ifndef OPENHO_CLI_APPLICATION_H
#define OPENHO_CLI_APPLICATION_H

#include "ui_adapter.h"
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class GameSession;

/// Line-oriented, platform-neutral command-line frontend for OpenHo.
class CliApplication
{
public:
    explicit CliApplication(std::string assets_directory = "TextAssets",
                            uint32_t player_count = 2,
                            uint32_t planet_count = 10,
                            uint32_t human_count = 1);
    ~CliApplication();

    CliApplication(const CliApplication&) = delete;
    CliApplication& operator=(const CliApplication&) = delete;

    /// Run the command loop until quit or end-of-input.
    int run();

private:
    bool handle_command(const std::string& line);
    bool initialize_new_session();
    bool mark_non_local_players_ready();
    bool switch_local_player(const std::string& player_name, std::string& error);
    bool save_pending_submission_for_active_player();
    bool synchronize_pending_submission();
    bool submit_pending_submission();
    bool adjust_savings_fraction(double value, std::string& error);
    bool adjust_research_budget_fraction(double value, std::string& error);
    bool adjust_planet_funding_fraction(const std::string& planet_name,
                                        double value,
                                        std::string& error);
    bool adjust_planet_development_fraction(const std::string& planet_name,
                                            bool mining,
                                            double value,
                                            std::string& error);
    bool set_planets_fraction(double value, std::string& error);
    bool adjust_research_stream_fraction(size_t stream_index,
                                         double value,
                                         std::string& error);
    void print_help() const;
    void print_status() const;
    void print_players() const;
    void print_planets() const;
    void print_economy() const;
    void print_research_allocations() const;
    void print_error(const std::string& operation, const std::string& detail) const;

    std::unique_ptr<GameSession> session_;
    uint32_t local_player_id_ = 0;
    uint32_t player_count_ = 2;
    uint32_t planet_count_ = 10;
    uint32_t human_count_ = 1;
    std::string default_save_path_ = "openho-cli.save";
    std::string assets_directory_;
    UiEconomicSubmission pending_submission_;
    std::unordered_map<uint32_t, UiEconomicSubmission> pending_submissions_;
    bool submission_dirty_ = false;
};

#endif // OPENHO_CLI_APPLICATION_H
