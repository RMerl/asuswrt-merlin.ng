# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# Test for the Entry class

import collections
import os
import sys
import unittest

import fdt
import fdt_util
import tools

entry = None

class TestEntry(unittest.TestCase):
    def setUp(self):
        tools.PrepareOutputDir(None)

    def tearDown(self):
        tools.FinaliseOutputDir()

    def GetNode(self):
        binman_dir = os.path.dirname(os.path.realpath(sys.argv[0]))
        fname = fdt_util.EnsureCompiled(
            os.path.join(binman_dir,('test/005_simple.dts')))
        dtb = fdt.FdtScan(fname)
        return dtb.GetNode('/binman/u-boot')

    def test1EntryNoImportLib(self):
        """Test that we can import Entry subclassess successfully"""

        sys.modules['importlib'] = None
        global entry
        import entry
        entry.Entry.Create(None, self.GetNode(), 'u-boot')

    def test2EntryImportLib(self):
        del sys.modules['importlib']
        global entry
        if entry:
            reload(entry)
        else:
            import entry
        entry.Entry.Create(None, self.GetNode(), 'u-boot-spl')
        del entry

    def testEntryContents(self):
        """Test the Entry bass class"""
        import entry
        base_entry = entry.Entry(None, None, None, read_node=False)
        self.assertEqual(True, base_entry.ObtainContents())

    def testUnknownEntry(self):
        """Test that unknown entry types are detected"""
        import entry
        Node = collections.namedtuple('Node', ['name', 'path'])
        node = Node('invalid-name', 'invalid-path')
        with self.assertRaises(ValueError) as e:
            entry.Entry.Create(None, node, node.name)
        self.assertIn("Unknown entry type 'invalid-name' in node "
                      "'invalid-path'", str(e.exception))

    def testUniqueName(self):
        """Test Entry.GetUniqueName"""
        import entry
        Node = collections.namedtuple('Node', ['name', 'parent'])
        base_node = Node('root', None)
        base_entry = entry.Entry(None, None, base_node, read_node=False)
        self.assertEqual('root', base_entry.GetUniqueName())
        sub_node = Node('subnode', base_node)
        sub_entry = entry.Entry(None, None, sub_node, read_node=False)
        self.assertEqual('root.subnode', sub_entry.GetUniqueName())

    def testGetDefaultFilename(self):
        """Trivial test for this base class function"""
        import entry
        base_entry = entry.Entry(None, None, None, read_node=False)
        self.assertIsNone(base_entry.GetDefaultFilename())

if __name__ == "__main__":
    unittest.main()
