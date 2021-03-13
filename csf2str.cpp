/**
 * 2021 implemented by BoolBada.
 * See https://www.modenc.renegadeprojects.com/CSF_File_Format for CSF format!
 */
#include <cassert>
#include <cstring>
#include <string>
#include <iostream>
#include <fstream>

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
    cout << "Usage: csf2str [input.csf] [output.str]" << endl;
}

void check_fnames(const string &ifname, const string &ofname, const string &extrafname, const string &metafname)
{
    ASSERT(ifname != ofname, "Input and output file names must have different file names");
    ASSERT(ifname != extrafname, "Input file must not be named " + extrafname);
    ASSERT(ifname != metafname, "Input file must not be named " + metafname);

    ASSERT(ofname != extrafname, "Output file must not be named " + extrafname);
    ASSERT(ofname != metafname, "Output file must not be named " + metafname);

    assert(extrafname != metafname); // Wrong programming haha
}

void save_json(const string &ofname, const json &j)
{
    ofstream f;
    f.open(ofname);
    f << j.dump(2);
    f.close();
}

void save_metadata(const string &ofname, const CSFHeader &header)
{
    json j;
    j["lang_code"] = header.lang_code;
    j["unused"] = header.unused;
    save_json(ofname, j);
    cout << "Wrote metadata to " << ofname << endl;
}

string read_ascii(FILE *f, int n)
{
    string result(n, '\0');
    fread(&result[0], sizeof(char), n, f);
    return result;
}

string read_flipped_utf16(FILE *f, int n)
{
    string buffer(2 * n + 2, '\0');
    fread(&buffer[0], sizeof(uint16_t), n, f);

    // Flip bits
    u16string tmp(n, '\0');
    uint16_t *cur = (uint16_t *) &buffer[0];
    for (size_t i = 0 ; i < n ; i++, cur++)
        tmp[i] = ~(*cur);
    // cout << n << " " << tmp.length() << endl; // == n + 1, for the null termination.

    wstring_convert<codecvt_utf8_utf16<char16_t>, char16_t> cv;
    string result = cv.to_bytes(tmp);
    // cout << n << " " << result.length() << endl; // result length is unpredictable!
    return result;
}

Entry parse_entry(FILE *f, json *extra_data)
{
    Entry result;

    // Read the label header
    LabelHeader label_header;
    fread(&label_header, sizeof(LabelHeader), 1, f);
    ASSERT(label_header.num_string_pairs == 1, "labels with more than 1 string pairs is not supported! (and should not appear in any of the C&C games...)");
    ASSERT(strncmp(label_header.magic, " LBL", 4) == 0, "Label header does not begin with \" LBL\"!");

    // Read the label
    result.label = read_ascii(f, label_header.length);

    // Read the string part
    StrHeader str_header;
    fread(&str_header, sizeof(StrHeader), 1, f);
    bool has_extra_data = false;
    if (strncmp(str_header.magic, "WRTS", 4) == 0) // reverse of STRW
        has_extra_data = true;
    else
        ASSERT(strncmp(str_header.magic, " RTS", 4) == 0, "Invalid string header, expecting WRTS or RTS"); // reverse of STR
    result.str = read_flipped_utf16(f, str_header.length);

    if (has_extra_data)
    {
        uint32_t length;
        fread(&length, sizeof(uint32_t), 1, f);
        result.extra_data = read_ascii(f, length);
    }

    return result;
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

void write_entry_to_str(FILE *of, const Entry &entry)
{
    static bool is_first = true;

    if (!is_first)
    {
        fprintf(of, "\n");
    }
    is_first = false;

    fprintf(of, "%s\n", entry.label.c_str());
    fprintf(of, "%s\n", escape_characters(entry.str).c_str());
    fprintf(of, "END\n");
}

void decode_and_write_files
(
    FILE *csff,
    const string &ifname,
    const string &ofname,
    const string &extrafname,
    const string &metafname
)
{
    // Read the header
    CSFHeader header;
    fread(&header, sizeof(CSFHeader), 1, csff);
    ASSERT(strncmp(header.magic, " FSC", 4) == 0, "Given input file does not begin with \" FSC\"!"); // reverse of CSF
    ASSERT(header.csf_format == 3, "CSF format 3 is not supported. (RA2 through RA3 should work though)");
    save_metadata(metafname, header);

    json extra_data;
    FILE *of = fopen(ofname.c_str(), "w");

    for (size_t i = 0 ; i < header.num_labels ; i++)
    {
        Entry entry = parse_entry(csff, &extra_data);
        write_entry_to_str(of, entry);
        if (entry.extra_data != "")
            extra_data[entry.label] = entry.extra_data;
    }

    fclose(of);
    cout << "Wrote " << ofname << endl;

    // Save extra data too, if any.
    if (extra_data.size() > 0)
    {
        save_json(extrafname, extra_data);
        cout << "Wrote extra_data to " << extrafname << endl;
    }
}

int main(int argc, const char *argv[])
{
    if (argc < 3)
    {
        show_usage();
        return 0;
    }

    // Get program arguments
    string ifname = argv[1];
    string ofname = argv[2];
    string extrafname = "extra_data.json"; // Extra data can't be converted as str. We create extra file to preserve it.
    string metafname = "meta.json";
    check_fnames(ifname, ofname, extrafname, metafname);

    FILE *f = fopen(ifname.c_str(), "rb");
    ASSERT(f != NULL, "Failed to open file " + ifname);
    decode_and_write_files(f, ifname, ofname, extrafname, metafname);
    fclose(f);
    return 0;
}
