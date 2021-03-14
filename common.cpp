#include <fstream>
#include <vector>
#include <cctype>
#include <regex>
#include "include/common.hpp"

using namespace std;

bool file_exists(const string &fname)
{
    std::ifstream infile(fname);
    return infile.good();
}

/**
 * Match whitespace and/or comment.
 * "" returns true.
 */
bool is_whitespace_or_comment(const string &line)
{
    const static regex expr("\\s*(//.*)?");
    smatch _match;
    return regex_match(line, _match, expr);
}

bool is_END(const string &line)
{
    const static regex expr("\\s*END(.*)");
    smatch match;
    bool is_match = regex_match(line, match, expr);
    if (!is_match)
        return false;
    //for (unsigned i=0; i<match.size(); ++i)
    //    std::cout << "match #" << i << ": " << match[i] << std::endl;
    string after_end = match[1].str();
    return is_whitespace_or_comment(after_end);
}

string strip_str(const int lineno, const string &line)
{
    const static regex expr("\\s*(\".+\")\\s*");
    smatch match;
    bool is_match = regex_match(line, match, expr);
    ASSERT(is_match, "\nError parsing input line " << lineno << ", got: " << line);
    //for (unsigned i=0; i<match.size(); ++i)
    //    std::cout << "match #" << i << ": " << match[i] << std::endl;
    return match[1].str();
}

string strip_label(const int lineno, const string &line)
{
    const static regex expr("\\s*(\\S+)\\s*");
    smatch match;
    bool is_match = regex_match(line, match, expr);
    ASSERT(is_match, "WTF");
    //for (unsigned i=0; i<match.size(); ++i)
    //    std::cout << "match #" << i << ": " << match[i] << std::endl;
    return match[1].str();
}

/**
 * Read entries from str files.
 * STR files look like this:
 *
 * TXT_POWER_DRAIN
 * "Power = %d \n Drain = %d"
 * END
 *
 * TXT_STAND_BY
 * "Please Stand By..."
 * END
 *
 * If do_strip_str is true, the string content will be stripped of comments and bad syntax will be checked.
 * Otherwise str will be kept as is. (Used for STR merging.)
 */
std::vector<Entry> read_entries(const std::string &fname, bool do_strip)
{
    vector<Entry> result;
    Entry entry;

    ASSERT(file_exists(fname), fname << " does not exists!");
    ifstream fs(fname);
    string line;

    enum state_t { SEEK_AND_READ_LABEL, READ_STR, READ_END };
    state_t state = SEEK_AND_READ_LABEL;
    int lineno = 1;
    while (getline(fs, line))
    {
        switch (state)
        {
            case SEEK_AND_READ_LABEL:
                if (is_whitespace_or_comment(line))
                    continue;
                entry.label = (do_strip) ? strip_label(lineno, line) : line;
                state = READ_STR;
                break;
            case READ_STR:
                entry.str = (do_strip) ? strip_str(lineno, line) : line;
                state = READ_END;
                break;
            case READ_END:
                ASSERT(is_END(line), "\nSTR file line " << lineno << ": END expected, got \"" + line + "\", invalid input!");
                state = SEEK_AND_READ_LABEL;
                // cout << entry.label << " " << entry.str << endl;
                result.push_back(entry);
                break;
            default:
                ASSERT(0, "Can't reach here");
                break;
        }
        lineno++;
    }

    fs.close();
    return result;
}
