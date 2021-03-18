# CsfStuff

CSF &lt;-> STR converter + Merger util for multiple STR files. These tools should work for all games from RA2 all the way to RA3.

## csf2str

```
Usage: ./csf2str INPUT.csf OUTPUT.str
```

* This will read INPUT.csf and convert it to OUTPUT.str.
* However, the STR file is not enough to fully reconstruct the CSF file. To prevent information loss, csf2str will create one extra file.
  * extra_data.json contains extra data tags attached to label entries in the string table. In case you are wondering what extra data is, those from ra2md.csf look like the following:

```
  "VOX:aprotr1": "aprotr1e",
  "VOX:aprotr2": "aprotr2e",
  "VOX:aprotr3": "aprotr3e",
```
* The emitted STR file will contain an extra entry that looks like the following:
```
CSFSTUFF:META
"{\"lang_code\":0,\"unused\":0}"
END
```
* This extra entry will be read by str2csf but will not appear in the final CSF file.
* With extra entry and extra_data.json combined, CSF file can be reconstructed with zero loss of information.

## str2csf

```
Usage: ./str2csf INPUT.str OUTPUT.csf [extra_data.json]

BE SURE TO USE UTF-8 ENCODING FOR STR FILES
```

Converts input STR file into output CSF file.
To create the CSF file without any loss of information,
this program will use optional extra_data.json if provided.
CSFSTUFF:META will be read, if exists.
If not, the CSF file will get lang_code=0, unused=0 by default.
CSFSTUFF:META will be only used by str2csf and will not appear in your final CSF file,
hence don't name your unit as CSFSTUFF:META   :)

## merge_str

```
Usage ./merge_str str1 str2 ... strN output.str
```

For modders and translators, merge_str will merge multiple STR files into one.
Note that order matters!
Later str files will overwrite on top of previous str files.
The final command line argument is considered as the output file name.

## Build instructions for developers

* On Linux, just type make.
* Makefile will run cmake for you.
* Or if you want to use cmake directly,
```
mkdir build
cd build
cmake ..
make
```
* On Windows, open this project folder with Visual Studio (Tested with VS2019).
* Visual Studio will detect CMakeLists.txt and will compile stuff for you.
