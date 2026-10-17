#!/usr/bin/env python
# SPDX-License-Identifier: GPL-2.0+
# Copyright 2019 Google LLC
# Written by Simon Glass <sjg@chromium.org>

"""Tests for cbfs_util

These create and read various CBFSs and compare the results with expected
values and with cbfstool
"""

import io
import os
import shutil
import struct
import tempfile
import unittest

from binman import bintool
from binman import cbfs_util
from binman.cbfs_util import CbfsWriter
from binman import elf
from u_boot_pylib import test_util
from u_boot_pylib import tools

U_BOOT_DATA           = b'1234'
U_BOOT_DTB_DATA       = b'udtb'
COMPRESS_DATA         = b'compress xxxxxxxxxxxxxxxxxxxxxx data'


class TestCbfs(unittest.TestCase):
    """Test of cbfs_util classes"""
    #pylint: disable=W0212
    @classmethod
    def setUpClass(cls):
        # Create a temporary directory for test files
        cls._indir = tempfile.mkdtemp(prefix='cbfs_util.')
        tools.set_input_dirs([cls._indir])

        # Set up some useful data files
        TestCbfs._make_input_file('u-boot.bin', U_BOOT_DATA)
        TestCbfs._make_input_file('u-boot.dtb', U_BOOT_DTB_DATA)
        TestCbfs._make_input_file('compress', COMPRESS_DATA)

        # Set up a temporary output directory, used by the tools library when
        # compressing files
        tools.prepare_output_dir(None)

        cls.cbfstool = bintool.Bintool.create('cbfstool')
        cls.have_cbfstool = cls.cbfstool.is_present()

        lz4 = bintool.Bintool.create('lz4')
        cls.have_lz4 = lz4.is_present()

    @classmethod
    def tearDownClass(cls):
        """Remove the temporary input directory and its contents"""
        if cls._indir:
            shutil.rmtree(cls._indir)
        cls._indir = None
        tools.finalise_output_dir()

    @classmethod
    def _make_input_file(cls, fname, contents):
        """Create a new test input file, creating directories as needed

        Args:
            fname: Filename to create
            contents: File contents to write in to the file
        Returns:
            Full pathname of file created
        """
        pathname = os.path.join(cls._indir, fname)
        tools.write_file(pathname, contents)
        return pathname

    def _check_hdr(self, data, size, offset=0, arch=cbfs_util.ARCHITECTURE_X86):
        """Check that the CBFS has the expected header

        Args:
            data: Data to check
            size: Expected ROM size
            offset: Expected offset to first CBFS file
            arch: Expected architecture

        Returns:
            CbfsReader object containing the CBFS
        """
        cbfs = cbfs_util.CbfsReader(data)
        self.assertEqual(cbfs_util.HEADER_MAGIC, cbfs.magic)
        self.assertEqual(cbfs_util.HEADER_VERSION2, cbfs.version)
        self.assertEqual(size, cbfs.rom_size)
        self.assertEqual(0, cbfs.boot_block_size)
        self.assertEqual(cbfs_util.ENTRY_ALIGN, cbfs.align)
        self.assertEqual(offset, cbfs.cbfs_offset)
        self.assertEqual(arch, cbfs.arch)
        return cbfs

    def _check_uboot(self, cbfs, ftype=cbfs_util.TYPE_RAW, offset=0x20,
                     data=U_BOOT_DATA, cbfs_offset=None):
        """Check that the U-Boot file is as expected

        Args:
            cbfs: CbfsReader object to check
            ftype: Expected file type
            offset: Expected offset of file
            data: Expected data in file
            cbfs_offset: Expected CBFS offset for file's data

        Returns:
            CbfsFile object containing the file
        """
        self.assertIn('u-boot', cbfs.files)
        cfile = cbfs.files['u-boot']
        self.assertEqual('u-boot', cfile.name)
        self.assertEqual(offset, cfile.offset)
        if cbfs_offset is not None:
            self.assertEqual(cbfs_offset, cfile.cbfs_offset)
        self.assertEqual(data, cfile.data)
        self.assertEqual(ftype, cfile.ftype)
        self.assertEqual(cbfs_util.COMPRESS_NONE, cfile.compress)
        self.assertEqual(len(data), cfile.memlen)
        return cfile

    def _check_dtb(self, cbfs, offset=0x24, data=U_BOOT_DTB_DATA,
                   cbfs_offset=None):
        """Check that the U-Boot dtb file is as expected

        Args:
            cbfs: CbfsReader object to check
            offset: Expected offset of file
            data: Expected data in file
            cbfs_offset: Expected CBFS offset for file's data
        """
        self.assertIn('u-boot-dtb', cbfs.files)
        cfile = cbfs.files['u-boot-dtb']
        self.assertEqual('u-boot-dtb', cfile.name)
        self.assertEqual(offset, cfile.offset)
        if cbfs_offset is not None:
            self.assertEqual(cbfs_offset, cfile.cbfs_offset)
        self.assertEqual(U_BOOT_DTB_DATA, cfile.data)
        self.assertEqual(cbfs_util.TYPE_RAW, cfile.ftype)
        self.assertEqual(cbfs_util.COMPRESS_NONE, cfile.compress)
        self.assertEqual(len(U_BOOT_DTB_DATA), cfile.memlen)

    def _check_raw(self, data, size, offset=0, arch=cbfs_util.ARCHITECTURE_X86):
        """Check that two raw files are added as expected

        Args:
            data: Data to check
            size: Expected ROM size
            offset: Expected offset to first CBFS file
            arch: Expected architecture
        """
        cbfs = self._check_hdr(data, size, offset=offset, arch=arch)
        self._check_uboot(cbfs)
        self._check_dtb(cbfs)

    def _get_expected_cbfs(self, size, arch='x86', compress=None, base=None):
        """Get the file created by cbfstool for a particular scenario

        Args:
            size: Size of the CBFS in bytes
            arch: Architecture of the CBFS, as a string
            compress: Compression to use, e.g. cbfs_util.COMPRESS_LZMA
            base: Base address of file, or None to put it anywhere

        Returns:
            Resulting CBFS file, or None if cbfstool is not available
        """
        if not self.have_cbfstool or not self.have_lz4:
            return None
        cbfs_fname = os.path.join(self._indir, 'test.cbfs')
        self.cbfstool.create_new(cbfs_fname, size, arch)
        if base:
            base = [(1 << 32) - size + b for b in base]
        self.cbfstool.add_raw(
            cbfs_fname, 'u-boot',
            tools.get_input_filename(compress and 'compress' or 'u-boot.bin'),
            compress[0] if compress else None,
            base[0] if base else None)
        self.cbfstool.add_raw(
            cbfs_fname, 'u-boot-dtb',
            tools.get_input_filename(compress and 'compress' or 'u-boot.dtb'),
            compress[1] if compress else None,
            base[1] if base else None)
        return cbfs_fname

    def _compare_expected_cbfs(self, data, cbfstool_fname):
        """Compare against what cbfstool creates

        This compares what binman creates with what cbfstool creates for what
        is proportedly the same thing.

        Args:
            data: CBFS created by binman
            cbfstool_fname: CBFS created by cbfstool
        """
        if not self.have_cbfstool or not self.have_lz4:
            return
        expect = tools.read_file(cbfstool_fname)
        if expect != data:
            tools.write_file('/tmp/expect', expect)
            tools.write_file('/tmp/actual', data)
            print('diff -y <(xxd -g1 /tmp/expect) <(xxd -g1 /tmp/actual) | colordiff')
            self.fail('cbfstool produced a different result')

    def test_cbfs_functions(self):
        """Test global functions of cbfs_util"""
        self.assertEqual(cbfs_util.ARCHITECTURE_X86, cbfs_util.find_arch('x86'))
        self.assertIsNone(cbfs_util.find_arch('bad-arch'))

        self.assertEqual(cbfs_util.COMPRESS_LZMA, cbfs_util.find_compress('lzma'))
        self.assertIsNone(cbfs_util.find_compress('bad-comp'))

    def test_cbfstool_failure(self):
        """Test failure to run cbfstool"""
        if not self.have_cbfstool:
            self.skipTest('No cbfstool available')
        with self.assertRaises(ValueError) as exc:
            out = self.cbfstool.fail()
        self.assertIn('cbfstool missing-file bad-command', str(exc.exception))

    def test_cbfs_raw(self):
        """Test base handling of a Coreboot Filesystem (CBFS)"""
        size = 0xb0
        cbw = CbfsWriter(size)
        cbw.add_file_raw('u-boot', U_BOOT_DATA)
        cbw.add_file_raw('u-boot-dtb', U_BOOT_DTB_DATA)
        data = cbw.get_data()
        self._check_raw(data, size)
        cbfs_fname = self._get_expected_cbfs(size=size)
        self._compare_expected_cbfs(data, cbfs_fname)

    def test_cbfs_invalid_file_type(self):
        """Check handling of an invalid file type when outputiing a CBFS"""
        size = 0xb0
        cbw = CbfsWriter(size)
        cfile = cbw.add_file_raw('u-boot', U_BOOT_DATA)

        # Change the type manually before generating the CBFS, and make sure
        # that the generator complains
        cfile.ftype = 0xff
        with self.assertRaises(ValueError) as e:
            cbw.get_data()
        self.assertIn('Unknown type 0xff when writing', str(e.exception))

    def test_cbfs_invalid_file_type_on_read(self):
        """Check handling of an invalid file type when reading the CBFS"""
        size = 0xb0
        cbw = CbfsWriter(size)
        cbw.add_file_raw('u-boot', U_BOOT_DATA)

        data = cbw.get_data()

        # Read in the first file header
        cbr = cbfs_util.CbfsReader(data, read=False)
        with io.BytesIO(data) as fd:
            self.assertTrue(cbr._find_and_read_header(fd, len(data)))
            pos = fd.tell()
            hdr_data = fd.read(cbfs_util.FILE_HEADER_LEN)
            magic, size, ftype, attr, offset = struct.unpack(
                cbfs_util.FILE_HEADER_FORMAT, hdr_data)

        # Create a new CBFS with a change to the file type
        ftype = 0xff
        newdata = data[:pos]
        newdata += struct.pack(cbfs_util.FILE_HEADER_FORMAT, magic, size, ftype,
                               attr, offset)
        newdata += data[pos + cbfs_util.FILE_HEADER_LEN:]

        # Read in this CBFS and make sure that the reader complains
        with self.assertRaises(ValueError) as e:
            cbfs_util.CbfsReader(newdata)
        self.assertIn('Unknown type 0xff when reading', str(e.exception))

    def test_cbfs_no_space(self):
        """Check handling of running out of space in the CBFS"""
        size = 0x60
        cbw = CbfsWriter(size)
        cbw.add_file_raw('u-boot', U_BOOT_DATA)
        with self.assertRaises(ValueError) as e:
            cbw.get_data()
        self.assertIn('No space for header', str(e.exception))

    def test_cbfs_no_space_skip(self):
        """Check handling of running out of space in CBFS with file header"""
        size = 0x5c
        cbw = CbfsWriter(size, arch=cbfs_util.ARCHITECTURE_PPC64)
        cbw._add_fileheader = True
        cbw.add_file_raw('u-boot', U_BOOT_DATA)
        with self.assertRaises(ValueError) as e:
            cbw.get_data()
        self.assertIn('No space for data before offset', str(e.exception))

    def test_cbfs_no_space_pad(self):
        """Check handling of running out of space in CBFS with file header"""
        size = 0x70
        cbw = CbfsWriter(size)
        cbw._add_fileheader = True
        cbw.add_file_raw('u-boot', U_BOOT_DATA)
        with self.assertRaises(ValueError) as e:
            cbw.get_data()
        self.assertIn('No space for data before pad offset', str(e.exception))

    def test_cbfs_bad_header_ptr(self):
        """Check handling of a bad master-header pointer"""
        size = 0x70
        cbw = CbfsWriter(size)
        cbw.add_file_raw('u-boot', U_BOOT_DATA)
        data = cbw.get_data()

        # Add one to the pointer to make it invalid
        newdata = data[:-4] + struct.pack('<I', cbw._header_offset + 1)

        # We should still be able to find the master header by searching
        with test_util.capture_sys_output() as (stdout, _stderr):
            cbfs = cbfs_util.CbfsReader(newdata)
        self.assertIn('Relative offset seems wrong', stdout.getvalue())
        self.assertIn('u-boot', cbfs.files)
        self.assertEqual(size, cbfs.rom_size)

    def test_cbfs_bad_header(self):
        """Check handling of a bad master header"""
        size = 0x70
        cbw = CbfsWriter(size)
        cbw.add_file_raw('u-boot', U_BOOT_DATA)
        data = cbw.get_data()

        # Drop most of the header and try reading the modified CBFS
        newdata = data[:cbw._header_offset + 4]

        with test_util.capture_sys_output() as (stdout, _stderr):
            with self.assertRaises(ValueError) as e:
                cbfs_util.CbfsReader(newdata)
        self.assertIn('Relative offset seems wrong', stdout.getvalue())
        self.assertIn('Cannot find master header', str(e.exception))

    def test_cbfs_bad_file_header(self):
        """Check handling of a bad file header"""
        size = 0x70
        cbw = CbfsWriter(size)
        cbw.add_file_raw('u-boot', U_BOOT_DATA)
        data = cbw.get_data()

        # Read in the CBFS master header (only), then stop
        cbr = cbfs_util.CbfsReader(data, read=False)
        with io.BytesIO(data) as fd:
            self.assertTrue(cbr._find_and_read_header(fd, len(data)))
            pos = fd.tell()

        # Remove all but 4 bytes of the file headerm and try to read the file
        newdata = data[:pos + 4]
        with test_util.capture_sys_output() as (stdout, _stderr):
            with io.BytesIO(newdata) as fd:
                fd.seek(pos)
                self.assertEqual(False, cbr._read_next_file(fd))
        self.assertIn('File header at 0x0 ran out of data', stdout.getvalue())

    def test_cbfs_bad_file_string(self):
        """Check handling of an incomplete filename string"""
        size = 0x70
        cbw = CbfsWriter(size)
        cbw.add_file_raw('16-characters xx', U_BOOT_DATA)
        data = cbw.get_data()

        # Read in the CBFS master header (only), then stop
        cbr = cbfs_util.CbfsReader(data, read=False)
        with io.BytesIO(data) as fd:
            self.assertTrue(cbr._find_and_read_header(fd, len(data)))
            pos = fd.tell()

        # Create a new CBFS with only the first 16 bytes of the file name, then
        # try to read the file
        newdata = data[:pos + cbfs_util.FILE_HEADER_LEN + 16]
        with test_util.capture_sys_output() as (stdout, _stderr):
            with io.BytesIO(newdata) as fd:
                fd.seek(pos)
                self.assertEqual(False, cbr._read_next_file(fd))
        self.assertIn('String at %#x ran out of data' %
                      cbfs_util.FILE_HEADER_LEN, stdout.getvalue())

    def test_cbfs_debug(self):
        """Check debug output"""
        size = 0x70
        cbw = CbfsWriter(size)
        cbw.add_file_raw('u-boot', U_BOOT_DATA)
        data = cbw.get_data()

        try:
            cbfs_util.DEBUG = True
            with test_util.capture_sys_output() as (stdout, _stderr):
                cbfs_util.CbfsReader(data)
            self.assertEqual('name u-boot\nftype 50\ndata %s\n' % U_BOOT_DATA,
                             stdout.getvalue())
        finally:
            cbfs_util.DEBUG = False

    def test_cbfs_bad_attribute(self):
        """Check handling of bad attribute tag"""
        if not self.have_lz4:
            self.skipTest('lz4 --no-frame-crc not available')
        size = 0x140
        cbw = CbfsWriter(size)
        cbw.add_file_raw('u-boot', COMPRESS_DATA, None,
                         compress=cbfs_util.COMPRESS_LZ4)
        data = cbw.get_data()

        # Search the CBFS for the expected compression tag
        with io.BytesIO(data) as fd:
            while True:
                pos = fd.tell()
                tag, = struct.unpack('>I', fd.read(4))
                if tag == cbfs_util.FILE_ATTR_TAG_COMPRESSION:
                    break

        # Create a new CBFS with the tag changed to something invalid
        newdata = data[:pos] + struct.pack('>I', 0x123) + data[pos + 4:]
        with test_util.capture_sys_output() as (stdout, _stderr):
            cbfs_util.CbfsReader(newdata)
        self.assertEqual('Unknown attribute tag 123\n', stdout.getvalue())

    def test_cbfs_missing_attribute(self):
        """Check handling of an incomplete attribute tag"""
        if not self.have_lz4:
            self.skipTest('lz4 --no-frame-crc not available')
        size = 0x140
        cbw = CbfsWriter(size)
        cbw.add_file_raw('u-boot', COMPRESS_DATA, None,
                         compress=cbfs_util.COMPRESS_LZ4)
        data = cbw.get_data()

        # Read in the CBFS master header (only), then stop
        cbr = cbfs_util.CbfsReader(data, read=False)
        with io.BytesIO(data) as fd:
            self.assertTrue(cbr._find_and_read_header(fd, len(data)))
            pos = fd.tell()

        # Create a new CBFS with only the first 4 bytes of the compression tag,
        # then try to read the file. Note that the tag gets pushed out 4 bytes
        tag_pos = (4 + pos + cbfs_util.FILE_HEADER_LEN +
                   cbfs_util.ATTRIBUTE_ALIGN)
        newdata = data[:tag_pos + 4]
        with test_util.capture_sys_output() as (stdout, _stderr):
            with io.BytesIO(newdata) as fd:
                fd.seek(pos)
                self.assertEqual(False, cbr._read_next_file(fd))
        self.assertIn('Attribute tag at %x ran out of data' % tag_pos,
                      stdout.getvalue())

    def test_cbfs_file_master_header(self):
        """Check handling of a file containing a master header"""
        size = 0x100
        cbw = CbfsWriter(size)
        cbw._add_fileheader = True
        cbw.add_file_raw('u-boot', U_BOOT_DATA)
        data = cbw.get_data()

        cbr = cbfs_util.CbfsReader(data)
        self.assertIn('u-boot', cbr.files)
        self.assertEqual(size, cbr.rom_size)

    def test_cbfs_arch(self):
        """Test on non-x86 architecture"""
        size = 0x100
        cbw = CbfsWriter(size, arch=cbfs_util.ARCHITECTURE_PPC64)
        cbw.add_file_raw('u-boot', U_BOOT_DATA)
        cbw.add_file_raw('u-boot-dtb', U_BOOT_DTB_DATA)
        data = cbw.get_data()
        self._check_raw(data, size, offset=0x40,
                        arch=cbfs_util.ARCHITECTURE_PPC64)

        # Compare against what cbfstool creates
        cbfs_fname = self._get_expected_cbfs(size=size, arch='ppc64')
        self._compare_expected_cbfs(data, cbfs_fname)

    def test_cbfs_stage(self):
        """Tests handling of a CBFS stage"""
        if not elf.ELF_TOOLS:
            self.skipTest('Python elftools not available')
        elf_fname = os.path.join(self._indir, 'cbfs-stage.elf')
        elf.MakeElf(elf_fname, U_BOOT_DATA, U_BOOT_DTB_DATA)

        size = 0xb0
        cbw = CbfsWriter(size)
        cbw.add_file_stage('u-boot', tools.read_file(elf_fname))

        data = cbw.get_data()
        cbfs = self._check_hdr(data, size)
        load = 0xfef20000
        entry = load + 2

        cfile = self._check_uboot(cbfs, cbfs_util.TYPE_STAGE, offset=0x38,
                                  data=U_BOOT_DATA + U_BOOT_DTB_DATA)

        self.assertEqual(entry, cfile.entry)
        self.assertEqual(load, cfile.load)
        self.assertEqual(len(U_BOOT_DATA) + len(U_BOOT_DTB_DATA),
                         cfile.data_len)

        # Compare against what cbfstool creates
        if self.have_cbfstool:
            cbfs_fname = os.path.join(self._indir, 'test.cbfs')
            self.cbfstool.create_new(cbfs_fname, size)
            self.cbfstool.add_stage(cbfs_fname, 'u-boot', elf_fname)
            self._compare_expected_cbfs(data, cbfs_fname)

    def test_cbfs_raw_compress(self):
        """Test base handling of compressing raw files"""
        if not self.have_lz4:
            self.skipTest('lz4 --no-frame-crc not available')
        size = 0x140
        cbw = CbfsWriter(size)
        cbw.add_file_raw('u-boot', COMPRESS_DATA, None,
                         compress=cbfs_util.COMPRESS_LZ4)
        cbw.add_file_raw('u-boot-dtb', COMPRESS_DATA, None,
                         compress=cbfs_util.COMPRESS_LZMA)
        data = cbw.get_data()

        cbfs = self._check_hdr(data, size)
        self.assertIn('u-boot', cbfs.files)
        cfile = cbfs.files['u-boot']
        self.assertEqual(cfile.name, 'u-boot')
        self.assertEqual(cfile.offset, 0x30)
        self.assertEqual(cfile.data, COMPRESS_DATA)
        self.assertEqual(cfile.ftype, cbfs_util.TYPE_RAW)
        self.assertEqual(cfile.compress, cbfs_util.COMPRESS_LZ4)
        self.assertEqual(cfile.memlen, len(COMPRESS_DATA))

        self.assertIn('u-boot-dtb', cbfs.files)
        cfile = cbfs.files['u-boot-dtb']
        self.assertEqual(cfile.name, 'u-boot-dtb')
        self.assertEqual(cfile.offset, 0x34)
        self.assertEqual(cfile.data, COMPRESS_DATA)
        self.assertEqual(cfile.ftype, cbfs_util.TYPE_RAW)
        self.assertEqual(cfile.compress, cbfs_util.COMPRESS_LZMA)
        self.assertEqual(cfile.memlen, len(COMPRESS_DATA))

        cbfs_fname = self._get_expected_cbfs(size=size, compress=['lz4', 'lzma'])
        self._compare_expected_cbfs(data, cbfs_fname)

    def test_cbfs_raw_space(self):
        """Test files with unused space in the CBFS"""
        size = 0xf0
        cbw = CbfsWriter(size)
        cbw.add_file_raw('u-boot', U_BOOT_DATA)
        cbw.add_file_raw('u-boot-dtb', U_BOOT_DTB_DATA)
        data = cbw.get_data()
        self._check_raw(data, size)
        cbfs_fname = self._get_expected_cbfs(size=size)
        self._compare_expected_cbfs(data, cbfs_fname)

    def test_cbfs_offset(self):
        """Test a CBFS with files at particular offsets"""
        size = 0x200
        cbw = CbfsWriter(size)
        cbw.add_file_raw('u-boot', U_BOOT_DATA, 0x40)
        cbw.add_file_raw('u-boot-dtb', U_BOOT_DTB_DATA, 0x140)

        data = cbw.get_data()
        cbfs = self._check_hdr(data, size)
        self._check_uboot(cbfs, ftype=cbfs_util.TYPE_RAW, offset=0x40,
                          cbfs_offset=0x40)
        self._check_dtb(cbfs, offset=0x40, cbfs_offset=0x140)

        cbfs_fname = self._get_expected_cbfs(size=size, base=(0x40, 0x140))
        self._compare_expected_cbfs(data, cbfs_fname)

    def test_cbfs_invalid_file_type_header(self):
        """Check handling of an invalid file type when outputting a header"""
        size = 0xb0
        cbw = CbfsWriter(size)
        cfile = cbw.add_file_raw('u-boot', U_BOOT_DATA, 0)

        # Change the type manually before generating the CBFS, and make sure
        # that the generator complains
        cfile.ftype = 0xff
        with self.assertRaises(ValueError) as e:
            cbw.get_data()
        self.assertIn('Unknown file type 0xff', str(e.exception))

    def test_cbfs_offset_conflict(self):
        """Test a CBFS with files that want to overlap"""
        size = 0x200
        cbw = CbfsWriter(size)
        cbw.add_file_raw('u-boot', U_BOOT_DATA, 0x40)
        cbw.add_file_raw('u-boot-dtb', U_BOOT_DTB_DATA, 0x80)

        with self.assertRaises(ValueError) as e:
            cbw.get_data()
        self.assertIn('No space for data before pad offset', str(e.exception))

    def test_cbfs_check_offset(self):
        """Test that we can discover the offset of a file after writing it"""
        size = 0xb0
        cbw = CbfsWriter(size)
        cbw.add_file_raw('u-boot', U_BOOT_DATA)
        cbw.add_file_raw('u-boot-dtb', U_BOOT_DTB_DATA)
        data = cbw.get_data()

        cbfs = cbfs_util.CbfsReader(data)
        self.assertEqual(0x20, cbfs.files['u-boot'].cbfs_offset)
        self.assertEqual(0x64, cbfs.files['u-boot-dtb'].cbfs_offset)


if __name__ == '__main__':
    unittest.main()
