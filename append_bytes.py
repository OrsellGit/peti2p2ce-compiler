# This script is needed for post-compile in order to append bytes to the EXEs that PyInstaller executables have in order for BEEMod not to replace these compilers.

import pathlib, sys

with pathlib.Path(sys.argv[1]).open("ab") as f:
    f.write(bytes.fromhex("4D45490C0B0A0B0E"))
    f.close();
