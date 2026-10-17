# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# Handle various things related to ELF images
#

from collections import namedtuple, OrderedDict
import io
import os
import re
import shutil
import struct
import tempfile

from u_boot_pylib import command
from u_boot_pylib import tools
from u_boot_pylib import tout

ELF_TOOLS = True
try:
    from elftools.elf.elffile import ELFFile
    from elftools.elf.elffile import ELFError
    from elftools.elf.sections import SymbolTableSection
except:  # pragma: no cover
    ELF_TOOLS = False

# BSYM in little endian, keep in sync with include/binman_sym.h
BINMAN_SYM_MAGIC_VALUE = 0x4d595342

# Information about an EFL symbol:
# section (str): Name of the section containing this symbol
# address (int): Address of the symbol (its value)
# size (int): Size of the symbol in bytes
# weak (bool): True if the symbol is weak
# offset (int or None): Offset of the symbol's data in the ELF file, or None if
#   not known
Symbol = namedtuple('Symbol', ['section', 'address', 'size', 'weak', 'offset'])

# Information about an ELF file:
#    data: Extracted program contents of ELF file (this would be loaded by an
#           ELF loader when reading this file
#    load: Load address of code
#    entry: Entry address of code
#    memsize: Number of bytes in memory occupied by loading this ELF file
ElfInfo = namedtuple('ElfInfo', ['data', 'load', 'entry', 'memsize'])


def GetSymbols(fname, patterns):
    """Get the symbols from an ELF file

    Args:
        fname: Filename of the ELF file to read
        patterns: List of regex patterns to search for, each a string

    Returns:
        None, if the file does not exist, or Dict:
          key: Name of symbol
          value: Hex value of symbol
    """
    stdout = tools.run('objdump', '-t', fname)
    lines = stdout.splitlines()
    if patterns:
        re_syms = re.compile('|'.join(patterns))
    else:
        re_syms = None
    syms = {}
    syms_started = False
    for line in lines:
        if not line or not syms_started:
            if 'SYMBOL TABLE' in line:
                syms_started = True
            line = None  # Otherwise code coverage complains about 'continue'
            continue
        if re_syms and not re_syms.search(line):
            continue

        space_pos = line.find(' ')
        value, rest = line[:space_pos], line[space_pos + 1:]
        flags = rest[:7]
        parts = rest[7:].split()
        section, size =  parts[:2]
        if len(parts) > 2:
            name = parts[2] if parts[2] != '.hidden' else parts[3]
            syms[name] = Symbol(section, int(value, 16), int(size, 16),
                                flags[1] == 'w', None)

    # Sort dict by address
    return OrderedDict(sorted(syms.items(), key=lambda x: x[1].address))

def _GetFileOffset(elf, addr):
    """Get the file offset for an address

    Args:
        elf (ELFFile): ELF file to check
        addr (int): Address to search for

    Returns
        int: Offset of that address in the ELF file, or None if not valid
    """
    for seg in elf.iter_segments():
        seg_end = seg['p_vaddr'] + seg['p_filesz']
        if seg.header['p_type'] == 'PT_LOAD':
            if addr >= seg['p_vaddr'] and addr < seg_end:
                return addr - seg['p_vaddr'] + seg['p_offset']

def GetFileOffset(fname, addr):
    """Get the file offset for an address

    Args:
        fname (str): Filename of ELF file to check
        addr (int): Address to search for

    Returns
        int: Offset of that address in the ELF file, or None if not valid
    """
    if not ELF_TOOLS:
        raise ValueError("Python: No module named 'elftools'")
    with open(fname, 'rb') as fd:
        elf = ELFFile(fd)
        return _GetFileOffset(elf, addr)

def GetSymbolFromAddress(fname, addr):
    """Get the symbol at a particular address

    Args:
        fname (str): Filename of ELF file to check
        addr (int): Address to search for

    Returns:
        str: Symbol name, or None if no symbol at that address
    """
    if not ELF_TOOLS:
        raise ValueError("Python: No module named 'elftools'")
    with open(fname, 'rb') as fd:
        elf = ELFFile(fd)
        syms = GetSymbols(fname, None)
    for name, sym in syms.items():
        if sym.address == addr:
            return name

