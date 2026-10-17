# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (c) 2016 Google, Inc
#

import glob
import os
import shlex
import shutil
import sys
import tempfile
import urllib.request

from u_boot_pylib import command
from u_boot_pylib import tout

# Output directly (generally this is temporary)
outdir = None

# True to keep the output directory around after exiting
preserve_outdir = False

# Path to the Chrome OS chroot, if we know it
chroot_path = None

# Search paths to use for filename(), used to find files
search_paths = []

tool_search_paths = []

# Tools and the packages that contain them, on debian
packages = {
    'lz4': 'liblz4-tool',
    }

# List of paths to use when looking for an input file
indir = []

def prepare_output_dir(dirname, preserve=False):
    """Select an output directory, ensuring it exists.

    This either creates a temporary directory or checks that the one supplied
    by the user is valid. For a temporary directory, it makes a note to
    remove it later if required.

    Args:
        dirname: a string, name of the output directory to use to store
                intermediate and output files. If is None - create a temporary
                directory.
        preserve: a Boolean. If outdir above is None and preserve is False, the
                created temporary directory will be destroyed on exit.

    Raises:
        OSError: If it cannot create the output directory.
    """
    global outdir, preserve_outdir

    preserve_outdir = dirname or preserve
    if dirname:
        outdir = dirname
        if not os.path.isdir(outdir):
            try:
                os.makedirs(outdir)
            except OSError as err:
                raise ValueError(
                    f"Cannot make output directory 'outdir': 'err.strerror'")
        tout.debug("Using output directory '%s'" % outdir)
    else:
        outdir = tempfile.mkdtemp(prefix='binman.')
        tout.debug("Using temporary directory '%s'" % outdir)

def _remove_output_dir():
    global outdir

    shutil.rmtree(outdir)
    tout.debug("Deleted temporary directory '%s'" % outdir)
    outdir = None

def finalise_output_dir():
    global outdir, preserve_outdir

    """Tidy up: delete output directory if temporary and not preserved."""
    if outdir and not preserve_outdir:
        _remove_output_dir()
        outdir = None

def get_output_filename(fname):
    """Return a filename within the output directory.

    Args:
        fname: Filename to use for new file

    Returns:
        The full path of the filename, within the output directory
    """
    return os.path.join(outdir, fname)

def get_output_dir():
    """Return the current output directory

    Returns:
        str: The output directory
    """
    return outdir

def _finalise_for_test():
    """Remove the output directory (for use by tests)"""
    global outdir

    if outdir:
        _remove_output_dir()
        outdir = None

def set_input_dirs(dirname):
    """Add a list of input directories, where input files are kept.

    Args:
        dirname: a list of paths to input directories to use for obtaining
                files needed by binman to place in the image.
    """
    global indir

    indir = dirname
    tout.debug("Using input directories %s" % indir)

def get_input_filename(fname, allow_missing=False):
    """Return a filename for use as input.

    Args:
        fname: Filename to use for new file
        allow_missing: True if the filename can be missing

    Returns:
        fname, if indir is None;
        full path of the filename, within the input directory;
        None, if file is missing and allow_missing is True

    Raises:
        ValueError if file is missing and allow_missing is False
    """
    if not indir or fname[:1] == '/':
        return fname
    for dirname in indir:
        pathname = os.path.join(dirname, fname)
        if os.path.exists(pathname):
            return pathname

    if allow_missing:
        return None
    raise ValueError("Filename '%s' not found in input path (%s) (cwd='%s')" %
                     (fname, ','.join(indir), os.getcwd()))

def get_input_filename_glob(pattern):
    """Return a list of filenames for use as input.

    Args:
        pattern: Filename pattern to search for

    Returns:
        A list of matching files in all input directories
    """
    if not indir:
        return glob.glob(pattern)
    files = []
    for dirname in indir:
        pathname = os.path.join(dirname, pattern)
        files += glob.glob(pathname)
    return sorted(files)

def align(pos, align):
    if align:
        mask = align - 1
        pos = (pos + mask) & ~mask
    return pos

def not_power_of_two(num):
    return num and (num & (num - 1))

