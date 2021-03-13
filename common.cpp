#include <fstream>
#include <vector>
#include "include/common.hpp"

using namespace std;

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

    ifstream fs(fname);
    string line;

    enum state_t { SEEK_AND_READ_LABEL, READ_STR, READ_END };
    state_t state = SEEK_AND_READ_LABEL;
    while (getline(fs, line))
    {
        switch (state)
        {
            case SEEK_AND_READ_LABEL:
                if (line == "")
                    continue;
                entry.label = line;
                state = READ_STR;
                break;
            case READ_STR:
                entry.str = line;
                state = READ_END;
                break;
            case READ_END:
                ASSERT(line == "END", "END expected, got \"" + line + "\", invalid input!");
                state = SEEK_AND_READ_LABEL;
                // cout << entry.label << " " << entry.str << endl;
                result.push_back(entry);
                break;
            default:
                ASSERT(0, "Can't reach here");
                break;
        }
    }

    fs.close();
    return result;
}

