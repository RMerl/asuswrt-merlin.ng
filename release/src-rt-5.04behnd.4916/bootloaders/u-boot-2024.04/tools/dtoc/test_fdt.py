#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0+

"""
Tests for the Fdt module
Copyright (c) 2018 Google, Inc
Written by Simon Glass <sjg@chromium.org>
"""

from argparse import ArgumentParser
import os
import shutil
import sys
import tempfile
import unittest

# Bring in the patman libraries
our_path = os.path.dirname(os.path.realpath(__file__))
sys.path.insert(1, os.path.join(our_path, '..'))

# Bring in the libfdt module
sys.path.insert(2, 'scripts/dtc/pylibfdt')
sys.path.insert(2, os.path.join(our_path, '../../scripts/dtc/pylibfdt'))
sys.path.insert(2, os.path.join(our_path,
                '../../build-sandbox_spl/scripts/dtc/pylibfdt'))

#pylint: disable=wrong-import-position
from dtoc import fdt
from dtoc import fdt_util
from dtoc.fdt_util import fdt32_to_cpu, fdt64_to_cpu
from dtoc.fdt import Type, BytesToValue
import libfdt
from u_boot_pylib import test_util
from u_boot_pylib import tools
from u_boot_pylib import tout

#pylint: disable=protected-access

def _get_property_value(dtb, node, prop_name):
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

def find_dtb_file(dts_fname):
    """Locate a test file in the test/ directory

    Args:
        dts_fname (str): Filename to find, e.g. 'dtoc_test_simple.dts]

    Returns:
        str: Path to the test filename
    """
    return os.path.join('tools/dtoc/test', dts_fname)


class TestFdt(unittest.TestCase):
    """Tests for the Fdt module

    This includes unit tests for some functions and functional tests for the fdt
    module.
    """
    @classmethod
    def setUpClass(cls):
        tools.prepare_output_dir(None)

    @classmethod
    def tearDownClass(cls):
        tools.finalise_output_dir()

    def setUp(self):
        self.dtb = fdt.FdtScan(find_dtb_file('dtoc_test_simple.dts'))

    def test_fdt(self):
        """Test that we can open an Fdt"""
        self.dtb.Scan()
        root = self.dtb.GetRoot()
        self.assertTrue(isinstance(root, fdt.Node))

    def test_get_node(self):
        """Test the GetNode() method"""
        node = self.dtb.GetNode('/spl-test')
        self.assertTrue(isinstance(node, fdt.Node))

        node = self.dtb.GetNode('/i2c@0/pmic@9')
        self.assertTrue(isinstance(node, fdt.Node))
        self.assertEqual('pmic@9', node.name)
        self.assertIsNone(self.dtb.GetNode('/i2c@0/pmic@9/missing'))

        node = self.dtb.GetNode('/')
        self.assertTrue(isinstance(node, fdt.Node))
        self.assertEqual(0, node.Offset())

    def test_flush(self):
        """Check that we can flush the device tree out to its file"""
        fname = self.dtb._fname
        with open(fname, 'rb') as inf:
            inf.read()
        os.remove(fname)
        with self.assertRaises(IOError):
            with open(fname, 'rb'):
                pass
        self.dtb.Flush()
        with open(fname, 'rb') as inf:
            inf.read()

    def test_pack(self):
        """Test that packing a device tree works"""
        self.dtb.Pack()

    def test_get_fdt_raw(self):
        """Tetst that we can access the raw device-tree data"""
        self.assertTrue(isinstance(self.dtb.GetContents(), bytes))

    def test_get_props(self):
        """Tests obtaining a list of properties"""
        node = self.dtb.GetNode('/spl-test')
        props = self.dtb.GetProps(node)
        self.assertEqual(['boolval', 'bootph-all', 'bytearray', 'byteval',
                          'compatible', 'int64val', 'intarray', 'intval',
                          'longbytearray', 'maybe-empty-int', 'notstring',
                          'stringarray', 'stringval', ],
                         sorted(props.keys()))

    def test_check_error(self):
        """Tests the ChecKError() function"""
        with self.assertRaises(ValueError) as exc:
            fdt.CheckErr(-libfdt.NOTFOUND, 'hello')
        self.assertIn('FDT_ERR_NOTFOUND: hello', str(exc.exception))

    def test_get_fdt(self):
        """Test getting an Fdt object from a node"""
        node = self.dtb.GetNode('/spl-test')
        self.assertEqual(self.dtb, node.GetFdt())

    def test_bytes_to_value(self):
        """Test converting a string list into Python"""
        self.assertEqual(BytesToValue(b'this\0is\0'),
                         (Type.STRING, ['this', 'is']))

