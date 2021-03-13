#!/usr/bin/env python
"""
After building the project, you can run python nosetests.
Just install nosetests then run nosetest command to run the tests.
"""

import os
import tempfile
from pathlib import Path

ORIGINAL_CWD = Path(os.getcwd())
CSF2STR = Path("build/csf2str").absolute()
STR2CSF = Path("build/str2csf").absolute()
assert CSF2STR.exists(), "csf2str is not compiled."
assert STR2CSF.exists(), "str2csf is not compiled."


def test_no_extra_data():
    """
    CSF reconstruction test, without extra data
    """
    input_csf = (ORIGINAL_CWD / "samples/gamestrings.csf").absolute()  # has no extra data
    strf = "xxx.str"
    ocsvf = "yyy.csf"
    assert input_csf.exists()

    with tempfile.TemporaryDirectory() as tmpd:
        os.chdir(tmpd)

        ret = os.system(f'"{CSF2STR}" "{input_csf}" {strf}')  # meta.json gets created
        assert ret == 0

        os.system(f'"{STR2CSF}" "{strf}" {ocsvf}')  # from str file and meta.json, recreate the CSF file.
        assert ret == 0

        ret = os.system(f'diff "{input_csf}" {ocsvf}')  # check if reconstructed CSF == input CSF.
        assert ret == 0

        os.chdir(ORIGINAL_CWD)

    print("Passed csf file creation, no extra_data case.")


def test_with_extra_data():
    """
    CSF reconstruction test, with extra data
    """
    input_csf = (ORIGINAL_CWD / "samples/ra2md.csf").absolute()  # has no extra data
    strf = "xxx.str"
    ocsvf = "yyy.csf"
    assert input_csf.exists()

    with tempfile.TemporaryDirectory() as tmpd:
        os.chdir(tmpd)

        ret = os.system(f'"{CSF2STR}" "{input_csf}" {strf}')  # meta.json gets created
        assert ret == 0

        os.system(f'"{STR2CSF}" "{strf}" {ocsvf} meta.json extra_data.json')  # from str file and meta.json, recreate the CSF file.
        assert ret == 0

        ret = os.system(f'diff "{input_csf}" {ocsvf}')  # check if reconstructed CSF == input CSF.
        assert ret == 0

        os.chdir(ORIGINAL_CWD)

    print("Passed csf file creation, no extra_data case.")


if __name__ == "__main__":
    test_no_extra_data()
