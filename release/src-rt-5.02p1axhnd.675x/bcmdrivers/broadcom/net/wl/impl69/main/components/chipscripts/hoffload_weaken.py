#!/usr/bin/env python
"""Usage: {script_name} -w <weaken_file> <objects>

where, weaken_file contains symbols to be wekened to the objects.
"""

import glob
import os
import sys

import hoffload_common as common

class WeakenObj(common.HoffloadApp):

    def __call__(self):

        objcopy = glob.glob(os.path.join(os.path.dirname(
                  self.args.tool_path_prefix), '*', 'bin', 'objcopy'))[0]
        nm = glob.glob(os.path.join(os.path.dirname(
                  self.args.tool_path_prefix), '*', 'bin', 'nm'))[0]
        args = '--weaken-symbols=%s' %self.args.weaken
        with open(self.args.weaken, 'r') as fin:
            weak_symbols = [line.strip('\n').strip() for line in fin.readlines()]
        objects = []
        for obj in self.args.objects:
            for symbol_str in self.cm.run_cmd(nm, '--defined-only', obj):
                symtype,symbol = symbol_str.split()[1:]
                if symtype.upper() == 'T' and symbol in weak_symbols:
                    objects.append(obj)
                    break
        for obj in objects:
            print 'Run ', objcopy, args, obj
            self.cm.run_cmd(objcopy, args, obj)


class CommandLine(common.CommandLine):

    def __init__(self):
        super(CommandLine, self).__init__()
        self.app_class = WeakenObj

    def set_options(self):
        super(CommandLine, self).set_options()
        parser = self.parser
        parser.add_argument('-w', '--weaken', required=True,
                            help='file containing list of module symbols')
        parser.add_argument('objects', nargs='+',
                            help='Objects to be weakened')

if __name__ == '__main__':
    sys.exit(CommandLine()())

