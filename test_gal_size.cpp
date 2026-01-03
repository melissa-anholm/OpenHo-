#include <iostream>
#include <cmath>
#include <iomanip>

// Test the corrected galaxy size formula
// Formula: gal_size = sqrt(n_planets) * (5.0 + 6.4 / density)

int main()
{
	const double Galaxy_Size_Scale_Base = 5.0;
	const double Galaxy_Size_Scale_Density = 6.4;
	const double min_planet_distance = 4.0;
	
	std::cout << "Galaxy Size Formula Validation (Corrected)\n";
	std::cout << "==========================================\n\n";
	std::cout << "Formula: gal_size = sqrt(n_planets) * (5.0 + 6.4 / density)\n";
	std::cout << "Minimum planet distance: " << min_planet_distance << " units\n\n";
	
	// Test cases
	struct TestCase {
		uint32_t n_planets;
		double density;
		double expected_neighbors_radius;
		int expected_neighbor_count;
		const char* description;
	};
	
	TestCase tests[] = {
		{100, 2.0, 6.0, 4, "High density: 100 planets, density=2.0"},
		{100, 0.5, 12.0, 3, "Low density: 100 planets, density=0.5"},
		{200, 2.0, 6.0, 4, "High density: 200 planets, density=2.0"},
		{200, 0.5, 12.0, 3, "Low density: 200 planets, density=0.5"},
		{50, 2.0, 6.0, 4, "High density: 50 planets, density=2.0"},
		{50, 0.5, 12.0, 3, "Low density: 50 planets, density=0.5"},
		{500, 2.0, 6.0, 4, "High density: 500 planets, density=2.0"},
		{500, 0.5, 12.0, 3, "Low density: 500 planets, density=0.5"},
	};
	
	std::cout << std::fixed << std::setprecision(2);
	
	for (const auto& test : tests)
	{
		// Calculate galaxy size using corrected formula
		double scale_factor = Galaxy_Size_Scale_Base + Galaxy_Size_Scale_Density / test.density;
		double gal_size = std::sqrt(double(test.n_planets)) * scale_factor;
		
		// Calculate area
		double area = gal_size * gal_size;
		
		// Expected number of planets in a circle of given radius
		double circle_area = M_PI * test.expected_neighbors_radius * test.expected_neighbors_radius;
		double expected_neighbors = (test.n_planets / area) * circle_area;
		
		// Average nearest neighbor distance
		double area_per_planet = area / test.n_planets;
		double avg_nearest_neighbor = std::sqrt(area_per_planet / M_PI);
		
		std::cout << test.description << "\n";
		std::cout << "  Scale factor: " << scale_factor << "\n";
		std::cout << "  Galaxy size: " << gal_size << " x " << gal_size << " units\n";
		std::cout << "  Total area: " << area << " square units\n";
		std::cout << "  Area per planet: " << area_per_planet << " square units\n";
		std::cout << "  Average nearest neighbor distance: " << avg_nearest_neighbor << " units\n";
		std::cout << "  Expected neighbors within radius " << test.expected_neighbors_radius 
		          << ": " << expected_neighbors << " (target: " << test.expected_neighbor_count << ")\n";
		
		// Validation
		bool valid = true;
		if (test.density == 2.0)
		{
			// Should have 4-5 neighbors within radius 6
			if (expected_neighbors < 3.5 || expected_neighbors > 5.5)
				valid = false;
		}
		else if (test.density == 0.5)
		{
			// Should have 3-4 neighbors within radius 12
			if (expected_neighbors < 2.5 || expected_neighbors > 4.5)
				valid = false;
			// Average nearest neighbor should be around 6
			if (avg_nearest_neighbor < 5.0 || avg_nearest_neighbor > 7.0)
				valid = false;
		}
		
		std::cout << "  Status: " << (valid ? "✓ PASS" : "✗ FAIL") << "\n\n";
	}
	
	std::cout << "\n==========================================\n";
	std::cout << "Summary:\n";
	std::cout << "- At density=2.0: gal_size = sqrt(n) * 8.2 (compact)\n";
	std::cout << "- At density=0.5: gal_size = sqrt(n) * 17.8 (sparse)\n";
	std::cout << "- Ratio: 17.8 / 8.2 ≈ 2.17 (relates to density ratio 4.0)\n";
	std::cout << "- Formula balances placement success with galaxy sparsity\n";
	
	return 0;
}
