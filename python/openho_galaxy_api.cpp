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
static std::string last_error_message;

extern "C" {

double* generate_galaxy_coords(GalaxyParamsC params, uint32_t* out_count)
{
	// Clear previous error
	last_error_message.clear();
	
	// Validate output pointer
	if (out_count == nullptr)
	{
		last_error_message = "out_count parameter cannot be NULL";
		return nullptr;
	}
	
	// Initialize output
	*out_count = 0;
	
	try
	{
		// Validate parameters
		if (params.n_planets == 0)
		{
			last_error_message = "n_planets must be greater than 0";
			return nullptr;
		}
		
		if (params.n_players == 0)
		{
			last_error_message = "n_players must be greater than 0";
			return nullptr;
		}
		
		if (params.density <= 0.0 || params.density > 1.0)
		{
			last_error_message = "density must be in range (0.0, 1.0]";
			return nullptr;
		}
		
		if (params.shape < GALAXY_SHAPE_RANDOM || params.shape > GALAXY_SHAPE_GRID)
		{
			last_error_message = "Invalid galaxy shape";
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
		
		// Create a minimal RNG for coordinate generation
		// Use the same seed for both deterministic and AI RNG (doesn't matter for coord generation)
		DeterministicRNG rng(params.seed, params.seed);
		
		// Create a minimal temporary GameState-like object
		// We need to create a TextAssets object for the Galaxy constructor
		class MinimalGameState
		{
		public:
			DeterministicRNG* rng_ptr;
			TextAssets* text_assets;
			
			DeterministicRNG& get_rng() { return *rng_ptr; }
		};
		
		MinimalGameState minimal_state;
		minimal_state.rng_ptr = &rng;
		minimal_state.text_assets = new TextAssets();
		
		// Create Galaxy object (temporary, just to access the generation method)
		Galaxy galaxy(cpp_params, reinterpret_cast<GameState*>(&minimal_state));
		
		// Generate coordinates using the Galaxy method
		std::vector<PlanetCoord> coords = galaxy.generate_planet_coordinates(cpp_params, reinterpret_cast<GameState*>(&minimal_state));
		
		// Clean up
		delete minimal_state.text_assets;
		
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
		last_error_message = std::string("Exception: ") + e.what();
		return nullptr;
	}
	catch (...)
	{
		last_error_message = "Unknown error occurred";
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
	if (last_error_message.empty())
	{
		return nullptr;
	}
	return last_error_message.c_str();
}

} // extern "C"
