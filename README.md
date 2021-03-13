# CsfStuff

CSF &lt;-> STR converter + Merger util for multiple STR files. These tools should work for all games from RA2 all the way to RA3.

## csf2str

Usage: ./csf2str INPUT.csf OUTPUT.str

* This will read INPUT.csf and convert it as OUTPUT.str.
* However, the STR file is not enough to fully reconstruct the CSF file. To prevent the information loss, csf2str will create two extra files.
  * meta.json contains string table's language information.
  * extra_data.json contains extra data tag attached to label entries in the string table. Extra data from ra2md.csf looks like the following:

```
  "VOX:aprotr1": "aprotr1e",
  "VOX:aprotr2": "aprotr2e",
  "VOX:aprotr3": "aprotr3e",
```

* These extra files will be used by str2csf to reconstruct the CSF file back intact.
