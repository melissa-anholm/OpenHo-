#include "cli_application.h"

#include <cstdlib>
#include <iostream>
#include <string>

namespace
{
std::string resolve_assets_directory(int argc, char** argv)
{
    std::string assets_directory;
    if (const char* environment = std::getenv("OPENHO_ASSETS_DIR"))
        assets_directory = environment;

    for (int index = 1; index < argc; ++index)
    {
        const std::string argument = argv[index];
        if (argument == "--assets" && index + 1 < argc)
        {
            assets_directory = argv[++index];
        }
        else if (argument == "--help" || argument == "-h")
        {
            std::cout << "Usage: openho_cli [--assets PATH]\n"
                      << "  --assets PATH  Directory containing the TextAssets JSON files.\n"
                      << "  OPENHO_ASSETS_DIR may also provide the asset directory.\n";
            std::exit(0);
        }
        else
        {
            std::cerr << "Unknown option: " << argument << "\n"
                      << "Use --help for usage.\n";
            std::exit(2);
        }
    }

    return assets_directory.empty() ? "TextAssets" : assets_directory;
}
}

int main(int argc, char** argv)
{
    CliApplication application(resolve_assets_directory(argc, argv));
    return application.run();
}
