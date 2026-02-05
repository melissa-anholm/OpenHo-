#include "rng.h"
#include "game_constants.h"
#include "utility.h"
#include <iostream>
#include <iomanip>
#include <cmath>
#include <vector>

typedef std::pair<double, double> PlanetCoord;

int main()
{
	std::cout << "=== Cluster Galaxy Generation Simulation ===" << std::endl;
	
	// Simulate the cluster generation algorithm
	DeterministicRNG rng(54321, 0);
	
	uint32_t n_planets = 150;
	uint32_t n_clusters = 3;
	double density = 0.5;
	
	std::cout << "\nParameters:" << std::endl;
	std::cout << "  Total planets: " << n_planets << std::endl;
	std::cout << "  Number of clusters: " << n_clusters << std::endl;
	std::cout << "  Density: " << density << std::endl;
	
	// Generate random angular offset
	double angular_offset = rng.nextDouble() * 360.0;
	std::cout << "\nGenerated angular offset: " << std::fixed << std::setprecision(2) << angular_offset << "°" << std::endl;
	
	// Calculate cluster parameters
	double gal_size = std::sqrt(double(n_planets)) * 
	                  (GameConstants::Galaxy_Size_Scale_Base + GameConstants::Galaxy_Size_Scale_Density / density);
	
	double cluster_radius = gal_size / (2.0 * std::sqrt(double(n_clusters)));
	double spacing_factor = 1.1 + (1.0 - density) * 0.9;
	double desired_spacing = 2.0 * cluster_radius * spacing_factor;
	double ring_radius = desired_spacing * n_clusters / (2.0 * M_PI);
	
	std::cout << "\nCluster Parameters:" << std::endl;
	std::cout << "  Galaxy size: " << gal_size << std::endl;
	std::cout << "  Cluster radius: " << cluster_radius << std::endl;
	std::cout << "  Ring radius: " << ring_radius << std::endl;
	std::cout << "  Min planet distance: " << GameConstants::min_planet_distance << std::endl;
	
	// Calculate target planets per cluster
	uint32_t planets_per_cluster = n_planets / n_clusters;
	uint32_t remaining_planets = n_planets % n_clusters;
	
	std::cout << "\nTarget Distribution:" << std::endl;
	std::cout << "  Base planets per cluster: " << planets_per_cluster << std::endl;
	std::cout << "  Remaining planets: " << remaining_planets << std::endl;
	
	// Simulate cluster generation
	std::vector<std::vector<PlanetCoord>> clusters(n_clusters);
	CheckDistanceSpatialGrid grid(GameConstants::min_planet_distance, gal_size * 2.0);
	
	for (uint32_t cluster_idx = 0; cluster_idx < n_clusters; ++cluster_idx)
	{
		// Calculate cluster center
		double angle = (2.0 * M_PI * cluster_idx) / n_clusters;
		double cluster_center_x = ring_radius * std::cos(angle);
		double cluster_center_y = ring_radius * std::sin(angle);
		
		// Determine target planets for this cluster
		uint32_t target_planets = planets_per_cluster;
		if (cluster_idx < remaining_planets)
		{
			target_planets++;
		}
		
		// Generate planets in this cluster
		uint32_t planets_placed = 0;
		uint32_t max_attempts = target_planets * 10;
		uint32_t attempts = 0;
		
		while (planets_placed < target_planets && attempts < max_attempts)
		{
			double angle_offset = rng.nextDouble() * 2.0 * M_PI;
			double radius_offset = rng.nextDouble() * cluster_radius;
			
			double x_coord = cluster_center_x + radius_offset * std::cos(angle_offset);
			double y_coord = cluster_center_y + radius_offset * std::sin(angle_offset);
			
			if (grid.is_position_valid(x_coord, y_coord, GameConstants::min_planet_distance))
			{
				clusters[cluster_idx].push_back({x_coord, y_coord});
				grid.add_planet(x_coord, y_coord, static_cast<uint32_t>(clusters[cluster_idx].size()));
				planets_placed++;
			}
			attempts++;
		}
	}
	
	// Report results
	std::cout << "\nGeneration Results:" << std::endl;
	uint32_t total_placed = 0;
	for (uint32_t i = 0; i < n_clusters; ++i)
	{
		uint32_t target = planets_per_cluster;
		if (i < remaining_planets)
		{
			target++;
		}
		
		uint32_t placed = clusters[i].size();
		total_placed += placed;
		
		std::cout << "  Cluster " << (i + 1) << ": " << std::setw(3) << placed 
		          << " planets (target: " << target << ")";
		
		if (placed < target)
		{
			std::cout << " [UNDER by " << (target - placed) << "]";
		}
		else if (placed > target)
		{
			std::cout << " [OVER by " << (placed - target) << "]";
		}
		std::cout << std::endl;
	}
	
	std::cout << "\nSummary:" << std::endl;
	std::cout << "  Total planets placed: " << total_placed << std::endl;
	std::cout << "  Expected: " << n_planets << std::endl;
	if (total_placed < n_planets)
	{
		std::cout << "  MISSING: " << (n_planets - total_placed) << " planets" << std::endl;
	}
	else if (total_placed > n_planets)
	{
		std::cout << "  EXTRA: " << (total_placed - n_planets) << " planets" << std::endl;
	}
	
	std::cout << "\n=== Test Complete ===" << std::endl;
	
	return 0;
}
