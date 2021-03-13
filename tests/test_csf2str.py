#!/usr/bin/env python
"""
After building the project, you can run python nosetests.
Just install nosetests then run nosetest command to run the tests.
"""

import os
import json


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
 

def test_str_generation():
    """
    Check if str file is correctly created.
    """
    os.system("build/csf2str samples/gamestrings.csf xxx.str")

    check_str('xxx.str')
   
    with open('meta.json') as f:
        _x = json.load(f)
    print('meta.json created')


def test_extra_data_generation():
    os.system("build/csf2str samples/ra2md.csf xxx.str")

    check_str('xxx.str')

    with open('extra_data.json') as f:
        _x = json.load(f)
    print('extra_data.json created')


if __name__ == "__main__":
    test_str_generation()