#!/usr/bin/python
# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2018 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#

from optparse import OptionParser
import glob
import os
import sys
import unittest

# Bring in the patman libraries
our_path = os.path.dirname(os.path.realpath(__file__))
for dirname in ['../patman', '..']:
    sys.path.insert(0, os.path.join(our_path, dirname))

import command
import fdt
from fdt import TYPE_BYTE, TYPE_INT, TYPE_STRING, TYPE_BOOL
import fdt_util
from fdt_util import fdt32_to_cpu
import libfdt
import test_util
import tools

def _GetPropertyValue(dtb, node, prop_name):
    """Low-level function to get the property value based on its offset

    This looks directly in the device tree at the property's offset to find
    its value. It is useful as a check that the property is in the correct
    place.

    Args:
        node: Node to look in
        prop_name: Property name to find

    Returns:
        Tuple:
            Prop object found
            Value of property as a string (found using property offset)
    """
    prop = node.props[prop_name]

    # Add 12, which is sizeof(struct fdt_property), to get to start of data
    offset = prop.GetOffset() + 12
    data = dtb.GetContents()[offset:offset + len(prop.value)]
    return prop, [chr(x) for x in data]


class TestFdt(unittest.TestCase):
    """Tests for the Fdt module

    This includes unit tests for some functions and functional tests for the fdt
    module.
    """
    @classmethod
    def setUpClass(cls):
        tools.PrepareOutputDir(None)

    @classmethod
    def tearDownClass(cls):
        tools.FinaliseOutputDir()

    def setUp(self):
        self.dtb = fdt.FdtScan('tools/dtoc/dtoc_test_simple.dts')

    def testFdt(self):
        """Test that we can open an Fdt"""
        self.dtb.Scan()
        root = self.dtb.GetRoot()
        self.assertTrue(isinstance(root, fdt.Node))

    def testGetNode(self):
        """Test the GetNode() method"""
        node = self.dtb.GetNode('/spl-test')
        self.assertTrue(isinstance(node, fdt.Node))
        node = self.dtb.GetNode('/i2c@0/pmic@9')
        self.assertTrue(isinstance(node, fdt.Node))
        self.assertEqual('pmic@9', node.name)
        self.assertIsNone(self.dtb.GetNode('/i2c@0/pmic@9/missing'))

    def testFlush(self):
        """Check that we can flush the device tree out to its file"""
        fname = self.dtb._fname
        with open(fname) as fd:
            data = fd.read()
        os.remove(fname)
        with self.assertRaises(IOError):
            open(fname)
        self.dtb.Flush()
        with open(fname) as fd:
            data = fd.read()

    def testPack(self):
        """Test that packing a device tree works"""
        self.dtb.Pack()

    def testGetFdt(self):
        """Tetst that we can access the raw device-tree data"""
        self.assertTrue(isinstance(self.dtb.GetContents(), bytearray))

    def testGetProps(self):
        """Tests obtaining a list of properties"""
        node = self.dtb.GetNode('/spl-test')
        props = self.dtb.GetProps(node)
        self.assertEqual(['boolval', 'bytearray', 'byteval', 'compatible',
                          'intarray', 'intval', 'longbytearray', 'notstring',
                          'stringarray', 'stringval', 'u-boot,dm-pre-reloc'],
                         sorted(props.keys()))

    def testCheckError(self):
        """Tests the ChecKError() function"""
        with self.assertRaises(ValueError) as e:
            fdt.CheckErr(-libfdt.NOTFOUND, 'hello')
        self.assertIn('FDT_ERR_NOTFOUND: hello', str(e.exception))

    def testGetFdt(self):
        node = self.dtb.GetNode('/spl-test')
        self.assertEqual(self.dtb, node.GetFdt())

