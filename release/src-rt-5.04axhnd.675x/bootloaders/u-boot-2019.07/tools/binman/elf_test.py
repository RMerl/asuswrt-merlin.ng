# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2017 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# Test for the elf module

import os
import sys
import unittest

import elf
import test_util
import tools

binman_dir = os.path.dirname(os.path.realpath(sys.argv[0]))


class FakeEntry:
    """A fake Entry object, usedfor testing

    This supports an entry with a given size.
    """
    def __init__(self, contents_size):
        self.contents_size = contents_size
        self.data = 'a' * contents_size

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

    def LookupSymbol(self, name, weak, msg):
        """Fake implementation which returns the same value for all symbols"""
        return self.sym_value


class TestElf(unittest.TestCase):
    @classmethod
    def setUpClass(self):
        tools.SetInputDirs(['.'])

    def testAllSymbols(self):
        """Test that we can obtain a symbol from the ELF file"""
        fname = os.path.join(binman_dir, 'test', 'u_boot_ucode_ptr')
        syms = elf.GetSymbols(fname, [])
        self.assertIn('.ucode', syms)

    def testRegexSymbols(self):
        """Test that we can obtain from the ELF file by regular expression"""
        fname = os.path.join(binman_dir, 'test', 'u_boot_ucode_ptr')
        syms = elf.GetSymbols(fname, ['ucode'])
        self.assertIn('.ucode', syms)
        syms = elf.GetSymbols(fname, ['missing'])
        self.assertNotIn('.ucode', syms)
        syms = elf.GetSymbols(fname, ['missing', 'ucode'])
        self.assertIn('.ucode', syms)

    def testMissingFile(self):
        """Test that a missing file is detected"""
        entry = FakeEntry(10)
        section = FakeSection()
        with self.assertRaises(ValueError) as e:
            syms = elf.LookupAndWriteSymbols('missing-file', entry, section)
        self.assertIn("Filename 'missing-file' not found in input path",
                      str(e.exception))

    def testOutsideFile(self):
        """Test a symbol which extends outside the entry area is detected"""
        entry = FakeEntry(10)
        section = FakeSection()
        elf_fname = os.path.join(binman_dir, 'test', 'u_boot_binman_syms')
        with self.assertRaises(ValueError) as e:
            syms = elf.LookupAndWriteSymbols(elf_fname, entry, section)
        self.assertIn('entry_path has offset 4 (size 8) but the contents size '
                      'is a', str(e.exception))

    def testMissingImageStart(self):
        """Test that we detect a missing __image_copy_start symbol

        This is needed to mark the start of the image. Without it we cannot
        locate the offset of a binman symbol within the image.
        """
        entry = FakeEntry(10)
        section = FakeSection()
        elf_fname = os.path.join(binman_dir, 'test', 'u_boot_binman_syms_bad')
        self.assertEqual(elf.LookupAndWriteSymbols(elf_fname, entry, section),
                         None)

    def testBadSymbolSize(self):
        """Test that an attempt to use an 8-bit symbol are detected

        Only 32 and 64 bits are supported, since we need to store an offset
        into the image.
        """
        entry = FakeEntry(10)
        section = FakeSection()
        elf_fname = os.path.join(binman_dir, 'test', 'u_boot_binman_syms_size')
        with self.assertRaises(ValueError) as e:
            syms = elf.LookupAndWriteSymbols(elf_fname, entry, section)
        self.assertIn('has size 1: only 4 and 8 are supported',
                      str(e.exception))

    def testNoValue(self):
        """Test the case where we have no value for the symbol

        This should produce -1 values for all thress symbols, taking up the
        first 16 bytes of the image.
        """
        entry = FakeEntry(20)
        section = FakeSection(sym_value=None)
        elf_fname = os.path.join(binman_dir, 'test', 'u_boot_binman_syms')
        syms = elf.LookupAndWriteSymbols(elf_fname, entry, section)
        self.assertEqual(chr(255) * 16 + 'a' * 4, entry.data)

    def testDebug(self):
        """Check that enabling debug in the elf module produced debug output"""
        elf.debug = True
        entry = FakeEntry(20)
        section = FakeSection()
        elf_fname = os.path.join(binman_dir, 'test', 'u_boot_binman_syms')
        with test_util.capture_sys_output() as (stdout, stderr):
            syms = elf.LookupAndWriteSymbols(elf_fname, entry, section)
        elf.debug = False
        self.assertTrue(len(stdout.getvalue()) > 0)


if __name__ == '__main__':
    unittest.main()
