#include "save_file.h"

#include <array>
#include <cstdio>
#include <fstream>
#include <limits>
#include <string>
#include <type_traits>

namespace
{
constexpr std::array<char, 8> kFileMagic = {'O', 'H', 'O', 'S', 'A', 'V', 'E', '1'};
constexpr std::array<char, 4> kRecordMagic = {'R', 'C', 'D', '1'};
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
    if (input.size() - offset < sizeof(T))
        return false;

    value = 0;
    for (size_t i = 0; i < sizeof(T); ++i)
        value |= static_cast<T>(input[offset++]) << (i * 8);
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
