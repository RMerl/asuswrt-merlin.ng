# a waf tool to add extension based build patterns for Samba

import Task
from TaskGen import extension
from samba_utils import *
from wafsamba import samba_version_file

def write_version_header(task):
    '''print version.h contents'''
    src = task.inputs[0].srcpath(task.env)
    tgt = task.outputs[0].bldpath(task.env)

    version = samba_version_file(src, task.env.srcdir, env=task.env, is_install=task.env.is_install)
    string = str(version)

    f = open(tgt, 'w')
    s = f.write(string)
    f.close()
    return 0


def SAMBA_MKVERSION(bld, target):
    '''generate the version.h header for Samba'''

    # We only force waf to re-generate this file if we are installing,
    # because only then is information not included in the deps (the
    # git revision) included in the version.
    t = bld.SAMBA_GENERATOR('VERSION',
                            rule=write_version_header,
                            source= 'VERSION',
                            target=target,
                            always=bld.is_install)
    t.env.is_install = bld.is_install
Build.BuildContext.SAMBA_MKVERSION = SAMBA_MKVERSION


def write_build_options_header(fp):
    '''write preamble for build_options.c'''
    fp.write("/*\n")
    fp.write("   Unix SMB/CIFS implementation.\n")
    fp.write("   Build Options for Samba Suite\n")
    fp.write("   Copyright (C) Vance Lankhaar <vlankhaar@linux.ca> 2003\n")
    fp.write("   Copyright (C) Andrew Bartlett <abartlet@samba.org> 2001\n")
    fp.write("\n")
    fp.write("   This program is free software; you can redistribute it and/or modify\n")
    fp.write("   it under the terms of the GNU General Public License as published by\n")
    fp.write("   the Free Software Foundation; either version 3 of the License, or\n")
    fp.write("   (at your option) any later version.\n")
    fp.write("\n")
    fp.write("   This program is distributed in the hope that it will be useful,\n")
    fp.write("   but WITHOUT ANY WARRANTY; without even the implied warranty of\n")
    fp.write("   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n")
    fp.write("   GNU General Public License for more details.\n")
    fp.write("\n")
    fp.write("   You should have received a copy of the GNU General Public License\n")
    fp.write("   along with this program; if not, see <http://www.gnu.org/licenses/>.\n")
    fp.write("*/\n")
    fp.write("\n")
    fp.write("#include \"includes.h\"\n")
    fp.write("#include \"build_env.h\"\n")
    fp.write("#include \"dynconfig/dynconfig.h\"\n")
    fp.write("#include \"lib/cluster_support.h\"\n")

    fp.write("\n")
    fp.write("static int output(bool screen, const char *format, ...) PRINTF_ATTRIBUTE(2,3);\n")
    fp.write("void build_options(bool screen);\n")
    fp.write("\n")
    fp.write("\n")
    fp.write("/****************************************************************************\n")
    fp.write("helper function for build_options\n")
    fp.write("****************************************************************************/\n")
    fp.write("static int output(bool screen, const char *format, ...)\n")
    fp.write("{\n")
    fp.write("       char *ptr = NULL;\n")
    fp.write("       int ret = 0;\n")
    fp.write("       va_list ap;\n")
    fp.write("       \n")
    fp.write("       va_start(ap, format);\n")
    fp.write("       ret = vasprintf(&ptr,format,ap);\n")
    fp.write("       va_end(ap);\n")
    fp.write("\n")
    fp.write("       if (screen) {\n")
    fp.write("              d_printf(\"%s\", ptr ? ptr : \"\");\n")
    fp.write("       } else {\n")
    fp.write("              DEBUG(4,(\"%s\", ptr ? ptr : \"\"));\n")
    fp.write("       }\n")
    fp.write("       \n")
    fp.write("       SAFE_FREE(ptr);\n")
    fp.write("       return ret;\n")
    fp.write("}\n")
    fp.write("\n")
    fp.write("/****************************************************************************\n")
    fp.write("options set at build time for the samba suite\n")
    fp.write("****************************************************************************/\n")
    fp.write("void build_options(bool screen)\n")
    fp.write("{\n")
    fp.write("       if ((DEBUGLEVEL < 4) && (!screen)) {\n")
    fp.write("              return;\n")
    fp.write("       }\n")
    fp.write("\n")
    fp.write("#ifdef _BUILD_ENV_H\n")
    fp.write("       /* Output information about the build environment */\n")
    fp.write("       output(screen,\"Build environment:\\n\");\n")
    fp.write("       output(screen,\"   Built by:    %s@%s\\n\",BUILD_ENV_USER,BUILD_ENV_HOST);\n")
    fp.write("       output(screen,\"   Built on:    %s\\n\",BUILD_ENV_DATE);\n")
    fp.write("\n")
    fp.write("       output(screen,\"   Built using: %s\\n\",BUILD_ENV_COMPILER);\n")
    fp.write("       output(screen,\"   Build host:  %s\\n\",BUILD_ENV_UNAME);\n")
    fp.write("       output(screen,\"   SRCDIR:      %s\\n\",BUILD_ENV_SRCDIR);\n")
    fp.write("       output(screen,\"   BUILDDIR:    %s\\n\",BUILD_ENV_BUILDDIR);\n")
    fp.write("\n")
    fp.write("\n")
    fp.write("#endif\n")
    fp.write("\n")
    fp.write("       /* Output various paths to files and directories */\n")
    fp.write("       output(screen,\"\\nPaths:\\n\");\n")
    fp.write("       output(screen,\"   SBINDIR: %s\\n\", get_dyn_SBINDIR());\n")
    fp.write("       output(screen,\"   BINDIR: %s\\n\", get_dyn_BINDIR());\n")
    fp.write("       output(screen,\"   CONFIGFILE: %s\\n\", get_dyn_CONFIGFILE());\n")
    fp.write("       output(screen,\"   LOGFILEBASE: %s\\n\", get_dyn_LOGFILEBASE());\n")
    fp.write("       output(screen,\"   LMHOSTSFILE: %s\\n\",get_dyn_LMHOSTSFILE());\n")
    fp.write("       output(screen,\"   LIBDIR: %s\\n\",get_dyn_LIBDIR());\n")
    fp.write("       output(screen,\"   MODULESDIR: %s\\n\",get_dyn_MODULESDIR());\n")
    fp.write("       output(screen,\"   SHLIBEXT: %s\\n\",get_dyn_SHLIBEXT());\n")
    fp.write("       output(screen,\"   LOCKDIR: %s\\n\",get_dyn_LOCKDIR());\n")
    fp.write("       output(screen,\"   STATEDIR: %s\\n\",get_dyn_STATEDIR());\n")
    fp.write("       output(screen,\"   CACHEDIR: %s\\n\",get_dyn_CACHEDIR());\n")
    fp.write("       output(screen,\"   PIDDIR: %s\\n\", get_dyn_PIDDIR());\n")
    fp.write("       output(screen,\"   SMB_PASSWD_FILE: %s\\n\",get_dyn_SMB_PASSWD_FILE());\n")
    fp.write("       output(screen,\"   PRIVATE_DIR: %s\\n\",get_dyn_PRIVATE_DIR());\n")
    fp.write("\n")

