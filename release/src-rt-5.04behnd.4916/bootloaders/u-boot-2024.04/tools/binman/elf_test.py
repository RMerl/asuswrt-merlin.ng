# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2017 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# Test for the elf module

import os
import shutil
import struct
import sys
import tempfile
import unittest

from binman import elf
from u_boot_pylib import command
from u_boot_pylib import test_util
from u_boot_pylib import tools
from u_boot_pylib import tout

binman_dir = os.path.dirname(os.path.realpath(sys.argv[0]))


class FakeEntry:
    """A fake Entry object, usedfor testing

    This supports an entry with a given size.
    """
    def __init__(self, contents_size):
        self.contents_size = contents_size
        self.data = tools.get_bytes(ord('a'), contents_size)

    def GetPath(self):
        return 'entry_path'


class FakeSection:
    """A fake Section object, used for testing

    This has the minimum feature set needed to support testing elf functions.
    A LookupSymbol() function is provided which returns a fake value for amu
    symbol requested.
    """
    def __init__(self, sym_value=1):
        self.sym_value = sym_value

    def GetPath(self):
        return 'section_path'

    def LookupImageSymbol(self, name, weak, msg, base_addr):
        """Fake implementation which returns the same value for all symbols"""
        return self.sym_value

    def GetImage(self):
        return self

def BuildElfTestFiles(target_dir):
    """Build ELF files used for testing in binman

    This compiles and links the test files into the specified directory. It uses
    the Makefile and source files in the binman test/ directory.

    Args:
        target_dir: Directory to put the files into
    """
    if not os.path.exists(target_dir):
        os.mkdir(target_dir)
    testdir = os.path.join(binman_dir, 'test')

    # If binman is involved from the main U-Boot Makefile the -r and -R
    # flags are set in MAKEFLAGS. This prevents this Makefile from working
    # correctly. So drop any make flags here.
    if 'MAKEFLAGS' in os.environ:
        del os.environ['MAKEFLAGS']
    try:
        tools.run('make', '-C', target_dir, '-f',
                  os.path.join(testdir, 'Makefile'), 'SRC=%s/' % testdir)
    except ValueError as e:
        # The test system seems to suppress this in a strange way
        print(e)