def set_tool_paths(toolpaths):
    """Set the path to search for tools

    Args:
        toolpaths: List of paths to search for tools executed by run()
    """
    global tool_search_paths

    tool_search_paths = toolpaths

def path_has_file(path_spec, fname):
    """Check if a given filename is in the PATH

    Args:
        path_spec: Value of PATH variable to check
        fname: Filename to check

    Returns:
        True if found, False if not
    """
    for dir in path_spec.split(':'):
        if os.path.exists(os.path.join(dir, fname)):
            return True
    return False

def get_host_compile_tool(env, name):
    """Get the host-specific version for a compile tool

    This checks the environment variables that specify which version of
    the tool should be used (e.g. ${HOSTCC}).

    The following table lists the host-specific versions of the tools
    this function resolves to:

        Compile Tool  | Host version
        --------------+----------------
        as            |  ${HOSTAS}
        ld            |  ${HOSTLD}
        cc            |  ${HOSTCC}
        cpp           |  ${HOSTCPP}
        c++           |  ${HOSTCXX}
        ar            |  ${HOSTAR}
        nm            |  ${HOSTNM}
        ldr           |  ${HOSTLDR}
        strip         |  ${HOSTSTRIP}
        objcopy       |  ${HOSTOBJCOPY}
        objdump       |  ${HOSTOBJDUMP}
        dtc           |  ${HOSTDTC}

    Args:
        name: Command name to run

    Returns:
        host_name: Exact command name to run instead
        extra_args: List of extra arguments to pass
    """
    host_name = None
    extra_args = []
    if name in ('as', 'ld', 'cc', 'cpp', 'ar', 'nm', 'ldr', 'strip',
                'objcopy', 'objdump', 'dtc'):
        host_name, *host_args = env.get('HOST' + name.upper(), '').split(' ')
    elif name == 'c++':
        host_name, *host_args = env.get('HOSTCXX', '').split(' ')

    if host_name:
        return host_name, extra_args
    return name, []

def get_target_compile_tool(name, cross_compile=None):
    """Get the target-specific version for a compile tool

    This first checks the environment variables that specify which
    version of the tool should be used (e.g. ${CC}). If those aren't
    specified, it checks the CROSS_COMPILE variable as a prefix for the
    tool with some substitutions (e.g. "${CROSS_COMPILE}gcc" for cc).

    The following table lists the target-specific versions of the tools
    this function resolves to:

        Compile Tool  | First choice   | Second choice
        --------------+----------------+----------------------------
        as            |  ${AS}         | ${CROSS_COMPILE}as
        ld            |  ${LD}         | ${CROSS_COMPILE}ld.bfd
                      |                |   or ${CROSS_COMPILE}ld
        cc            |  ${CC}         | ${CROSS_COMPILE}gcc
        cpp           |  ${CPP}        | ${CROSS_COMPILE}gcc -E
        c++           |  ${CXX}        | ${CROSS_COMPILE}g++
        ar            |  ${AR}         | ${CROSS_COMPILE}ar
        nm            |  ${NM}         | ${CROSS_COMPILE}nm
        ldr           |  ${LDR}        | ${CROSS_COMPILE}ldr
        strip         |  ${STRIP}      | ${CROSS_COMPILE}strip
        objcopy       |  ${OBJCOPY}    | ${CROSS_COMPILE}objcopy
        objdump       |  ${OBJDUMP}    | ${CROSS_COMPILE}objdump
        dtc           |  ${DTC}        | (no CROSS_COMPILE version)

    Args:
        name: Command name to run

    Returns:
        target_name: Exact command name to run instead
        extra_args: List of extra arguments to pass
    """
    env = dict(os.environ)

    target_name = None
    extra_args = []
    if name in ('as', 'ld', 'cc', 'cpp', 'ar', 'nm', 'ldr', 'strip',
                'objcopy', 'objdump', 'dtc'):
        target_name, *extra_args = env.get(name.upper(), '').split(' ')
    elif name == 'c++':
        target_name, *extra_args = env.get('CXX', '').split(' ')

    if target_name:
        return target_name, extra_args

    if cross_compile is None:
        cross_compile = env.get('CROSS_COMPILE', '')

    if name in ('as', 'ar', 'nm', 'ldr', 'strip', 'objcopy', 'objdump'):
        target_name = cross_compile + name
    elif name == 'ld':
        try:
            if run(cross_compile + 'ld.bfd', '-v'):
                target_name = cross_compile + 'ld.bfd'
        except:
            target_name = cross_compile + 'ld'
    elif name == 'cc':
        target_name = cross_compile + 'gcc'
    elif name == 'cpp':
        target_name = cross_compile + 'gcc'
        extra_args = ['-E']
    elif name == 'c++':
        target_name = cross_compile + 'g++'
    else:
        target_name = name
    return target_name, extra_args

