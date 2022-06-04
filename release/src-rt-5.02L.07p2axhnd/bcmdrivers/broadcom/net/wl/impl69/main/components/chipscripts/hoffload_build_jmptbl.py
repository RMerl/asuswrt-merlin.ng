#!/usr/bin/env python

"""Generate host offload jump table assembly file."""

# $ Copyright Broadcom Corporation $
__id__ = '$Id: hoffload_build_jmptbl.py 697795 2017-05-05 02:51:21Z nisar $'
__url__ = '$URL: svn://bcawlan-svn.lvn.broadcom.net/svn/bcawlan/components/chips/scripts/branches/CHIPSSCRIPTS_BRANCH_17_100/hoffload_build_jmptbl.py $'
__version__ = '1.0'

#
# <<Broadcom-WL-IPTag/Proprietary:>>
#

import glob
import os
import re
import sys

import hoffload_common as common

SIZE_UINT32 = 4


class GenJumpTable(common.HoffloadApp):
    """Generate Jump Table."""

    def __init__(self, *args, **kws):
        super(GenJumpTable, self).__init__(*args, **kws)
        self.functions = []
        self.symbol_list = []
        self.cparser = common.HoffloadConfigParser(self.args.config,
                                                   enum=self.args.enum_file)

    def parse_object(self):
        """Parse an object file and gather symbols."""

        nm = glob.glob(os.path.join(os.path.dirname(
            self.args.tool_path_prefix), '*', 'bin', 'nm'))[0]
        for line in self.cm.run_cmd(nm, self.args.object):
            if 'T' in line:
                fields = line.split()
                self.symbol_list.append(' '.join((fields[0], fields[2])))

    def extract_functions(self):
        """Get a function from a multiline function prototype."""

        prev_line = cont_line = None

        for include in self.args.include:
            for line in open(include, 'r'):
                line = line.strip()
                if '(' in line:
                    if ')' in line:
                        line = line.rstrip(';')
                        self.functions.append(line)
                    else:
                        cont_line = True
                        prev_line = line
                        continue
                if cont_line:
                    if ')' in line:
                        line = line.rstrip(';')
                        self.functions.append(prev_line + line)
                        cont_line = False
                        prev_line = None
                    else:
                        prev_line += line

    def __call__(self):
        """Generate jump table code."""

        module_id = self.cparser.get_module_id(self.args.module)
        self.extract_functions()
        self.parse_object()

        # Compute the offset into the hoffload_area based on module location.
        module_index = self.cparser.get_module_index(self.args.module) * SIZE_UINT32

        # Match symbols obtained from object file with the function
        # prototypes in header file.
        func_table = {}
        for symbol_string in self.symbol_list:
            tokens = symbol_string.split()
            symbol = tokens[1].strip()
            for func in self.functions:
                values = range(2)
                # Strip extern, static etc. keywords from function proto.
                func = re.sub('extern|static', '', func)
                if re.search(r'\W' + symbol + r'\W', func):
                    values[1] = func.strip()
                    values[0] = tokens[0].lstrip('0')
                    value_data = 0
                    if values[0]:
                        value_data = int(values[0], 16)
                    # Set LSB to 1 for processor state change.
                    value = str(hex(value_data | 1))
                    values[0] = value
                    func_table[symbol] = values
        # Generate C shim file.
        text_bgn = [('\t' + line) for line in
                    '.thumb', '.syntax unified', '.align 2', '.text']
        text_func = []
        for key, values in func_table.iteritems():
            text_bgn.append('.global %s' % key)
            text_func.append('.thumb_func')
            text_func.append('%s:' % key)
            # jumptable must not alter any of the registers or the stack
            # jumptable jumps to target function and target function returns to caller
            text_func.extend([('\t' + line) for line in (
                'push\t{r11-r12,lr}\t/* save registers and lr */',
                'push\t{r0-r10,lr}',
                'mov\tr0,#%d\t/* pass module id */' % module_id,
                'bl\tbcm_hoffload_sync_load_module\t/* load module */',
                'pop\t{r0-r10,lr}',
                'ldr\tr12,=hoffload_area',
                'ldr\tr12,[r12]',
                'add\tr12,r12,#%d' % module_index,
                'ldr\tr12,[r12]',
                'ldr\tr11,=%s' % values[0],
                'add\tr12,r12,r11',
                'str\tr12,[sp,#8]\t/* overwrite lr on stack with the target function */',
                'pop\t{r11-r12,pc}\t/* restore registers and jump to target function */',
                '.ltorg',)
            ])
        with open(self.args.result, 'w') as fp:
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
        self.app_class = GenJumpTable

    def set_options(self):
        super(CommandLine, self).set_options()
        parser = self.parser
        parser.add_argument('-c', '--config', required=True,
                            help='Host offload configuration file')
        parser.add_argument('-m', '--module', required=True,
                            help='Offload module name')
        parser.add_argument(
            '-i', '--include', required=True, nargs='+',
            help='Header file with function prototypes of all ' +
            'functions exported by module.')
        parser.add_argument('-o', '--object', required=True,
                            help='Offload module object file')
        parser.add_argument('-e', '--enum-file', required=True,
                            help='Header file for enum of module types')


if __name__ == '__main__':
    sys.exit(CommandLine()())

# vim: ts=4:sw=4:tw=80:et