class TestNode(unittest.TestCase):
    """Test operation of the Node class"""

    @classmethod
    def setUpClass(cls):
        tools.PrepareOutputDir(None)

    @classmethod
    def tearDownClass(cls):
        tools.FinaliseOutputDir()

    def setUp(self):
        self.dtb = fdt.FdtScan('tools/dtoc/dtoc_test_simple.dts')
        self.node = self.dtb.GetNode('/spl-test')

    def testOffset(self):
        """Tests that we can obtain the offset of a node"""
        self.assertTrue(self.node.Offset() > 0)

    def testDelete(self):
        """Tests that we can delete a property"""
        node2 = self.dtb.GetNode('/spl-test2')
        offset1 = node2.Offset()
        self.node.DeleteProp('intval')
        offset2 = node2.Offset()
        self.assertTrue(offset2 < offset1)
        self.node.DeleteProp('intarray')
        offset3 = node2.Offset()
        self.assertTrue(offset3 < offset2)
        with self.assertRaises(libfdt.FdtException):
            self.node.DeleteProp('missing')

    def testDeleteGetOffset(self):
        """Test that property offset update when properties are deleted"""
        self.node.DeleteProp('intval')
        prop, value = _GetPropertyValue(self.dtb, self.node, 'longbytearray')
        self.assertEqual(prop.value, value)

    def testFindNode(self):
        """Tests that we can find a node using the FindNode() functoin"""
        node = self.dtb.GetRoot().FindNode('i2c@0')
        self.assertEqual('i2c@0', node.name)
        subnode = node.FindNode('pmic@9')
        self.assertEqual('pmic@9', subnode.name)
        self.assertEqual(None, node.FindNode('missing'))

    def testRefreshMissingNode(self):
        """Test refreshing offsets when an extra node is present in dtb"""
        # Delete it from our tables, not the device tree
        del self.dtb._root.subnodes[-1]
        with self.assertRaises(ValueError) as e:
            self.dtb.Refresh()
        self.assertIn('Internal error, offset', str(e.exception))

    def testRefreshExtraNode(self):
        """Test refreshing offsets when an expected node is missing"""
        # Delete it from the device tre, not our tables
        self.dtb.GetFdtObj().del_node(self.node.Offset())
        with self.assertRaises(ValueError) as e:
            self.dtb.Refresh()
        self.assertIn('Internal error, node name mismatch '
                      'spl-test != spl-test2', str(e.exception))

    def testRefreshMissingProp(self):
        """Test refreshing offsets when an extra property is present in dtb"""
        # Delete it from our tables, not the device tree
        del self.node.props['notstring']
        with self.assertRaises(ValueError) as e:
            self.dtb.Refresh()
        self.assertIn("Internal error, property 'notstring' missing, offset ",
                      str(e.exception))

    def testLookupPhandle(self):
        """Test looking up a single phandle"""
        dtb = fdt.FdtScan('tools/dtoc/dtoc_test_phandle.dts')
        node = dtb.GetNode('/phandle-source2')
        prop = node.props['clocks']
        target = dtb.GetNode('/phandle-target')
        self.assertEqual(target, dtb.LookupPhandle(fdt32_to_cpu(prop.value)))