def get_env_with_path():
    """Get an updated environment with the PATH variable set correctly

    If there are any search paths set, these need to come first in the PATH so
    that these override any other version of the tools.

    Returns:
        dict: New environment with PATH updated, or None if there are not search
            paths
    """
    if tool_search_paths:
        env = dict(os.environ)
        env['PATH'] = ':'.join(tool_search_paths) + ':' + env['PATH']
        return env

def run_result(name, *args, **kwargs):
    """Run a tool with some arguments

    This runs a 'tool', which is a program used by binman to process files and
    perhaps produce some output. Tools can be located on the PATH or in a
    search path.

    Args:
        name: Command name to run
        args: Arguments to the tool
        for_host: True to resolve the command to the version for the host
        for_target: False to run the command as-is, without resolving it
                   to the version for the compile target
        raise_on_error: Raise an error if the command fails (True by default)

    Returns:
        CommandResult object
    """
    try:
        binary = kwargs.get('binary')
        for_host = kwargs.get('for_host', False)
        for_target = kwargs.get('for_target', not for_host)
        raise_on_error = kwargs.get('raise_on_error', True)
        env = get_env_with_path()
        if for_target:
            name, extra_args = get_target_compile_tool(name)
            args = tuple(extra_args) + args
        elif for_host:
            name, extra_args = get_host_compile_tool(env, name)
            args = tuple(extra_args) + args
        name = os.path.expanduser(name)  # Expand paths containing ~
        all_args = (name,) + args
        result = command.run_pipe([all_args], capture=True, capture_stderr=True,
                                 env=env, raise_on_error=False, binary=binary)
        if result.return_code:
            if raise_on_error:
                raise ValueError("Error %d running '%s': %s" %
                                 (result.return_code,' '.join(all_args),
                                  result.stderr or result.stdout))
        return result
    except ValueError:
        if env and not path_has_file(env['PATH'], name):
            msg = "Please install tool '%s'" % name
            package = packages.get(name)
            if package:
                 msg += " (e.g. from package '%s')" % package
            raise ValueError(msg)
        raise

def tool_find(name):
    """Search the current path for a tool

    This uses both PATH and any value from set_tool_paths() to search for a tool

    Args:
        name (str): Name of tool to locate

    Returns:
        str: Full path to tool if found, else None
    """
    name = os.path.expanduser(name)  # Expand paths containing ~
    paths = []
    pathvar = os.environ.get('PATH')
    if pathvar:
        paths = pathvar.split(':')
    if tool_search_paths:
        paths += tool_search_paths
    for path in paths:
        fname = os.path.join(path, name)
        if os.path.isfile(fname) and os.access(fname, os.X_OK):
            return fname

def run(name, *args, **kwargs):
    """Run a tool with some arguments

    This runs a 'tool', which is a program used by binman to process files and
    perhaps produce some output. Tools can be located on the PATH or in a
    search path.

    Args:
        name: Command name to run
        args: Arguments to the tool
        for_host: True to resolve the command to the version for the host
        for_target: False to run the command as-is, without resolving it
                   to the version for the compile target

    Returns:
        CommandResult object
    """
    result = run_result(name, *args, **kwargs)
    if result is not None:
        return result.stdout

