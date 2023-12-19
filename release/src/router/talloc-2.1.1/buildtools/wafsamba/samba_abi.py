# functions for handling ABI checking of libraries

import Options, Utils, os, Logs, samba_utils, sys, Task, fnmatch, re, Build
from TaskGen import feature, before, after

# these type maps cope with platform specific names for common types
# please add new type mappings into the list below
abi_type_maps = {
    '_Bool' : 'bool',
    'struct __va_list_tag *' : 'va_list'
    }

version_key = lambda x: map(int, x.split("."))

def normalise_signature(sig):
    '''normalise a signature from gdb'''
    sig = sig.strip()
    sig = re.sub('^\$[0-9]+\s=\s\{(.+)\}$', r'\1', sig)
    sig = re.sub('^\$[0-9]+\s=\s\{(.+)\}(\s0x[0-9a-f]+\s<\w+>)+$', r'\1', sig)
    sig = re.sub('^\$[0-9]+\s=\s(0x[0-9a-f]+)\s?(<\w+>)?$', r'\1', sig)
    sig = re.sub('0x[0-9a-f]+', '0xXXXX', sig)
    sig = re.sub('", <incomplete sequence (\\\\[a-z0-9]+)>', r'\1"', sig)

    for t in abi_type_maps:
        # we need to cope with non-word characters in mapped types
        m = t
        m = m.replace('*', '\*')
        if m[-1].isalnum() or m[-1] == '_':
            m += '\\b'
        if m[0].isalnum() or m[0] == '_':
            m = '\\b' + m
        sig = re.sub(m, abi_type_maps[t], sig)
    return sig


def normalise_varargs(sig):
    '''cope with older versions of gdb'''
    sig = re.sub(',\s\.\.\.', '', sig)
    return sig


def parse_sigs(sigs, abi_match):
    '''parse ABI signatures file'''
    abi_match = samba_utils.TO_LIST(abi_match)
    ret = {}
    a = sigs.split('\n')
    for s in a:
        if s.find(':') == -1:
            continue
        sa = s.split(':')
        if abi_match:
            matched = False
            negative = False
            for p in abi_match:
                if p[0] == '!' and fnmatch.fnmatch(sa[0], p[1:]):
                    negative = True
                    break
                elif fnmatch.fnmatch(sa[0], p):
                    matched = True
                    break
            if (not matched) and negative:
                continue
        Logs.debug("%s -> %s" % (sa[1], normalise_signature(sa[1])))
        ret[sa[0]] = normalise_signature(sa[1])
    return ret

def save_sigs(sig_file, parsed_sigs):
    '''save ABI signatures to a file'''
    sigs = ''
    for s in sorted(parsed_sigs.keys()):
        sigs += '%s: %s\n' % (s, parsed_sigs[s])
    return samba_utils.save_file(sig_file, sigs, create_dir=True)


def abi_check_task(self):
    '''check if the ABI has changed'''
    abi_gen = self.ABI_GEN

    libpath = self.inputs[0].abspath(self.env)
    libname = os.path.basename(libpath)

    sigs = Utils.cmd_output([abi_gen, libpath])
    parsed_sigs = parse_sigs(sigs, self.ABI_MATCH)

    sig_file = self.ABI_FILE

    old_sigs = samba_utils.load_file(sig_file)
    if old_sigs is None or Options.options.ABI_UPDATE:
        if not save_sigs(sig_file, parsed_sigs):
            raise Utils.WafError('Failed to save ABI file "%s"' % sig_file)
        Logs.warn('Generated ABI signatures %s' % sig_file)
        return

    parsed_old_sigs = parse_sigs(old_sigs, self.ABI_MATCH)

    # check all old sigs
    got_error = False
    for s in parsed_old_sigs:
        if not s in parsed_sigs:
            Logs.error('%s: symbol %s has been removed - please update major version\n\tsignature: %s' % (
                libname, s, parsed_old_sigs[s]))
            got_error = True
        elif normalise_varargs(parsed_old_sigs[s]) != normalise_varargs(parsed_sigs[s]):
            Logs.error('%s: symbol %s has changed - please update major version\n\told_signature: %s\n\tnew_signature: %s' % (
                libname, s, parsed_old_sigs[s], parsed_sigs[s]))
            got_error = True

    for s in parsed_sigs:
        if not s in parsed_old_sigs:
            Logs.error('%s: symbol %s has been added - please mark it _PRIVATE_ or update minor version\n\tsignature: %s' % (
                libname, s, parsed_sigs[s]))
            got_error = True

    if got_error:
        raise Utils.WafError('ABI for %s has changed - please fix library version then build with --abi-update\nSee http://wiki.samba.org/index.php/Waf#ABI_Checking for more information\nIf you have not changed any ABI, and your platform always gives this error, please configure with --abi-check-disable to skip this check' % libname)


