import sys

data = open(sys.argv[1], "rb").read()
data = data.rstrip(b'\x00')
open(sys.argv[2], "wt").write(','.join([str(c) for c in data]))