#ifndef OPENHO_GALAXY_API_H
#define OPENHO_GALAXY_API_H

#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * C API for OpenHo Galaxy Coordinate Generation
 * 
 * This API provides a stable interface for generating galaxy coordinates
 * from Python or other languages. It wraps the C++ implementation and
 * provides simple C-compatible types.
 */

// Galaxy shape enumeration (matches GalaxyShape enum in enums.h)
typedef enum
{
	GALAXY_SHAPE_RANDOM = 0,
	GALAXY_SHAPE_SPIRAL = 1,
	GALAXY_SHAPE_CIRCLE = 2,
	GALAXY_SHAPE_RING = 3,
	GALAXY_SHAPE_CLUSTER = 4,
	GALAXY_SHAPE_GRID = 5
} GalaxyShapeC;

// Galaxy generation parameters (C-compatible version)
typedef struct
{
	uint32_t n_planets;  // Number of planets to generate (5-500)
	uint32_t n_players;  // Number of players (determines home planet count)
	double density;      // Planet distribution density (0.0-1.0)
	int32_t shape;       // Distribution pattern (use GalaxyShapeC enum)
	uint64_t seed;       // Random seed for generation
} GalaxyParamsC;

/**
 * Generate galaxy coordinates based on parameters.
 * 
 * Returns a flat array of doubles: [x1, y1, x2, y2, x3, y3, ...]
 * The number of coordinate pairs is written to out_count.
 * 
 * The caller is responsible for freeing the returned array using free_galaxy_coords().
 * 
 * @param params Galaxy generation parameters
 * @param out_count Pointer to receive the number of coordinate pairs generated
 * @return Pointer to array of coordinates, or NULL on error
 */
double* generate_galaxy_coords(GalaxyParamsC params, uint32_t* out_count);

/**
 * Free memory allocated by generate_galaxy_coords().
 * 
 * @param coords Pointer returned by generate_galaxy_coords()
 */
void free_galaxy_coords(double* coords);

/**
 * Get the last error message (if any).
 * Returns NULL if no error occurred.
 * The returned string is valid until the next API call.
 */
const char* get_last_error();

#ifdef __cplusplus
}
#endif

#endif // OPENHO_GALAXY_API_H