def GetSymbolFileOffset(fname, patterns):
    """Get the symbols from an ELF file

    Args:
        fname: Filename of the ELF file to read
        patterns: List of regex patterns to search for, each a string

    Returns:
        None, if the file does not exist, or Dict:
          key: Name of symbol
          value: Hex value of symbol
    """
    if not ELF_TOOLS:
        raise ValueError("Python: No module named 'elftools'")

    syms = {}
    with open(fname, 'rb') as fd:
        elf = ELFFile(fd)

        re_syms = re.compile('|'.join(patterns))
        for section in elf.iter_sections():
            if isinstance(section, SymbolTableSection):
                for symbol in section.iter_symbols():
                    if not re_syms or re_syms.search(symbol.name):
                        addr = symbol.entry['st_value']
                        syms[symbol.name] = Symbol(
                            section.name, addr, symbol.entry['st_size'],
                            symbol.entry['st_info']['bind'] == 'STB_WEAK',
                            _GetFileOffset(elf, addr))

    # Sort dict by address
    return OrderedDict(sorted(syms.items(), key=lambda x: x[1].address))

def GetSymbolAddress(fname, sym_name):
    """Get a value of a symbol from an ELF file

    Args:
        fname: Filename of the ELF file to read
        patterns: List of regex patterns to search for, each a string

    Returns:
        Symbol value (as an integer) or None if not found
    """
    syms = GetSymbols(fname, [sym_name])
    sym = syms.get(sym_name)
    if not sym:
        return None
    return sym.address

def GetPackString(sym, msg):
    """Get the struct.pack/unpack string to use with a given symbol

    Args:
        sym (Symbol): Symbol to check. Only the size member is checked
        @msg (str): String which indicates the entry being processed, used for
            errors

    Returns:
        str: struct string to use, .e.g. '<I'

    Raises:
        ValueError: Symbol has an unexpected size
    """
    if sym.size == 4:
        return '<I'
    elif sym.size == 8:
        return '<Q'
    else:
        raise ValueError('%s has size %d: only 4 and 8 are supported' %
                         (msg, sym.size))

def GetSymbolOffset(elf_fname, sym_name, base_sym=None):
    """Read the offset of a symbol compared to base symbol

    This is useful for obtaining the value of a single symbol relative to the
    base of a binary blob.

    Args:
        elf_fname: Filename of the ELF file to read
        sym_name (str): Name of symbol to read
        base_sym (str): Base symbol to sue to calculate the offset (or None to
            use '__image_copy_start'

    Returns:
        int: Offset of the symbol relative to the base symbol
    """
    if not base_sym:
        base_sym = '__image_copy_start'
    fname = tools.get_input_filename(elf_fname)
    syms = GetSymbols(fname, [base_sym, sym_name])
    base = syms[base_sym].address
    val = syms[sym_name].address
    return val - base

def LookupAndWriteSymbols(elf_fname, entry, section, is_elf=False,
                          base_sym=None):
    """Replace all symbols in an entry with their correct values

    The entry contents is updated so that values for referenced symbols will be
    visible at run time. This is done by finding out the symbols offsets in the
    entry (using the ELF file) and replacing them with values from binman's data
    structures.

    Args:
        elf_fname: Filename of ELF image containing the symbol information for
            entry
        entry: Entry to process
        section: Section which can be used to lookup symbol values
        base_sym: Base symbol marking the start of the image

    Returns:
        int: Number of symbols written
    """
    if not base_sym:
        base_sym = '__image_copy_start'
    fname = tools.get_input_filename(elf_fname)
    syms = GetSymbols(fname, ['image', 'binman'])
    if is_elf:
        if not ELF_TOOLS:
            msg = ("Section '%s': entry '%s'" %
                   (section.GetPath(), entry.GetPath()))
            raise ValueError(f'{msg}: Cannot write symbols to an ELF file without Python elftools')
        new_syms = {}
        with open(fname, 'rb') as fd:
            elf = ELFFile(fd)
            for name, sym in syms.items():
                offset = _GetFileOffset(elf, sym.address)
                new_syms[name] = Symbol(sym.section, sym.address, sym.size,
                                        sym.weak, offset)
            syms = new_syms

    if not syms:
        tout.debug('LookupAndWriteSymbols: no syms')
        return 0
    base = syms.get(base_sym)
    if not base and not is_elf:
        tout.debug('LookupAndWriteSymbols: no base')
        return 0
    base_addr = 0 if is_elf else base.address
    count = 0
    for name, sym in syms.items():
        if name.startswith('_binman'):
            msg = ("Section '%s': Symbol '%s'\n   in entry '%s'" %
                   (section.GetPath(), name, entry.GetPath()))
            if is_elf:
                # For ELF files, use the file offset
                offset = sym.offset
            else:
                # For blobs use the offset of the symbol, calculated by
                # subtracting the base address which by definition is at the
                # start
                offset = sym.address - base.address
                if offset < 0 or offset + sym.size > entry.contents_size:
                    raise ValueError('%s has offset %x (size %x) but the contents '
                                     'size is %x' % (entry.GetPath(), offset,
                                                     sym.size,
                                                     entry.contents_size))
            pack_string = GetPackString(sym, msg)
            if name == '_binman_sym_magic':
                value = BINMAN_SYM_MAGIC_VALUE
            else:
                # Look up the symbol in our entry tables.
                value = section.GetImage().LookupImageSymbol(name, sym.weak,
                                                             msg, base_addr)
            if value is None:
                value = -1
                pack_string = pack_string.lower()
            value_bytes = struct.pack(pack_string, value)
            tout.debug('%s:\n   insert %s, offset %x, value %x, length %d' %
                       (msg, name, offset, value, len(value_bytes)))
            entry.data = (entry.data[:offset] + value_bytes +
                        entry.data[offset + sym.size:])
            count += 1
    if count:
        tout.detail(
            f"Section '{section.GetPath()}': entry '{entry.GetPath()}' : {count} symbols")
    return count

