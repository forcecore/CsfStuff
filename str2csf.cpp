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
    cout << "Usage: str2csf input.str output.csf [extra_data.json]" << endl;
    cout << endl;
    cout << "    optional arguments:" << endl;
    cout << "        extra_data.json: provide extra data attached to labels, if any." << endl;
}

json read_json_file(const string &fname)
{
    std::ifstream i(fname);
    json j;
    i >> j;
    return j;
}

json read_metadata(vector<Entry> *entries)
{
    const Entry &e = entries->at(0);
    if (e.label == "CSFSTUFF:META")
    {
        json j;
        try
        {
            j = json::parse(e.str);
        }
        catch(const std::exception& ex)
        {
            std::cerr << ex.what() << '\n';
            std::cerr << "Error parsing CSF metadata in CSFSTUFF:META: got \'" << e.str << "\'" << endl;
            throw ex;
        }
        entries->erase(entries->begin()); // Delete the entry so that we get perfect reconstruction.
        return j;
    }

    // By default, lang_code == 0 (en_US), unused == 0.
    cerr << "Warning: CSFSTUFF:META must exist and appear as the first item. Falling back to default CSF metadata of language_code=0 and unused=0" << endl;
    return json::parse("{\"lang_code\":0,\"unused\":0}");
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

void parse_args
(
    int argc, const char *argv[],
    string *ifname, string *ofname,
    string *extrafname
)
{
    *ifname = argv[1];
    *ofname = argv[2];

    if (argc >= 4)
        *extrafname = argv[3];

    ASSERT(*ifname != *ofname, "Input and output file names must have different file names");
    ASSERT(*ifname != "", "Input file name must be given!");
    ASSERT(*ofname != "", "Output file name must be given!");
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
    write_flipped_utf16(fp, e.str);

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
    string extrafname = ""; // Extra data can't be converted as str. We create extra file to preserve it.
    parse_args(argc, argv, &ifname, &ofname, &extrafname);

    vector<Entry> entries = read_entries(ifname);
    json metadata = read_metadata(&entries);
    json extra_data;
    if (extrafname != "")
        extra_data = read_json_file(extrafname);

    write_csf(ofname, entries, metadata, extra_data);
    return 0;
}
