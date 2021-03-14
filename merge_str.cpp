/**
 * A program to merge multiple STR files into one.
 * While it is neater to merge CSF files, the users can not easily see inside CSF files
 * without dedicated tools. Hence STR merging would be much better.
 */

#include <vector>
#include <map>
#include <string>
#include "include/common.hpp"

using namespace std;

void show_usage()
{
    cout << "Usage: merge_str input1.str input2.str ... inputN.str output.str" << endl;
    cout << endl;
    cout << "    Merges multiple STR files into one." << endl;
    cout << "    The last command line argument specifies the output STR file." << endl;
    cout << "    Later STR files will overwrite onto earlier ones." << endl;
    cout << "    That is, input1.str has the lowest priority." << endl;
}

/**
 * Create the map so thate we can find where the overlapping entries are
 * in the destination entries.
 */
map<string, int> make_lookup_table(const vector<Entry> &entries)
{
    map<string, int> result;
    for (size_t i = 0 ; i < entries.size() ; i++)
    {
        const string &label = entries[i].label;
        auto it = result.find(label);
        ASSERT(it == result.end(), "\nDuplicate entry found, label is " << label);
        result[label] = i;
    }
    return result;
}

/**
 * Replace overlapping entries on top of main_entries, from main_entries.
 * Non-overlapping entries will be appended.
 *
 * lut: [inout] lookup table.
 */
void merge_entries(vector<Entry> *main_entries, const vector<Entry> &new_entries, map<string, int> *lut)
{
    for (const Entry &e: new_entries)
    {
        auto it = lut->find(e.label);
        if (it == lut->end())
        {
            // Non-overlapping entry. Just append to main entries.
            (*lut)[e.label] = main_entries->size();
            main_entries->push_back(e);
        }
        else
        {
            Entry &existing = main_entries->at(it->second);
            existing.str = e.str;
        }
    }
}

void write_entry_to_str_as_is(FILE *of, const Entry &entry)
{
    static bool is_first = true;

    if (!is_first)
    {
        fprintf(of, "\n");
    }
    is_first = false;

    fprintf(of, "%s\n", entry.label.c_str());
    fprintf(of, "%s\n", entry.str.c_str()); // no escape of back slashes.
    fprintf(of, "END\n");
}

void write_entries_to_str(const string &ofname, const vector<Entry> &entries)
{
    FILE *fp = fopen(ofname.c_str(), "w");
    for (const Entry &e: entries)
        write_entry_to_str_as_is(fp, e);
    fclose(fp);
}

int main(int argc, const char *argv[])
{
    if (argc < 3)
    {
        show_usage();
        return 0;
    }
    if (argc < 4)
    {
        cout << "You only specified one input file, nothing to merge." << endl;
        return 0;
    }

    const char *ofname = argv[argc - 1];
    cout << "Primary file is " << argv[1] << endl;
    vector<Entry> main_entries = read_entries(argv[1], true);

    // to merge the STR entries while preserving order of entry appearance, we need to create a lookup table
    map<string, int> lut = make_lookup_table(main_entries);

    for (size_t i = 2 ; i < argc - 1 ; i++)
    {
        vector<Entry> more_entries = read_entries(argv[i], true);
        map<string, int> _lut = make_lookup_table(more_entries); // Just to check for duplicate entries in more_entries
        cout << "Merging " << argv[i] << endl;
        merge_entries(&main_entries, more_entries, &lut);
    }

    write_entries_to_str(ofname, main_entries);
    cout << "Merged as " << ofname << endl;
    return 0;
}
