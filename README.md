# CsfStuff

CSF &lt;-> STR converter + Merger util for multiple STR files. These tools should work for all games from RA2 all the way to RA3.

## csf2str

```
Usage: ./csf2str INPUT.csf OUTPUT.str
```

* This will read INPUT.csf and convert it as OUTPUT.str.
* However, the STR file is not enough to fully reconstruct the CSF file. To prevent the information loss, csf2str will create two extra files.
  * meta.json contains string table's language information.
  * extra_data.json contains extra data tag attached to label entries in the string table. In case you are wondering what extra data is, those from ra2md.csf looks like the following:

```
  "VOX:aprotr1": "aprotr1e",
  "VOX:aprotr2": "aprotr2e",
  "VOX:aprotr3": "aprotr3e",
```

* These extra files will be used by str2csf to reconstruct the CSF file back intact.

## str2csf

```
Usage: ./str2csf INPUT.str OUTPUT.csf \[meta.json\] \[extra_data.json\]

BE SURE TO USE UTF-8 ENCODING FOR STR FILES
```

Converts input STR file into output CSF file.
To create the CSF file without any loss of information,
this program will use meta.json and optional extra_data.json if provided.

## merge_str

```
Usage ./merge_str str1 str2 ... strN output.str
```

For modders and translators, merge_str will merge multiple STR files into one.
Note that order matters!
Later str files will overwrite on top of previous str files.
The final command line argument is considers as the output file name.