class TestElf(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls._indir = tempfile.mkdtemp(prefix='elf.')
        tools.set_input_dirs(['.'])
        BuildElfTestFiles(cls._indir)

    @classmethod
    def tearDownClass(cls):
        if cls._indir:
            shutil.rmtree(cls._indir)

    @classmethod
    def ElfTestFile(cls, fname):
        return os.path.join(cls._indir, fname)

    def testAllSymbols(self):
        """Test that we can obtain a symbol from the ELF file"""
        fname = self.ElfTestFile('u_boot_ucode_ptr')
        syms = elf.GetSymbols(fname, [])
        self.assertIn('_dt_ucode_base_size', syms)

    def testRegexSymbols(self):
        """Test that we can obtain from the ELF file by regular expression"""
        fname = self.ElfTestFile('u_boot_ucode_ptr')
        syms = elf.GetSymbols(fname, ['ucode'])
        self.assertIn('_dt_ucode_base_size', syms)
        syms = elf.GetSymbols(fname, ['missing'])
        self.assertNotIn('_dt_ucode_base_size', syms)
        syms = elf.GetSymbols(fname, ['missing', 'ucode'])
        self.assertIn('_dt_ucode_base_size', syms)

    def testMissingFile(self):
        """Test that a missing file is detected"""
        entry = FakeEntry(10)
        section = FakeSection()
        with self.assertRaises(ValueError) as e:
            elf.LookupAndWriteSymbols('missing-file', entry, section)
        self.assertIn("Filename 'missing-file' not found in input path",
                      str(e.exception))

    def testOutsideFile(self):
        """Test a symbol which extends outside the entry area is detected"""
        if not elf.ELF_TOOLS:
            self.skipTest('Python elftools not available')
        entry = FakeEntry(10)
        section = FakeSection()
        elf_fname = self.ElfTestFile('u_boot_binman_syms')
        with self.assertRaises(ValueError) as e:
            elf.LookupAndWriteSymbols(elf_fname, entry, section)
        self.assertIn('entry_path has offset 8 (size 8) but the contents size '
                      'is a', str(e.exception))

    def testMissingImageStart(self):
        """Test that we detect a missing __image_copy_start symbol

        This is needed to mark the start of the image. Without it we cannot
        locate the offset of a binman symbol within the image.
        """
        entry = FakeEntry(10)
        section = FakeSection()
        elf_fname = self.ElfTestFile('u_boot_binman_syms_bad')
        count = elf.LookupAndWriteSymbols(elf_fname, entry, section)
        self.assertEqual(0, count)

    def testBadSymbolSize(self):
        """Test that an attempt to use an 8-bit symbol are detected

        Only 32 and 64 bits are supported, since we need to store an offset
        into the image.
        """
        if not elf.ELF_TOOLS:
            self.skipTest('Python elftools not available')
        entry = FakeEntry(10)
        section = FakeSection()
        elf_fname =self.ElfTestFile('u_boot_binman_syms_size')
        with self.assertRaises(ValueError) as e:
            elf.LookupAndWriteSymbols(elf_fname, entry, section)
        self.assertIn('has size 1: only 4 and 8 are supported',
                      str(e.exception))

    def testNoValue(self):
        """Test the case where we have no value for the symbol

        This should produce -1 values for all three symbols, taking up the
        first 16 bytes of the image.
        """
        if not elf.ELF_TOOLS:
            self.skipTest('Python elftools not available')
        entry = FakeEntry(28)
        section = FakeSection(sym_value=None)
        elf_fname = self.ElfTestFile('u_boot_binman_syms')
        count = elf.LookupAndWriteSymbols(elf_fname, entry, section)
        self.assertEqual(5, count)
        expected = (struct.pack('<L', elf.BINMAN_SYM_MAGIC_VALUE) +
                    tools.get_bytes(255, 20) +
                    tools.get_bytes(ord('a'), 4))
        self.assertEqual(expected, entry.data)

    def testDebug(self):
        """Check that enabling debug in the elf module produced debug output"""
        if not elf.ELF_TOOLS:
            self.skipTest('Python elftools not available')
        try:
            tout.init(tout.DEBUG)
            entry = FakeEntry(24)
            section = FakeSection()
            elf_fname = self.ElfTestFile('u_boot_binman_syms')
            with test_util.capture_sys_output() as (stdout, stderr):
                elf.LookupAndWriteSymbols(elf_fname, entry, section)
            self.assertTrue(len(stdout.getvalue()) > 0)
        finally:
            tout.init(tout.WARNING)

    def testMakeElf(self):
        """Test for the MakeElf function"""
        outdir = tempfile.mkdtemp(prefix='elf.')
        expected_text = b'1234'
        expected_data = b'wxyz'
        elf_fname = os.path.join(outdir, 'elf')
        bin_fname = os.path.join(outdir, 'bin')

        # Make an Elf file and then convert it to a fkat binary file. This
        # should produce the original data.
        elf.MakeElf(elf_fname, expected_text, expected_data)
        objcopy, args = tools.get_target_compile_tool('objcopy')
        args += ['-O', 'binary', elf_fname, bin_fname]
        stdout = command.output(objcopy, *args)
        with open(bin_fname, 'rb') as fd:
            data = fd.read()
        self.assertEqual(expected_text + expected_data, data)
        shutil.rmtree(outdir)

    def testDecodeElf(self):
        """Test for the MakeElf function"""
        if not elf.ELF_TOOLS:
            self.skipTest('Python elftools not available')
        outdir = tempfile.mkdtemp(prefix='elf.')
        expected_text = b'1234'
        expected_data = b'wxyz'
        elf_fname = os.path.join(outdir, 'elf')
        elf.MakeElf(elf_fname, expected_text, expected_data)
        data = tools.read_file(elf_fname)

        load = 0xfef20000
        entry = load + 2
        expected = expected_text + expected_data
        self.assertEqual(elf.ElfInfo(expected, load, entry, len(expected)),
                         elf.DecodeElf(data, 0))
        self.assertEqual(elf.ElfInfo(b'\0\0' + expected[2:],
                                     load, entry, len(expected)),
                         elf.DecodeElf(data, load + 2))
        shutil.rmtree(outdir)

    def testEmbedData(self):
        """Test for the GetSymbolFileOffset() function"""
        if not elf.ELF_TOOLS:
            self.skipTest('Python elftools not available')

        fname = self.ElfTestFile('embed_data')
        offset = elf.GetSymbolFileOffset(fname, ['embed_start', 'embed_end'])
        start = offset['embed_start'].offset
        end = offset['embed_end'].offset
        data = tools.read_file(fname)
        embed_data = data[start:end]
        expect = struct.pack('<IIIII', 2, 3, 0x1234, 0x5678, 0)
        self.assertEqual(expect, embed_data)

    def testEmbedFail(self):
        """Test calling GetSymbolFileOffset() without elftools"""
        old_val = elf.ELF_TOOLS
        try:
            elf.ELF_TOOLS = False
            fname = self.ElfTestFile('embed_data')
            with self.assertRaises(ValueError) as e:
                elf.GetSymbolFileOffset(fname, ['embed_start', 'embed_end'])
            with self.assertRaises(ValueError) as e:
                elf.DecodeElf(tools.read_file(fname), 0xdeadbeef)
            with self.assertRaises(ValueError) as e:
                elf.GetFileOffset(fname, 0xdeadbeef)
            with self.assertRaises(ValueError) as e:
                elf.GetSymbolFromAddress(fname, 0xdeadbeef)
            with self.assertRaises(ValueError) as e:
                entry = FakeEntry(10)
                section = FakeSection()
                elf.LookupAndWriteSymbols(fname, entry, section, True)

            self.assertIn(
                "Section 'section_path': entry 'entry_path': Cannot write symbols to an ELF file without Python elftools",
                str(e.exception))
        finally:
            elf.ELF_TOOLS = old_val

    def testEmbedDataNoSym(self):
        """Test for GetSymbolFileOffset() getting no symbols"""
        if not elf.ELF_TOOLS:
            self.skipTest('Python elftools not available')

        fname = self.ElfTestFile('embed_data')
        offset = elf.GetSymbolFileOffset(fname, ['missing_sym'])
        self.assertEqual({}, offset)

    def test_read_loadable_segments(self):
        """Test for read_loadable_segments()"""
        if not elf.ELF_TOOLS:
            self.skipTest('Python elftools not available')
        fname = self.ElfTestFile('embed_data')
        segments, entry = elf.read_loadable_segments(tools.read_file(fname))

    def test_read_segments_fail(self):
        """Test for read_loadable_segments() without elftools"""
        old_val = elf.ELF_TOOLS
        try:
            elf.ELF_TOOLS = False
            fname = self.ElfTestFile('embed_data')
            with self.assertRaises(ValueError) as e:
                elf.read_loadable_segments(tools.read_file(fname))
            self.assertIn("Python: No module named 'elftools'",
                          str(e.exception))
        finally:
            elf.ELF_TOOLS = old_val

    def test_read_segments_bad_data(self):
        """Test for read_loadable_segments() with an invalid ELF file"""
        if not elf.ELF_TOOLS:
            self.skipTest('Python elftools not available')
        fname = self.ElfTestFile('embed_data')
        with self.assertRaises(ValueError) as e:
            elf.read_loadable_segments(tools.get_bytes(100, 100))
        self.assertIn('Magic number does not match', str(e.exception))

    def test_get_file_offset(self):
        """Test GetFileOffset() gives the correct file offset for a symbol"""
        if not elf.ELF_TOOLS:
            self.skipTest('Python elftools not available')
        fname = self.ElfTestFile('embed_data')
        syms = elf.GetSymbols(fname, ['embed'])
        addr = syms['embed'].address
        offset = elf.GetFileOffset(fname, addr)
        data = tools.read_file(fname)

        # Just use the first 4 bytes and assume it is little endian
        embed_data = data[offset:offset + 4]
        embed_value = struct.unpack('<I', embed_data)[0]
        self.assertEqual(0x1234, embed_value)

    def test_get_file_offset_fail(self):
        """Test calling GetFileOffset() without elftools"""
        old_val = elf.ELF_TOOLS
        try:
            elf.ELF_TOOLS = False
            fname = self.ElfTestFile('embed_data')
            with self.assertRaises(ValueError) as e:
                elf.GetFileOffset(fname, 0)
            self.assertIn("Python: No module named 'elftools'",
                      str(e.exception))
        finally:
            elf.ELF_TOOLS = old_val

    def test_get_symbol_from_address(self):
        """Test GetSymbolFromAddress()"""
        if not elf.ELF_TOOLS:
            self.skipTest('Python elftools not available')
        fname = self.ElfTestFile('elf_sections')
        sym_name = 'calculate'
        syms = elf.GetSymbols(fname, [sym_name])
        addr = syms[sym_name].address
        sym = elf.GetSymbolFromAddress(fname, addr)
        self.assertEqual(sym_name, sym)

    def test_get_symbol_from_address_fail(self):
        """Test calling GetSymbolFromAddress() without elftools"""
        old_val = elf.ELF_TOOLS
        try:
            elf.ELF_TOOLS = False
            fname = self.ElfTestFile('embed_data')
            with self.assertRaises(ValueError) as e:
                elf.GetSymbolFromAddress(fname, 0x1000)
            self.assertIn("Python: No module named 'elftools'",
                          str(e.exception))
        finally:
            elf.ELF_TOOLS = old_val

    def test_is_valid(self):
        """Test is_valid()"""
        self.assertEqual(False, elf.is_valid(b''))
        self.assertEqual(False, elf.is_valid(b'1234'))

        fname = self.ElfTestFile('elf_sections')
        data = tools.read_file(fname)
        self.assertEqual(True, elf.is_valid(data))
        self.assertEqual(False, elf.is_valid(data[4:]))

    def test_get_symbol_offset(self):
        fname = self.ElfTestFile('embed_data')
        syms = elf.GetSymbols(fname, ['embed_start', 'embed'])
        expected = syms['embed'].address - syms['embed_start'].address
        val = elf.GetSymbolOffset(fname, 'embed', 'embed_start')
        self.assertEqual(expected, val)

        with self.assertRaises(KeyError) as e:
            elf.GetSymbolOffset(fname, 'embed')
        self.assertIn('__image_copy_start', str(e.exception))

    def test_get_symbol_address(self):
        fname = self.ElfTestFile('embed_data')
        addr = elf.GetSymbolAddress(fname, 'region_size')
        self.assertEqual(0, addr)


if __name__ == '__main__':
    unittest.main()