class TestNode(unittest.TestCase):
    """Test operation of the Node class"""

    @classmethod
    def setUpClass(cls):
        tools.prepare_output_dir(None)

    @classmethod
    def tearDownClass(cls):
        tools.finalise_output_dir()

    def setUp(self):
        self.dtb = fdt.FdtScan(find_dtb_file('dtoc_test_simple.dts'))
        self.node = self.dtb.GetNode('/spl-test')
        self.fdt = self.dtb.GetFdtObj()

    def test_offset(self):
        """Tests that we can obtain the offset of a node"""
        self.assertTrue(self.node.Offset() > 0)

    def test_delete(self):
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

    def test_delete_get_offset(self):
        """Test that property offset update when properties are deleted"""
        self.node.DeleteProp('intval')
        prop, value = _get_property_value(self.dtb, self.node, 'longbytearray')
        self.assertEqual(prop.value, value)

    def test_find_node(self):
        """Tests that we can find a node using the FindNode() functoin"""
        node = self.dtb.GetRoot().FindNode('i2c@0')
        self.assertEqual('i2c@0', node.name)
        subnode = node.FindNode('pmic@9')
        self.assertEqual('pmic@9', subnode.name)
        self.assertEqual(None, node.FindNode('missing'))

    def test_refresh_missing_node(self):
        """Test refreshing offsets when an extra node is present in dtb"""
        # Delete it from our tables, not the device tree
        del self.dtb._root.subnodes[-1]
        with self.assertRaises(ValueError) as exc:
            self.dtb.Refresh()
        self.assertIn('Internal error, offset', str(exc.exception))

    def test_refresh_extra_node(self):
        """Test refreshing offsets when an expected node is missing"""
        # Delete it from the device tre, not our tables
        self.fdt.del_node(self.node.Offset())
        with self.assertRaises(ValueError) as exc:
            self.dtb.Refresh()
        self.assertIn('Internal error, node name mismatch '
                      'spl-test != spl-test2', str(exc.exception))

    def test_refresh_missing_prop(self):
        """Test refreshing offsets when an extra property is present in dtb"""
        # Delete it from our tables, not the device tree
        del self.node.props['notstring']
        with self.assertRaises(ValueError) as exc:
            self.dtb.Refresh()
        self.assertIn("Internal error, node '/spl-test' property 'notstring' missing, offset ",
                      str(exc.exception))

    def test_lookup_phandle(self):
        """Test looking up a single phandle"""
        dtb = fdt.FdtScan(find_dtb_file('dtoc_test_phandle.dts'))
        node = dtb.GetNode('/phandle-source2')
        prop = node.props['clocks']
        target = dtb.GetNode('/phandle-target')
        self.assertEqual(target, dtb.LookupPhandle(fdt32_to_cpu(prop.value)))

    def test_add_node_space(self):
        """Test adding a single node when out of space"""
        self.fdt.pack()
        self.node.AddSubnode('subnode')
        with self.assertRaises(libfdt.FdtException) as exc:
            self.dtb.Sync(auto_resize=False)
        self.assertIn('FDT_ERR_NOSPACE', str(exc.exception))

        self.dtb.Sync(auto_resize=True)
        offset = self.fdt.path_offset('/spl-test/subnode')
        self.assertTrue(offset > 0)

    def test_add_nodes(self):
        """Test adding various subnode and properies"""
        node = self.dtb.GetNode('/i2c@0')

        # Add one more node next to the pmic one
        sn1 = node.AddSubnode('node-one')
        sn1.AddInt('integer-a', 12)
        sn1.AddInt('integer-b', 23)

        # Sync so that everything is clean
        self.dtb.Sync(auto_resize=True)

        # Add two subnodes next to pmic and node-one
        sn2 = node.AddSubnode('node-two')
        sn2.AddInt('integer-2a', 34)
        sn2.AddInt('integer-2b', 45)

        sn3 = node.AddSubnode('node-three')
        sn3.AddInt('integer-3', 123)

        # Add a property to the node after i2c@0 to check that this is not
        # disturbed by adding a subnode to i2c@0
        orig_node = self.dtb.GetNode('/orig-node')
        orig_node.AddInt('integer-4', 456)

        # Add a property to the pmic node to check that pmic properties are not
        # disturbed
        pmic = self.dtb.GetNode('/i2c@0/pmic@9')
        pmic.AddInt('integer-5', 567)

        self.dtb.Sync(auto_resize=True)

    def test_add_one_node(self):
        """Testing deleting and adding a subnode before syncing"""
        subnode = self.node.AddSubnode('subnode')
        self.node.AddSubnode('subnode2')
        self.dtb.Sync(auto_resize=True)

        # Delete a node and add a new one
        subnode.Delete()
        self.node.AddSubnode('subnode3')
        self.dtb.Sync()

    def test_refresh_name_mismatch(self):
        """Test name mismatch when syncing nodes and properties"""
        self.node.AddInt('integer-a', 12)

        wrong_offset = self.dtb.GetNode('/i2c@0')._offset
        self.node._offset = wrong_offset
        with self.assertRaises(ValueError) as exc:
            self.dtb.Sync()
        self.assertIn("Internal error, node '/spl-test' name mismatch 'i2c@0'",
                      str(exc.exception))

        with self.assertRaises(ValueError) as exc:
            self.node.Refresh(wrong_offset)
        self.assertIn("Internal error, node '/spl-test' name mismatch 'i2c@0'",
                      str(exc.exception))

    def test_copy_node(self):
        """Test copy_node() function"""
        def do_copy_checks(dtb, dst, second1_ph_val, expect_none):
            self.assertEqual(
                ['/dest/base', '/dest/first@0', '/dest/existing'],
                [n.path for n in dst.subnodes])

            chk = dtb.GetNode('/dest/base')
            self.assertTrue(chk)
            self.assertEqual(
                {'compatible', 'bootph-all', '#address-cells', '#size-cells'},
                chk.props.keys())

            # Check the first property
            prop = chk.props['bootph-all']
            self.assertEqual('bootph-all', prop.name)
            self.assertEqual(True, prop.value)
            self.assertEqual(chk.path, prop._node.path)

            # Check the second property
            prop2 = chk.props['compatible']
            self.assertEqual('compatible', prop2.name)
            self.assertEqual('sandbox,i2c', prop2.value)
            self.assertEqual(chk.path, prop2._node.path)

            base = chk.FindNode('base')
            self.assertTrue(chk)

            first = dtb.GetNode('/dest/base/first@0')
            self.assertTrue(first)
            over = dtb.GetNode('/dest/base/over')
            self.assertTrue(over)

            # Make sure that the phandle for 'over' is copied
            self.assertIn('phandle', over.props.keys())

            second = dtb.GetNode('/dest/base/second')
            self.assertTrue(second)
            self.assertEqual([over.name, first.name, second.name],
                             [n.name for n in chk.subnodes])
            self.assertEqual(chk, over.parent)
            self.assertEqual(
                {'bootph-all', 'compatible', 'reg', 'low-power', 'phandle'},
                over.props.keys())

            if expect_none:
                self.assertIsNone(prop._offset)
                self.assertIsNone(prop2._offset)
                self.assertIsNone(over._offset)
            else:
                self.assertTrue(prop._offset)
                self.assertTrue(prop2._offset)
                self.assertTrue(over._offset)

            # Now check ordering of the subnodes
            self.assertEqual(
                ['second1', 'second2', 'second3', 'second4'],
                [n.name for n in second.subnodes])

            # Check the 'second_1_bad' phandle is not copied over
            second1 = second.FindNode('second1')
            self.assertTrue(second1)
            sph = second1.props.get('phandle')
            self.assertTrue(sph)
            self.assertEqual(second1_ph_val, sph.bytes)


        dtb = fdt.FdtScan(find_dtb_file('dtoc_test_copy.dts'))
        tmpl = dtb.GetNode('/base')
        dst = dtb.GetNode('/dest')
        second1_ph_val = (dtb.GetNode('/dest/base/second/second1').
                          props['phandle'].bytes)
        dst.copy_node(tmpl)

        do_copy_checks(dtb, dst, second1_ph_val, expect_none=True)

        dtb.Sync(auto_resize=True)

        # Now check the resulting FDT. It should have duplicate phandles since
        # 'over' has been copied to 'dest/base/over' but still exists in its old
        # place
        new_dtb = fdt.Fdt.FromData(dtb.GetContents())
        with self.assertRaises(ValueError) as exc:
            new_dtb.Scan()
        self.assertIn(
            'Duplicate phandle 1 in nodes /dest/base/over and /base/over',
            str(exc.exception))

        # Remove the source nodes for the copy
        new_dtb.GetNode('/base').Delete()

        # Now it should scan OK
        new_dtb.Scan()

        dst = new_dtb.GetNode('/dest')
        do_copy_checks(new_dtb, dst, second1_ph_val, expect_none=False)

    def test_copy_subnodes_from_phandles(self):
        """Test copy_node() function"""
        dtb = fdt.FdtScan(find_dtb_file('dtoc_test_copy.dts'))

        orig = dtb.GetNode('/')
        node_list = fdt_util.GetPhandleList(orig, 'copy-list')

        dst = dtb.GetNode('/dest')
        dst.copy_subnodes_from_phandles(node_list)

        pmic = dtb.GetNode('/dest/over')
        self.assertTrue(pmic)

        subn = dtb.GetNode('/dest/first@0')
        self.assertTrue(subn)
        self.assertEqual({'a-prop', 'b-prop', 'reg'}, subn.props.keys())

        self.assertEqual(
            ['/dest/earlier', '/dest/later', '/dest/over', '/dest/first@0',
             '/dest/second', '/dest/existing', '/dest/base'],
            [n.path for n in dst.subnodes])

        # Make sure that the phandle for 'over' is not copied
        over = dst.FindNode('over')
        tout.debug(f'keys: {over.props.keys()}')
        self.assertNotIn('phandle', over.props.keys())

        # Check the merged properties, first the base ones in '/dest'
        expect = {'bootph-all', 'compatible', 'stringarray', 'longbytearray',
                  'maybe-empty-int'}

        # Properties from 'base'
        expect.update({'#address-cells', '#size-cells'})

        # Properties from 'another'
        expect.add('new-prop')

        self.assertEqual(expect, set(dst.props.keys()))


