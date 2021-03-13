#!/usr/bin/env python
"""
After building the project, you can run python nosetests.
Just install nosetests then run nosetest command to run the tests.
"""

import os
import tempfile
from pathlib import Path

ORIGINAL_CWD = Path(os.getcwd())
MERGE_STR = Path("build/merge_str").absolute()
assert MERGE_STR.exists(), "str_merger is not compiled."


def test_2merge_ab():
    """
    CSF reconstruction test, merge 2 files. a <- b case.
    """
    input_a = (ORIGINAL_CWD / "samples/a.str").absolute()
    input_b = (ORIGINAL_CWD / "samples/b.str").absolute()
    outf = "xxx.str"
    assert input_a.exists()
    assert input_b.exists()

    with tempfile.TemporaryDirectory() as tmpd:
        os.chdir(tmpd)

        ret = os.system(f'"{MERGE_STR}" "{input_a}" "{input_b}" {outf}')
        assert ret == 0

        with open(outf) as f:
            lines = f.readlines()

        assert lines[1].rstrip() != '"This will be overwritten"'
        assert lines[5].rstrip() == '"This will stay as is."'
        assert lines[17].rstrip() == '"Cyborg Ninja Pirate Mecha Death Tanya, this is a new entry created by the merge"'
        os.chdir(ORIGINAL_CWD)

    print("Passed a <- b merge")


def test_2merge_ba():
    """
    CSF reconstruction test, merge 2 files. b <- a case.
    """
    input_a = (ORIGINAL_CWD / "samples/a.str").absolute()
    input_b = (ORIGINAL_CWD / "samples/b.str").absolute()
    outf = "xxx.str"
    assert input_a.exists()
    assert input_b.exists()

    with tempfile.TemporaryDirectory() as tmpd:
        os.chdir(tmpd)

        ret = os.system(f'"{MERGE_STR}" "{input_b}" "{input_a}" {outf}')
        assert ret == 0

        with open(outf) as f:
            lines = f.readlines()

        assert lines[1].rstrip() == '"This will be overwritten"'
        assert lines[5].rstrip() != '"This will stay as is."'
        assert lines[13].rstrip() == '"Cyborg Ninja Pirate Mecha Death Tanya, this is a new entry created by the merge"'
        os.chdir(ORIGINAL_CWD)

    print("Passed b <- a merge")


def test_3merge():
    """
    CSF reconstruction test, merge 3 files.
    """
    input_a = (ORIGINAL_CWD / "samples/a.str").absolute()
    input_b = (ORIGINAL_CWD / "samples/b.str").absolute()
    input_c = (ORIGINAL_CWD / "samples/c.str").absolute()
    outf = "xxx.str"
    assert input_a.exists()
    assert input_b.exists()
    assert input_c.exists()

    with tempfile.TemporaryDirectory() as tmpd:
        os.chdir(tmpd)

        ret = os.system(f'"{MERGE_STR}" "{input_a}" "{input_b}" "{input_c}" {outf}')
        assert ret == 0

        with open(outf) as f:
            lines = f.readlines()

        assert lines[1].rstrip() != '"This will be overwritten"'
        assert lines[5].rstrip() == '"This will stay as is."'
        assert lines[17].rstrip() == '"Cyborg Ninja Pirate Mecha Death Tanya, this is a new entry created by the merge"'
        assert lines[21].rstrip() == '"Nod Cyborg Commando, newly created by another merge"'
        os.chdir(ORIGINAL_CWD)

    print("Passed a <- b merge")


if __name__ == "__main__":
    test_2merge_ab()
