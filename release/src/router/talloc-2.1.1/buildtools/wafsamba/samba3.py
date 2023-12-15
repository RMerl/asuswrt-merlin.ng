# a waf tool to add autoconf-like macros to the configure section
# and for SAMBA_ macros for building libraries, binaries etc

import Options, Build, os
from optparse import SUPPRESS_HELP
from samba_utils import os_path_relpath, TO_LIST
from samba_autoconf import library_flags

def SAMBA3_ADD_OPTION(opt, option, help=(), dest=None, default=True,
                      with_name="with", without_name="without"):
    if default is None:
        default_str="auto"
    elif default == True:
        default_str="yes"
    elif default == False:
        default_str="no"
    else:
        default_str=str(default)

    if help == ():
        help = ("Build with %s support (default=%s)" % (option, default_str))
    if dest is None:
        dest = "with_%s" % option.replace('-', '_')

    with_val = "--%s-%s" % (with_name, option)
    without_val = "--%s-%s" % (without_name, option)

    #FIXME: This is broken and will always default to "default" no matter if
    # --with or --without is chosen.
    opt.add_option(with_val, help=help, action="store_true", dest=dest,
                   default=default)
    opt.add_option(without_val, help=SUPPRESS_HELP, action="store_false",
                   dest=dest)
Options.Handler.SAMBA3_ADD_OPTION = SAMBA3_ADD_OPTION

def SAMBA3_IS_STATIC_MODULE(bld, module):
    '''Check whether module is in static list'''
    if module in bld.env['static_modules']:
        return True
    return False
Build.BuildContext.SAMBA3_IS_STATIC_MODULE = SAMBA3_IS_STATIC_MODULE

def SAMBA3_IS_SHARED_MODULE(bld, module):
    '''Check whether module is in shared list'''
    if module in bld.env['shared_modules']:
        return True
    return False
Build.BuildContext.SAMBA3_IS_SHARED_MODULE = SAMBA3_IS_SHARED_MODULE

def SAMBA3_IS_ENABLED_MODULE(bld, module):
    '''Check whether module is in either shared or static list '''
    return SAMBA3_IS_STATIC_MODULE(bld, module) or SAMBA3_IS_SHARED_MODULE(bld, module)
Build.BuildContext.SAMBA3_IS_ENABLED_MODULE = SAMBA3_IS_ENABLED_MODULE



def s3_fix_kwargs(bld, kwargs):
    '''fix the build arguments for s3 build rules to include the
    necessary includes, subdir and cflags options '''
    s3dir = os.path.join(bld.env.srcdir, 'source3')
    s3reldir = os_path_relpath(s3dir, bld.curdir)

    # the extra_includes list is relative to the source3 directory
    extra_includes = [ '.', 'include', 'lib', '../lib/tdb_compat' ]
    # local heimdal paths only included when USING_SYSTEM_KRB5 is not set
    if not bld.CONFIG_SET("USING_SYSTEM_KRB5"):
        extra_includes += [ '../source4/heimdal/lib/com_err',
                            '../source4/heimdal/lib/krb5',
                            '../source4/heimdal/lib/gssapi',
                            '../source4/heimdal_build',
                            '../bin/default/source4/heimdal/lib/asn1' ]

    if bld.CONFIG_SET('USING_SYSTEM_TDB'):
        (tdb_includes, tdb_ldflags, tdb_cpppath) = library_flags(bld, 'tdb')
        extra_includes += tdb_cpppath
    else:
        extra_includes += [ '../lib/tdb/include' ]

    if bld.CONFIG_SET('USING_SYSTEM_TEVENT'):
        (tevent_includes, tevent_ldflags, tevent_cpppath) = library_flags(bld, 'tevent')
        extra_includes += tevent_cpppath
    else:
        extra_includes += [ '../lib/tevent' ]

    if bld.CONFIG_SET('USING_SYSTEM_TALLOC'):
        (talloc_includes, talloc_ldflags, talloc_cpppath) = library_flags(bld, 'talloc')
        extra_includes += talloc_cpppath
    else:
        extra_includes += [ '../lib/talloc' ]

    if bld.CONFIG_SET('USING_SYSTEM_POPT'):
        (popt_includes, popt_ldflags, popt_cpppath) = library_flags(bld, 'popt')
        extra_includes += popt_cpppath
    else:
        extra_includes += [ '../lib/popt' ]

    if bld.CONFIG_SET('USING_SYSTEM_INIPARSER'):
        (iniparser_includes, iniparser_ldflags, iniparser_cpppath) = library_flags(bld, 'iniparser')
        extra_includes += iniparser_cpppath
    else:
        extra_includes += [ '../lib/iniparser' ]

    # s3 builds assume that they will have a bunch of extra include paths
    includes = []
    for d in extra_includes:
        includes += [ os.path.join(s3reldir, d) ]

    # the rule may already have some includes listed
    if 'includes' in kwargs:
        includes += TO_LIST(kwargs['includes'])
    kwargs['includes'] = includes

# these wrappers allow for mixing of S3 and S4 build rules in the one build

def SAMBA3_LIBRARY(bld, name, *args, **kwargs):
    s3_fix_kwargs(bld, kwargs)
    return bld.SAMBA_LIBRARY(name, *args, **kwargs)
Build.BuildContext.SAMBA3_LIBRARY = SAMBA3_LIBRARY

def SAMBA3_MODULE(bld, name, *args, **kwargs):
    s3_fix_kwargs(bld, kwargs)
    return bld.SAMBA_MODULE(name, *args, **kwargs)
Build.BuildContext.SAMBA3_MODULE = SAMBA3_MODULE

def SAMBA3_SUBSYSTEM(bld, name, *args, **kwargs):
    s3_fix_kwargs(bld, kwargs)
    return bld.SAMBA_SUBSYSTEM(name, *args, **kwargs)
Build.BuildContext.SAMBA3_SUBSYSTEM = SAMBA3_SUBSYSTEM

def SAMBA3_BINARY(bld, name, *args, **kwargs):
    s3_fix_kwargs(bld, kwargs)
    return bld.SAMBA_BINARY(name, *args, **kwargs)
Build.BuildContext.SAMBA3_BINARY = SAMBA3_BINARY

def SAMBA3_PYTHON(bld, name, *args, **kwargs):
    s3_fix_kwargs(bld, kwargs)
    return bld.SAMBA_PYTHON(name, *args, **kwargs)
Build.BuildContext.SAMBA3_PYTHON = SAMBA3_PYTHON
