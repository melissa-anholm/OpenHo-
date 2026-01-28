#include "openho_galaxy_api.h"
#include "galaxy.h"
#include "game.h"
#include "enums.h"
#include "text_assets.h"
#include "rng.h"
#include <cstring>
#include <string>
#include <exception>

// Error message storage
// Note: Not thread-safe, but Python GIL provides synchronization
// Using C-style buffer to avoid C++ static initialization issues on macOS 10.14
static char last_error_message[1024] = {0};

extern "C" {

double* generate_galaxy_coords(GalaxyParamsC params, uint32_t* out_count)
{
	// Clear previous error
	last_error_message[0] = '\0';
	
	// Validate output pointer
	if (out_count == nullptr)
	{
		strncpy(last_error_message, "out_count parameter cannot be NULL", sizeof(last_error_message) - 1);
		return nullptr;
	}
	
	// Initialize output
	*out_count = 0;
	
	try
	{
		// Validate parameters
		if (params.n_planets == 0)
		{
			strncpy(last_error_message, "n_planets must be greater than 0", sizeof(last_error_message) - 1);
			return nullptr;
		}
		
		if (params.n_players == 0)
		{
			strncpy(last_error_message, "n_players must be greater than 0", sizeof(last_error_message) - 1);
			return nullptr;
		}
		
		if (params.density <= 0.0 || params.density > 1.0)
		{
			strncpy(last_error_message, "density must be in range (0.0, 1.0]", sizeof(last_error_message) - 1);
			return nullptr;
		}
		
		if (params.shape < GALAXY_SHAPE_RANDOM || params.shape > GALAXY_SHAPE_GRID)
		{
			strncpy(last_error_message, "Invalid galaxy shape", sizeof(last_error_message) - 1);
			return nullptr;
		}
		
		// Convert C parameters to C++ parameters
		GalaxyShape cpp_shape = static_cast<GalaxyShape>(params.shape);
		GalaxyGenerationParams cpp_params(
			params.n_planets,
			params.n_players,
			params.density,
			cpp_shape,
			params.seed
		);
		
		// Create RNG directly (no GameState needed!)
		DeterministicRNG rng(params.seed, params.seed);
		
		// Call the actual C++ coordinate generation methods with RNG overloads
		std::vector<PlanetCoord> coords;
		
		switch (cpp_shape)
		{
			case GALAXY_RANDOM:
				coords = Galaxy::generate_coordinates_random(cpp_params, rng);
				break;
			case GALAXY_SPIRAL:
				coords = Galaxy::generate_coordinates_spiral(cpp_params, rng);
				break;
			case GALAXY_CIRCLE:
				coords = Galaxy::generate_coordinates_circle(cpp_params, rng);
				break;
			case GALAXY_RING:
				coords = Galaxy::generate_coordinates_ring(cpp_params, rng);
				break;
			case GALAXY_CLUSTER:
				coords = Galaxy::generate_coordinates_cluster(cpp_params, rng);
				break;
			case GALAXY_GRID:
				coords = Galaxy::generate_coordinates_grid(cpp_params, rng);
				break;
			default:
				strncpy(last_error_message, "Unknown galaxy shape", sizeof(last_error_message) - 1);
				return nullptr;
		}
		
		// Check if generation succeeded
		if (coords.empty() && params.n_planets > 0)
		{
			strncpy(last_error_message, "Galaxy coordinate generation failed (returned empty)", sizeof(last_error_message) - 1);
			return nullptr;
		}
		
		// Allocate C array for output
		size_t coord_count = coords.size();
		double* result = new double[coord_count * 2];
		
		// Copy coordinates to flat array
		for (size_t i = 0; i < coord_count; ++i)
		{
			result[i * 2 + 0] = coords[i].first;   // x
			result[i * 2 + 1] = coords[i].second;  // y
		}
		
		*out_count = static_cast<uint32_t>(coord_count);
		return result;
	}
	catch (const std::exception& e)
	{
		snprintf(last_error_message, sizeof(last_error_message), "Exception: %s", e.what());
		return nullptr;
	}
	catch (...)
	{
		strncpy(last_error_message, "Unknown error occurred", sizeof(last_error_message) - 1);
		return nullptr;
	}
}

void free_galaxy_coords(double* coords)
{
	if (coords != nullptr)
	{
		delete[] coords;
	}
}

const char* get_last_error()
{
	if (last_error_message[0] == '\0')
	{
		return nullptr;
	}
	return last_error_message;
}

} // extern "C"