def filename(fname):
    """Resolve a file path to an absolute path.

    If fname starts with ##/ and chroot is available, ##/ gets replaced with
    the chroot path. If chroot is not available, this file name can not be
    resolved, `None' is returned.

    If fname is not prepended with the above prefix, and is not an existing
    file, the actual file name is retrieved from the passed in string and the
    search_paths directories (if any) are searched to for the file. If found -
    the path to the found file is returned, `None' is returned otherwise.

    Args:
      fname: a string,  the path to resolve.

    Returns:
      Absolute path to the file or None if not found.
    """
    if fname.startswith('##/'):
      if chroot_path:
        fname = os.path.join(chroot_path, fname[3:])
      else:
        return None

    # Search for a pathname that exists, and return it if found
    if fname and not os.path.exists(fname):
        for path in search_paths:
            pathname = os.path.join(path, os.path.basename(fname))
            if os.path.exists(pathname):
                return pathname

    # If not found, just return the standard, unchanged path
    return fname

def read_file(fname, binary=True):
    """Read and return the contents of a file.

    Args:
      fname: path to filename to read, where ## signifiies the chroot.

    Returns:
      data read from file, as a string.
    """
    with open(filename(fname), binary and 'rb' or 'r') as fd:
        data = fd.read()
    #self._out.Info("Read file '%s' size %d (%#0x)" %
                   #(fname, len(data), len(data)))
    return data

def write_file(fname, data, binary=True):
    """Write data into a file.

    Args:
        fname: path to filename to write
        data: data to write to file, as a string
    """
    #self._out.Info("Write file '%s' size %d (%#0x)" %
                   #(fname, len(data), len(data)))
    with open(filename(fname), binary and 'wb' or 'w') as fd:
        fd.write(data)

def get_bytes(byte, size):
    """Get a string of bytes of a given size

    Args:
        byte: Numeric byte value to use
        size: Size of bytes/string to return

    Returns:
        A bytes type with 'byte' repeated 'size' times
    """
    return bytes([byte]) * size

def to_bytes(string):
    """Convert a str type into a bytes type

    Args:
        string: string to convert

    Returns:
        A bytes type
    """
    return string.encode('utf-8')

def to_string(bval):
    """Convert a bytes type into a str type

    Args:
        bval: bytes value to convert

    Returns:
        Python 3: A bytes type
        Python 2: A string type
    """
    return bval.decode('utf-8')

def to_hex(val):
    """Convert an integer value (or None) to a string

    Returns:
        hex value, or 'None' if the value is None
    """
    return 'None' if val is None else '%#x' % val

def to_hex_size(val):
    """Return the size of an object in hex

    Returns:
        hex value of size, or 'None' if the value is None
    """
    return 'None' if val is None else '%#x' % len(val)

def print_full_help(fname):
    """Print the full help message for a tool using an appropriate pager.

    Args:
        fname: Path to a file containing the full help message
    """
    pager = shlex.split(os.getenv('PAGER', ''))
    if not pager:
        lesspath = shutil.which('less')
        pager = [lesspath] if lesspath else None
    if not pager:
        pager = ['more']
    command.run(*pager, fname)

def download(url, tmpdir_pattern='.patman'):
    """Download a file to a temporary directory

    Args:
        url (str): URL to download
        tmpdir_pattern (str): pattern to use for the temporary directory

    Returns:
        Tuple:
            Full path to the downloaded archive file in that directory,
                or None if there was an error while downloading
            Temporary directory name
    """
    print('- downloading: %s' % url)
    leaf = url.split('/')[-1]
    tmpdir = tempfile.mkdtemp(tmpdir_pattern)
    response = urllib.request.urlopen(url)
    fname = os.path.join(tmpdir, leaf)
    fd = open(fname, 'wb')
    meta = response.info()
    size = int(meta.get('Content-Length'))
    done = 0
    block_size = 1 << 16
    status = ''

    # Read the file in chunks and show progress as we go
    while True:
        buffer = response.read(block_size)
        if not buffer:
            print(chr(8) * (len(status) + 1), '\r', end=' ')
            break

        done += len(buffer)
        fd.write(buffer)
        status = r'%10d MiB  [%3d%%]' % (done // 1024 // 1024,
                                            done * 100 // size)
        status = status + chr(8) * (len(status) + 1)
        print(status, end=' ')
        sys.stdout.flush()
    print('\r', end='')
    sys.stdout.flush()
    fd.close()
    if done != size:
        print('Error, failed to download')
        os.remove(fname)
        fname = None
    return fname, tmpdir
