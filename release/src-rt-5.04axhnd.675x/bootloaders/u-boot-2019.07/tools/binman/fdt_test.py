# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# Test for the fdt modules

import os
import sys
import tempfile
import unittest

import fdt
from fdt import FdtScan
import fdt_util
import tools

class TestFdt(unittest.TestCase):
    @classmethod
    def setUpClass(self):
        self._binman_dir = os.path.dirname(os.path.realpath(sys.argv[0]))
        self._indir = tempfile.mkdtemp(prefix='binmant.')
        tools.PrepareOutputDir(self._indir, True)

    @classmethod
    def tearDownClass(self):
        tools._FinaliseForTest()

    def TestFile(self, fname):
        return os.path.join(self._binman_dir, 'test', fname)

    def GetCompiled(self, fname):
        return fdt_util.EnsureCompiled(self.TestFile(fname))

    def _DeleteProp(self, dt):
        node = dt.GetNode('/microcode/update@0')
        node.DeleteProp('data')

    def testFdtNormal(self):
        fname = self.GetCompiled('034_x86_ucode.dts')
        dt = FdtScan(fname)
        self._DeleteProp(dt)

    def testFdtNormalProp(self):
        fname = self.GetCompiled('045_prop_test.dts')
        dt = FdtScan(fname)
        node = dt.GetNode('/binman/intel-me')
        self.assertEquals('intel-me', node.name)
        val = fdt_util.GetString(node, 'filename')
        self.assertEquals(str, type(val))
        self.assertEquals('me.bin', val)

        prop = node.props['intval']
        self.assertEquals(fdt.TYPE_INT, prop.type)
        self.assertEquals(3, fdt_util.GetInt(node, 'intval'))

        prop = node.props['intarray']
        self.assertEquals(fdt.TYPE_INT, prop.type)
        self.assertEquals(list, type(prop.value))
        self.assertEquals(2, len(prop.value))
        self.assertEquals([5, 6],
                          [fdt_util.fdt32_to_cpu(val) for val in prop.value])

        prop = node.props['byteval']
        self.assertEquals(fdt.TYPE_BYTE, prop.type)
        self.assertEquals(chr(8), prop.value)

        prop = node.props['bytearray']
        self.assertEquals(fdt.TYPE_BYTE, prop.type)
        self.assertEquals(list, type(prop.value))
        self.assertEquals(str, type(prop.value[0]))
        self.assertEquals(3, len(prop.value))
        self.assertEquals([chr(1), '#', '4'], prop.value)

        prop = node.props['longbytearray']
        self.assertEquals(fdt.TYPE_INT, prop.type)
        self.assertEquals(0x090a0b0c, fdt_util.GetInt(node, 'longbytearray'))

        prop = node.props['stringval']
        self.assertEquals(fdt.TYPE_STRING, prop.type)
        self.assertEquals('message2', fdt_util.GetString(node, 'stringval'))

        prop = node.props['stringarray']
        self.assertEquals(fdt.TYPE_STRING, prop.type)
        self.assertEquals(list, type(prop.value))
        self.assertEquals(3, len(prop.value))
        self.assertEquals(['another', 'multi-word', 'message'], prop.value)
