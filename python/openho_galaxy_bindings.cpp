#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>
#include "openho_galaxy_api.h"
#include <stdexcept>
#include <string>

namespace py = pybind11;

/**
 * Python wrapper for galaxy coordinate generation.
 * 
 * This module provides a simple interface to generate galaxy coordinates
 * using the OpenHo C++ core library.
 */

// Convert shape string to enum
int32_t shape_string_to_enum(const std::string& shape)
{
	if (shape == "RANDOM") return GALAXY_SHAPE_RANDOM;
	if (shape == "SPIRAL") return GALAXY_SHAPE_SPIRAL;
	if (shape == "CIRCLE") return GALAXY_SHAPE_CIRCLE;
	if (shape == "RING") return GALAXY_SHAPE_RING;
	if (shape == "CLUSTER") return GALAXY_SHAPE_CLUSTER;
	if (shape == "GRID") return GALAXY_SHAPE_GRID;
	
	throw std::invalid_argument("Invalid galaxy shape: " + shape + 
	                           ". Valid options: RANDOM, SPIRAL, CIRCLE, RING, CLUSTER, GRID");
}

// Python-friendly wrapper function
py::array_t<double> generate_coordinates(
	uint32_t n_planets,
	uint32_t n_players,
	double density,
	const std::string& shape,
	uint64_t seed)
{
	// Convert shape string to enum
	int32_t shape_enum = shape_string_to_enum(shape);
	
	// Create C parameters
	GalaxyParamsC params;
	params.n_planets = n_planets;
	params.n_players = n_players;
	params.density = density;
	params.shape = shape_enum;
	params.seed = seed;
	
	// Call C API
	uint32_t count = 0;
	double* coords = generate_galaxy_coords(params, &count);
	
	// Check for errors
	if (coords == nullptr)
	{
		const char* error = get_last_error();
		std::string error_msg = error ? error : "Unknown error";
		throw std::runtime_error("Failed to generate galaxy coordinates: " + error_msg);
	}
	
	// Create NumPy array (shape: [count, 2])
	// We'll copy the data and then free the C array
	py::array_t<double> result({static_cast<py::ssize_t>(count), static_cast<py::ssize_t>(2)});
	auto result_buf = result.mutable_unchecked<2>();
	
	for (size_t i = 0; i < count; ++i)
	{
		result_buf(i, 0) = coords[i * 2 + 0];  // x
		result_buf(i, 1) = coords[i * 2 + 1];  // y
	}
	
	// Free C array
	free_galaxy_coords(coords);
	
	return result;
}

// Python module definition
PYBIND11_MODULE(openho_galaxy, m)
{
	m.doc() = "OpenHo Galaxy Coordinate Generation\n\n"
	          "This module provides functions to generate galaxy coordinates\n"
	          "for the OpenHo game using various distribution patterns.";
	
	m.def("generate_coordinates", &generate_coordinates,
	      py::arg("n_planets"),
	      py::arg("n_players"),
	      py::arg("density"),
	      py::arg("shape"),
	      py::arg("seed"),
	      R"pbdoc(
	      Generate galaxy planet coordinates.
	      
	      Parameters
	      ----------
	      n_planets : int
	          Number of planets to generate (5-500)
	      n_players : int
	          Number of players (determines home planet count)
	      density : float
	          Planet distribution density (0.0-1.0)
	      shape : str
	          Galaxy shape: 'RANDOM', 'SPIRAL', 'CIRCLE', 'RING', 'CLUSTER', or 'GRID'
	      seed : int
	          Random seed for reproducible generation
	      
	      Returns
	      -------
	      numpy.ndarray
	          Array of shape (n, 2) containing (x, y) coordinates
	      
	      Examples
	      --------
	      >>> import openho_galaxy as og
	      >>> coords = og.generate_coordinates(100, 4, 0.5, 'CIRCLE', 42)
	      >>> print(coords.shape)
	      (100, 2)
	      >>> print(coords[:3])
	      [[ 12.34  56.78]
	       [-23.45  67.89]
	       [ 34.56 -12.34]]
	      )pbdoc");
	
	// Add version info
	m.attr("__version__") = "1.0.0";
	
	// Add shape constants for convenience
	m.attr("RANDOM") = "RANDOM";
	m.attr("SPIRAL") = "SPIRAL";
	m.attr("CIRCLE") = "CIRCLE";
	m.attr("RING") = "RING";
	m.attr("CLUSTER") = "CLUSTER";
	m.attr("GRID") = "GRID";
}
