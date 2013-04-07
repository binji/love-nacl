#!/usr/bin/env python

import os
import optparse
import sys

def main(args):
  parser = optparse.OptionParser()
  options, args = parser.parse_args(args)

  infname = args[0]
  outfname = os.path.splitext(infname)[0]

  with open(infname) as inf:
    data = inf.read()
  first_brace = data.find('{')
  second_brace = data.find('{', first_brace + 1)
  close_brace = data.find('}', second_brace)
  l = eval ('[%s]' % data[second_brace+1:close_brace])
  with open(outfname, 'w') as outf:
    outf.write(''.join(chr(x) for x in l))


if __name__ == '__main__':
  sys.exit(main(sys.argv[1:]))
