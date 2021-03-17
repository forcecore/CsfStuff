#!/usr/bin/env python
"""
After building the project, you can run python nosetests.
Just install nosetests then run nosetest command to run the tests.
"""

import json
import os
import tempfile
from pathlib import Path

ORIGINAL_CWD = Path(os.getcwd())
CSF2STR = Path("build/csf2str").absolute()
assert CSF2STR.exists(), "csf2str is not compiled."


def check_str(fname):
    """
    Check the format of the given str file.
    """
    with open(fname) as f:
        for i, line in enumerate(f):
            line = line.rstrip()
            if i % 4 == 0:
                assert line != ""
            elif i % 4 == 1:
                assert line.startswith('"')
                assert line.endswith('"')
            elif i % 4 == 2:
                assert line == "END"
            else:
                assert line == ""
    print(f'{fname} check complete')


def check_metadata(fname):
    with open('xxx.str') as f:
        lines = f.readlines()
    return json.loads(lines[1])


def test_str_generation():
    """
    STR file generation check, without extra_data
    """
    input_csf = (ORIGINAL_CWD / "samples/gamestrings.csf").absolute()  # has no extra data
    assert input_csf.exists(), "csf2str is not compiled."

    with tempfile.TemporaryDirectory() as tmpd:
        os.chdir(tmpd)
        os.system(f'"{CSF2STR}" "{input_csf}" xxx.str')

        check_str('xxx.str')
        check_metadata('xxx.str')
        print('meta data is OK')

        os.chdir(ORIGINAL_CWD)


def test_extra_data_generation():
    """
    STR file generation check, with extra_data
    """
    input_csf = (ORIGINAL_CWD / "samples/ra2md.csf").absolute()  # has extra data
    assert input_csf.exists(), "csf2str is not compiled."

    with tempfile.TemporaryDirectory() as tmpd:
        os.chdir(tmpd)
        os.system(f'"{CSF2STR}" "{input_csf}" xxx.str')

        check_str('xxx.str')

        check_metadata('xxx.str')
        print('meta data is OK')

        with open('extra_data.json') as f:
            _x = json.load(f)
        print('extra_data.json created')

        os.chdir(ORIGINAL_CWD)


if __name__ == "__main__":
    test_str_generation()
