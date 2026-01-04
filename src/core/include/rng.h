#ifndef OPENHO_RNG_H
#define OPENHO_RNG_H

#include <cstdint>
#include <vector>
#include <sstream>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/random/uniform_real_distribution.hpp>
#include <boost/random/normal_distribution.hpp>

// ============================================================================
// DeterministicRNG Class
// ============================================================================

/**
 * Provides deterministic random number generation using Boost.Random.
 * 
 * The class maintains two separate RNG engines:
 * 1. deterministicRNG: For shared game mechanics (deterministic across all players)
 * 2. aiRNG: For AI decisions (seeded each turn by the host)
 * 
 * This ensures:
 * - Multiplayer games are deterministic (same sequence of events on all clients)
 * - AI decisions can vary without affecting core game mechanics
 * - Host migration is possible (AI seed can be communicated to new host)
 */
class DeterministicRNG
{
public:
	// Constructor
	DeterministicRNG(uint64_t deterministicSeed, uint64_t aiSeed);
	
	// Destructor
	~DeterministicRNG();
	
	// Deterministic RNG methods (for shared game mechanics)
	int32_t nextInt32();
	uint32_t nextUInt32();
	int64_t nextInt64();
	uint64_t nextUInt64();
	double nextDouble();  // Range [0.0, 1.0)
	
	// AI RNG methods (for AI decisions)
	int32_t nextAIInt32();
	uint32_t nextAIUInt32();
	int64_t nextAIInt64();
	uint64_t nextAIUInt64();
	double nextAIDouble();  // Range [0.0, 1.0)
	
	// Seed management
	void setDeterministicSeed(uint64_t seed);
	void setAISeed(uint64_t seed);
	uint64_t getAISeed() const;
	
	// Bounded random numbers
	int32_t nextInt32Range(int32_t min, int32_t max);  // [min, max]
	uint32_t nextUInt32Range(uint32_t min, uint32_t max);  // [min, max]
	double nextDoubleRange(double min, double max);  // [min, max]
	
	// AI bounded random numbers
	int32_t nextAIInt32Range(int32_t min, int32_t max);  // [min, max]
	uint32_t nextAIUInt32Range(uint32_t min, uint32_t max);  // [min, max]
	double nextAIDoubleRange(double min, double max);  // [min, max]
	
	// Normal distribution (Gaussian)
	/// Generate a random number from a normal distribution
	/// @param mean The mean of the distribution
	/// @param sigma The standard deviation of the distribution
	/// @return A random value from N(mean, sigma)
	double nextNormal(double mean, double sigma);
	
	/// Generate a random number from a truncated normal distribution
	/// Values outside [min, max] are rejected and resampled
	/// @param mean The mean of the distribution
	/// @param sigma The standard deviation of the distribution
	/// @param min The minimum allowed value (inclusive)
	/// @param max The maximum allowed value (inclusive)
	/// @return A random value from N(mean, sigma) truncated to [min, max]
	double nextNormalTruncated(double mean, double sigma, double min, double max);
	
	// RNG state serialization for multiplayer host migration
	/// Serialize the AI RNG state to a byte vector for network transmission
	[[nodiscard]] std::vector<uint8_t> serialize_ai_rng_state() const;
	
	/// Deserialize the AI RNG state from a byte vector received from network
	void deserialize_ai_rng_state(const std::vector<uint8_t>& data);
	
	/// Get the size of serialized AI RNG state (for network planning)
	[[nodiscard]] static size_t get_serialized_ai_rng_state_size();
	
	/// Serialize the deterministic RNG state to a byte vector
	[[nodiscard]] std::vector<uint8_t> serialize_deterministic_rng_state() const;
	
	/// Deserialize the deterministic RNG state from a byte vector
	void deserialize_deterministic_rng_state(const std::vector<uint8_t>& data);
	
private:
	// Boost.Random engines (Mersenne Twister)
	boost::random::mt19937_64 deterministicEngine;
	boost::random::mt19937_64 aiEngine;
	
	// Distributions
	boost::random::uniform_int_distribution<int32_t> int32Dist;
	boost::random::uniform_int_distribution<uint32_t> uint32Dist;
	boost::random::uniform_int_distribution<int64_t> int64Dist;
	boost::random::uniform_int_distribution<uint64_t> uint64Dist;
	boost::random::uniform_real_distribution<double> doubleDist;
};

#endif // OPENHO_RNG_H
