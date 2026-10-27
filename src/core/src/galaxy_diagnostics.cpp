#include "galaxy.h"
#include <fstream>
#include <iostream>

void log_galaxy_generation_failure(
	const GalaxyGenerationParams& params,
	const std::vector<PlanetCoord>& generated_coords,
	const std::string& log_filename)
{
	std::ofstream log_file(log_filename);
	if (!log_file.is_open())
	{
		std::cerr << "ERROR: Could not open log file: " << log_filename << std::endl;
		return;
	}

	// Write header
	log_file << "===============================================================================\n";
	log_file << "GALAXY GENERATION FAILURE REPORT\n";
	log_file << "===============================================================================\n\n";

	// Write summary
	log_file << "SUMMARY\n";
	log_file << "-------\n";
	log_file << "Requested planets: " << params.n_planets << "\n";
	log_file << "Generated planets: " << generated_coords.size() << "\n";
	log_file << "Shortfall: " << (params.n_planets - generated_coords.size()) << " planets\n";
	log_file << "Success rate: " << (100.0 * generated_coords.size() / params.n_planets) << "%\n\n";

	// Write parameters
	log_file << "GALAXY GENERATION PARAMETERS\n";
	log_file << "----------------------------\n";
	log_file << "Number of planets requested: " << params.n_planets << "\n";
	log_file << "Number of players: " << params.n_players << "\n";
	log_file << "Density parameter: " << params.density << "\n";

	// Convert shape enum to string
	std::string shape_name;
	switch (params.shape)
	{
		case GALAXY_RANDOM: shape_name = "RANDOM"; break;
		case GALAXY_SPIRAL: shape_name = "SPIRAL"; break;
		case GALAXY_CIRCLE: shape_name = "CIRCLE"; break;
		case GALAXY_RING: shape_name = "RING"; break;
		case GALAXY_CLUSTER: shape_name = "CLUSTER"; break;
		case GALAXY_GRID: shape_name = "GRID"; break;
		default: shape_name = "UNKNOWN"; break;
	}
	log_file << "Galaxy shape: " << shape_name << "\n";
	log_file << "Random seed: " << params.seed << "\n\n";

	// Write generated coordinates
	log_file << "GENERATED PLANET COORDINATES\n";
	log_file << "----------------------------\n";
	log_file << "Total generated: " << generated_coords.size() << "\n\n";

	for (size_t i = 0; i < generated_coords.size(); ++i)
	{
		log_file << "Planet " << (i + 1) << ": ("
		         << generated_coords[i].first << ", "
		         << generated_coords[i].second << ")\n";
	}

	log_file << "\n===============================================================================\n";
	log_file << "END OF REPORT\n";
	log_file << "===============================================================================\n";

	log_file.close();

	std::cerr << "Galaxy generation failure logged to: " << log_filename << std::endl;
}
