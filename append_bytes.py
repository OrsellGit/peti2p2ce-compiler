import pathlib, sys

with pathlib.Path(sys.argv[1]).open("ab") as f:
    f.write(bytes.fromhex("4D45490C0B0A0B0E"))
    f.close();
