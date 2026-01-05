#include "rng.h"
#include <algorithm>

// ============================================================================
// DeterministicRNG Implementation
// ============================================================================

DeterministicRNG::DeterministicRNG(uint64_t det_seed, uint64_t ai_seed)
	: deterministicEngine(det_seed),
	  aiEngine(ai_seed),
	  det_seed_value(det_seed),
	  ai_seed_value(ai_seed),
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
	det_seed_value = seed;
	deterministicEngine.seed(seed);
}

void DeterministicRNG::setAISeed(uint64_t seed)
{
	ai_seed_value = seed;
	aiEngine.seed(seed);
}

uint64_t DeterministicRNG::getDeterministicSeed() const
{
	return det_seed_value;
}

uint64_t DeterministicRNG::getAISeed() const
{
	return ai_seed_value;
}

// ============================================================================
// Bounded Random Numbers
// ============================================================================

int32_t DeterministicRNG::nextInt32Range(int32_t min, int32_t max)
{
	if (min > max)
	{
		std::swap(min, max);
	}
	boost::random::uniform_int_distribution<int32_t> dist(min, max);
	return dist(deterministicEngine);
}

uint32_t DeterministicRNG::nextUInt32Range(uint32_t min, uint32_t max)
{
	if (min > max)
	{
		std::swap(min, max);
	}
	boost::random::uniform_int_distribution<uint32_t> dist(min, max);
	return dist(deterministicEngine);
}

double DeterministicRNG::nextDoubleRange(double min, double max)
{
	if (min > max)
	{
		std::swap(min, max);
	}
	boost::random::uniform_real_distribution<double> dist(min, max);
	return dist(deterministicEngine);
}

// ============================================================================
// AI Bounded Random Numbers
// ============================================================================
int32_t DeterministicRNG::nextAIInt32Range(int32_t min, int32_t max)
{
	if (min > max)
	{
		std::swap(min, max);
	}
	boost::random::uniform_int_distribution<int32_t> dist(min, max);
	return dist(aiEngine);
}
uint32_t DeterministicRNG::nextAIUInt32Range(uint32_t min, uint32_t max)
{
	if (min > max)
	{
		std::swap(min, max);
	}
	boost::random::uniform_int_distribution<uint32_t> dist(min, max);
	return dist(aiEngine);
}

double DeterministicRNG::nextAIDoubleRange(double min, double max)
{
	if (min > max)
	{
		std::swap(min, max);
	}
	boost::random::uniform_real_distribution<double> dist(min, max);
	return dist(aiEngine);
}

// ============================================================================
// RNG State Serialization
// ============================================================================
std::vector<uint8_t> DeterministicRNG::serialize_ai_rng_state() const
{
	std::ostringstream oss;
	oss << aiEngine;
	const std::string& str = oss.str();
	return std::vector<uint8_t>(str.begin(), str.end());
}

void DeterministicRNG::deserialize_ai_rng_state(const std::vector<uint8_t>& data)
{
	std::string str(data.begin(), data.end());
	std::istringstream iss(str);
	iss >> aiEngine;
}

size_t DeterministicRNG::get_serialized_ai_rng_state_size()
{
	// MT19937-64 has 312 uint64_t values + 1 position index
	// Text serialization is roughly 20-30 bytes per uint64_t
	// Approximate size: ~10-15 KB for text serialization
	// This is a rough estimate; actual size depends on the serialization format
	return 15000;  // Conservative estimate
}

std::vector<uint8_t> DeterministicRNG::serialize_deterministic_rng_state() const
{
	std::ostringstream oss;
	oss << deterministicEngine;
	const std::string& str = oss.str();
	return std::vector<uint8_t>(str.begin(), str.end());
}

void DeterministicRNG::deserialize_deterministic_rng_state(const std::vector<uint8_t>& data)
{
	std::string str(data.begin(), data.end());
	std::istringstream iss(str);
	iss >> deterministicEngine;
}


// ============================================================================
// Normal Distribution Methods
// ============================================================================

double DeterministicRNG::nextNormal(double mean, double sigma)
{
	boost::random::normal_distribution<double> normalDist(mean, sigma);
	return normalDist(deterministicEngine);
}

double DeterministicRNG::nextNormalTruncated(double mean, double sigma, double min, double max)
{
	// Rejection sampling: generate from normal distribution until value is in [min, max]
	boost::random::normal_distribution<double> normalDist(mean, sigma);
	double value;
	do {
		value = normalDist(deterministicEngine);
	} while (value < min || value > max);
	return value;
}