class TestProp(unittest.TestCase):
    """Test operation of the Prop class"""

    @classmethod
    def setUpClass(cls):
        tools.PrepareOutputDir(None)

    @classmethod
    def tearDownClass(cls):
        tools.FinaliseOutputDir()

    def setUp(self):
        self.dtb = fdt.FdtScan('tools/dtoc/dtoc_test_simple.dts')
        self.node = self.dtb.GetNode('/spl-test')
        self.fdt = self.dtb.GetFdtObj()

    def testMissingNode(self):
        self.assertEqual(None, self.dtb.GetNode('missing'))

    def testPhandle(self):
        dtb = fdt.FdtScan('tools/dtoc/dtoc_test_phandle.dts')
        node = dtb.GetNode('/phandle-source2')
        prop = node.props['clocks']
        self.assertTrue(fdt32_to_cpu(prop.value) > 0)

    def _ConvertProp(self, prop_name):
        """Helper function to look up a property in self.node and return it

        Args:
            Property name to find

        Return fdt.Prop object for this property
        """
        p = self.fdt.getprop(self.node.Offset(), prop_name)
        return fdt.Prop(self.node, -1, prop_name, p)

    def testMakeProp(self):
        """Test we can convert all the the types that are supported"""
        prop = self._ConvertProp('boolval')
        self.assertEqual(fdt.TYPE_BOOL, prop.type)
        self.assertEqual(True, prop.value)

        prop = self._ConvertProp('intval')
        self.assertEqual(fdt.TYPE_INT, prop.type)
        self.assertEqual(1, fdt32_to_cpu(prop.value))

        prop = self._ConvertProp('intarray')
        self.assertEqual(fdt.TYPE_INT, prop.type)
        val = [fdt32_to_cpu(val) for val in prop.value]
        self.assertEqual([2, 3, 4], val)

        prop = self._ConvertProp('byteval')
        self.assertEqual(fdt.TYPE_BYTE, prop.type)
        self.assertEqual(5, ord(prop.value))

        prop = self._ConvertProp('longbytearray')
        self.assertEqual(fdt.TYPE_BYTE, prop.type)
        val = [ord(val) for val in prop.value]
        self.assertEqual([9, 10, 11, 12, 13, 14, 15, 16, 17], val)

        prop = self._ConvertProp('stringval')
        self.assertEqual(fdt.TYPE_STRING, prop.type)
        self.assertEqual('message', prop.value)

        prop = self._ConvertProp('stringarray')
        self.assertEqual(fdt.TYPE_STRING, prop.type)
        self.assertEqual(['multi-word', 'message'], prop.value)

        prop = self._ConvertProp('notstring')
        self.assertEqual(fdt.TYPE_BYTE, prop.type)
        val = [ord(val) for val in prop.value]
        self.assertEqual([0x20, 0x21, 0x22, 0x10, 0], val)

    def testGetEmpty(self):
        """Tests the GetEmpty() function for the various supported types"""
        self.assertEqual(True, fdt.Prop.GetEmpty(fdt.TYPE_BOOL))
        self.assertEqual(chr(0), fdt.Prop.GetEmpty(fdt.TYPE_BYTE))
        self.assertEqual(chr(0) * 4, fdt.Prop.GetEmpty(fdt.TYPE_INT))
        self.assertEqual('', fdt.Prop.GetEmpty(fdt.TYPE_STRING))

    def testGetOffset(self):
        """Test we can get the offset of a property"""
        prop, value = _GetPropertyValue(self.dtb, self.node, 'longbytearray')
        self.assertEqual(prop.value, value)

    def testWiden(self):
        """Test widening of values"""
        node2 = self.dtb.GetNode('/spl-test2')
        prop = self.node.props['intval']

        # No action
        prop2 = node2.props['intval']
        prop.Widen(prop2)
        self.assertEqual(fdt.TYPE_INT, prop.type)
        self.assertEqual(1, fdt32_to_cpu(prop.value))

        # Convert singla value to array
        prop2 = self.node.props['intarray']
        prop.Widen(prop2)
        self.assertEqual(fdt.TYPE_INT, prop.type)
        self.assertTrue(isinstance(prop.value, list))

        # A 4-byte array looks like a single integer. When widened by a longer
        # byte array, it should turn into an array.
        prop = self.node.props['longbytearray']
        prop2 = node2.props['longbytearray']
        self.assertFalse(isinstance(prop2.value, list))
        self.assertEqual(4, len(prop2.value))
        prop2.Widen(prop)
        self.assertTrue(isinstance(prop2.value, list))
        self.assertEqual(9, len(prop2.value))

        # Similarly for a string array
        prop = self.node.props['stringval']
        prop2 = node2.props['stringarray']
        self.assertFalse(isinstance(prop.value, list))
        self.assertEqual(7, len(prop.value))
        prop.Widen(prop2)
        self.assertTrue(isinstance(prop.value, list))
        self.assertEqual(3, len(prop.value))

        # Enlarging an existing array
        prop = self.node.props['stringarray']
        prop2 = node2.props['stringarray']
        self.assertTrue(isinstance(prop.value, list))
        self.assertEqual(2, len(prop.value))
        prop.Widen(prop2)
        self.assertTrue(isinstance(prop.value, list))
        self.assertEqual(3, len(prop.value))

    def testAdd(self):
        """Test adding properties"""
        self.fdt.pack()
        # This function should automatically expand the device tree
        self.node.AddZeroProp('one')
        self.node.AddZeroProp('two')
        self.node.AddZeroProp('three')
        self.dtb.Sync(auto_resize=True)

        # Updating existing properties should be OK, since the device-tree size
        # does not change
        self.fdt.pack()
        self.node.SetInt('one', 1)
        self.node.SetInt('two', 2)
        self.node.SetInt('three', 3)
        self.dtb.Sync(auto_resize=False)

        # This should fail since it would need to increase the device-tree size
        self.node.AddZeroProp('four')
        with self.assertRaises(libfdt.FdtException) as e:
            self.dtb.Sync(auto_resize=False)
        self.assertIn('FDT_ERR_NOSPACE', str(e.exception))
        self.dtb.Sync(auto_resize=True)

    def testAddNode(self):
        self.fdt.pack()
        self.node.AddSubnode('subnode')
        with self.assertRaises(libfdt.FdtException) as e:
            self.dtb.Sync(auto_resize=False)
        self.assertIn('FDT_ERR_NOSPACE', str(e.exception))

        self.dtb.Sync(auto_resize=True)
        offset = self.fdt.path_offset('/spl-test/subnode')
        self.assertTrue(offset > 0)

    def testAddMore(self):
        """Test various other methods for adding and setting properties"""
        self.node.AddZeroProp('one')
        self.dtb.Sync(auto_resize=True)
        data = self.fdt.getprop(self.node.Offset(), 'one')
        self.assertEqual(0, fdt32_to_cpu(data))

        self.node.SetInt('one', 1)
        self.dtb.Sync(auto_resize=False)
        data = self.fdt.getprop(self.node.Offset(), 'one')
        self.assertEqual(1, fdt32_to_cpu(data))

        val = '123' + chr(0) + '456'
        self.node.AddString('string', val)
        self.dtb.Sync(auto_resize=True)
        data = self.fdt.getprop(self.node.Offset(), 'string')
        self.assertEqual(val + '\0', data)

        self.fdt.pack()
        self.node.SetString('string', val + 'x')
        with self.assertRaises(libfdt.FdtException) as e:
            self.dtb.Sync(auto_resize=False)
        self.assertIn('FDT_ERR_NOSPACE', str(e.exception))
        self.node.SetString('string', val[:-1])

        prop = self.node.props['string']
        prop.SetData(val)
        self.dtb.Sync(auto_resize=False)
        data = self.fdt.getprop(self.node.Offset(), 'string')
        self.assertEqual(val, data)

        self.node.AddEmptyProp('empty', 5)
        self.dtb.Sync(auto_resize=True)
        prop = self.node.props['empty']
        prop.SetData(val)
        self.dtb.Sync(auto_resize=False)
        data = self.fdt.getprop(self.node.Offset(), 'empty')
        self.assertEqual(val, data)

        self.node.SetData('empty', '123')
        self.assertEqual('123', prop.bytes)

    def testFromData(self):
        dtb2 = fdt.Fdt.FromData(self.dtb.GetContents())
        self.assertEqual(dtb2.GetContents(), self.dtb.GetContents())

        self.node.AddEmptyProp('empty', 5)
        self.dtb.Sync(auto_resize=True)
        self.assertTrue(dtb2.GetContents() != self.dtb.GetContents())


