# SPDX-License-Identifier: GPL-2.0+
# Copyright 2019 Google LLC
# Written by Simon Glass <sjg@chromium.org>

"""Support for coreboot's CBFS format

CBFS supports a header followed by a number of files, generally targeted at SPI
flash.

The format is somewhat defined by documentation in the coreboot tree although
it is necessary to rely on the C structures and source code (mostly cbfstool)
to fully understand it.

Currently supported: raw and stage types with compression, padding empty areas
    with empty files, fixed-offset files
"""

from collections import OrderedDict
import io
import struct
import sys

from binman import bintool
from binman import elf
from u_boot_pylib import command
from u_boot_pylib import tools

# Set to True to enable printing output while working
DEBUG = False

# Set to True to enable output from running cbfstool for debugging
VERBOSE = False

# The master header, at the start of the CBFS
HEADER_FORMAT      = '>IIIIIIII'
HEADER_LEN         = 0x20
HEADER_MAGIC       = 0x4f524243
HEADER_VERSION1    = 0x31313131
HEADER_VERSION2    = 0x31313132

# The file header, at the start of each file in the CBFS
FILE_HEADER_FORMAT = b'>8sIIII'
FILE_HEADER_LEN    = 0x18
FILE_MAGIC         = b'LARCHIVE'
ATTRIBUTE_ALIGN    = 4   # All attribute sizes must be divisible by this

# A stage-header attribute containing information about 'stage' files
# Yes this is correct: this header is in litte-endian format
ATTR_STAGE_FORMAT       = '>IIQII'
ATTR_STAGE_LEN          = 0x18

# An attribute describring the compression used in a file
ATTR_COMPRESSION_FORMAT = '>IIII'
ATTR_COMPRESSION_LEN = 0x10

# Attribute tags
FILE_ATTR_TAG_COMPRESSION   = 0x42435a4c
FILE_ATTR_TAG_HASH          = 0x68736148
FILE_ATTR_TAG_POSITION      = 0x42435350  # PSCB
FILE_ATTR_TAG_ALIGNMENT     = 0x42434c41  # ALCB
FILE_ATTR_TAG_PADDING       = 0x47444150  # PDNG
FILE_ATTR_TAG_STAGEHEADER   = 0x53746748  # StgH

# This is 'the size of bootblock reserved in firmware image (cbfs.txt)'
# Not much more info is available, but we set it to 4, due to this comment in
# cbfstool.c:
# This causes 4 bytes to be left out at the end of the image, for two reasons:
# 1. The cbfs master header pointer resides there
# 2. Ssme cbfs implementations assume that an image that resides below 4GB has
#    a bootblock and get confused when the end of the image is at 4GB == 0.
MIN_BOOTBLOCK_SIZE     = 4

# Files start aligned to this boundary in the CBFS
ENTRY_ALIGN    = 0x40

# CBFSs must declare an architecture since much of the logic is designed with
# x86 in mind. The effect of setting this value is not well documented, but in
# general x86 is used and this makes use of a boot block and an image that ends
# at the end of 32-bit address space.
ARCHITECTURE_UNKNOWN  = 0xffffffff
ARCHITECTURE_X86      = 0x00000001
ARCHITECTURE_ARM      = 0x00000010
ARCHITECTURE_AARCH64  = 0x0000aa64
ARCHITECTURE_MIPS     = 0x00000100
ARCHITECTURE_RISCV    = 0xc001d0de
ARCHITECTURE_PPC64    = 0x407570ff

ARCH_NAMES = {
    ARCHITECTURE_UNKNOWN  : 'unknown',
    ARCHITECTURE_X86      : 'x86',
    ARCHITECTURE_ARM      : 'arm',
    ARCHITECTURE_AARCH64  : 'arm64',
    ARCHITECTURE_MIPS     : 'mips',
    ARCHITECTURE_RISCV    : 'riscv',
    ARCHITECTURE_PPC64    : 'ppc64',
    }

# File types. Only supported ones are included here
TYPE_CBFSHEADER     = 0x02   # Master header, HEADER_FORMAT
TYPE_LEGACY_STAGE   = 0x10   # Stage, holding an executable
TYPE_STAGE          = 0x11   # New-type stage with ATTR_STAGE_FORMAT
TYPE_RAW            = 0x50   # Raw file, possibly compressed
TYPE_EMPTY          = 0xffffffff     # Empty data