t = Task.task_type_from_func('abi_check', abi_check_task, color='BLUE', ext_in='.bin')
t.quiet = True
# allow "waf --abi-check" to force re-checking the ABI
if '--abi-check' in sys.argv:
    Task.always_run(t)

@after('apply_link')
@feature('abi_check')
def abi_check(self):
    '''check that ABI matches saved signatures'''
    env = self.bld.env
    if not env.ABI_CHECK or self.abi_directory is None:
        return

    # if the platform doesn't support -fvisibility=hidden then the ABI
    # checks become fairly meaningless
    if not env.HAVE_VISIBILITY_ATTR:
        return

    topsrc = self.bld.srcnode.abspath()
    abi_gen = os.path.join(topsrc, 'buildtools/scripts/abi_gen.sh')

    abi_file = "%s/%s-%s.sigs" % (self.abi_directory, self.name, self.vnum)

    tsk = self.create_task('abi_check', self.link_task.outputs[0])
    tsk.ABI_FILE = abi_file
    tsk.ABI_MATCH = self.abi_match
    tsk.ABI_GEN = abi_gen


def abi_process_file(fname, version, symmap):
    '''process one ABI file, adding new symbols to the symmap'''
    f = open(fname, mode='r')
    for line in f:
        symname = line.split(":")[0]
        if not symname in symmap:
            symmap[symname] = version
    f.close()


def abi_write_vscript(f, libname, current_version, versions, symmap, abi_match):
    """Write a vscript file for a library in --version-script format.

    :param f: File-like object to write to
    :param libname: Name of the library, uppercased
    :param current_version: Current version
    :param versions: Versions to consider
    :param symmap: Dictionary mapping symbols -> version
    :param abi_match: List of symbols considered to be public in the current
        version
    """

    invmap = {}
    for s in symmap:
        invmap.setdefault(symmap[s], []).append(s)

    last_key = ""
    versions = sorted(versions, key=version_key)
    for k in versions:
        symver = "%s_%s" % (libname, k)
        if symver == current_version:
            break
        f.write("%s {\n" % symver)
        if k in sorted(invmap.keys()):
            f.write("\tglobal:\n")
            for s in invmap.get(k, []):
                f.write("\t\t%s;\n" % s);
        f.write("}%s;\n\n" % last_key)
        last_key = " %s" % symver
    f.write("%s {\n" % current_version)
    local_abi = filter(lambda x: x[0] == '!', abi_match)
    global_abi = filter(lambda x: x[0] != '!', abi_match)
    f.write("\tglobal:\n")
    if len(global_abi) > 0:
        for x in global_abi:
            f.write("\t\t%s;\n" % x)
    else:
        f.write("\t\t*;\n")
    if abi_match != ["*"]:
        f.write("\tlocal:\n")
        for x in local_abi:
            f.write("\t\t%s;\n" % x[1:])
        if len(global_abi) > 0:
            f.write("\t\t*;\n")
    f.write("};\n")


def abi_build_vscript(task):
    '''generate a vscript file for our public libraries'''

    tgt = task.outputs[0].bldpath(task.env)

    symmap = {}
    versions = []
    for f in task.inputs:
        fname = f.abspath(task.env)
        basename = os.path.basename(fname)
        version = basename[len(task.env.LIBNAME)+1:-len(".sigs")]
        versions.append(version)
        abi_process_file(fname, version, symmap)
    f = open(tgt, mode='w')
    try:
        abi_write_vscript(f, task.env.LIBNAME, task.env.VERSION, versions,
            symmap, task.env.ABI_MATCH)
    finally:
        f.close()


def ABI_VSCRIPT(bld, libname, abi_directory, version, vscript, abi_match=None):
    '''generate a vscript file for our public libraries'''
    if abi_directory:
        source = bld.path.ant_glob('%s/%s-[0-9]*.sigs' % (abi_directory, libname))
        def abi_file_key(path):
            return version_key(path[:-len(".sigs")].rsplit("-")[-1])
        source = sorted(source.split(), key=abi_file_key)
    else:
        source = ''

    libname = os.path.basename(libname)
    version = os.path.basename(version)
    libname = libname.replace("-", "_").replace("+","_").upper()
    version = version.replace("-", "_").replace("+","_").upper()

    t = bld.SAMBA_GENERATOR(vscript,
                            rule=abi_build_vscript,
                            source=source,
                            group='vscripts',
                            target=vscript)
    if abi_match is None:
        abi_match = ["*"]
    else:
        abi_match = samba_utils.TO_LIST(abi_match)
    t.env.ABI_MATCH = abi_match
    t.env.VERSION = version
    t.env.LIBNAME = libname
    t.vars = ['LIBNAME', 'VERSION', 'ABI_MATCH']
Build.BuildContext.ABI_VSCRIPT = ABI_VSCRIPT
