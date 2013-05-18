#!/usr/bin/env python

import optparse
import os
import sys

import easy_template


def main(args):
  parser = optparse.OptionParser()
  parser.add_option('-o', dest='out')
  parser.add_option('-c', '--config')
  parser.add_option('-t', '--template')
  parser.add_option('-k', '--key', action='store_true')
  options, args = parser.parse_args(args)

  if not options.out:
    parser.error('Need output file (-o)')
  if not options.template:
    parser.error('Need template (-t)')
  if not options.config:
    parser.error('Need config (-c)')

  template_dict = {
    'config': options.config,
    'resources': args
  }
  template_dict['key'] = options.key
  easy_template.RunTemplateFile(options.template, options.out, template_dict)
  return 0

if __name__ == '__main__':
  sys.exit(main(sys.argv[1:]))
