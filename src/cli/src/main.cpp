#include "cli_application.h"

#include <cstdlib>
#include <iostream>
#include <string>

namespace
{
struct CliOptions
{
    std::string assets_directory;
    uint32_t player_count = 2;
    uint32_t planet_count = 10;
    uint32_t human_count = 1;
};

uint32_t parse_count(const std::string& option, const std::string& value)
{
    try
    {
        size_t consumed = 0;
        const unsigned long parsed = std::stoul(value, &consumed);
        if (consumed != value.size() || parsed > 100000)
            throw std::invalid_argument("out of range");
        return static_cast<uint32_t>(parsed);
    }
    catch (const std::exception&)
    {
        std::cerr << "Invalid value for " << option << ": " << value << "\n";
        std::exit(2);
    }
}

CliOptions parse_options(int argc, char** argv)
{
    CliOptions options;
    if (const char* environment = std::getenv("OPENHO_ASSETS_DIR"))
        options.assets_directory = environment;

    for (int index = 1; index < argc; ++index)
    {
        const std::string argument = argv[index];
        auto require_value = [&](const std::string& option) {
            if (index + 1 >= argc)
            {
                std::cerr << "Missing value for " << option << "\n";
                std::exit(2);
            }
            return std::string(argv[++index]);
        };
        if (argument == "--assets")
            options.assets_directory = require_value(argument);
        else if (argument == "--players")
            options.player_count = parse_count(argument, require_value(argument));
        else if (argument == "--planets")
            options.planet_count = parse_count(argument, require_value(argument));
        else if (argument == "--humans")
            options.human_count = parse_count(argument, require_value(argument));
        else if (argument == "--help" || argument == "-h")
        {
            std::cout << "Usage: openho_cli [--assets PATH] [--players N] "
                         "[--planets N] [--humans N]\n"
                      << "  --assets PATH  Directory containing TextAssets JSON files.\n"
                      << "  --players N    Total players, from 1 through 8.\n"
                      << "  --planets N    Total planets, at least the player count.\n"
                      << "  --humans N     Human-controlled players; remaining players are AI.\n"
                      << "  Use play_as NAME to switch among human players.\n";
            std::exit(0);
        }
        else
        {
            std::cerr << "Unknown option: " << argument << "\n"
                      << "Use --help for usage.\n";
            std::exit(2);
        }
    }

    if (options.assets_directory.empty())
        options.assets_directory = "TextAssets";
    return options;
}
}

int main(int argc, char** argv)
{
    const CliOptions options = parse_options(argc, argv);
    CliApplication application(
        options.assets_directory,
        options.player_count,
        options.planet_count,
        options.human_count);
    return application.run();
}