class TestFdtUtil(unittest.TestCase):
    """Tests for the fdt_util module

    This module will likely be mostly replaced at some point, once upstream
    libfdt has better Python support. For now, this provides tests for current
    functionality.
    """
    @classmethod
    def setUpClass(cls):
        tools.PrepareOutputDir(None)

    @classmethod
    def tearDownClass(cls):
        tools.FinaliseOutputDir()

    def setUp(self):
        self.dtb = fdt.FdtScan('tools/dtoc/dtoc_test_simple.dts')
        self.node = self.dtb.GetNode('/spl-test')

    def testGetInt(self):
        self.assertEqual(1, fdt_util.GetInt(self.node, 'intval'))
        self.assertEqual(3, fdt_util.GetInt(self.node, 'missing', 3))

        with self.assertRaises(ValueError) as e:
            self.assertEqual(3, fdt_util.GetInt(self.node, 'intarray'))
        self.assertIn("property 'intarray' has list value: expecting a single "
                      'integer', str(e.exception))

    def testGetString(self):
        self.assertEqual('message', fdt_util.GetString(self.node, 'stringval'))
        self.assertEqual('test', fdt_util.GetString(self.node, 'missing',
                                                    'test'))

        with self.assertRaises(ValueError) as e:
            self.assertEqual(3, fdt_util.GetString(self.node, 'stringarray'))
        self.assertIn("property 'stringarray' has list value: expecting a "
                      'single string', str(e.exception))

    def testGetBool(self):
        self.assertEqual(True, fdt_util.GetBool(self.node, 'boolval'))
        self.assertEqual(False, fdt_util.GetBool(self.node, 'missing'))
        self.assertEqual(True, fdt_util.GetBool(self.node, 'missing', True))
        self.assertEqual(False, fdt_util.GetBool(self.node, 'missing', False))

    def testGetByte(self):
        self.assertEqual(5, fdt_util.GetByte(self.node, 'byteval'))
        self.assertEqual(3, fdt_util.GetByte(self.node, 'missing', 3))

        with self.assertRaises(ValueError) as e:
            fdt_util.GetByte(self.node, 'longbytearray')
        self.assertIn("property 'longbytearray' has list value: expecting a "
                      'single byte', str(e.exception))

        with self.assertRaises(ValueError) as e:
            fdt_util.GetByte(self.node, 'intval')
        self.assertIn("property 'intval' has length 4, expecting 1",
                      str(e.exception))

    def testGetPhandleList(self):
        dtb = fdt.FdtScan('tools/dtoc/dtoc_test_phandle.dts')
        node = dtb.GetNode('/phandle-source2')
        self.assertEqual([1], fdt_util.GetPhandleList(node, 'clocks'))
        node = dtb.GetNode('/phandle-source')
        self.assertEqual([1, 2, 11, 3, 12, 13, 1],
                         fdt_util.GetPhandleList(node, 'clocks'))
        self.assertEqual(None, fdt_util.GetPhandleList(node, 'missing'))

    def testGetDataType(self):
        self.assertEqual(1, fdt_util.GetDatatype(self.node, 'intval', int))
        self.assertEqual('message', fdt_util.GetDatatype(self.node, 'stringval',
                                                         str))
        with self.assertRaises(ValueError) as e:
            self.assertEqual(3, fdt_util.GetDatatype(self.node, 'boolval',
                                                     bool))
    def testFdtCellsToCpu(self):
        val = self.node.props['intarray'].value
        self.assertEqual(0, fdt_util.fdt_cells_to_cpu(val, 0))
        self.assertEqual(2, fdt_util.fdt_cells_to_cpu(val, 1))

        dtb2 = fdt.FdtScan('tools/dtoc/dtoc_test_addr64.dts')
        node2 = dtb2.GetNode('/test1')
        val = node2.props['reg'].value
        self.assertEqual(0x1234, fdt_util.fdt_cells_to_cpu(val, 2))

    def testEnsureCompiled(self):
        """Test a degenerate case of this function"""
        dtb = fdt_util.EnsureCompiled('tools/dtoc/dtoc_test_simple.dts')
        self.assertEqual(dtb, fdt_util.EnsureCompiled(dtb))

    def testGetPlainBytes(self):
        self.assertEqual('fred', fdt_util.get_plain_bytes('fred'))