# Compression types
COMPRESS_NONE, COMPRESS_LZMA, COMPRESS_LZ4 = range(3)

COMPRESS_NAMES = {
    COMPRESS_NONE : 'none',
    COMPRESS_LZMA : 'lzma',
    COMPRESS_LZ4  : 'lz4',
    }

def find_arch(find_name):
    """Look up an architecture name

    Args:
        find_name: Architecture name to find

    Returns:
        ARCHITECTURE_... value or None if not found
    """
    for arch, name in ARCH_NAMES.items():
        if name == find_name:
            return arch
    return None

def find_compress(find_name):
    """Look up a compression algorithm name

    Args:
        find_name: Compression algorithm name to find

    Returns:
        COMPRESS_... value or None if not found
    """
    for compress, name in COMPRESS_NAMES.items():
        if name == find_name:
            return compress
    return None

def compress_name(compress):
    """Look up the name of a compression algorithm

    Args:
        compress: Compression algorithm number to find (COMPRESS_...)

    Returns:
        Compression algorithm name (string)

    Raises:
        KeyError if the algorithm number is invalid
    """
    return COMPRESS_NAMES[compress]

def align_int(val, align):
    """Align a value up to the given alignment

    Args:
        val: Integer value to align
        align: Integer alignment value (e.g. 4 to align to 4-byte boundary)

    Returns:
        integer value aligned to the required boundary, rounding up if necessary
    """
    return int((val + align - 1) / align) * align

def align_int_down(val, align):
    """Align a value down to the given alignment

    Args:
        val: Integer value to align
        align: Integer alignment value (e.g. 4 to align to 4-byte boundary)

    Returns:
        integer value aligned to the required boundary, rounding down if
            necessary
    """
    return int(val / align) * align

def _pack_string(instr):
    """Pack a string to the required aligned size by adding padding

    Args:
        instr: String to process

    Returns:
        String with required padding (at least one 0x00 byte) at the end
    """
    val = tools.to_bytes(instr)
    pad_len = align_int(len(val) + 1, ATTRIBUTE_ALIGN)
    return val + tools.get_bytes(0, pad_len - len(val))


