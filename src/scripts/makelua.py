#!/usr/bin/env python

import os
import optparse
import sys

HEADER="""\
/**
* Copyright (c) 2006-2012 LOVE Development Team
* 
* This software is provided 'as-is', without any express or implied
* warranty.  In no event will the authors be held liable for any damages
* arising from the use of this software.
* 
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
  * 
  * 1. The origin of this software must not be misrepresented; you must not
  *    claim that you wrote the original software. If you use this software
  *    in a product, an acknowledgment in the product documentation would be
  *    appreciated but is not required.
  * 2. Altered source versions must be plainly marked as such, and must not be
  *    misrepresented as being the original software.
  * 3. This notice may not be removed or altered from any source distribution.
  **/

  namespace love
  {{

  // [{0}]
  const unsigned char {1}[] = 
  {{
"""

FOOTER="""\
}}; // [{0}]
}} // love
"""


def main(args):
  parser = optparse.OptionParser()
  options, args = parser.parse_args(args)

  infname = args[0]
  outfname = args[0] + '.h'

  with open(infname) as inf:
    data = inf.read()

  base_infname = os.path.basename(infname)
  output = HEADER.format(base_infname, base_infname.replace('.', '_'))
  while data:
    line = data[:12]
    data = data[12:]
    output += '\t%s,\n' % ', '.join('0x%02x' % ord(x) for x in line)

  output += FOOTER.format(base_infname)

  with open(outfname, 'w') as outf:
    outf.write(output)


if __name__ == '__main__':
  sys.exit(main(sys.argv[1:]))

