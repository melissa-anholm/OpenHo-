#ifndef OPENHO_UTILITY_H
#define OPENHO_UTILITY_H

#include <vector>
#include <string>
#include <cstdint>
#include <algorithm>

// Forward declaration
class DeterministicRNG;

// ============================================================================
// Randomized Subset Generation
// ============================================================================

/**
 * Generate a randomized subset of N items from a source list.
 * 
 * If n_items <= source_list.size():
 *   Returns n_items unique items in random order (no repeats)
 * 
 * If n_items > source_list.size():
 *   Cycles through the list multiple times with new random order each cycle.
 *   Appends a suffix to items in subsequent cycles: " 2", " 3", etc.
 *   Example: If source has ["Alpha", "Beta"] and n_items=5:
 *     Result: ["Beta", "Alpha 2", "Beta 2", "Alpha 3", "Beta 3"]
 * 
 * @tparam T Type of items in the list (typically std::string)
 * @param source_list The list to select from
 * @param n_items Number of items to generate
 * @param rng Deterministic RNG for reproducible randomization
 * @return Vector of n_items in randomized order
 */
template<typename T>
std::vector<T> generate_randomized_subset(
	const std::vector<T>& source_list,
	uint32_t n_items,
	class DeterministicRNG& rng);

// ============================================================================
// Template Implementation
// ============================================================================

template<typename T>
std::vector<T> generate_randomized_subset(
	const std::vector<T>& source_list,
	uint32_t n_items,
	class DeterministicRNG& rng)
{
	std::vector<T> result;
	
	if (source_list.empty() || n_items == 0)
		{ return result; }
	
	uint32_t source_size = static_cast<uint32_t>(source_list.size());
	uint32_t cycle = 1;  // Current cycle (1 for first pass, 2 for second, etc.)
	uint32_t items_added = 0;
	
	while (items_added < n_items)
	{
		// Create a shuffled copy of indices for this cycle
		std::vector<uint32_t> indices;
		for (uint32_t i = 0; i < source_size; ++i)
			{ indices.push_back(i); }
		
		// Fisher-Yates shuffle using deterministic RNG
		for (uint32_t i = source_size - 1; i > 0; --i)
		{
			uint32_t j = rng.nextUInt32Range(0, i);
			std::swap(indices[i], indices[j]);
		}
		
		// Add items from this cycle
		for (uint32_t idx : indices)
		{
			if (items_added >= n_items)
				{ break; }
			
			T item = source_list[idx];
			
			// Append cycle suffix if not the first cycle
			if (cycle > 1)
			{
				// For std::string, append the suffix
				if constexpr (std::is_same_v<T, std::string>)
				{
					item += " " + std::to_string(cycle);
				}
				// For other types, this template specialization would need custom handling
				// For now, we primarily support std::string
			}
			
			result.push_back(item);
			items_added++;
		}
		
		cycle++;
	}
	
	return result;
}

#endif  // OPENHO_UTILITY_H