class CbfsFile(object):
    """Class to represent a single CBFS file

    This is used to hold the information about a file, including its contents.
    Use the get_data_and_offset() method to obtain the raw output for writing to
    CBFS.

    Properties:
        name: Name of file
        offset: Offset of file data from start of file header
        cbfs_offset: Offset of file data in bytes from start of CBFS, or None to
            place this file anyway
        data: Contents of file, uncompressed
        orig_data: Original data added to the file, possibly compressed
        data_len: Length of (possibly compressed) data in bytes
        ftype: File type (TYPE_...)
        compression: Compression type (COMPRESS_...)
        memlen: Length of data in memory, i.e. the uncompressed length, None if
            no compression algortihm is selected
        load: Load address in memory if known, else None
        entry: Entry address in memory if known, else None. This is where
            execution starts after the file is loaded
        base_address: Base address to use for 'stage' files
        erase_byte: Erase byte to use for padding between the file header and
            contents (used for empty files)
        size: Size of the file in bytes (used for empty files)
    """
    def __init__(self, name, ftype, data, cbfs_offset, compress=COMPRESS_NONE):
        self.name = name
        self.offset = None
        self.cbfs_offset = cbfs_offset
        self.data = data
        self.orig_data = data
        self.ftype = ftype
        self.compress = compress
        self.memlen = None
        self.load = None
        self.entry = None
        self.base_address = None
        self.data_len = len(data)
        self.erase_byte = None
        self.size = None
        if self.compress == COMPRESS_LZ4:
            self.comp_bintool = bintool.Bintool.create('lz4')
        elif self.compress == COMPRESS_LZMA:
            self.comp_bintool = bintool.Bintool.create('lzma_alone')
        else:
            self.comp_bintool = None

    def decompress(self):
        """Handle decompressing data if necessary"""
        indata = self.data
        if self.comp_bintool:
            data = self.comp_bintool.decompress(indata)
        else:
            data = indata
        self.memlen = len(data)
        self.data = data
        self.data_len = len(indata)

    @classmethod
    def stage(cls, base_address, name, data, cbfs_offset):
        """Create a new stage file

        Args:
            base_address: Int base address for memory-mapping of ELF file
            name: String file name to put in CBFS (does not need to correspond
                to the name that the file originally came from)
            data: Contents of file
            cbfs_offset: Offset of file data in bytes from start of CBFS, or
                None to place this file anyway

        Returns:
            CbfsFile object containing the file information
        """
        cfile = CbfsFile(name, TYPE_STAGE, data, cbfs_offset)
        cfile.base_address = base_address
        return cfile

    @classmethod
    def raw(cls, name, data, cbfs_offset, compress):
        """Create a new raw file

        Args:
            name: String file name to put in CBFS (does not need to correspond
                to the name that the file originally came from)
            data: Contents of file
            cbfs_offset: Offset of file data in bytes from start of CBFS, or
                None to place this file anyway
            compress: Compression algorithm to use (COMPRESS_...)

        Returns:
            CbfsFile object containing the file information
        """
        return CbfsFile(name, TYPE_RAW, data, cbfs_offset, compress)

    @classmethod
    def empty(cls, space_to_use, erase_byte):
        """Create a new empty file of a given size

        Args:
            space_to_use:: Size of available space, which must be at least as
                large as the alignment size for this CBFS
            erase_byte: Byte to use for contents of file (repeated through the
                whole file)

        Returns:
            CbfsFile object containing the file information
        """
        cfile = CbfsFile('', TYPE_EMPTY, b'', None)
        cfile.size = space_to_use - FILE_HEADER_LEN - ATTRIBUTE_ALIGN
        cfile.erase_byte = erase_byte
        return cfile

    def calc_start_offset(self):
        """Check if this file needs to start at a particular offset in CBFS

        Returns:
            None if the file can be placed anywhere, or
            the largest offset where the file could start (integer)
        """
        if self.cbfs_offset is None:
            return None
        return self.cbfs_offset - self.get_header_len()

    def get_header_len(self):
        """Get the length of headers required for a file

        This is the minimum length required before the actual data for this file
        could start. It might start later if there is padding.

        Returns:
            Total length of all non-data fields, in bytes
        """
        name = _pack_string(self.name)
        hdr_len = len(name) + FILE_HEADER_LEN
        if self.ftype == TYPE_STAGE:
            hdr_len += ATTR_STAGE_LEN
        elif self.ftype == TYPE_RAW:
            if self.compress:
                hdr_len += ATTR_COMPRESSION_LEN
        elif self.ftype == TYPE_EMPTY:
            pass
        else:
            raise ValueError('Unknown file type %#x\n' % self.ftype)
        return hdr_len

    def get_data_and_offset(self, offset=None, pad_byte=None):
        """Obtain the contents of the file, in CBFS format and the offset of
        the data within the file

        Returns:
            tuple:
                bytes representing the contents of this file, packed and aligned
                    for directly inserting into the final CBFS output
                offset to the file data from the start of the returned data.
        """
        name = _pack_string(self.name)
        hdr_len = len(name) + FILE_HEADER_LEN
        attr_pos = 0
        content = b''
        attr = b''
        pad = b''
        data = self.data
        if self.ftype == TYPE_STAGE:
            elf_data = elf.DecodeElf(data, self.base_address)
            attr = struct.pack(ATTR_STAGE_FORMAT, FILE_ATTR_TAG_STAGEHEADER,
                               ATTR_STAGE_LEN, elf_data.load,
                               elf_data.entry - elf_data.load, elf_data.memsize)
            data = elf_data.data
        elif self.ftype == TYPE_RAW:
            orig_data = data
            if self.comp_bintool:
                data = self.comp_bintool.compress(orig_data)
            self.memlen = len(orig_data)
            self.data_len = len(data)
            if self.compress:
                attr = struct.pack(ATTR_COMPRESSION_FORMAT,
                                   FILE_ATTR_TAG_COMPRESSION,
                                   ATTR_COMPRESSION_LEN, self.compress,
                                   self.memlen)
        elif self.ftype == TYPE_EMPTY:
            data = tools.get_bytes(self.erase_byte, self.size)
        else:
            raise ValueError('Unknown type %#x when writing\n' % self.ftype)
        if attr:
            attr_pos = hdr_len
            hdr_len += len(attr)
        if self.cbfs_offset is not None:
            pad_len = self.cbfs_offset - offset - hdr_len
            if pad_len < 0:  # pragma: no cover
                # Test coverage of this is not available since this should never
                # happen. It indicates that get_header_len() provided an
                # incorrect value (too small) so that we decided that we could
                # put this file at the requested place, but in fact a previous
                # file extends far enough into the CBFS that this is not
                # possible.
                raise ValueError("Internal error: CBFS file '%s': Requested offset %#x but current output position is %#x" %
                                 (self.name, self.cbfs_offset, offset))
            pad = tools.get_bytes(pad_byte, pad_len)
            if attr_pos:
                attr_pos += pad_len
            hdr_len += pad_len

        # This is the offset of the start of the file's data,
        size = len(content) + len(data)
        hdr = struct.pack(FILE_HEADER_FORMAT, FILE_MAGIC, size,
                          self.ftype, attr_pos, hdr_len)

        # Do a sanity check of the get_header_len() function, to ensure that it
        # stays in lockstep with this function
        expected_len = self.get_header_len()
        actual_len = len(hdr + name + attr)
        if expected_len != actual_len:  # pragma: no cover
            # Test coverage of this is not available since this should never
            # happen. It probably indicates that get_header_len() is broken.
            raise ValueError("Internal error: CBFS file '%s': Expected headers of %#x bytes, got %#x" %
                             (self.name, expected_len, actual_len))
        return hdr + name + pad + attr + content + data, hdr_len