def GetSymbolValue(sym, data, msg):
    """Get the value of a symbol

    This can only be used on symbols with an integer value.

    Args:
        sym (Symbol): Symbol to check
        data (butes): Data for the ELF file - the symbol data appears at offset
            sym.offset
        @msg (str): String which indicates the entry being processed, used for
            errors

    Returns:
        int: Value of the symbol

    Raises:
        ValueError: Symbol has an unexpected size
    """
    pack_string = GetPackString(sym, msg)
    value = struct.unpack(pack_string, data[sym.offset:sym.offset + sym.size])
    return value[0]

def MakeElf(elf_fname, text, data):
    """Make an elf file with the given data in a single section

    The output file has a several section including '.text' and '.data',
    containing the info provided in arguments.

    Args:
        elf_fname: Output filename
        text: Text (code) to put in the file's .text section
        data: Data to put in the file's .data section
    """
    outdir = tempfile.mkdtemp(prefix='binman.elf.')
    s_file = os.path.join(outdir, 'elf.S')

    # Spilt the text into two parts so that we can make the entry point two
    # bytes after the start of the text section
    text_bytes1 = ['\t.byte\t%#x' % byte for byte in text[:2]]
    text_bytes2 = ['\t.byte\t%#x' % byte for byte in text[2:]]
    data_bytes = ['\t.byte\t%#x' % byte for byte in data]
    with open(s_file, 'w') as fd:
        print('''/* Auto-generated C program to produce an ELF file for testing */

.section .text
.code32
.globl _start
.type _start, @function
%s
_start:
%s
.ident "comment"

.comm fred,8,4

.section .empty
.globl _empty
_empty:
.byte 1

.globl ernie
.data
.type ernie, @object
.size ernie, 4
ernie:
%s
''' % ('\n'.join(text_bytes1), '\n'.join(text_bytes2), '\n'.join(data_bytes)),
        file=fd)
    lds_file = os.path.join(outdir, 'elf.lds')

    # Use a linker script to set the alignment and text address.
    with open(lds_file, 'w') as fd:
        print('''/* Auto-generated linker script to produce an ELF file for testing */

PHDRS
{
    text PT_LOAD ;
    data PT_LOAD ;
    empty PT_LOAD FLAGS ( 6 ) ;
    note PT_NOTE ;
}

SECTIONS
{
    . = 0xfef20000;
    ENTRY(_start)
    .text . : SUBALIGN(0)
    {
        *(.text)
    } :text
    .data : {
        *(.data)
    } :data
    _bss_start = .;
    .empty : {
        *(.empty)
    } :empty
    /DISCARD/ : {
        *(.note.gnu.property)
    }
    .note : {
        *(.comment)
    } :note
    .bss _bss_start  (OVERLAY) : {
        *(.bss)
    }
}
''', file=fd)
    # -static: Avoid requiring any shared libraries
    # -nostdlib: Don't link with C library
    # -Wl,--build-id=none: Don't generate a build ID, so that we just get the
    #   text section at the start
    # -m32: Build for 32-bit x86
    # -T...: Specifies the link script, which sets the start address
    cc, args = tools.get_target_compile_tool('cc')
    args += ['-static', '-nostdlib', '-Wl,--build-id=none', '-m32', '-T',
            lds_file, '-o', elf_fname, s_file]
    stdout = command.output(cc, *args)
    shutil.rmtree(outdir)

