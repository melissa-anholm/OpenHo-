#include "rng.h"
#include <algorithm>

// ============================================================================
// DeterministicRNG Implementation
// ============================================================================

DeterministicRNG::DeterministicRNG(uint64_t deterministicSeed, uint64_t aiSeed)
	: deterministicEngine(deterministicSeed),
	  aiEngine(aiSeed),
	  int32Dist(std::numeric_limits<int32_t>::min(), std::numeric_limits<int32_t>::max()),
	  uint32Dist(0, std::numeric_limits<uint32_t>::max()),
	  int64Dist(std::numeric_limits<int64_t>::min(), std::numeric_limits<int64_t>::max()),
	  uint64Dist(0, std::numeric_limits<uint64_t>::max()),
	  doubleDist(0.0, 1.0)
{
}

DeterministicRNG::~DeterministicRNG()
{
}

// ============================================================================
// Deterministic RNG Methods
// ============================================================================

int32_t DeterministicRNG::nextInt32()
{
	return int32Dist(deterministicEngine);
}

uint32_t DeterministicRNG::nextUInt32()
{
	return uint32Dist(deterministicEngine);
}

int64_t DeterministicRNG::nextInt64()
{
	return int64Dist(deterministicEngine);
}

uint64_t DeterministicRNG::nextUInt64()
{
	return uint64Dist(deterministicEngine);
}

double DeterministicRNG::nextDouble()
{
	return doubleDist(deterministicEngine);
}

// ============================================================================
// AI RNG Methods
// ============================================================================

int32_t DeterministicRNG::nextAIInt32()
{
	return int32Dist(aiEngine);
}

uint32_t DeterministicRNG::nextAIUInt32()
{
	return uint32Dist(aiEngine);
}

int64_t DeterministicRNG::nextAIInt64()
{
	return int64Dist(aiEngine);
}

uint64_t DeterministicRNG::nextAIUInt64()
{
	return uint64Dist(aiEngine);
}

double DeterministicRNG::nextAIDouble()
{
	return doubleDist(aiEngine);
}

// ============================================================================
// Seed Management
// ============================================================================

void DeterministicRNG::setDeterministicSeed(uint64_t seed)
{
	deterministicEngine.seed(seed);
}

void DeterministicRNG::setAISeed(uint64_t seed)
{
	aiEngine.seed(seed);
}

uint64_t DeterministicRNG::getAISeed() const
{
	// Note: Boost's mt19937_64 doesn't provide a direct way to get the current seed.
	// This is a limitation of the Boost library. In practice, we track the seed separately
	// in the Galaxy structure or store it when set.
	// For now, return 0 as a placeholder.
	return 0;
}

// ============================================================================
// Bounded Random Numbers
// ============================================================================

int32_t DeterministicRNG::nextInt32Range(int32_t min, int32_t max)
{
	if (min > max)
		std::swap(min, max);
	boost::random::uniform_int_distribution<int32_t> dist(min, max);
	return dist(deterministicEngine);
}

uint32_t DeterministicRNG::nextUInt32Range(uint32_t min, uint32_t max)
{
	if (min > max)
		std::swap(min, max);
	boost::random::uniform_int_distribution<uint32_t> dist(min, max);
	return dist(deterministicEngine);
}

double DeterministicRNG::nextDoubleRange(double min, double max)
{
	if (min > max)
		std::swap(min, max);
	boost::random::uniform_real_distribution<double> dist(min, max);
	return dist(deterministicEngine);
}

// ============================================================================
// AI Bounded Random Numbers
// ============================================================================

int32_t DeterministicRNG::nextAIInt32Range(int32_t min, int32_t max)
{
	if (min > max)
		std::swap(min, max);
	boost::random::uniform_int_distribution<int32_t> dist(min, max);
	return dist(aiEngine);
}

uint32_t DeterministicRNG::nextAIUInt32Range(uint32_t min, uint32_t max)
{
	if (min > max)
		std::swap(min, max);
	boost::random::uniform_int_distribution<uint32_t> dist(min, max);
	return dist(aiEngine);
}

double DeterministicRNG::nextAIDoubleRange(double min, double max)
{
	if (min > max)
		std::swap(min, max);
	boost::random::uniform_real_distribution<double> dist(min, max);
	return dist(aiEngine);
}
