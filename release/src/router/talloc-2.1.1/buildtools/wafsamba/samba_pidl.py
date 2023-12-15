# waf build tool for building IDL files with pidl

from TaskGen import before
import Build, os, sys, Logs
from samba_utils import *

def SAMBA_PIDL(bld, pname, source,
               options='',
               output_dir='.',
               generate_tables=True):
    '''Build a IDL file using pidl.
       This will produce up to 13 output files depending on the options used'''

    bname = source[0:-4]; # strip off the .idl suffix
    bname = os.path.basename(bname)
    name = "%s_%s" % (pname, bname.upper())

    if not SET_TARGET_TYPE(bld, name, 'PIDL'):
        return

    bld.SET_BUILD_GROUP('build_source')

    # the output files depend on the options used. Use this dictionary
    # to map between the options and the resulting file names
    options_map = { '--header'            : '%s.h',
                    '--ndr-parser'        : 'ndr_%s.c ndr_%s.h',
                    '--samba3-ndr-server' : 'srv_%s.c srv_%s.h',
                    '--samba3-ndr-client' : 'cli_%s.c cli_%s.h',
                    '--server'            : 'ndr_%s_s.c',
                    '--client'            : 'ndr_%s_c.c ndr_%s_c.h',
                    '--python'            : 'py_%s.c',
                    '--tdr-parser'        : 'tdr_%s.c tdr_%s.h',
                    '--dcom-proxy'        : '%s_p.c',
                    '--com-header'        : 'com_%s.h'
                    }

    table_header_idx = None
    out_files = []
    options_list = TO_LIST(options)

    for o in options_list:
        if o in options_map:
            ofiles = TO_LIST(options_map[o])
            for f in ofiles:
                out_files.append(os.path.join(output_dir, f % bname))
                if f == 'ndr_%s.h':
                    # remember this one for the tables generation
                    table_header_idx = len(out_files) - 1

    # depend on the full pidl sources
    source = TO_LIST(source)
    try:
        pidl_src_nodes = bld.pidl_files_cache
    except AttributeError:
        bld.pidl_files_cache = bld.srcnode.ant_glob('pidl/lib/Parse/**/*.pm', flat=False)
        bld.pidl_files_cache.extend(bld.srcnode.ant_glob('pidl', flat=False))
        pidl_src_nodes = bld.pidl_files_cache

    # the cd .. is needed because pidl currently is sensitive to the directory it is run in
    cpp = ""
    cc = ""
    if bld.CONFIG_SET("CPP") and bld.CONFIG_GET("CPP") != "":
        if isinstance(bld.CONFIG_GET("CPP"), list):
            cpp = 'CPP="%s"' % " ".join(bld.CONFIG_GET("CPP"))
        else:
            cpp = 'CPP="%s"' % bld.CONFIG_GET("CPP")

    if cpp == "CPP=xlc_r":
        cpp = ""


    if bld.CONFIG_SET("CC"):
        if isinstance(bld.CONFIG_GET("CC"), list):
            cc = 'CC="%s"' % " ".join(bld.CONFIG_GET("CC"))
        else:
            cc = 'CC="%s"' % bld.CONFIG_GET("CC")

    t = bld(rule='cd .. && %s %s ${PERL} "${PIDL}" --quiet ${OPTIONS} --outputdir ${OUTPUTDIR} -- "${SRC[0].abspath(env)}"' % (cpp, cc),
            ext_out    = '.c',
            before     = 'cc',
            on_results = True,
            shell      = True,
            source     = source,
            target     = out_files,
            name       = name,
            samba_type = 'PIDL')

    # prime the list of nodes we are dependent on with the cached pidl sources
    t.allnodes = pidl_src_nodes

    t.env.PIDL = os.path.join(bld.srcnode.abspath(), 'pidl/pidl')
    t.env.OPTIONS = TO_LIST(options)
    t.env.OUTPUTDIR = bld.bldnode.name + '/' + bld.path.find_dir(output_dir).bldpath(t.env)

    if generate_tables and table_header_idx is not None:
        pidl_headers = LOCAL_CACHE(bld, 'PIDL_HEADERS')
        pidl_headers[name] = [bld.path.find_or_declare(out_files[table_header_idx])]

    t.more_includes = '#' + bld.path.relpath_gen(bld.srcnode)
Build.BuildContext.SAMBA_PIDL = SAMBA_PIDL


def SAMBA_PIDL_LIST(bld, name, source,
                    options='',
                    output_dir='.',
                    generate_tables=True):
    '''A wrapper for building a set of IDL files'''
    for p in TO_LIST(source):
        bld.SAMBA_PIDL(name, p, options=options, output_dir=output_dir, generate_tables=generate_tables)
Build.BuildContext.SAMBA_PIDL_LIST = SAMBA_PIDL_LIST


#################################################################
# the rule for generating the NDR tables
from TaskGen import feature, before
@feature('collect')
@before('exec_rule')
def collect(self):
    pidl_headers = LOCAL_CACHE(self.bld, 'PIDL_HEADERS')
    for (name, hd) in pidl_headers.items():
        y = self.bld.name_to_obj(name, self.env)
        self.bld.ASSERT(y is not None, 'Failed to find PIDL header %s' % name)
        y.post()
        for node in hd:
            self.bld.ASSERT(node is not None, 'Got None as build node generating PIDL table for %s' % name)
            self.source += " " + node.relpath_gen(self.path)


def SAMBA_PIDL_TABLES(bld, name, target):
    '''generate the pidl NDR tables file'''
    headers = bld.env.PIDL_HEADERS
    bld.SET_BUILD_GROUP('main')
    t = bld(
            features = 'collect',
            rule     = '${PERL} ${SRC} --output ${TGT} | sed "s|default/||" > ${TGT}',
            ext_out  = '.c',
            before   = 'cc',
            on_results = True,
            shell    = True,
            source   = '../../librpc/tables.pl',
            target   = target,
            name     = name)
    t.env.LIBRPC = os.path.join(bld.srcnode.abspath(), 'librpc')
Build.BuildContext.SAMBA_PIDL_TABLES = SAMBA_PIDL_TABLES

