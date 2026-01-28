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

// Helper function to generate coordinates for a specific shape
// This avoids needing to create a full Galaxy object
static std::vector<PlanetCoord> generate_coords_for_shape(
	const GalaxyGenerationParams& params,
	DeterministicRNG& rng)
{
	std::vector<PlanetCoord> coords;
	
	switch (params.shape)
	{
		case GALAXY_RANDOM:
		{
			// Random distribution within a circle
			double max_radius = 100.0;
			for (uint32_t i = 0; i < params.n_planets; ++i)
			{
				// Use rejection sampling to get uniform distribution in circle
				double x, y;
				do {
					x = rng.nextDoubleRange(-max_radius, max_radius);
					y = rng.nextDoubleRange(-max_radius, max_radius);
				} while (x*x + y*y > max_radius*max_radius);
				
				coords.push_back({x, y});
			}
			break;
		}
		
		case GALAXY_CIRCLE:
		{
			// Uniform distribution within a circle
			double max_radius = 100.0;
			for (uint32_t i = 0; i < params.n_planets; ++i)
			{
				// Use rejection sampling
				double x, y;
				do {
					x = rng.nextDoubleRange(-max_radius, max_radius);
					y = rng.nextDoubleRange(-max_radius, max_radius);
				} while (x*x + y*y > max_radius*max_radius);
				
				coords.push_back({x, y});
			}
			break;
		}
		
		case GALAXY_RING:
		{
			// Ring/annulus distribution
			double outer_radius = 100.0;
			double inner_radius = 60.0;
			for (uint32_t i = 0; i < params.n_planets; ++i)
			{
				// Use rejection sampling for ring
				double x, y;
				do {
					x = rng.nextDoubleRange(-outer_radius, outer_radius);
					y = rng.nextDoubleRange(-outer_radius, outer_radius);
					double r_sq = x*x + y*y;
					if (r_sq >= inner_radius*inner_radius && r_sq <= outer_radius*outer_radius)
						break;
				} while (true);
				
				coords.push_back({x, y});
			}
			break;
		}
		
		case GALAXY_GRID:
		{
			// Regular grid pattern
			uint32_t grid_size = static_cast<uint32_t>(std::ceil(std::sqrt(params.n_planets)));
			double spacing = 20.0;
			double offset = -(grid_size - 1) * spacing / 2.0;
			
			for (uint32_t i = 0; i < params.n_planets; ++i)
			{
				uint32_t row = i / grid_size;
				uint32_t col = i % grid_size;
				double x = offset + col * spacing;
				double y = offset + row * spacing;
				coords.push_back({x, y});
			}
			break;
		}
		
		case GALAXY_SPIRAL:
		case GALAXY_CLUSTER:
		default:
			// For unimplemented shapes, return empty vector
			// The full implementation in galaxy.cpp handles these
			break;
	}
	
	return coords;
}

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
		
		// Create RNG
		DeterministicRNG rng(params.seed, params.seed);
		
		// Generate coordinates directly without creating Galaxy object
		std::vector<PlanetCoord> coords = generate_coords_for_shape(cpp_params, rng);
		
		// Check if shape is implemented
		if (coords.empty() && params.n_planets > 0)
		{
			last_error_message = "Galaxy shape not yet implemented in Python wrapper";
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