def write_build_options_footer(fp):
    fp.write("       /* Output the sizes of the various cluster features */\n")
    fp.write("       output(screen, \"\\n%s\", cluster_support_features());\n")
    fp.write("\n")
    fp.write("       /* Output the sizes of the various types */\n")
    fp.write("       output(screen, \"\\nType sizes:\\n\");\n")
    fp.write("       output(screen, \"   sizeof(char):         %lu\\n\",(unsigned long)sizeof(char));\n")
    fp.write("       output(screen, \"   sizeof(int):          %lu\\n\",(unsigned long)sizeof(int));\n")
    fp.write("       output(screen, \"   sizeof(long):         %lu\\n\",(unsigned long)sizeof(long));\n")
    fp.write("#if HAVE_LONGLONG\n")
    fp.write("       output(screen, \"   sizeof(long long):    %lu\\n\",(unsigned long)sizeof(long long));\n")
    fp.write("#endif\n")
    fp.write("       output(screen, \"   sizeof(uint8):        %lu\\n\",(unsigned long)sizeof(uint8));\n")
    fp.write("       output(screen, \"   sizeof(uint16):       %lu\\n\",(unsigned long)sizeof(uint16));\n")
    fp.write("       output(screen, \"   sizeof(uint32):       %lu\\n\",(unsigned long)sizeof(uint32));\n")
    fp.write("       output(screen, \"   sizeof(short):        %lu\\n\",(unsigned long)sizeof(short));\n")
    fp.write("       output(screen, \"   sizeof(void*):        %lu\\n\",(unsigned long)sizeof(void*));\n")
    fp.write("       output(screen, \"   sizeof(size_t):       %lu\\n\",(unsigned long)sizeof(size_t));\n")
    fp.write("       output(screen, \"   sizeof(off_t):        %lu\\n\",(unsigned long)sizeof(off_t));\n")
    fp.write("       output(screen, \"   sizeof(ino_t):        %lu\\n\",(unsigned long)sizeof(ino_t));\n")
    fp.write("       output(screen, \"   sizeof(dev_t):        %lu\\n\",(unsigned long)sizeof(dev_t));\n")
    fp.write("\n")
    fp.write("       output(screen, \"\\nBuiltin modules:\\n\");\n")
    fp.write("       output(screen, \"   %s\\n\", STRING_STATIC_MODULES);\n")
    fp.write("}\n")

