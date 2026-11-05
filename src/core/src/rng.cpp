#include "rng.h"
#include <algorithm>
#include <array>
#include <limits>
#include <sstream>

namespace
{
constexpr std::array<uint8_t, 4> kRngStateMagic = {'O', 'H', 'R', '1'};
constexpr uint32_t kRngStateVersion = 1;
// Boost's mt19937_64 stream format contains 312 uint64_t values: the
// position/state representation is carried in that fixed sequence.
constexpr uint32_t kMt19937StateWords = 312;

void append_u32(std::vector<uint8_t>& output, uint32_t value)
{
	for (size_t i = 0; i < sizeof(value); ++i)
		output.push_back(static_cast<uint8_t>((value >> (i * 8)) & 0xffu));
}

void append_u64(std::vector<uint8_t>& output, uint64_t value)
{
	for (size_t i = 0; i < sizeof(value); ++i)
		output.push_back(static_cast<uint8_t>((value >> (i * 8)) & 0xffu));
}

bool read_u32(const std::vector<uint8_t>& input, size_t& offset, uint32_t& value)
{
	if (offset > input.size() || input.size() - offset < sizeof(value))
		return false;
	value = 0;
	for (size_t i = 0; i < sizeof(value); ++i)
		value |= static_cast<uint32_t>(input[offset++]) << (i * 8);
	return true;
}

bool read_u64(const std::vector<uint8_t>& input, size_t& offset, uint64_t& value)
{
	if (offset > input.size() || input.size() - offset < sizeof(value))
		return false;
	value = 0;
	for (size_t i = 0; i < sizeof(value); ++i)
		value |= static_cast<uint64_t>(input[offset++]) << (i * 8);
	return true;
}

template <typename Engine>
std::vector<uint8_t> serialize_engine(const Engine& engine)
{
	std::ostringstream stream;
	stream << engine;
	if (!stream)
		return {};

	std::istringstream state_stream(stream.str());
	std::vector<uint64_t> state_words;
	uint64_t word = 0;
	while (state_stream >> word)
		state_words.push_back(word);
	if (!state_stream.eof() || state_words.size() != kMt19937StateWords)
		return {};

	std::vector<uint8_t> output;
	output.insert(output.end(), kRngStateMagic.begin(), kRngStateMagic.end());
	append_u32(output, kRngStateVersion);
	append_u32(output, static_cast<uint32_t>(state_words.size()));
	for (uint64_t state_word : state_words)
		append_u64(output, state_word);
	return output;
}

template <typename Engine>
bool deserialize_engine(const std::vector<uint8_t>& data, Engine& engine)
{
	constexpr size_t header_size = kRngStateMagic.size() + sizeof(uint32_t) * 2;
	if (data.size() < header_size)
		return false;

	for (size_t i = 0; i < kRngStateMagic.size(); ++i)
	{
		if (data[i] != kRngStateMagic[i])
			return false;
	}

	size_t offset = kRngStateMagic.size();
	uint32_t version = 0;
	uint32_t word_count = 0;
	if (!read_u32(data, offset, version) || !read_u32(data, offset, word_count) ||
		version != kRngStateVersion || word_count != kMt19937StateWords ||
		data.size() != header_size + static_cast<size_t>(word_count) * sizeof(uint64_t))
		return false;

	std::ostringstream state_stream;
	for (uint32_t i = 0; i < word_count; ++i)
	{
		uint64_t state_word = 0;
		if (!read_u64(data, offset, state_word))
			return false;
		if (i != 0)
			state_stream << ' ';
		state_stream << state_word;
	}
	std::istringstream input(state_stream.str());
	input >> engine;
	return static_cast<bool>(input) && offset == data.size();
}
}

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
	return serialize_engine(aiEngine);
}

void DeterministicRNG::deserialize_ai_rng_state(const std::vector<uint8_t>& data)
{
	(void)deserialize_engine(data, aiEngine);
}

size_t DeterministicRNG::get_serialized_ai_rng_state_size()
{
	return kRngStateMagic.size() + sizeof(uint32_t) * 2 +
	       static_cast<size_t>(kMt19937StateWords) * sizeof(uint64_t);
}

std::vector<uint8_t> DeterministicRNG::serialize_deterministic_rng_state() const
{
	return serialize_engine(deterministicEngine);
}

void DeterministicRNG::deserialize_deterministic_rng_state(const std::vector<uint8_t>& data)
{
	(void)deserialize_engine(data, deterministicEngine);
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