class TestProp(unittest.TestCase):
    """Test operation of the Prop class"""

    @classmethod
    def setUpClass(cls):
        tools.prepare_output_dir(None)

    @classmethod
    def tearDownClass(cls):
        tools.finalise_output_dir()

    def setUp(self):
        self.dtb = fdt.FdtScan(find_dtb_file('dtoc_test_simple.dts'))
        self.node = self.dtb.GetNode('/spl-test')
        self.fdt = self.dtb.GetFdtObj()

    def test_missing_node(self):
        """Test GetNode() when the node is missing"""
        self.assertEqual(None, self.dtb.GetNode('missing'))

    def test_phandle(self):
        """Test GetNode() on a phandle"""
        dtb = fdt.FdtScan(find_dtb_file('dtoc_test_phandle.dts'))
        node = dtb.GetNode('/phandle-source2')
        prop = node.props['clocks']
        self.assertTrue(fdt32_to_cpu(prop.value) > 0)

    def _convert_prop(self, prop_name):
        """Helper function to look up a property in self.node and return it

        Args:
            str: Property name to find

        Returns:
            fdt.Prop: object for this property
        """
        prop = self.fdt.getprop(self.node.Offset(), prop_name)
        return fdt.Prop(self.node, -1, prop_name, prop)

    def test_make_prop(self):
        """Test we can convert all the the types that are supported"""
        prop = self._convert_prop('boolval')
        self.assertEqual(Type.BOOL, prop.type)
        self.assertEqual(True, prop.value)

        prop = self._convert_prop('intval')
        self.assertEqual(Type.INT, prop.type)
        self.assertEqual(1, fdt32_to_cpu(prop.value))

        prop = self._convert_prop('int64val')
        self.assertEqual(Type.INT, prop.type)
        self.assertEqual(0x123456789abcdef0, fdt64_to_cpu(prop.value))

        prop = self._convert_prop('intarray')
        self.assertEqual(Type.INT, prop.type)
        val = [fdt32_to_cpu(val) for val in prop.value]
        self.assertEqual([2, 3, 4], val)

        prop = self._convert_prop('byteval')
        self.assertEqual(Type.BYTE, prop.type)
        self.assertEqual(5, ord(prop.value))

        prop = self._convert_prop('longbytearray')
        self.assertEqual(Type.BYTE, prop.type)
        val = [ord(val) for val in prop.value]
        self.assertEqual([9, 10, 11, 12, 13, 14, 15, 16, 17], val)

        prop = self._convert_prop('stringval')
        self.assertEqual(Type.STRING, prop.type)
        self.assertEqual('message', prop.value)

        prop = self._convert_prop('stringarray')
        self.assertEqual(Type.STRING, prop.type)
        self.assertEqual(['multi-word', 'message'], prop.value)

        prop = self._convert_prop('notstring')
        self.assertEqual(Type.BYTE, prop.type)
        val = [ord(val) for val in prop.value]
        self.assertEqual([0x20, 0x21, 0x22, 0x10, 0], val)

    def test_get_empty(self):
        """Tests the GetEmpty() function for the various supported types"""
        self.assertEqual(True, fdt.Prop.GetEmpty(Type.BOOL))
        self.assertEqual(chr(0), fdt.Prop.GetEmpty(Type.BYTE))
        self.assertEqual(tools.get_bytes(0, 4), fdt.Prop.GetEmpty(Type.INT))
        self.assertEqual('', fdt.Prop.GetEmpty(Type.STRING))

    def test_get_offset(self):
        """Test we can get the offset of a property"""
        prop, value = _get_property_value(self.dtb, self.node, 'longbytearray')
        self.assertEqual(prop.value, value)

    def test_widen(self):
        """Test widening of values"""
        node2 = self.dtb.GetNode('/spl-test2')
        node3 = self.dtb.GetNode('/spl-test3')
        prop = self.node.props['intval']

        # No action
        prop2 = node2.props['intval']
        prop.Widen(prop2)
        self.assertEqual(Type.INT, prop.type)
        self.assertEqual(1, fdt32_to_cpu(prop.value))

        # Convert single value to array
        prop2 = self.node.props['intarray']
        prop.Widen(prop2)
        self.assertEqual(Type.INT, prop.type)
        self.assertTrue(isinstance(prop.value, list))

        # A 4-byte array looks like a single integer. When widened by a longer
        # byte array, it should turn into an array.
        prop = self.node.props['longbytearray']
        prop2 = node2.props['longbytearray']
        prop3 = node3.props['longbytearray']
        self.assertFalse(isinstance(prop2.value, list))
        self.assertEqual(4, len(prop2.value))
        self.assertEqual(b'\x09\x0a\x0b\x0c', prop2.value)
        prop2.Widen(prop)
        self.assertTrue(isinstance(prop2.value, list))
        self.assertEqual(9, len(prop2.value))
        self.assertEqual(['\x09', '\x0a', '\x0b', '\x0c', '\0',
                          '\0', '\0', '\0', '\0'], prop2.value)
        prop3.Widen(prop)
        self.assertTrue(isinstance(prop3.value, list))
        self.assertEqual(9, len(prop3.value))
        self.assertEqual(['\x09', '\x0a', '\x0b', '\x0c', '\x0d',
                          '\x0e', '\x0f', '\x10', '\0'], prop3.value)

    def test_widen_more(self):
        """More tests of widening values"""
        node2 = self.dtb.GetNode('/spl-test2')
        node3 = self.dtb.GetNode('/spl-test3')
        prop = self.node.props['intval']

        # Test widening a single string into a string array
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

        # Widen an array of ints with an int (should do nothing)
        prop = self.node.props['intarray']
        prop2 = node2.props['intval']
        self.assertEqual(Type.INT, prop.type)
        self.assertEqual(3, len(prop.value))
        prop.Widen(prop2)
        self.assertEqual(Type.INT, prop.type)
        self.assertEqual(3, len(prop.value))

        # Widen an empty bool to an int
        prop = self.node.props['maybe-empty-int']
        prop3 = node3.props['maybe-empty-int']
        self.assertEqual(Type.BOOL, prop.type)
        self.assertEqual(True, prop.value)
        self.assertEqual(Type.INT, prop3.type)
        self.assertFalse(isinstance(prop.value, list))
        self.assertEqual(4, len(prop3.value))
        prop.Widen(prop3)
        self.assertEqual(Type.INT, prop.type)
        self.assertTrue(isinstance(prop.value, list))
        self.assertEqual(1, len(prop.value))

    def test_add(self):
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
        with self.assertRaises(libfdt.FdtException) as exc:
            self.dtb.Sync(auto_resize=False)
        self.assertIn('FDT_ERR_NOSPACE', str(exc.exception))
        self.dtb.Sync(auto_resize=True)

    def test_add_more(self):
        """Test various other methods for adding and setting properties"""
        self.node.AddZeroProp('one')
        self.dtb.Sync(auto_resize=True)
        data = self.fdt.getprop(self.node.Offset(), 'one')
        self.assertEqual(0, fdt32_to_cpu(data))

        self.node.SetInt('one', 1)
        self.dtb.Sync(auto_resize=False)
        data = self.fdt.getprop(self.node.Offset(), 'one')
        self.assertEqual(1, fdt32_to_cpu(data))

        val = 1234
        self.node.AddInt('integer', val)
        self.dtb.Sync(auto_resize=True)
        data = self.fdt.getprop(self.node.Offset(), 'integer')
        self.assertEqual(val, fdt32_to_cpu(data))

        val = '123' + chr(0) + '456'
        self.node.AddString('string', val)
        self.dtb.Sync(auto_resize=True)
        data = self.fdt.getprop(self.node.Offset(), 'string')
        self.assertEqual(tools.to_bytes(val) + b'\0', data)

        self.fdt.pack()
        self.node.SetString('string', val + 'x')
        with self.assertRaises(libfdt.FdtException) as exc:
            self.dtb.Sync(auto_resize=False)
        self.assertIn('FDT_ERR_NOSPACE', str(exc.exception))
        self.node.SetString('string', val[:-1])

        prop = self.node.props['string']
        prop.SetData(tools.to_bytes(val))
        self.dtb.Sync(auto_resize=False)
        data = self.fdt.getprop(self.node.Offset(), 'string')
        self.assertEqual(tools.to_bytes(val), data)

        self.node.AddEmptyProp('empty', 5)
        self.dtb.Sync(auto_resize=True)
        prop = self.node.props['empty']
        prop.SetData(tools.to_bytes(val))
        self.dtb.Sync(auto_resize=False)
        data = self.fdt.getprop(self.node.Offset(), 'empty')
        self.assertEqual(tools.to_bytes(val), data)

        self.node.SetData('empty', b'123')
        self.assertEqual(b'123', prop.bytes)

        # Trying adding a lot of data at once
        self.node.AddData('data', tools.get_bytes(65, 20000))
        self.dtb.Sync(auto_resize=True)

    def test_string_list(self):
        """Test adding string-list property to a node"""
        val = ['123', '456']
        self.node.AddStringList('stringlist', val)
        self.dtb.Sync(auto_resize=True)
        data = self.fdt.getprop(self.node.Offset(), 'stringlist')
        self.assertEqual(b'123\x00456\0', data)

        val = []
        self.node.AddStringList('stringlist', val)
        self.dtb.Sync(auto_resize=True)
        data = self.fdt.getprop(self.node.Offset(), 'stringlist')
        self.assertEqual(b'', data)

    def test_delete_node(self):
        """Test deleting a node"""
        old_offset = self.fdt.path_offset('/spl-test')
        self.assertGreater(old_offset, 0)
        self.node.Delete()
        self.dtb.Sync()
        new_offset = self.fdt.path_offset('/spl-test', libfdt.QUIET_NOTFOUND)
        self.assertEqual(-libfdt.NOTFOUND, new_offset)

    def test_from_data(self):
        """Test creating an FDT from data"""
        dtb2 = fdt.Fdt.FromData(self.dtb.GetContents())
        self.assertEqual(dtb2.GetContents(), self.dtb.GetContents())

        self.node.AddEmptyProp('empty', 5)
        self.dtb.Sync(auto_resize=True)
        self.assertTrue(dtb2.GetContents() != self.dtb.GetContents())

    def test_missing_set_int(self):
        """Test handling of a missing property with SetInt"""
        with self.assertRaises(ValueError) as exc:
            self.node.SetInt('one', 1)
        self.assertIn("node '/spl-test': Missing property 'one'",
                      str(exc.exception))

    def test_missing_set_data(self):
        """Test handling of a missing property with SetData"""
        with self.assertRaises(ValueError) as exc:
            self.node.SetData('one', b'data')
        self.assertIn("node '/spl-test': Missing property 'one'",
                      str(exc.exception))

    def test_missing_set_string(self):
        """Test handling of a missing property with SetString"""
        with self.assertRaises(ValueError) as exc:
            self.node.SetString('one', 1)
        self.assertIn("node '/spl-test': Missing property 'one'",
                      str(exc.exception))

    def test_get_filename(self):
        """Test the dtb filename can be provided"""
        self.assertEqual(tools.get_output_filename('source.dtb'),
                         self.dtb.GetFilename())