def DecodeElf(data, location):
    """Decode an ELF file and return information about it

    Args:
        data: Data from ELF file
        location: Start address of data to return

    Returns:
        ElfInfo object containing information about the decoded ELF file
    """
    if not ELF_TOOLS:
        raise ValueError("Python: No module named 'elftools'")
    file_size = len(data)
    with io.BytesIO(data) as fd:
        elf = ELFFile(fd)
        data_start = 0xffffffff
        data_end = 0
        mem_end = 0
        virt_to_phys = 0

        for i in range(elf.num_segments()):
            segment = elf.get_segment(i)
            if segment['p_type'] != 'PT_LOAD' or not segment['p_memsz']:
                skipped = 1  # To make code-coverage see this line
                continue
            start = segment['p_paddr']
            mend = start + segment['p_memsz']
            rend = start + segment['p_filesz']
            data_start = min(data_start, start)
            data_end = max(data_end, rend)
            mem_end = max(mem_end, mend)
            if not virt_to_phys:
                virt_to_phys = segment['p_paddr'] - segment['p_vaddr']

        output = bytearray(data_end - data_start)
        for i in range(elf.num_segments()):
            segment = elf.get_segment(i)
            if segment['p_type'] != 'PT_LOAD' or not segment['p_memsz']:
                skipped = 1  # To make code-coverage see this line
                continue
            start = segment['p_paddr']
            offset = 0
            if start < location:
                offset = location - start
                start = location
            # A legal ELF file can have a program header with non-zero length
            # but zero-length file size and a non-zero offset which, added
            # together, are greater than input->size (i.e. the total file size).
            #  So we need to not even test in the case that p_filesz is zero.
            # Note: All of this code is commented out since we don't have a test
            # case for it.
            size = segment['p_filesz']
            #if not size:
                #continue
            #end = segment['p_offset'] + segment['p_filesz']
            #if end > file_size:
                #raise ValueError('Underflow copying out the segment. File has %#x bytes left, segment end is %#x\n',
                                 #file_size, end)
            output[start - data_start:start - data_start + size] = (
                segment.data()[offset:])
    return ElfInfo(output, data_start, elf.header['e_entry'] + virt_to_phys,
                   mem_end - data_start)

def UpdateFile(infile, outfile, start_sym, end_sym, insert):
    tout.notice("Creating file '%s' with data length %#x (%d) between symbols '%s' and '%s'" %
                (outfile, len(insert), len(insert), start_sym, end_sym))
    syms = GetSymbolFileOffset(infile, [start_sym, end_sym])
    if len(syms) != 2:
        raise ValueError("Expected two symbols '%s' and '%s': got %d: %s" %
                         (start_sym, end_sym, len(syms),
                          ','.join(syms.keys())))

    size = syms[end_sym].offset - syms[start_sym].offset
    if len(insert) > size:
        raise ValueError("Not enough space in '%s' for data length %#x (%d); size is %#x (%d)" %
                         (infile, len(insert), len(insert), size, size))

    data = tools.read_file(infile)
    newdata = data[:syms[start_sym].offset]
    newdata += insert + tools.get_bytes(0, size - len(insert))
    newdata += data[syms[end_sym].offset:]
    tools.write_file(outfile, newdata)
    tout.info('Written to offset %#x' % syms[start_sym].offset)

def read_loadable_segments(data):
    """Read segments from an ELF file

    Args:
        data (bytes): Contents of file

    Returns:
        tuple:
            list of segments, each:
                int: Segment number (0 = first)
                int: Start address of segment in memory
                bytes: Contents of segment
            int: entry address for image

    Raises:
        ValueError: elftools is not available
    """
    if not ELF_TOOLS:
        raise ValueError("Python: No module named 'elftools'")
    with io.BytesIO(data) as inf:
        try:
            elf = ELFFile(inf)
        except ELFError as err:
            raise ValueError(err)
        entry = elf.header['e_entry']
        segments = []
        for i in range(elf.num_segments()):
            segment = elf.get_segment(i)
            if segment['p_type'] != 'PT_LOAD' or not segment['p_memsz']:
                skipped = 1  # To make code-coverage see this line
                continue
            start = segment['p_offset']
            rend = start + segment['p_filesz']
            segments.append((i, segment['p_paddr'], data[start:rend]))
    return segments, entry

def is_valid(data):
    """Check if some binary data is a valid ELF file

    Args:
        data (bytes): Bytes to check

    Returns:
        bool: True if a valid Elf file, False if not
    """
    try:
        DecodeElf(data, 0)
        return True
    except ELFError:
        return False
