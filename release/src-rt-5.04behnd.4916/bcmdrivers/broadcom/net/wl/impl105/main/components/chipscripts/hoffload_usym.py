#!/usr/bin/env python

"""Usage: {script_name} -l <loader_script> <modules>

where loader_script, linker script file of loader (linker) commands. This file
also generates a file containing symbols exported from all the modules.

      A new set of functions are added after text.debugptr
      something like:

      KEEP(*(.text.wlc_xyz))
      """

# $ Copyright Broadcom Corporation $
__id__ = '$Id: hoffload_replace.py 666337 2016-10-20 20:37:28Z anchakra $'
__url__ = '$URL: http://svn.sj.broadcom.com/svn/wlansvn/proj/branches/HMLSCRPT_BRANCH_1_1/hoffload_usym.py $'
__version__ = '1.0'

import hoffload_common as common
import glob
import os
import sys

HOFFLOAD_SYMBOL_TEXT = 'KEEP(*(.text.debugptr))'
LIBS = ['bcmstdlib.o', 'event_log.o']
GEN_SYMBOLS = ['__bss_end__', '_edata', '_end', '__bss_start__', '__bss_start',
    '__end__', '__data_start', '_bss_end__']

class LinkerScriptUtil(common.HoffloadApp):
    """Util to replace TARGET_ARCH, TEXT_AREA in linker script template
       and insert hoffload speific memory region.
    """

    def __init__(self, *args, **kws):
        super(LinkerScriptUtil, self).__init__(*args, **kws)
        self.symbol_list = []
        self.nm = glob.glob(os.path.join(os.path.dirname(
            self.args.tool_path_prefix), '*', 'bin', 'nm'))[0]

    def get_symbols(self, modules):
        symbols = []
        for lib in modules:
            for symbol in self.cm.run_cmd(self.nm, '--defined-only', lib):
                sym_type,symbol = symbol.split()[1:]
                if sym_type.upper() == 'T' and symbol not in GEN_SYMBOLS:
                    symbols.append(symbol)
        symbols = list(set(symbols))
        return symbols

    def fetch_unresolveds(self):
        """Parse an object file and gather unresolved symbols."""

        # gather symbols from bcmstdlib.o
        libsyms = self.get_symbols(LIBS)
        #gather symbols from each module objects
        list1 = dict()
        for module in self.args.modules:
       	    for line in self.cm.run_cmd(self.nm, '-u', module):
                field = line.split()[1].strip()
                if '_start' == field:
                    continue
                else:
                    if field and field not in list1.keys():
                        list1[field] = field
        self.symbol_list += list1.keys()

    def __call__(self):
        """Replace hoffload placeholder(s) in loader template."""

        def format_hoffload_symbol(symbols):
            text = ''
            for symbol in symbols:
                text += '\t\tKEEP(*(.text.%s))\n' %symbol
            return text

        self.fetch_unresolveds()
        hoffload_symbol_notadded = True
        lines = open(self.args.ldsin, 'r').readlines()
        with open(self.args.ldsin, 'w') as out:
            for line in lines:
                out.write(line)
                if hoffload_symbol_notadded:
                    if HOFFLOAD_SYMBOL_TEXT in line:
                        out.write(''.join(format_hoffload_symbol(
                            self.symbol_list)))
                        hoffload_symbol_notadded = False
        symout =''
        with open(self.args.weaken, 'w') as wout:
            for symbol in self.get_symbols(self.args.modules):
                symout += symbol + '\n'
            wout.write(symout)


class CommandLine(common.CommandLine):
    """ wrapper """

    def __init__(self):
        super(CommandLine, self).__init__()
        self.version = __version__
        self.idkw = __id__
        self.urlkw = __url__
        self.desc = __doc__
        self.app_class = LinkerScriptUtil

    def set_options(self):
        super(CommandLine, self).set_options()
        parser = self.parser
        parser.add_argument('-l', '--ldsin', required=True,
                            help='linker script file')
        parser.add_argument('-w', '--weaken', required=True,
                            help='file containing list of module symbols')
        parser.add_argument('modules', nargs='+',
                            help='Modules to be resolved')


if __name__ == '__main__':
    sys.exit(CommandLine()())

# vim: ts=4:sw=4:tw=80:et
