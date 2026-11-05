#include "save_file.h"

#include <array>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <limits>
#include <string>
#include <type_traits>
#include <cstring>
#include <unordered_set>

namespace
{
constexpr std::array<char, 8> kFileMagic = {'O', 'H', 'O', 'S', 'A', 'V', 'E', '1'};
constexpr std::array<char, 4> kRecordMagic = {'R', 'C', 'D', '1'};
constexpr std::array<char, 8> kDefinitionMagic = {'O', 'H', 'O', 'D', 'E', 'F', '0', '1'};
constexpr std::array<char, 8> kRuntimeMagic = {'O', 'H', 'O', 'R', 'T', 'C', 'P', '1'};
constexpr uint32_t kFormatVersion = 1;
constexpr uint64_t kMaxSectionSize = 256ULL * 1024ULL * 1024ULL;

uint32_t checksum(const std::vector<uint8_t>& bytes)
{
    uint32_t value = 2166136261u;
    for (uint8_t byte : bytes)
    {
        value ^= byte;
        value *= 16777619u;
    }
    return value;
}

template <typename T>
void append_integer(std::vector<uint8_t>& output, T value)
{
    static_assert(std::is_unsigned<T>::value, "save integers must be unsigned");
    for (size_t i = 0; i < sizeof(T); ++i)
        output.push_back(static_cast<uint8_t>((value >> (i * 8)) & 0xffu));
}

template <typename T>
bool read_integer(const std::vector<uint8_t>& input, size_t& offset, T& value)
{
    static_assert(std::is_unsigned<T>::value, "save integers must be unsigned");
    if (offset > input.size() || input.size() - offset < sizeof(T))
        return false;

    value = 0;
    for (size_t i = 0; i < sizeof(T); ++i)
        value |= static_cast<T>(input[offset++]) << (i * 8);
    return true;
}

void append_double(std::vector<uint8_t>& output, double value)
{
    uint64_t bits = 0;
    std::memcpy(&bits, &value, sizeof(bits));
    append_integer<uint64_t>(output, bits);
}

bool read_double(const std::vector<uint8_t>& input, size_t& offset, double& value)
{
    uint64_t bits = 0;
    if (!read_integer(input, offset, bits))
        return false;
    std::memcpy(&value, &bits, sizeof(value));
    return std::isfinite(value);
}

void append_string(std::vector<uint8_t>& output, const std::string& value)
{
    append_integer<uint32_t>(output, static_cast<uint32_t>(value.size()));
    output.insert(output.end(), value.begin(), value.end());
}

bool read_string(const std::vector<uint8_t>& input, size_t& offset, std::string& value)
{
    uint32_t length = 0;
    if (!read_integer(input, offset, length) || offset > input.size() ||
        length > input.size() - offset)
        return false;
    value.assign(reinterpret_cast<const char*>(input.data() + offset), length);
    offset += length;
    return true;
}

template <typename T>
void append_signed(std::vector<uint8_t>& output, T value)
{
    append_integer<uint64_t>(output, static_cast<uint64_t>(value));
}

template <typename T>
bool read_signed(const std::vector<uint8_t>& input, size_t& offset, T& value)
{
    uint64_t encoded = 0;
    if (!read_integer(input, offset, encoded))
        return false;
    value = static_cast<T>(encoded);
    return true;
}

void append_bool(std::vector<uint8_t>& output, bool value)
{
    output.push_back(value ? 1 : 0);
}

bool read_bool(const std::vector<uint8_t>& input, size_t& offset, bool& value)
{
    if (offset >= input.size() || input[offset] > 1)
        return false;
    value = input[offset++] != 0;
    return true;
}

void append_bytes(std::vector<uint8_t>& output, const std::vector<uint8_t>& value)
{
    append_integer<uint32_t>(output, static_cast<uint32_t>(value.size()));
    output.insert(output.end(), value.begin(), value.end());
}

bool read_bytes(const std::vector<uint8_t>& input, size_t& offset,
                std::vector<uint8_t>& value, size_t maximum_size)
{
    uint32_t length = 0;
    if (!read_integer(input, offset, length) || length > maximum_size ||
        offset > input.size() || length > input.size() - offset)
        return false;
    value.assign(input.begin() + static_cast<std::ptrdiff_t>(offset),
                 input.begin() + static_cast<std::ptrdiff_t>(offset + length));
    offset += length;
    return true;
}

bool valid_enum_values(const GameDefinition& definition)
{
    if (definition.galaxy_params.shape < GALAXY_RANDOM ||
        definition.galaxy_params.shape > GALAXY_GRID)
        return false;
    for (const auto& setup : definition.player_setups)
    {
        if (setup.player_gender < GENDER_OTHER || setup.player_gender > GENDER_M ||
            setup.type < PLAYER_HUMAN || setup.type > PLAYER_COMPUTER ||
            setup.starting_colony_quality < START_OUTPOST ||
            setup.starting_colony_quality > START_ABUNDANT)
            return false;
    }
    return true;
}

bool read_file(const std::string& path, std::vector<uint8_t>& bytes)
{
    std::ifstream input(path, std::ios::binary);
    if (!input)
        return false;

    input.seekg(0, std::ios::end);
    const std::streamoff length = input.tellg();
    if (length < 0 || static_cast<uint64_t>(length) > kMaxSectionSize * 16)
        return false;
    input.seekg(0, std::ios::beg);

    bytes.resize(static_cast<size_t>(length));
    if (!bytes.empty())
        input.read(reinterpret_cast<char*>(bytes.data()), length);
    return input.good() || input.eof();
}

bool parse_header(const std::vector<uint8_t>& file,
                 size_t& offset,
                 std::vector<uint8_t>* definition)
{
    constexpr size_t header_size = 8 + 4 + 8 + 4;
    if (file.size() < header_size)
        return false;

    for (char expected : kFileMagic)
    {
        if (file[offset++] != static_cast<uint8_t>(expected))
            return false;
    }

    uint32_t version = 0;
    uint64_t definition_size = 0;
    uint32_t definition_checksum = 0;
    if (!read_integer(file, offset, version) ||
        !read_integer(file, offset, definition_size) ||
        !read_integer(file, offset, definition_checksum))
        return false;

    if (version != kFormatVersion || definition_size > kMaxSectionSize ||
        definition_size > file.size() - offset)
        return false;

    std::vector<uint8_t> parsed_definition(
        file.begin() + static_cast<std::ptrdiff_t>(offset),
        file.begin() + static_cast<std::ptrdiff_t>(offset + definition_size));
    if (checksum(parsed_definition) != definition_checksum)
        return false;

    offset += static_cast<size_t>(definition_size);
    if (definition)
        *definition = std::move(parsed_definition);
    return true;
}

bool append_header(std::ofstream& output, const std::vector<uint8_t>& definition)
{
    std::vector<uint8_t> header;
    header.insert(header.end(), kFileMagic.begin(), kFileMagic.end());
    append_integer<uint32_t>(header, kFormatVersion);
    append_integer<uint64_t>(header, static_cast<uint64_t>(definition.size()));
    append_integer<uint32_t>(header, checksum(definition));
    output.write(reinterpret_cast<const char*>(header.data()),
                 static_cast<std::streamsize>(header.size()));
    output.write(reinterpret_cast<const char*>(definition.data()),
                 static_cast<std::streamsize>(definition.size()));
    return static_cast<bool>(output);
}

std::vector<uint8_t> encode_definition_payload(const GameDefinition& definition)
{
    if (definition.schema_version != GameDefinition::CURRENT_SCHEMA_VERSION ||
        definition.planets.size() > 500 || definition.player_setups.size() > 100 ||
        definition.home_planet_ids.size() > definition.planets.size() ||
        definition.player_ideal_temperatures.size() != definition.player_setups.size() ||
        definition.player_ideal_gravities.size() != definition.player_setups.size() ||
        !std::isfinite(definition.galaxy_size) || !std::isfinite(definition.galaxy_params.density) ||
        !std::isfinite(definition.galaxy_params.cluster_angular_offset) ||
        !valid_enum_values(definition))
        return {};

    for (double value : definition.player_ideal_temperatures)
        if (!std::isfinite(value))
            return {};
    for (double value : definition.player_ideal_gravities)
        if (!std::isfinite(value))
            return {};

    std::vector<uint8_t> output;
    output.insert(output.end(), kDefinitionMagic.begin(), kDefinitionMagic.end());
    append_integer<uint32_t>(output, definition.schema_version);
    append_integer<uint32_t>(output, definition.galaxy_params.n_planets);
    append_integer<uint32_t>(output, definition.galaxy_params.n_players);
    append_double(output, definition.galaxy_params.density);
    output.push_back(static_cast<uint8_t>(definition.galaxy_params.shape));
    append_integer<uint64_t>(output, definition.galaxy_params.seed);
    append_double(output, definition.galaxy_params.cluster_angular_offset);
    append_double(output, definition.galaxy_size);

    append_integer<uint32_t>(output, static_cast<uint32_t>(definition.planets.size()));
    for (const auto& planet : definition.planets)
    {
        append_integer<uint32_t>(output, planet.id);
        append_string(output, planet.name);
        append_double(output, planet.x);
        append_double(output, planet.y);
        append_double(output, planet.true_gravity);
    }

    append_integer<uint32_t>(output, static_cast<uint32_t>(definition.home_planet_ids.size()));
    for (uint32_t id : definition.home_planet_ids)
        append_integer<uint32_t>(output, id);

    append_integer<uint32_t>(output, static_cast<uint32_t>(definition.player_setups.size()));
    for (const auto& setup : definition.player_setups)
    {
        append_string(output, setup.name);
        output.push_back(static_cast<uint8_t>(setup.player_gender));
        output.push_back(static_cast<uint8_t>(setup.type));
        append_integer<uint32_t>(output, static_cast<uint32_t>(setup.ai_iq));
        output.push_back(static_cast<uint8_t>(setup.starting_colony_quality));
    }
    append_integer<uint32_t>(output, static_cast<uint32_t>(definition.player_ideal_temperatures.size()));
    for (double ideal_temperature : definition.player_ideal_temperatures)
        append_double(output, ideal_temperature);
    append_integer<uint32_t>(output, static_cast<uint32_t>(definition.player_ideal_gravities.size()));
    for (double ideal_gravity : definition.player_ideal_gravities)
        append_double(output, ideal_gravity);
    return output;
}

bool decode_definition_payload(const std::vector<uint8_t>& payload,
                               GameDefinition& definition)
{
    if (payload.size() < 8 + 4 + 4 + 4 + 8 + 1 + 8 + 8 + 4)
        return false;

    size_t offset = 0;
    for (char expected : kDefinitionMagic)
    {
        if (payload[offset++] != static_cast<uint8_t>(expected))
            return false;
    }

    GameDefinition parsed;
    uint32_t schema_version = 0;
    uint8_t shape = 0;
    if (!read_integer(payload, offset, schema_version) ||
        !read_integer(payload, offset, parsed.galaxy_params.n_planets) ||
        !read_integer(payload, offset, parsed.galaxy_params.n_players) ||
        !read_double(payload, offset, parsed.galaxy_params.density) ||
        offset >= payload.size())
        return false;
    shape = payload[offset++];
    if (!read_integer(payload, offset, parsed.galaxy_params.seed) ||
        !read_double(payload, offset, parsed.galaxy_params.cluster_angular_offset) ||
        !read_double(payload, offset, parsed.galaxy_size))
        return false;
    parsed.schema_version = schema_version;
    parsed.galaxy_params.shape = static_cast<GalaxyShape>(shape);

    uint32_t planet_count = 0;
    if (!read_integer(payload, offset, planet_count) || planet_count > 500)
        return false;
    parsed.planets.reserve(planet_count);
    for (uint32_t i = 0; i < planet_count; ++i)
    {
        ImmutablePlanetDefinition planet;
        if (!read_integer(payload, offset, planet.id) ||
            !read_string(payload, offset, planet.name) ||
            !read_double(payload, offset, planet.x) ||
            !read_double(payload, offset, planet.y) ||
            !read_double(payload, offset, planet.true_gravity))
            return false;
        parsed.planets.push_back(std::move(planet));
    }

    uint32_t home_count = 0;
    if (!read_integer(payload, offset, home_count) || home_count > planet_count)
        return false;
    parsed.home_planet_ids.resize(home_count);
    for (uint32_t& id : parsed.home_planet_ids)
        if (!read_integer(payload, offset, id))
            return false;

    std::unordered_set<uint32_t> planet_ids;
    for (const auto& planet : parsed.planets)
        if (!planet_ids.insert(planet.id).second)
            return false;
    std::unordered_set<uint32_t> home_ids;
    for (uint32_t id : parsed.home_planet_ids)
        if (planet_ids.find(id) == planet_ids.end() || !home_ids.insert(id).second)
            return false;

    uint32_t player_count = 0;
    if (!read_integer(payload, offset, player_count) || player_count > 100)
        return false;
    parsed.player_setups.reserve(player_count);
    for (uint32_t i = 0; i < player_count; ++i)
    {
        PlayerSetup setup;
        uint8_t gender = 0;
        uint8_t type = 0;
        uint32_t iq = 0;
        uint8_t quality = 0;
        if (!read_string(payload, offset, setup.name) || offset + 2 > payload.size())
            return false;
        gender = payload[offset++];
        type = payload[offset++];
        if (!read_integer(payload, offset, iq) || offset >= payload.size())
            return false;
        quality = payload[offset++];
        setup.player_gender = static_cast<Gender>(gender);
        setup.type = static_cast<PlayerType>(type);
        setup.ai_iq = static_cast<int32_t>(iq);
        setup.starting_colony_quality = static_cast<StartingColonyQuality>(quality);
        parsed.player_setups.push_back(std::move(setup));
    }

    uint32_t ideal_temperature_count = 0;
    if (!read_integer(payload, offset, ideal_temperature_count) ||
        ideal_temperature_count != player_count)
        return false;
    parsed.player_ideal_temperatures.resize(ideal_temperature_count);
    for (double& ideal_temperature : parsed.player_ideal_temperatures)
        if (!read_double(payload, offset, ideal_temperature))
            return false;

    uint32_t ideal_gravity_count = 0;
    if (!read_integer(payload, offset, ideal_gravity_count) ||
        ideal_gravity_count != player_count)
        return false;
    parsed.player_ideal_gravities.resize(ideal_gravity_count);
    for (double& ideal_gravity : parsed.player_ideal_gravities)
        if (!read_double(payload, offset, ideal_gravity))
            return false;

    if (offset != payload.size() || parsed.schema_version != GameDefinition::CURRENT_SCHEMA_VERSION ||
        parsed.galaxy_params.n_planets != planet_count ||
        parsed.galaxy_params.n_players != player_count ||
        parsed.home_planet_ids.size() != player_count || !valid_enum_values(parsed))
        return false;

    definition = std::move(parsed);
    return true;
}

std::vector<uint8_t> encode_runtime_payload(const RuntimeCheckpointPayload& checkpoint)
{
    if (checkpoint.schema_version != RuntimeCheckpointPayload::CURRENT_SCHEMA_VERSION ||
        checkpoint.planets.size() > 500 || checkpoint.players.size() > 100)
        return {};

    std::vector<uint8_t> output;
    output.insert(output.end(), kRuntimeMagic.begin(), kRuntimeMagic.end());
    append_integer<uint32_t>(output, checkpoint.schema_version);
    append_integer<uint32_t>(output, checkpoint.turn);
    append_integer<uint32_t>(output, checkpoint.year);
    append_integer<uint64_t>(output, checkpoint.deterministic_seed);
    append_integer<uint64_t>(output, checkpoint.ai_seed);
    append_bytes(output, checkpoint.deterministic_rng_state);
    append_bytes(output, checkpoint.ai_rng_state);
    append_integer<uint32_t>(output, static_cast<uint32_t>(checkpoint.planets.size()));
    for (const auto& planet : checkpoint.planets)
    {
        if (!std::isfinite(planet.true_temperature) || planet.views.size() > 100)
            return {};
        append_integer<uint32_t>(output, planet.id);
        append_signed<int32_t>(output, planet.owner);
        append_double(output, planet.true_temperature);
        append_signed<int32_t>(output, planet.metal);
        append_signed<int32_t>(output, planet.population);
        if (planet.nova_state < PLANET_NORMAL || planet.nova_state > PLANET_DESTROYED)
            return {};
        append_signed<int32_t>(output, planet.nova_state);
        append_integer<uint32_t>(output, static_cast<uint32_t>(planet.views.size()));
        for (const auto& view : planet.views)
        {
            if (!std::isfinite(view.apparent_temperature))
                return {};
            append_integer<uint32_t>(output, view.observer_id);
            append_double(output, view.apparent_temperature);
            append_bool(output, view.has_observed_gravity);
            append_signed<int32_t>(output, view.metal);
            append_signed<int32_t>(output, view.apparent_owner);
            append_signed<int32_t>(output, view.apparent_population);
            append_signed<int32_t>(output, view.observation_year);
            append_signed<int32_t>(output, view.income);
            append_signed<int32_t>(output, view.can_be_profitable);
            append_signed<int32_t>(output, view.perceived_value);
        }
    }

    append_integer<uint32_t>(output, static_cast<uint32_t>(checkpoint.players.size()));
    for (const auto& player : checkpoint.players)
    {
        if (!std::isfinite(player.savings_rate_modifier) || player.savings_rate_modifier < 0.0 ||
            player.colonies.size() > checkpoint.planets.size())
            return {};
        append_integer<uint32_t>(output, player.id);
        append_bool(output, player.eliminated);
        append_signed<int64_t>(output, player.money_savings);
        append_signed<int64_t>(output, player.metal_reserve);
        append_signed<int64_t>(output, player.metal_income);
        append_double(output, player.savings_rate_modifier);
        append_signed<int64_t>(output, player.pending_dip_amount);
        append_bool(output, player.pending_dip_persistent);
        append_signed<int64_t>(output, player.locked_dip_amount);
        append_bool(output, player.locked_dip_persistent);
        append_double(output, player.savings_fraction);
        append_double(output, player.research_fraction);
        append_double(output, player.planets_fraction);
        for (double value : player.research_fractions)
        {
            if (!std::isfinite(value))
                return {};
            append_double(output, value);
        }
        for (int32_t value : player.technology_levels)
            append_signed<int32_t>(output, value);
        for (int64_t value : player.partial_research)
            append_signed<int64_t>(output, value);
        append_integer<uint32_t>(output, static_cast<uint32_t>(player.colonies.size()));
        for (const auto& colony : player.colonies)
        {
            if (!std::isfinite(colony.funding_fraction) ||
                !std::isfinite(colony.mining_fraction) ||
                !std::isfinite(colony.terraforming_fraction))
                return {};
            append_integer<uint32_t>(output, colony.planet_id);
            append_signed<int32_t>(output, colony.income);
            append_double(output, colony.funding_fraction);
            append_double(output, colony.mining_fraction);
            append_double(output, colony.terraforming_fraction);
        }
    }
    return output;
}

bool decode_runtime_payload(const std::vector<uint8_t>& payload,
                            RuntimeCheckpointPayload& checkpoint)
{
    if (payload.size() < 8 + 4 + 4 + 4 + 4 + 4)
        return false;
    size_t offset = 0;
    for (char expected : kRuntimeMagic)
        if (payload[offset++] != static_cast<uint8_t>(expected))
            return false;

    RuntimeCheckpointPayload parsed;
    if (!read_integer(payload, offset, parsed.schema_version) ||
        !read_integer(payload, offset, parsed.turn) ||
        !read_integer(payload, offset, parsed.year) ||
        !read_integer(payload, offset, parsed.deterministic_seed) ||
        !read_integer(payload, offset, parsed.ai_seed) ||
        !read_bytes(payload, offset, parsed.deterministic_rng_state, 1024 * 1024) ||
        !read_bytes(payload, offset, parsed.ai_rng_state, 1024 * 1024))
        return false;

    uint32_t planet_count = 0;
    if (!read_integer(payload, offset, planet_count) || planet_count > 500)
        return false;
    parsed.planets.resize(planet_count);
    for (auto& planet : parsed.planets)
    {
            if (!read_integer(payload, offset, planet.id) ||
                !read_signed<int32_t>(payload, offset, planet.owner) ||
                !read_double(payload, offset, planet.true_temperature) ||
                !read_signed<int32_t>(payload, offset, planet.metal) ||
                !read_signed<int32_t>(payload, offset, planet.population) ||
                !read_signed<int32_t>(payload, offset, planet.nova_state) ||
                planet.nova_state < PLANET_NORMAL || planet.nova_state > PLANET_DESTROYED)
            return false;

        uint32_t view_count = 0;
        if (!read_integer(payload, offset, view_count) || view_count > 100)
            return false;
        planet.views.resize(view_count);
        for (auto& view : planet.views)
        {
            if (!read_integer(payload, offset, view.observer_id) ||
                !read_double(payload, offset, view.apparent_temperature) ||
                !read_bool(payload, offset, view.has_observed_gravity) ||
                !read_signed<int32_t>(payload, offset, view.metal) ||
                !read_signed<int32_t>(payload, offset, view.apparent_owner) ||
                !read_signed<int32_t>(payload, offset, view.apparent_population) ||
                !read_signed<int32_t>(payload, offset, view.observation_year) ||
                !read_signed<int32_t>(payload, offset, view.income) ||
                !read_signed<int32_t>(payload, offset, view.can_be_profitable) ||
                !read_signed<int32_t>(payload, offset, view.perceived_value))
                return false;
        }
    }

    uint32_t player_count = 0;
    if (!read_integer(payload, offset, player_count) || player_count > 100)
        return false;
    parsed.players.resize(player_count);
    for (auto& player : parsed.players)
    {
        if (!read_integer(payload, offset, player.id) ||
            !read_bool(payload, offset, player.eliminated) ||
            !read_signed<int64_t>(payload, offset, player.money_savings) ||
            !read_signed<int64_t>(payload, offset, player.metal_reserve) ||
            !read_signed<int64_t>(payload, offset, player.metal_income) ||
            !read_double(payload, offset, player.savings_rate_modifier) ||
            !read_signed<int64_t>(payload, offset, player.pending_dip_amount) ||
            !read_bool(payload, offset, player.pending_dip_persistent) ||
            !read_signed<int64_t>(payload, offset, player.locked_dip_amount) ||
            !read_bool(payload, offset, player.locked_dip_persistent) ||
            !read_double(payload, offset, player.savings_fraction) ||
            !read_double(payload, offset, player.research_fraction) ||
            !read_double(payload, offset, player.planets_fraction))
            return false;
        for (double& value : player.research_fractions)
            if (!read_double(payload, offset, value))
                return false;
        for (int32_t& value : player.technology_levels)
            if (!read_signed<int32_t>(payload, offset, value))
                return false;
        for (int64_t& value : player.partial_research)
            if (!read_signed<int64_t>(payload, offset, value))
                return false;

        uint32_t colony_count = 0;
        if (!read_integer(payload, offset, colony_count) || colony_count > planet_count)
            return false;
        player.colonies.resize(colony_count);
        for (auto& colony : player.colonies)
        {
            if (!read_integer(payload, offset, colony.planet_id) ||
                !read_signed<int32_t>(payload, offset, colony.income) ||
                !read_double(payload, offset, colony.funding_fraction) ||
                !read_double(payload, offset, colony.mining_fraction) ||
                !read_double(payload, offset, colony.terraforming_fraction))
                return false;
        }
    }

    if (offset != payload.size() ||
        parsed.schema_version != RuntimeCheckpointPayload::CURRENT_SCHEMA_VERSION)
        return false;
    checkpoint = std::move(parsed);
    return true;
}

bool append_record(std::ofstream& output,
                   uint32_t turn,
                   SaveRecordPhase phase,
                   const std::vector<uint8_t>& payload)
{
    std::vector<uint8_t> header;
    header.insert(header.end(), kRecordMagic.begin(), kRecordMagic.end());
    append_integer<uint32_t>(header, kFormatVersion);
    append_integer<uint32_t>(header, turn);
    header.push_back(static_cast<uint8_t>(phase));
    header.push_back(0);
    header.push_back(0);
    header.push_back(0);
    append_integer<uint64_t>(header, static_cast<uint64_t>(payload.size()));
    append_integer<uint32_t>(header, checksum(payload));

    output.write(reinterpret_cast<const char*>(header.data()),
                 static_cast<std::streamsize>(header.size()));
    output.write(reinterpret_cast<const char*>(payload.data()),
                 static_cast<std::streamsize>(payload.size()));
    output.flush();
    return static_cast<bool>(output);
}

bool parse_records(const std::vector<uint8_t>& file,
                   size_t offset,
                   SaveCheckpoint* latest,
                   size_t* valid_end = nullptr)
{
    constexpr size_t record_header_size = 4 + 4 + 4 + 4 + 8 + 4;
    bool found = false;
    if (valid_end)
        *valid_end = offset;

    while (file.size() - offset >= record_header_size)
    {
        const size_t record_start = offset;
        bool magic_matches = true;
        for (char expected : kRecordMagic)
        {
            if (file[offset++] != static_cast<uint8_t>(expected))
            {
                magic_matches = false;
                break;
            }
        }
        if (!magic_matches)
            break; // Treat unknown/incomplete trailing data as an interrupted append.

        uint32_t version = 0;
        uint32_t turn = 0;
        uint64_t payload_size = 0;
        uint32_t payload_checksum = 0;
        if (!read_integer(file, offset, version) ||
            !read_integer(file, offset, turn))
            break;

        if (file.size() - offset < 4)
            break;
        const uint8_t phase_value = file[offset++];
        offset += 3; // reserved bytes

        if (!read_integer(file, offset, payload_size) ||
            !read_integer(file, offset, payload_checksum) ||
            version != kFormatVersion || payload_size > kMaxSectionSize ||
            payload_size > file.size() - offset)
        {
            offset = record_start;
            break;
        }

        std::vector<uint8_t> payload(
            file.begin() + static_cast<std::ptrdiff_t>(offset),
            file.begin() + static_cast<std::ptrdiff_t>(offset + payload_size));
        offset += static_cast<size_t>(payload_size);

        if (phase_value != static_cast<uint8_t>(SaveRecordPhase::TURN_CHECKPOINT) &&
            phase_value != static_cast<uint8_t>(SaveRecordPhase::MANUAL_CHECKPOINT))
            break;
        if (checksum(payload) != payload_checksum)
            break;

        if (latest)
        {
            latest->turn = turn;
            latest->phase = static_cast<SaveRecordPhase>(phase_value);
            latest->payload = std::move(payload);
        }
        found = true;
        if (valid_end)
            *valid_end = offset;
    }

    return found;
}
}

