#!/usr/bin/env python

"""Create host offload jump table text, if there are unresolved symbols."""

# $ Copyright Broadcom Corporation $
__id__ = '$Id: hoffload_link.py 697795 2017-05-05 02:51:21Z nisar $'
__url__ = '$URL: svn://bcawlan-svn.lvn.broadcom.net/svn/bcawlan/components/chips/scripts/branches/CHIPSSCRIPTS_BRANCH_17_100/hoffload_link.py $'
__version__ = '1.0'

#
# <<Broadcom-WL-IPTag/Proprietary:>>
#

import glob
import os
import re
import sys

import hoffload_common as common


class CreateJmptblUnresolvedSymbol(common.HoffloadApp):
    """Parse unresolved symbols from object files."""

    def __init__(self, *args, **kws):
        super(CreateJmptblUnresolvedSymbol, self).__init__(*args, **kws)
        self.symbol_list = []

    def fetch_unresolveds(self):
        """Parse an object file and gather unresolved symbols."""

        nm = glob.glob(os.path.join(os.path.dirname(
            self.args.tool_path_prefix), '*', 'bin', 'nm'))[0]
        for line in self.cm.run_cmd(nm, '-u', self.args.object):
            field = line.split()[1].strip()
            if field:
                self.symbol_list.append(field)

    def __call__(self):
        """Get symbols from the given object file."""

        self.fetch_unresolveds()
        map_lines = open(self.args.map).readlines()
        # Match symbols obtained from object file with the symbol
        # defined in the map file.
        text_bgn = [('\t' + line) for line in
                    '.thumb', '.syntax unified', '.align 2', '.text']
        text_func = []
        for symbol in self.symbol_list:
            for line in map_lines:
                if re.search(r'\W' + symbol + '$', line):
                    # only match functions in text section
                    if any(c in line.split()[1] for c in 'WTA'):
                        data = line.split()[0]
                        int_data = int(data.strip(), 16)
                        data = hex(int_data if int_data % 2 else int_data + 1)
                        text_bgn.append('\t.global\t%s' % symbol)
                        text_func.append('.thumb_func')
                        text_func.append('%s:' % symbol)
                        # jumptable must not alter any of the registers or the stack
                        # jumptable jumps to target function and target function returns to caller
                        text_func.extend([('\t' + line) for line in (
                            'push\t{r12,lr}\t/* save registers and lr */',
                            'ldr\tr12, =%s' % data,
                            'str\tr12,[sp,#4]\t/* overwrite lr on stack with the target function */',
                            'pop\t{r12,pc}\t/* restore registers and jump to target function */',
                            '.ltorg' )
                        ])
        with open(self.args.result, 'w') as fp:
            if text_func:
                fp.write('\n'.join(
                    [common.AUTO_GEN % sys.argv[0], ''] +
                    text_bgn + text_func + ['']))


class CommandLine(common.CommandLine):
    """Command-line methods."""

    def __init__(self):
        super(CommandLine, self).__init__()
        self.version = __version__
        self.idkw = __id__
        self.urlkw = __url__
        self.desc = __doc__
        self.app_class = CreateJmptblUnresolvedSymbol

    def set_options(self):
        super(CommandLine, self).set_options()
        parser = self.parser
        parser.add_argument('-o', '--object', required=True,
                            help='Offload module object file')
        parser.add_argument('-m', '--map', required=True,
                            help='Map file containing symbols and ' +
                            'their addresses.')

if __name__ == '__main__':
    sys.exit(CommandLine()())

# vim: ts=4:sw=4:tw=80:et