class TestFdtUtil(unittest.TestCase):
    """Tests for the fdt_util module

    This module will likely be mostly replaced at some point, once upstream
    libfdt has better Python support. For now, this provides tests for current
    functionality.
    """
    @classmethod
    def setUpClass(cls):
        tools.prepare_output_dir(None)

    @classmethod
    def tearDownClass(cls):
        tools.finalise_output_dir()

    def setUp(self):
        self.dtb = fdt.FdtScan(find_dtb_file('dtoc_test_simple.dts'))
        self.node = self.dtb.GetNode('/spl-test')

    def test_get_int(self):
        """Test getting an int from a node"""
        self.assertEqual(1, fdt_util.GetInt(self.node, 'intval'))
        self.assertEqual(3, fdt_util.GetInt(self.node, 'missing', 3))

        with self.assertRaises(ValueError) as exc:
            fdt_util.GetInt(self.node, 'intarray')
        self.assertIn("property 'intarray' has list value: expecting a single "
                      'integer', str(exc.exception))

    def test_get_int64(self):
        """Test getting a 64-bit int from a node"""
        self.assertEqual(0x123456789abcdef0,
                         fdt_util.GetInt64(self.node, 'int64val'))
        self.assertEqual(3, fdt_util.GetInt64(self.node, 'missing', 3))

        with self.assertRaises(ValueError) as exc:
            fdt_util.GetInt64(self.node, 'intarray')
        self.assertIn(
            "property 'intarray' should be a list with 2 items for 64-bit values",
            str(exc.exception))

    def test_get_string(self):
        """Test getting a string from a node"""
        self.assertEqual('message', fdt_util.GetString(self.node, 'stringval'))
        self.assertEqual('test', fdt_util.GetString(self.node, 'missing',
                                                    'test'))
        self.assertEqual('', fdt_util.GetString(self.node, 'boolval'))

        with self.assertRaises(ValueError) as exc:
            self.assertEqual(3, fdt_util.GetString(self.node, 'stringarray'))
        self.assertIn("property 'stringarray' has list value: expecting a "
                      'single string', str(exc.exception))

    def test_get_string_list(self):
        """Test getting a string list from a node"""
        self.assertEqual(['message'],
                         fdt_util.GetStringList(self.node, 'stringval'))
        self.assertEqual(
            ['multi-word', 'message'],
            fdt_util.GetStringList(self.node, 'stringarray'))
        self.assertEqual(['test'],
                         fdt_util.GetStringList(self.node, 'missing', ['test']))
        self.assertEqual([], fdt_util.GetStringList(self.node, 'boolval'))

    def test_get_args(self):
        """Test getting arguments from a node"""
        node = self.dtb.GetNode('/orig-node')
        self.assertEqual(['message'], fdt_util.GetArgs(self.node, 'stringval'))
        self.assertEqual(
            ['multi-word', 'message'],
            fdt_util.GetArgs(self.node, 'stringarray'))
        self.assertEqual([], fdt_util.GetArgs(self.node, 'boolval'))
        self.assertEqual(['-n first', 'second', '-p', '123,456', '-x'],
                         fdt_util.GetArgs(node, 'args'))
        self.assertEqual(['a space', 'there'],
                         fdt_util.GetArgs(node, 'args2'))
        self.assertEqual(['-n', 'first', 'second', '-p', '123,456', '-x'],
                         fdt_util.GetArgs(node, 'args3'))
        with self.assertRaises(ValueError) as exc:
            fdt_util.GetArgs(self.node, 'missing')
        self.assertIn(
            "Node '/spl-test': Expected property 'missing'",
            str(exc.exception))

    def test_get_bool(self):
        """Test getting a bool from a node"""
        self.assertEqual(True, fdt_util.GetBool(self.node, 'boolval'))
        self.assertEqual(False, fdt_util.GetBool(self.node, 'missing'))
        self.assertEqual(True, fdt_util.GetBool(self.node, 'missing', True))
        self.assertEqual(False, fdt_util.GetBool(self.node, 'missing', False))

    def test_get_byte(self):
        """Test getting a byte from a node"""
        self.assertEqual(5, fdt_util.GetByte(self.node, 'byteval'))
        self.assertEqual(3, fdt_util.GetByte(self.node, 'missing', 3))

        with self.assertRaises(ValueError) as exc:
            fdt_util.GetByte(self.node, 'longbytearray')
        self.assertIn("property 'longbytearray' has list value: expecting a "
                      'single byte', str(exc.exception))

        with self.assertRaises(ValueError) as exc:
            fdt_util.GetByte(self.node, 'intval')
        self.assertIn("property 'intval' has length 4, expecting 1",
                      str(exc.exception))

    def test_get_bytes(self):
        """Test getting multiple bytes from a node"""
        self.assertEqual(bytes([5]), fdt_util.GetBytes(self.node, 'byteval', 1))
        self.assertEqual(None, fdt_util.GetBytes(self.node, 'missing', 3))
        self.assertEqual(
            bytes([3]), fdt_util.GetBytes(self.node, 'missing', 3,  bytes([3])))

        with self.assertRaises(ValueError) as exc:
            fdt_util.GetBytes(self.node, 'longbytearray', 7)
        self.assertIn(
            "Node 'spl-test' property 'longbytearray' has length 9, expecting 7",
             str(exc.exception))

        self.assertEqual(
            bytes([0, 0, 0, 1]), fdt_util.GetBytes(self.node, 'intval', 4))
        self.assertEqual(
            bytes([3]), fdt_util.GetBytes(self.node, 'missing', 3,  bytes([3])))

    def test_get_phandle_list(self):
        """Test getting a list of phandles from a node"""
        dtb = fdt.FdtScan(find_dtb_file('dtoc_test_phandle.dts'))
        node = dtb.GetNode('/phandle-source2')
        self.assertEqual([1], fdt_util.GetPhandleList(node, 'clocks'))
        node = dtb.GetNode('/phandle-source')
        self.assertEqual([1, 2, 11, 3, 12, 13, 1],
                         fdt_util.GetPhandleList(node, 'clocks'))
        self.assertEqual(None, fdt_util.GetPhandleList(node, 'missing'))

    def test_get_data_type(self):
        """Test getting a value of a particular type from a node"""
        self.assertEqual(1, fdt_util.GetDatatype(self.node, 'intval', int))
        self.assertEqual('message', fdt_util.GetDatatype(self.node, 'stringval',
                                                         str))
        with self.assertRaises(ValueError):
            self.assertEqual(3, fdt_util.GetDatatype(self.node, 'boolval',
                                                     bool))
    def test_fdt_cells_to_cpu(self):
        """Test getting cells with the correct endianness"""
        val = self.node.props['intarray'].value
        self.assertEqual(0, fdt_util.fdt_cells_to_cpu(val, 0))
        self.assertEqual(2, fdt_util.fdt_cells_to_cpu(val, 1))

        dtb2 = fdt.FdtScan(find_dtb_file('dtoc_test_addr64.dts'))
        node1 = dtb2.GetNode('/test1')
        val = node1.props['reg'].value
        self.assertEqual(0x1234, fdt_util.fdt_cells_to_cpu(val, 2))

        node2 = dtb2.GetNode('/test2')
        val = node2.props['reg'].value
        self.assertEqual(0x1234567890123456, fdt_util.fdt_cells_to_cpu(val, 2))
        self.assertEqual(0x9876543210987654, fdt_util.fdt_cells_to_cpu(val[2:],
                                                                       2))
        self.assertEqual(0x12345678, fdt_util.fdt_cells_to_cpu(val, 1))

    def test_ensure_compiled(self):
        """Test a degenerate case of this function (file already compiled)"""
        dtb = fdt_util.EnsureCompiled(find_dtb_file('dtoc_test_simple.dts'))
        self.assertEqual(dtb, fdt_util.EnsureCompiled(dtb))

    def test_ensure_compiled_tmpdir(self):
        """Test providing a temporary directory"""
        old_outdir = tools.outdir
        try:
            tools.outdir= None
            tmpdir = tempfile.mkdtemp(prefix='test_fdt.')
            dtb = fdt_util.EnsureCompiled(find_dtb_file('dtoc_test_simple.dts'),
                                          tmpdir)
            self.assertEqual(tmpdir, os.path.dirname(dtb))
            shutil.rmtree(tmpdir)
        finally:
            tools.outdir = old_outdir

    def test_get_phandle_name_offset(self):
        val = fdt_util.GetPhandleNameOffset(self.node, 'missing')
        self.assertIsNone(val)

        dtb = fdt.FdtScan(find_dtb_file('dtoc_test_phandle.dts'))
        node = dtb.GetNode('/phandle-source')
        node, name, offset = fdt_util.GetPhandleNameOffset(node,
                                                           'phandle-name-offset')
        self.assertEqual('phandle3-target', node.name)
        self.assertEqual('fred', name)
        self.assertEqual(123, offset)