std::vector<uint8_t> encode_game_definition(const GameDefinition& definition)
{
    return encode_definition_payload(definition);
}

bool decode_game_definition(const std::vector<uint8_t>& payload,
                            GameDefinition& definition)
{
    return decode_definition_payload(payload, definition);
}

std::vector<uint8_t> encode_runtime_checkpoint(
    const RuntimeCheckpointPayload& checkpoint)
{
    return encode_runtime_payload(checkpoint);
}

bool decode_runtime_checkpoint(
    const std::vector<uint8_t>& payload,
    RuntimeCheckpointPayload& checkpoint)
{
    return decode_runtime_payload(payload, checkpoint);
}

bool SaveFile::create(const std::string& path,
                      const std::vector<uint8_t>& immutable_definition)
{
    if (immutable_definition.size() > kMaxSectionSize)
        return false;

    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    if (!output)
        return false;
    return append_header(output, immutable_definition);
}

bool SaveFile::append_checkpoint(const std::string& path,
                                 uint32_t turn,
                                 SaveRecordPhase phase,
                                 const std::vector<uint8_t>& payload)
{
    if (payload.size() > kMaxSectionSize)
        return false;

    std::vector<uint8_t> file;
    if (!read_file(path, file))
        return false;
    size_t offset = 0;
    if (!parse_header(file, offset, nullptr))
        return false;

    size_t valid_end = offset;
    parse_records(file, offset, nullptr, &valid_end);

    // If a prior process left an incomplete trailing record, discard only that
    // invalid tail before appending the next checkpoint.
    if (valid_end != file.size())
    {
        const std::string temporary_path = path + ".tmp";
        std::ofstream repaired(temporary_path, std::ios::binary | std::ios::trunc);
        if (!repaired)
            return false;
        repaired.write(reinterpret_cast<const char*>(file.data()),
                       static_cast<std::streamsize>(valid_end));
        if (!repaired || !append_record(repaired, turn, phase, payload))
        {
            repaired.close();
            std::remove(temporary_path.c_str());
            return false;
        }
        repaired.close();
        if (std::rename(temporary_path.c_str(), path.c_str()) != 0)
        {
            std::remove(temporary_path.c_str());
            return false;
        }
        return true;
    }

    std::ofstream output(path, std::ios::binary | std::ios::app);
    if (!output)
        return false;
    return append_record(output, turn, phase, payload);
}

