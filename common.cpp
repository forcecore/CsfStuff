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

string escape_characters(const string &s)
{
    string result = "\"";

    for (char ch: s)
    {
        switch (ch)
        {
            case '\n':
                result += "\\n";
                break;
            case '\\':
                result += "\\\\";
                break;
            case '"':
                result += "\\\"";
                break;
            default:
                result += ch;
        }
    }

    result += "\"";
    return result;
}

string unescape_characters(const string &s)
{
    enum mode_t { NORMAL, ESCAPED };

    mode_t mode = NORMAL;
    string result;

    for (char ch: s)
    {
        if (mode == NORMAL)
        {
            if (ch == '\\')
                mode = ESCAPED;
            else
                result += ch;
        }
        else
        {
            switch (ch)
            {
                case 'n':
                    result += '\n';
                    break;
                case '\\':
                    result += '\\';
                    break;
                case '"':
                    result += '"';
                    break;
                default:
                    ASSERT(0, "Invalid character! Got '" << ch << "': " << s);
                    break;
            }
            mode = NORMAL;
        }
    }

    return result;
}

void write_entry_to_str(FILE *fp, const Entry &entry)
{
    static bool is_first = true;

    if (!is_first)
    {
        fprintf(fp, "\n");
    }
    is_first = false;

    fprintf(fp, "%s\n", entry.label.c_str());
    fprintf(fp, "%s\n", escape_characters(entry.str).c_str());
    fprintf(fp, "END\n");
}

/**
 * Match whitespace and/or comment.
 * "" returns true.
 */
bool is_whitespace_or_comment(const string &line)
{
    // matches whitespace and an optional //-comment
    const static regex expr("\\s*(//.*)?");
    smatch _match;
    return regex_match(line, _match, expr);
}

bool is_END(const string &line)
{
    // Matches END, which may or may not be surrounded by white spaces.
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
    // Matches quoted block, which may or may not be surrounded by white spaces.
    const static regex expr("\\s*\"(.*)\"\\s*");
    smatch match;
    bool is_match = regex_match(line, match, expr);
    ASSERT(is_match, "\nline " << lineno << ": \"" << line << "\" is not a proper in-game string. It must not be commented and properly quoted at the start and at the end.");
    //for (unsigned i=0; i<match.size(); ++i)
    //    std::cout << "match #" << i << ": " << match[i] << std::endl;
    return match[1].str();
}

string strip_label(const int lineno, const string &line)
{
    const static regex expr("\\s*(.+)\\s*");
    smatch match;
    bool is_match = regex_match(line, match, expr);
    ASSERT(is_match, "\nline " << lineno << ": \"" << line << "\" label must not have comment part.");
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
 */
std::vector<Entry> read_entries(const std::string &fname)
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
                entry.label = strip_label(lineno, line);
                state = READ_STR;
                break;
            case READ_STR:
                entry.str = unescape_characters( strip_str(lineno, line) );
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
