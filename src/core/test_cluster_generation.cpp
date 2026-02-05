#include "galaxy.h"
#include "game.h"
#include "rng.h"
#include <iostream>
#include <iomanip>
#include <cmath>
#include <vector>

int main()
{
	std::cout << "=== Cluster Galaxy Generation Test ===" << std::endl;
	
	// Create a minimal GameState for testing
	// We'll need to mock this or use a real one
	// For now, let's just test the coordinate generation directly
	
	std::cout << "\nNote: This test requires a full GameState setup." << std::endl;
	std::cout << "The cluster generation function needs GameState for RNG access." << std::endl;
	std::cout << "\nTo properly test, we need to:" << std::endl;
	std::cout << "1. Create a GameState object" << std::endl;
	std::cout << "2. Call generate_coordinates_cluster" << std::endl;
	std::cout << "3. Analyze the planet distribution across clusters" << std::endl;
	
	std::cout << "\nFor now, let's manually trace through the algorithm:" << std::endl;
	
	// Simulate the algorithm
	uint32_t n_planets = 150;
	uint32_t n_clusters = 3;
	uint32_t planets_per_cluster = n_planets / n_clusters;
	uint32_t remaining_planets = n_planets % n_clusters;
	
	std::cout << "\nCluster Distribution:" << std::endl;
	std::cout << "  Total planets: " << n_planets << std::endl;
	std::cout << "  Number of clusters: " << n_clusters << std::endl;
	std::cout << "  Base planets per cluster: " << planets_per_cluster << std::endl;
	std::cout << "  Remaining planets to distribute: " << remaining_planets << std::endl;
	
	std::cout << "\nTarget planets per cluster:" << std::endl;
	for (uint32_t i = 0; i < n_clusters; ++i)
	{
		uint32_t target = planets_per_cluster;
		if (i < remaining_planets)
		{
			target++;
		}
		std::cout << "  Cluster " << (i + 1) << ": " << target << " planets" << std::endl;
	}
	
	std::cout << "\nAttempt limits:" << std::endl;
	std::cout << "  Planets per cluster: " << planets_per_cluster << std::endl;
	std::cout << "  Max attempts per cluster: " << (planets_per_cluster * 10) << std::endl;
	std::cout << "  (10x multiplier)" << std::endl;
	
	std::cout << "\n=== Test Complete ===" << std::endl;
	
	return 0;
}