bool SaveFile::read_latest_checkpoint(const std::string& path,
                                      SaveCheckpoint& checkpoint)
{
    std::vector<uint8_t> file;
    if (!read_file(path, file))
        return false;

    size_t offset = 0;
    if (!parse_header(file, offset, nullptr))
        return false;
    return parse_records(file, offset, &checkpoint);
}

bool SaveFile::read_immutable_definition(const std::string& path,
                                         std::vector<uint8_t>& definition)
{
    std::vector<uint8_t> file;
    if (!read_file(path, file))
        return false;

    size_t offset = 0;
    return parse_header(file, offset, &definition);
}

bool SaveFile::compact(const std::string& path)
{
    std::vector<uint8_t> definition;
    if (!read_immutable_definition(path, definition))
        return false;

    SaveCheckpoint latest;
    const bool has_checkpoint = read_latest_checkpoint(path, latest);
    const std::string temporary_path = path + ".tmp";
    if (!create(temporary_path, definition))
        return false;

    if (has_checkpoint && !append_checkpoint(temporary_path, latest.turn,
                                               latest.phase, latest.payload))
    {
        std::remove(temporary_path.c_str());
        return false;
    }

    if (std::rename(temporary_path.c_str(), path.c_str()) != 0)
    {
        std::remove(temporary_path.c_str());
        return false;
    }
    return true;
}