def write_build_options_section(fp, keys, section):
    fp.write("\n\t/* Show %s */\n" % section)
    fp.write("       output(screen, \"\\n%s:\\n\");\n\n" % section)

    for k in sorted(keys):
        fp.write("#ifdef %s\n" % k)
        fp.write("       output(screen, \"   %s\\n\");\n" % k)
        fp.write("#endif\n")
    fp.write("\n")

def write_build_options(task):
    tbl = task.env['defines']
    keys_option_with = []
    keys_option_utmp = []
    keys_option_have = []
    keys_header_sys = []
    keys_header_other = []
    keys_misc = []
    for key in tbl:
        if key.startswith("HAVE_UT_UT_") or key.find("UTMP") >= 0:
            keys_option_utmp.append(key)
        elif key.startswith("WITH_"):
            keys_option_with.append(key)
        elif key.startswith("HAVE_SYS_"):
            keys_header_sys.append(key)
        elif key.startswith("HAVE_"):
            if key.endswith("_H"):
                keys_header_other.append(key)
            else:
                keys_option_have.append(key)
        else:
            keys_misc.append(key)

    tgt = task.outputs[0].bldpath(task.env)
    f = open(tgt, 'w')
    write_build_options_header(f)
    write_build_options_section(f, keys_header_sys, "System Headers")
    write_build_options_section(f, keys_header_other, "Headers")
    write_build_options_section(f, keys_option_utmp, "UTMP Options")
    write_build_options_section(f, keys_option_have, "HAVE_* Defines")
    write_build_options_section(f, keys_option_with, "--with Options")
    write_build_options_section(f, keys_misc, "Build Options")
    write_build_options_footer(f)
    f.close()
    return 0


def SAMBA_BLDOPTIONS(bld, target):
    '''generate the bld_options.c for Samba'''
    t = bld.SAMBA_GENERATOR(target,
                            rule=write_build_options,
                            target=target,
                            always=True)
Build.BuildContext.SAMBA_BLDOPTIONS = SAMBA_BLDOPTIONS
