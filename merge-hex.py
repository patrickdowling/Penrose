#!/usr/bin/env python3

import argparse
import sys
try:
    from intelhex import IntelHex
except ImportError:
    sys.exit('Error: IntelHex module not found '
             '<https://pypi.org/project/IntelHex/>')

class MergeHex:

    def __init__(self, *args):
        self._ihex = IntelHex()
        for ihex in args:
            self.add(ihex)

    def add(self, ihex):
        self._ihex.merge(ihex, overlap='replace')

    def merged(self):
        return self._ihex

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('-o', dest='output_file', default='-',
        help='Output hex file (default stdout)')
    parser.add_argument('-i', dest='input_file', required=True, nargs='+',
        help='Input hex files')
    args = parser.parse_args()

    if args.output_file == '-':
        args.output_file = sys.stdout

    ihex = map(IntelHex, args.input_file)
    ihex = MergeHex(*ihex).merged()

    try:
        ihex.write_hex_file(args.output_file, False)
    except BrokenPipeError:
        pass

if __name__ == '__main__':
    main()