class CbfsWriter(object):
    """Class to handle writing a Coreboot File System (CBFS)

    Usage is something like:

        cbw = CbfsWriter(size)
        cbw.add_file_raw('u-boot', tools.read_file('u-boot.bin'))
        ...
        data, cbfs_offset = cbw.get_data_and_offset()

    Attributes:
        _master_name: Name of the file containing the master header
        _size: Size of the filesystem, in bytes
        _files: Ordered list of files in the CBFS, each a CbfsFile
        _arch: Architecture of the CBFS (ARCHITECTURE_...)
        _bootblock_size: Size of the bootblock, typically at the end of the CBFS
        _erase_byte: Byte to use for empty space in the CBFS
        _align: Alignment to use for files, typically ENTRY_ALIGN
        _base_address: Boot block offset in bytes from the start of CBFS.
            Typically this is located at top of the CBFS. It is 0 when there is
            no boot block
        _header_offset: Offset of master header in bytes from start of CBFS
        _contents_offset: Offset of first file header
        _hdr_at_start: True if the master header is at the start of the CBFS,
            instead of the end as normal for x86
        _add_fileheader: True to add a fileheader around the master header
    """
    def __init__(self, size, arch=ARCHITECTURE_X86):
        """Set up a new CBFS

        This sets up all properties to default values. Files can be added using
        add_file_raw(), etc.

        Args:
            size: Size of CBFS in bytes
            arch: Architecture to declare for CBFS
        """
        self._master_name = 'cbfs master header'
        self._size = size
        self._files = OrderedDict()
        self._arch = arch
        self._bootblock_size = 0
        self._erase_byte = 0xff

        # Small padding to align a file uses 0
        self._small_pad_byte = 0
        self._align = ENTRY_ALIGN
        self._add_fileheader = False
        if self._arch == ARCHITECTURE_X86:
            # Allow 4 bytes for the header pointer. That holds the
            # twos-compliment negative offset of the master header in bytes
            # measured from one byte past the end of the CBFS
            self._base_address = self._size - max(self._bootblock_size,
                                                  MIN_BOOTBLOCK_SIZE)
            self._header_offset = self._base_address - HEADER_LEN
            self._contents_offset = 0
            self._hdr_at_start = False
        else:
            # For non-x86, different rules apply
            self._base_address = 0
            self._header_offset = align_int(self._base_address +
                                            self._bootblock_size, 4)
            self._contents_offset = align_int(self._header_offset +
                                              FILE_HEADER_LEN +
                                              self._bootblock_size, self._align)
            self._hdr_at_start = True

    def _skip_to(self, fd, offset, pad_byte):
        """Write out pad bytes until a given offset

        Args:
            fd: File objext to write to
            offset: Offset to write to
        """
        if fd.tell() > offset:
            raise ValueError('No space for data before offset %#x (current offset %#x)' %
                             (offset, fd.tell()))
        fd.write(tools.get_bytes(pad_byte, offset - fd.tell()))

    def _pad_to(self, fd, offset, pad_byte):
        """Write out pad bytes and/or an empty file until a given offset

        Args:
            fd: File objext to write to
            offset: Offset to write to
        """
        self._align_to(fd, self._align, pad_byte)
        upto = fd.tell()
        if upto > offset:
            raise ValueError('No space for data before pad offset %#x (current offset %#x)' %
                             (offset, upto))
        todo = align_int_down(offset - upto, self._align)
        if todo:
            cbf = CbfsFile.empty(todo, self._erase_byte)
            fd.write(cbf.get_data_and_offset()[0])
        self._skip_to(fd, offset, pad_byte)

    def _align_to(self, fd, align, pad_byte):
        """Write out pad bytes until a given alignment is reached

        This only aligns if the resulting output would not reach the end of the
        CBFS, since we want to leave the last 4 bytes for the master-header
        pointer.

        Args:
            fd: File objext to write to
            align: Alignment to require (e.g. 4 means pad to next 4-byte
                boundary)
        """
        offset = align_int(fd.tell(), align)
        if offset < self._size:
            self._skip_to(fd, offset, pad_byte)

    def add_file_stage(self, name, data, cbfs_offset=None):
        """Add a new stage file to the CBFS

        Args:
            name: String file name to put in CBFS (does not need to correspond
                to the name that the file originally came from)
            data: Contents of file
            cbfs_offset: Offset of this file's data within the CBFS, in bytes,
                or None to place this file anywhere

        Returns:
            CbfsFile object created
        """
        cfile = CbfsFile.stage(self._base_address, name, data, cbfs_offset)
        self._files[name] = cfile
        return cfile

    def add_file_raw(self, name, data, cbfs_offset=None,
                     compress=COMPRESS_NONE):
        """Create a new raw file

        Args:
            name: String file name to put in CBFS (does not need to correspond
                to the name that the file originally came from)
            data: Contents of file
            cbfs_offset: Offset of this file's data within the CBFS, in bytes,
                or None to place this file anywhere
            compress: Compression algorithm to use (COMPRESS_...)

        Returns:
            CbfsFile object created
        """
        cfile = CbfsFile.raw(name, data, cbfs_offset, compress)
        self._files[name] = cfile
        return cfile

    def _write_header(self, fd, add_fileheader):
        """Write out the master header to a CBFS

        Args:
            fd: File object
            add_fileheader: True to place the master header in a file header
                record
        """
        if fd.tell() > self._header_offset:
            raise ValueError('No space for header at offset %#x (current offset %#x)' %
                             (self._header_offset, fd.tell()))
        if not add_fileheader:
            self._pad_to(fd, self._header_offset, self._erase_byte)
        hdr = struct.pack(HEADER_FORMAT, HEADER_MAGIC, HEADER_VERSION2,
                          self._size, self._bootblock_size, self._align,
                          self._contents_offset, self._arch, 0xffffffff)
        if add_fileheader:
            name = _pack_string(self._master_name)
            fd.write(struct.pack(FILE_HEADER_FORMAT, FILE_MAGIC, len(hdr),
                                 TYPE_CBFSHEADER, 0,
                                 FILE_HEADER_LEN + len(name)))
            fd.write(name)
            self._header_offset = fd.tell()
            fd.write(hdr)
            self._align_to(fd, self._align, self._erase_byte)
        else:
            fd.write(hdr)

    def get_data(self):
        """Obtain the full contents of the CBFS

        Thhis builds the CBFS with headers and all required files.

        Returns:
            'bytes' type containing the data
        """
        fd = io.BytesIO()

        # THe header can go at the start in some cases
        if self._hdr_at_start:
            self._write_header(fd, add_fileheader=self._add_fileheader)
        self._skip_to(fd, self._contents_offset, self._erase_byte)

        # Write out each file
        for cbf in self._files.values():
            # Place the file at its requested place, if any
            offset = cbf.calc_start_offset()
            if offset is not None:
                self._pad_to(fd, align_int_down(offset, self._align),
                             self._erase_byte)
            pos = fd.tell()
            data, data_offset = cbf.get_data_and_offset(pos,
                                                        self._small_pad_byte)
            fd.write(data)
            self._align_to(fd, self._align, self._erase_byte)
            cbf.calced_cbfs_offset = pos + data_offset
        if not self._hdr_at_start:
            self._write_header(fd, add_fileheader=self._add_fileheader)

        # Pad to the end and write a pointer to the CBFS master header
        self._pad_to(fd, self._base_address or self._size - 4, self._erase_byte)
        rel_offset = self._header_offset - self._size
        fd.write(struct.pack('<I', rel_offset & 0xffffffff))

        return fd.getvalue()


