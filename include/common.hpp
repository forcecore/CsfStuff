#pragma once

#include <string>

#ifndef NDEBUG
#   define ASSERT(condition, message) \
    do { \
        if (! (condition)) { \
            std::cerr << "Assertion `" #condition "` failed in " << __FILE__ \
                      << " line " << __LINE__ << ": " << message << std::endl; \
            assert(condition); \
        } \
    } while (false)
#else
#   define ASSERT(condition, message) do { } while (false)
#endif

struct CSFHeader
{
    char magic[4] = {' ', 'F', 'S', 'C'}; // CSF in reverse
    uint32_t csf_format;
    uint32_t num_labels;
    uint32_t num_strings;
    uint32_t unused;
    uint32_t lang_code;
};

struct LabelHeader
{
    char magic[4] = {' ', 'L', 'B', 'L'};
    uint32_t num_string_pairs;
    uint32_t length;
};

struct StrHeader
{
    char magic[4];
    uint32_t length;
};

class Entry
{
public:
    std::string label;
    std::string str;
    std::string extra_data;
};
