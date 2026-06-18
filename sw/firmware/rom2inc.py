import sys

data = open(sys.argv[1], "rb").read()
open(sys.argv[2], "wt").write(','.join([str(c) for c in data]))