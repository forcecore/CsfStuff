/**
 * 2021 implemented by BoolBada.
 * See https://www.modenc.renegadeprojects.com/CSF_File_Format for CSF format!
 */
#include <cassert>
#include <cstring>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>

// UTF-16 stuff
// Let's use C++11, although C++17 deprecates this.
#include <locale>
#include <codecvt>

#include "include/common.hpp"
#include "include/json.hpp"

using namespace std;
using json = nlohmann::json;

void show_usage()
{
    cout << "Usage: str2csf input.str output.csf [meta.json] [extra_data.json]" << endl;
    cout << endl;
    cout << "    optional arguments:" << endl;
    cout << "        meta.json: meta data from the CSF file. The file must exist!" << endl;
    cout << "        extra_data.json: extra data attached to each label, if any." << endl;
}

json read_json_file(const string &fname)
{
    std::ifstream i(fname);
    json j;
    i >> j;
    return j;
}

void write_ascii(FILE *fp, const string &s)
{
    uint32_t len = s.length();
    fwrite(&len, sizeof(uint32_t), 1, fp); // write length
    fwrite(s.c_str(), sizeof(char), len, fp); // write string
}

void write_flipped_utf16(FILE *fp, const string &s)
{
    wstring_convert<codecvt_utf8_utf16<char16_t>, char16_t> cv;
    u16string tmp = cv.from_bytes(s);
    uint32_t len = tmp.length();

    // Flip bits
    for (size_t i = 0 ; i < len ; i++)
        tmp[i] = ~(tmp[i]);

    fwrite(&len, sizeof(uint32_t), 1, fp); // write length
    fwrite(&tmp[0], sizeof(char16_t), len, fp); // write string
}

string unescape_characters(const string &s)
{
    ASSERT(s[0] == '"', "STR must start with '\"'.");
    ASSERT(s[s.length() - 1] == '"', "STR must end with '\"'.");

    enum mode_t { NORMAL, ESCAPED };

    mode_t mode = NORMAL;
    string result;

    for (size_t i = 1 ; i < s.length() - 1 ; ++i) // loop, excluding quotes.
    {
        char ch = s[i];
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

void parse_args
(
    int argc, const char *argv[],
    string *ifname, string *ofname,
    string *metafname, string *extrafname
)
{
    *ifname = argv[1];
    *ofname = argv[2];

    if (argc >= 4)
        *metafname = argv[3];

    if (argc >= 5)
        *extrafname = argv[4];

    ASSERT(*ifname != *ofname, "Input and output file names must have different file names");
    ASSERT(*ifname != "", "Input file name must be given!");
    ASSERT(*ofname != "", "Output file name must be given!");
    ASSERT(*metafname != "", "Meta file name must be empty!");
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
vector<Entry> read_entries(const string &fname)
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

void write_csf_header(FILE *fp, size_t num_labels, const json &metadata)
{
    CSFHeader header;
    header.num_labels = num_labels;
    header.num_strings = num_labels;
    header.unused = metadata["unused"];
    header.lang_code = metadata["lang_code"];
    fwrite(&header, sizeof(CSFHeader), 1, fp);
}

void write_entry(FILE *fp, const Entry &e, const json &extra_data)
{
    static const char *STR = " RTS";
    static const char *STRW = "WRTS";

    LabelHeader lh;
    lh.length = e.label.length();
    fwrite(&lh, sizeof(LabelHeader), 1, fp);
    for (char ch: e.label)
        fputc(ch, fp);

    // Check if there's extra data.
    auto ed = extra_data.find(e.label);

    // Now, let's write content.
    const char *str_magic = (ed == extra_data.end()) ? STR : STRW;
    fwrite(str_magic, sizeof(char), 4, fp);
    string str = unescape_characters(e.str);
    write_flipped_utf16(fp, str);

    // Write extra data, if there is.
    if (ed != extra_data.end())
        write_ascii(fp, *ed);
}

void write_csf
(
    const string &ofname,
    const vector<Entry> &entries,
    const json &metadata,
    const json &extra_data
)
{
    FILE *fp = fopen(ofname.c_str(), "wb");

    write_csf_header(fp, entries.size(), metadata);
    for(const Entry e: entries)
    {
        write_entry(fp, e, extra_data);
    }

    fclose(fp);
    return;
}

int main(int argc, const char *argv[])
{
    if (argc < 3)
    {
        show_usage();
        return 0;
    }
    string ifname;
    string ofname;
    string metafname = "meta.json";
    string extrafname = ""; // Extra data can't be converted as str. We create extra file to preserve it.
    parse_args(argc, argv, &ifname, &ofname, &metafname, &extrafname);

    vector<Entry> entries = read_entries(ifname);
    json metadata = read_json_file(metafname);
    json extra_data;
    if (extrafname != "")
        extra_data = read_json_file(extrafname);

    write_csf(ofname, entries, metadata, extra_data);
    return 0;
}