def RunTestCoverage():
    """Run the tests and check that we get 100% coverage"""
    test_util.RunTestCoverage('tools/dtoc/test_fdt.py', None,
            ['tools/patman/*.py', '*test_fdt.py'], options.build_dir)


def RunTests(args):
    """Run all the test we have for the fdt model

    Args:
        args: List of positional args provided to fdt. This can hold a test
            name to execute (as in 'fdt -t testFdt', for example)
    """
    result = unittest.TestResult()
    sys.argv = [sys.argv[0]]
    test_name = args and args[0] or None
    for module in (TestFdt, TestNode, TestProp, TestFdtUtil):
        if test_name:
            try:
                suite = unittest.TestLoader().loadTestsFromName(test_name, module)
            except AttributeError:
                continue
        else:
            suite = unittest.TestLoader().loadTestsFromTestCase(module)
        suite.run(result)

    print result
    for _, err in result.errors:
        print err
    for _, err in result.failures:
        print err

if __name__ != '__main__':
    sys.exit(1)

parser = OptionParser()
parser.add_option('-B', '--build-dir', type='string', default='b',
        help='Directory containing the build output')
parser.add_option('-P', '--processes', type=int,
                  help='set number of processes to use for running tests')
parser.add_option('-t', '--test', action='store_true', dest='test',
                  default=False, help='run tests')
parser.add_option('-T', '--test-coverage', action='store_true',
                default=False, help='run tests and check for 100% coverage')
(options, args) = parser.parse_args()

# Run our meagre tests
if options.test:
    RunTests(args)
elif options.test_coverage:
    RunTestCoverage()