def run_test_coverage(build_dir):
    """Run the tests and check that we get 100% coverage

    Args:
        build_dir (str): Directory containing the build output
    """
    test_util.run_test_coverage('tools/dtoc/test_fdt.py', None,
            ['tools/patman/*.py', 'tools/u_boot_pylib/*', '*test_fdt.py'],
            build_dir)


def run_tests(names, processes):
    """Run all the test we have for the fdt model

    Args:
        names (list of str): List of test names provided. Only the first is used
        processes (int): Number of processes to use (None means as many as there
            are CPUs on the system. This must be set to 1 when running under
            the python3-coverage tool

    Returns:
        int: Return code, 0 on success
    """
    test_name = names[0] if names else None
    result = test_util.run_test_suites(
        'test_fdt', False, False, False, processes, test_name, None,
        [TestFdt, TestNode, TestProp, TestFdtUtil])

    return (0 if result.wasSuccessful() else 1)


def main():
    """Main program for this tool"""
    parser = ArgumentParser()
    parser.add_argument('-B', '--build-dir', type=str, default='b',
                        help='Directory containing the build output')
    parser.add_argument('-P', '--processes', type=int,
                        help='set number of processes to use for running tests')
    parser.add_argument('-t', '--test', action='store_true', dest='test',
                        default=False, help='run tests')
    parser.add_argument('-T', '--test-coverage', action='store_true',
                        default=False,
                        help='run tests and check for 100% coverage')
    parser.add_argument('name', nargs='*')
    args = parser.parse_args()

    # Run our meagre tests
    if args.test:
        ret_code = run_tests(args.name, args.processes)
        return ret_code
    if args.test_coverage:
        run_test_coverage(args.build_dir)
    return 0

if __name__ == '__main__':
    sys.exit(main())