class CbfsReader(object):
    """Class to handle reading a Coreboot File System (CBFS)

    Usage is something like:
        cbfs = cbfs_util.CbfsReader(data)
        cfile = cbfs.files['u-boot']
        self.WriteFile('u-boot.bin', cfile.data)

    Attributes:
        files: Ordered list of CbfsFile objects
        align: Alignment to use for files, typically ENTRT_ALIGN
        stage_base_address: Base address to use when mapping ELF files into the
            CBFS for TYPE_STAGE files. If this is larger than the code address
            of the ELF file, then data at the start of the ELF file will not
            appear in the CBFS. Currently there are no tests for behaviour as
            documentation is sparse
        magic: Integer magic number from master header (HEADER_MAGIC)
        version: Version number of CBFS (HEADER_VERSION2)
        rom_size: Size of CBFS
        boot_block_size: Size of boot block
        cbfs_offset: Offset of the first file in bytes from start of CBFS
        arch: Architecture of CBFS file (ARCHITECTURE_...)
    """
    def __init__(self, data, read=True):
        self.align = ENTRY_ALIGN
        self.arch = None
        self.boot_block_size = None
        self.cbfs_offset = None
        self.files = OrderedDict()
        self.magic = None
        self.rom_size = None
        self.stage_base_address = 0
        self.version = None
        self.data = data
        if read:
            self.read()

    def read(self):
        """Read all the files in the CBFS and add them to self.files"""
        with io.BytesIO(self.data) as fd:
            # First, get the master header
            if not self._find_and_read_header(fd, len(self.data)):
                raise ValueError('Cannot find master header')
            fd.seek(self.cbfs_offset)

            # Now read in the files one at a time
            while True:
                cfile = self._read_next_file(fd)
                if cfile:
                    self.files[cfile.name] = cfile
                elif cfile is False:
                    break

    def _find_and_read_header(self, fd, size):
        """Find and read the master header in the CBFS

        This looks at the pointer word at the very end of the CBFS. This is an
        offset to the header relative to the size of the CBFS, which is assumed
        to be known. Note that the offset is in *little endian* format.

        Args:
            fd: File to read from
            size: Size of file

        Returns:
            True if header was found, False if not
        """
        orig_pos = fd.tell()
        fd.seek(size - 4)
        rel_offset, = struct.unpack('<I', fd.read(4))
        pos = (size + rel_offset) & 0xffffffff
        fd.seek(pos)
        found = self._read_header(fd)
        if not found:
            print('Relative offset seems wrong, scanning whole image')
            for pos in range(0, size - HEADER_LEN, 4):
                fd.seek(pos)
                found = self._read_header(fd)
                if found:
                    break
        fd.seek(orig_pos)
        return found

    def _read_next_file(self, fd):
        """Read the next file from a CBFS

        Args:
            fd: File to read from

        Returns:
            CbfsFile object, if found
            None if no object found, but data was parsed (e.g. TYPE_CBFSHEADER)
            False if at end of CBFS and reading should stop
        """
        file_pos = fd.tell()
        data = fd.read(FILE_HEADER_LEN)
        if len(data) < FILE_HEADER_LEN:
            print('File header at %#x ran out of data' % file_pos)
            return False
        magic, size, ftype, attr, offset = struct.unpack(FILE_HEADER_FORMAT,
                                                         data)
        if magic != FILE_MAGIC:
            return False
        pos = fd.tell()
        name = self._read_string(fd)
        if name is None:
            print('String at %#x ran out of data' % pos)
            return False

        if DEBUG:
            print('name', name)

        # If there are attribute headers present, read those
        attrs = self._read_attr(fd, file_pos, attr, offset)
        if attrs is None:
            return False

        # Create the correct CbfsFile object depending on the type
        cfile = None
        cbfs_offset = file_pos + offset
        fd.seek(cbfs_offset, io.SEEK_SET)
        if DEBUG:
            print(f'ftype {ftype:x}')
        if ftype == TYPE_CBFSHEADER:
            self._read_header(fd)
        elif ftype == TYPE_STAGE:
            cfile = CbfsFile.stage(self.stage_base_address, name, b'',
                                   cbfs_offset)
            cfile.load, entry_offset, cfile.memlen = attrs
            cfile.entry = cfile.load + entry_offset
            cfile.data = fd.read(cfile.memlen)
            cfile.data_len = cfile.memlen
        elif ftype == TYPE_RAW:
            data = fd.read(size)
            cfile = CbfsFile.raw(name, data, cbfs_offset, attrs)
            cfile.decompress()
            if DEBUG:
                print('data', data)
        elif ftype == TYPE_EMPTY:
            # Just read the data and discard it, since it is only padding
            fd.read(size)
            cfile = CbfsFile('', TYPE_EMPTY, b'', cbfs_offset)
        else:
            raise ValueError('Unknown type %#x when reading\n' % ftype)
        if cfile:
            cfile.offset = offset

        # Move past the padding to the start of a possible next file. If we are
        # already at an alignment boundary, then there is no padding.
        pad = (self.align - fd.tell() % self.align) % self.align
        fd.seek(pad, io.SEEK_CUR)
        return cfile

    @classmethod
    def _read_attr(cls, fd, file_pos, attr, offset):
        """Read attributes from the file

        CBFS files can have attributes which are things that cannot fit into the
        header. The only attributes currently supported are compression, stage
        header and the unused tag

        Args:
            fd: File to read from
            file_pos: Position of file in fd
            attr: Offset of attributes, 0 if none
            offset: Offset of file data (used to indicate the end of the
                                         attributes)

        Returns:
            Either:
                Compression to use for the file (COMPRESS_...)
                tuple containing stage info:
                    load address
                    entry offset
                    memory size
        """
        attrs = None
        if not attr:
            return COMPRESS_NONE
        attr_size = offset - attr
        fd.seek(file_pos + attr, io.SEEK_SET)
        while attr_size:
            pos = fd.tell()
            hdr = fd.read(8)
            if len(hdr) < 8:
                print('Attribute tag at %x ran out of data' % pos)
                return None
            atag, alen = struct.unpack(">II", hdr)
            data = hdr + fd.read(alen - 8)
            if atag == FILE_ATTR_TAG_COMPRESSION:
                # We don't currently use this information
                atag, alen, compress, _decomp_size = struct.unpack(
                    ATTR_COMPRESSION_FORMAT, data)
                attrs = compress
            elif atag == FILE_ATTR_TAG_STAGEHEADER:
                atag, alen, load, entry_offset, memsize = struct.unpack(
                    ATTR_STAGE_FORMAT, data)
                attrs = (load, entry_offset, memsize)
            else:
                print('Unknown attribute tag %x' % atag)
            attr_size -= len(data)
        return attrs

    def _read_header(self, fd):
        """Read the master header

        Reads the header and stores the information obtained into the member
        variables.

        Args:
            fd: File to read from

        Returns:
            True if header was read OK, False if it is truncated or has the
                wrong magic or version
        """
        pos = fd.tell()
        data = fd.read(HEADER_LEN)
        if len(data) < HEADER_LEN:
            print('Header at %x ran out of data' % pos)
            return False
        (self.magic, self.version, self.rom_size, self.boot_block_size,
         self.align, self.cbfs_offset, self.arch, _) = struct.unpack(
             HEADER_FORMAT, data)
        return self.magic == HEADER_MAGIC and (
            self.version == HEADER_VERSION1 or
            self.version == HEADER_VERSION2)

    @classmethod
    def _read_string(cls, fd):
        """Read a string from a file

        This reads a string and aligns the data to the next alignment boundary.
        The string must be nul-terminated

        Args:
            fd: File to read from

        Returns:
            string read ('str' type) encoded to UTF-8, or None if we ran out of
                data
        """
        val = b''
        while True:
            data = fd.read(ATTRIBUTE_ALIGN)
            if len(data) < ATTRIBUTE_ALIGN:
                return None
            pos = data.find(b'\0')
            if pos == -1:
                val += data
            else:
                val += data[:pos]
                break
        return val.decode('utf-8')
