#!/usr/bin/env python

"""Usage: {script_name} -c <config> -l <loader_script>

where config, host offload config file.
      loader_script, linker script file of loader (linker) commands.

      A new section is added after HOFFLOAD_SECTION in the linker script file,
      something like:

      hoffload_dev_addr = .;
      . += 10240;
      hoffload_dev_addr_end = .;
      """

# $ Copyright Broadcom Corporation $
__id__ = '$Id: hoffload_replace.py 697795 2017-05-05 02:51:21Z nisar $'
__url__ = '$URL: svn://bcawlan-svn.lvn.broadcom.net/svn/bcawlan/components/chips/scripts/branches/CHIPSSCRIPTS_BRANCH_17_100/hoffload_replace.py $'
__version__ = '1.0'

#
# <<Broadcom-WL-IPTag/Proprietary:>>
#

import sys

import hoffload_common as common

HOFFLOAD_SECTION = 'bss_end = .;'


class LinkerScriptUtil(common.HoffloadApp):
    """Util to replace TARGET_ARCH, TEXT_AREA in linker script template
       and insert hoffload speific memory region.
    """

    def __init__(self, *args, **kws):
        super(LinkerScriptUtil, self).__init__(*args, **kws)

    def __call__(self):
        """Replace hoffload placeholder(s) in loader template."""

        # Read config.
        cfg_parser = common.HoffloadConfigParser(self.args.config)
        hoffload_reg = {}
        try:
            hoffload_reg[common.HOFFLOAD_MEM_TAG] = cfg_parser.get_element(
                common.HOFFLOAD_MEM_TAG)
        except ValueError:
            raise common.Error(self.args.config +
                               ', %s not found\n' % common.HOFFLOAD_MEM_TAG)

        def format_hoffload_reg(reg_name, reg_size):
            """Format hoffload region.

            Result:
                reg_name = .;
                . = . + reg_size;
                reg_name_end = .;
            """
            return """
            %s = .;
            . = . + %s;
            %s_end = .;
            """ % (reg_name, reg_size, reg_name)

        found = False
        lines = open(self.args.ldsin, 'r').readlines()
        with open(self.args.ldsin, 'w') as out:
            for line in lines:
                out.write(line)
                if HOFFLOAD_SECTION in line:
                    found = True
                if found and '}' in line:
                    out.write(''.join(format_hoffload_reg(
                              common.HOFFLOAD_MEM_TAG,
                              hoffload_reg[common.HOFFLOAD_MEM_TAG])))
                    found = False


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
        parser.add_argument('-c', '--config', required=True,
                            help='Configuration file')
        parser.add_argument('-l', '--ldsin', required=True,
                            help='linker script file')


if __name__ == '__main__':
    sys.exit(CommandLine()())

# vim: ts=4:sw=4:tw=80:et
