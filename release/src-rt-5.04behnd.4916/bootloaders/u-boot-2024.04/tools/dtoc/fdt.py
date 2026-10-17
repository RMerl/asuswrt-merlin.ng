#!/usr/bin/python
# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (C) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#

from enum import IntEnum
import struct
import sys

from dtoc import fdt_util
import libfdt
from libfdt import QUIET_NOTFOUND
from u_boot_pylib import tools
from u_boot_pylib import tout

# This deals with a device tree, presenting it as an assortment of Node and
# Prop objects, representing nodes and properties, respectively. This file
# contains the base classes and defines the high-level API. You can use
# FdtScan() as a convenience function to create and scan an Fdt.

# This implementation uses a libfdt Python library to access the device tree,
# so it is fairly efficient.

# A list of types we support
class Type(IntEnum):
    # Types in order from widest to narrowest
    (BYTE, INT, STRING, BOOL, INT64) = range(5)

    def needs_widening(self, other):
        """Check if this type needs widening to hold a value from another type

        A wider type is one that can hold a wider array of information than
        another one, or is less restrictive, so it can hold the information of
        another type as well as its own. This is similar to the concept of
        type-widening in C.

        This uses a simple arithmetic comparison, since type values are in order
        from widest (BYTE) to narrowest (INT64).

        Args:
            other: Other type to compare against

        Return:
            True if the other type is wider
        """
        return self.value > other.value

def CheckErr(errnum, msg):
    if errnum:
        raise ValueError('Error %d: %s: %s' %
            (errnum, libfdt.fdt_strerror(errnum), msg))


def BytesToValue(data):
    """Converts a string of bytes into a type and value

    Args:
        A bytes value (which on Python 2 is an alias for str)

    Return:
        A tuple:
            Type of data
            Data, either a single element or a list of elements. Each element
            is one of:
                Type.STRING: str/bytes value from the property
                Type.INT: a byte-swapped integer stored as a 4-byte str/bytes
                Type.BYTE: a byte stored as a single-byte str/bytes
    """
    data = bytes(data)
    size = len(data)
    strings = data.split(b'\0')
    is_string = True
    count = len(strings) - 1
    if count > 0 and not len(strings[-1]):
        for string in strings[:-1]:
            if not string:
                is_string = False
                break
            for ch in string:
                if ch < 32 or ch > 127:
                    is_string = False
                    break
    else:
        is_string = False
    if is_string:
        if count == 1: 
            return Type.STRING, strings[0].decode()
        else:
            return Type.STRING, [s.decode() for s in strings[:-1]]
    if size % 4:
        if size == 1:
            return Type.BYTE, chr(data[0])
        else:
            return Type.BYTE, [chr(ch) for ch in list(data)]
    val = []
    for i in range(0, size, 4):
        val.append(data[i:i + 4])
    if size == 4:
        return Type.INT, val[0]
    else:
        return Type.INT, val


class Prop:
    """A device tree property

    Properties:
        node: Node containing this property
        offset: Offset of the property (None if still to be synced)
        name: Property name (as per the device tree)
        value: Property value as a string of bytes, or a list of strings of
            bytes
        type: Value type
    """
    def __init__(self, node, offset, name, data):
        self._node = node
        self._offset = offset
        self.name = name
        self.value = None
        self.bytes = bytes(data)
        self.dirty = offset is None
        if not data:
            self.type = Type.BOOL
            self.value = True
            return
        self.type, self.value = BytesToValue(bytes(data))

    def RefreshOffset(self, poffset):
        self._offset = poffset

    def Widen(self, newprop):
        """Figure out which property type is more general

        Given a current property and a new property, this function returns the
        one that is less specific as to type. The less specific property will
        be ble to represent the data in the more specific property. This is
        used for things like:

            node1 {
                compatible = "fred";
                value = <1>;
            };
            node1 {
                compatible = "fred";
                value = <1 2>;
            };

        He we want to use an int array for 'value'. The first property
        suggests that a single int is enough, but the second one shows that
        it is not. Calling this function with these two propertes would
        update the current property to be like the second, since it is less
        specific.
        """
        if self.type.needs_widening(newprop.type):

            # A boolean has an empty value: if it exists it is True and if not
            # it is False. So when widening we always start with an empty list
            # since the only valid integer property would be an empty list of
            # integers.
            # e.g. this is a boolean:
            #    some-prop;
            # and it would be widened to int list by:
            #    some-prop = <1 2>;
            if self.type == Type.BOOL:
                self.type = Type.INT
                self.value = [self.GetEmpty(self.type)]
            if self.type == Type.INT and newprop.type == Type.BYTE:
                if type(self.value) == list:
                    new_value = []
                    for val in self.value:
                        new_value += [chr(by) for by in val]
                else:
                    new_value = [chr(by) for by in self.value]
                self.value = new_value
            self.type = newprop.type

        if type(newprop.value) == list:
            if type(self.value) != list:
                self.value = [self.value]

            if len(newprop.value) > len(self.value):
                val = self.GetEmpty(self.type)
                while len(self.value) < len(newprop.value):
                    self.value.append(val)

    @classmethod
    def GetEmpty(self, type):
        """Get an empty / zero value of the given type

        Returns:
            A single value of the given type
        """
        if type == Type.BYTE:
            return chr(0)
        elif type == Type.INT:
            return struct.pack('>I', 0);
        elif type == Type.STRING:
            return ''
        else:
            return True

    def GetOffset(self):
        """Get the offset of a property

        Returns:
            The offset of the property (struct fdt_property) within the file
        """
        self._node._fdt.CheckCache()
        return self._node._fdt.GetStructOffset(self._offset)

    def SetInt(self, val):
        """Set the integer value of the property

        The device tree is marked dirty so that the value will be written to
        the block on the next sync.

        Args:
            val: Integer value (32-bit, single cell)
        """
        self.bytes = struct.pack('>I', val);
        self.value = self.bytes
        self.type = Type.INT
        self.dirty = True

    def SetData(self, bytes):
        """Set the value of a property as bytes

        Args:
            bytes: New property value to set
        """
        self.bytes = bytes
        self.type, self.value = BytesToValue(bytes)
        self.dirty = True

    def Sync(self, auto_resize=False):
        """Sync property changes back to the device tree

        This updates the device tree blob with any changes to this property
        since the last sync.

        Args:
            auto_resize: Resize the device tree automatically if it does not
                have enough space for the update

        Raises:
            FdtException if auto_resize is False and there is not enough space
        """
        if self.dirty:
            node = self._node
            tout.debug(f'sync {node.path}: {self.name}')
            fdt_obj = node._fdt._fdt_obj
            node_name = fdt_obj.get_name(node._offset)
            if node_name and node_name != node.name:
                raise ValueError("Internal error, node '%s' name mismatch '%s'" %
                                 (node.path, node_name))

            if auto_resize:
                while fdt_obj.setprop(node.Offset(), self.name, self.bytes,
                                    (libfdt.NOSPACE,)) == -libfdt.NOSPACE:
                    fdt_obj.resize(fdt_obj.totalsize() + 1024 +
                                   len(self.bytes))
                    fdt_obj.setprop(node.Offset(), self.name, self.bytes)
            else:
                fdt_obj.setprop(node.Offset(), self.name, self.bytes)
            self.dirty = False

    def purge(self):
        """Set a property offset to None

        The property remains in the tree structure and will be recreated when
        the FDT is synced
        """
        self._offset = None
        self.dirty = True

class Node:
    """A device tree node

    Properties:
        parent: Parent Node
        offset: Integer offset in the device tree (None if to be synced)
        name: Device tree node tname
        path: Full path to node, along with the node name itself
        _fdt: Device tree object
        subnodes: A list of subnodes for this node, each a Node object
        props: A dict of properties for this node, each a Prop object.
            Keyed by property name
    """
    def __init__(self, fdt, parent, offset, name, path):
        self._fdt = fdt
        self.parent = parent
        self._offset = offset
        self.name = name
        self.path = path
        self.subnodes = []
        self.props = {}

    def GetFdt(self):
        """Get the Fdt object for this node

        Returns:
            Fdt object
        """
        return self._fdt

    def FindNode(self, name):
        """Find a node given its name

        Args:
            name: Node name to look for
        Returns:
            Node object if found, else None
        """
        for subnode in self.subnodes:
            if subnode.name == name:
                return subnode
        return None

    def Offset(self):
        """Returns the offset of a node, after checking the cache

        This should be used instead of self._offset directly, to ensure that
        the cache does not contain invalid offsets.
        """
        self._fdt.CheckCache()
        return self._offset

    def Scan(self):
        """Scan a node's properties and subnodes

        This fills in the props and subnodes properties, recursively
        searching into subnodes so that the entire tree is built.
        """
        fdt_obj = self._fdt._fdt_obj
        self.props = self._fdt.GetProps(self)
        phandle = fdt_obj.get_phandle(self.Offset())
        if phandle:
            dup = self._fdt.phandle_to_node.get(phandle)
            if dup:
                raise ValueError(
                    f'Duplicate phandle {phandle} in nodes {dup.path} and {self.path}')

            self._fdt.phandle_to_node[phandle] = self

        offset = fdt_obj.first_subnode(self.Offset(), QUIET_NOTFOUND)
        while offset >= 0:
            sep = '' if self.path[-1] == '/' else '/'
            name = fdt_obj.get_name(offset)
            path = self.path + sep + name
            node = Node(self._fdt, self, offset, name, path)
            self.subnodes.append(node)

            node.Scan()
            offset = fdt_obj.next_subnode(offset, QUIET_NOTFOUND)

    def Refresh(self, my_offset):
        """Fix up the _offset for each node, recursively

        Note: This does not take account of property offsets - these will not
        be updated.
        """
        fdt_obj = self._fdt._fdt_obj
        if self._offset != my_offset:
            self._offset = my_offset
        name = fdt_obj.get_name(self._offset)
        if name and self.name != name:
            raise ValueError("Internal error, node '%s' name mismatch '%s'" %
                             (self.path, name))

        offset = fdt_obj.first_subnode(self._offset, QUIET_NOTFOUND)
        for subnode in self.subnodes:
            if subnode._offset is None:
                continue
            if subnode.name != fdt_obj.get_name(offset):
                raise ValueError('Internal error, node name mismatch %s != %s' %
                                 (subnode.name, fdt_obj.get_name(offset)))
            subnode.Refresh(offset)
            offset = fdt_obj.next_subnode(offset, QUIET_NOTFOUND)
        if offset != -libfdt.FDT_ERR_NOTFOUND:
            raise ValueError('Internal error, offset == %d' % offset)

        poffset = fdt_obj.first_property_offset(self._offset, QUIET_NOTFOUND)
        while poffset >= 0:
            p = fdt_obj.get_property_by_offset(poffset)
            prop = self.props.get(p.name)
            if not prop:
                raise ValueError("Internal error, node '%s' property '%s' missing, "
                                 'offset %d' % (self.path, p.name, poffset))
            prop.RefreshOffset(poffset)
            poffset = fdt_obj.next_property_offset(poffset, QUIET_NOTFOUND)

    def DeleteProp(self, prop_name):
        """Delete a property of a node

        The property is deleted and the offset cache is invalidated.

        Args:
            prop_name: Name of the property to delete
        Raises:
            ValueError if the property does not exist
        """
        CheckErr(self._fdt._fdt_obj.delprop(self.Offset(), prop_name),
                 "Node '%s': delete property: '%s'" % (self.path, prop_name))
        del self.props[prop_name]
        self._fdt.Invalidate()

    def AddZeroProp(self, prop_name):
        """Add a new property to the device tree with an integer value of 0.

        Args:
            prop_name: Name of property
        """
        self.props[prop_name] = Prop(self, None, prop_name,
                                     tools.get_bytes(0, 4))

    def AddEmptyProp(self, prop_name, len):
        """Add a property with a fixed data size, for filling in later

        The device tree is marked dirty so that the value will be written to
        the blob on the next sync.

        Args:
            prop_name: Name of property
            len: Length of data in property
        """
        value = tools.get_bytes(0, len)
        self.props[prop_name] = Prop(self, None, prop_name, value)

    def _CheckProp(self, prop_name):
        """Check if a property is present

        Args:
            prop_name: Name of property

        Returns:
            self

        Raises:
            ValueError if the property is missing
        """
        if prop_name not in self.props:
            raise ValueError("Fdt '%s', node '%s': Missing property '%s'" %
                             (self._fdt._fname, self.path, prop_name))
        return self

    def SetInt(self, prop_name, val):
        """Update an integer property int the device tree.

        This is not allowed to change the size of the FDT.

        The device tree is marked dirty so that the value will be written to
        the blob on the next sync.

        Args:
            prop_name: Name of property
            val: Value to set
        """
        self._CheckProp(prop_name).props[prop_name].SetInt(val)

    def SetData(self, prop_name, val):
        """Set the data value of a property

        The device tree is marked dirty so that the value will be written to
        the blob on the next sync.

        Args:
            prop_name: Name of property to set
            val: Data value to set
        """
        self._CheckProp(prop_name).props[prop_name].SetData(val)

    def SetString(self, prop_name, val):
        """Set the string value of a property

        The device tree is marked dirty so that the value will be written to
        the blob on the next sync.

        Args:
            prop_name: Name of property to set
            val: String value to set (will be \0-terminated in DT)
        """
        if type(val) == str:
            val = val.encode('utf-8')
        self._CheckProp(prop_name).props[prop_name].SetData(val + b'\0')

    def AddData(self, prop_name, val):
        """Add a new property to a node

        The device tree is marked dirty so that the value will be written to
        the blob on the next sync.

        Args:
            prop_name: Name of property to add
            val: Bytes value of property

        Returns:
            Prop added
        """
        prop = Prop(self, None, prop_name, val)
        self.props[prop_name] = prop
        return prop

    def AddString(self, prop_name, val):
        """Add a new string property to a node

        The device tree is marked dirty so that the value will be written to
        the blob on the next sync.

        Args:
            prop_name: Name of property to add
            val: String value of property

        Returns:
            Prop added
        """
        val = bytes(val, 'utf-8')
        return self.AddData(prop_name, val + b'\0')

    def AddStringList(self, prop_name, val):
        """Add a new string-list property to a node

        The device tree is marked dirty so that the value will be written to
        the blob on the next sync.

        Args:
            prop_name: Name of property to add
            val (list of str): List of strings to add

        Returns:
            Prop added
        """
        out = b'\0'.join(bytes(s, 'utf-8') for s in val) + b'\0' if val else b''
        return self.AddData(prop_name, out)

    def AddInt(self, prop_name, val):
        """Add a new integer property to a node

        The device tree is marked dirty so that the value will be written to
        the blob on the next sync.

        Args:
            prop_name: Name of property to add
            val: Integer value of property

        Returns:
            Prop added
        """
        return self.AddData(prop_name, struct.pack('>I', val))

    def Subnode(self, name):
        """Create new subnode for the node

        Args:
            name: name of node to add

        Returns:
            New subnode that was created
        """
        path = self.path + '/' + name
        return Node(self._fdt, self, None, name, path)

    def AddSubnode(self, name):
        """Add a new subnode to the node, after all other subnodes

        Args:
            name: name of node to add

        Returns:
            New subnode that was created
        """
        subnode = self.Subnode(name)
        self.subnodes.append(subnode)
        return subnode

    def insert_subnode(self, name):
        """Add a new subnode to the node, before all other subnodes

        This deletes other subnodes and sets their offset to None, so that they
        will be recreated after this one.

        Args:
            name: name of node to add

        Returns:
            New subnode that was created
        """
        # Deleting a node invalidates the offsets of all following nodes, so
        # process in reverse order so that the offset of each node remains valid
        # until deletion.
        for subnode in reversed(self.subnodes):
            subnode.purge(True)
        subnode = self.Subnode(name)
        self.subnodes.insert(0, subnode)
        return subnode

    def purge(self, delete_it=False):
        """Purge this node, setting offset to None and deleting from FDT"""
        if self._offset is not None:
            if delete_it:
                CheckErr(self._fdt._fdt_obj.del_node(self.Offset()),
                     "Node '%s': delete" % self.path)
            self._offset = None
            self._fdt.Invalidate()

        for prop in self.props.values():
            prop.purge()

        for subnode in self.subnodes:
            subnode.purge(False)

    def move_to_first(self):
        """Move the current node to first in its parent's node list"""
        parent = self.parent
        if parent.subnodes and parent.subnodes[0] == self:
            return
        for subnode in reversed(parent.subnodes):
            subnode.purge(True)

        new_subnodes = [self]
        for subnode in parent.subnodes:
            #subnode.purge(False)
            if subnode != self:
                new_subnodes.append(subnode)
        parent.subnodes = new_subnodes

    def Delete(self):
        """Delete a node

        The node is deleted and the offset cache is invalidated.

        Args:
            node (Node): Node to delete

        Raises:
            ValueError if the node does not exist
        """
        CheckErr(self._fdt._fdt_obj.del_node(self.Offset()),
                 "Node '%s': delete" % self.path)
        parent = self.parent
        self._fdt.Invalidate()
        parent.subnodes.remove(self)

    def Sync(self, auto_resize=False):
        """Sync node changes back to the device tree

        This updates the device tree blob with any changes to this node and its
        subnodes since the last sync.

        Args:
            auto_resize: Resize the device tree automatically if it does not
                have enough space for the update

        Returns:
            True if the node had to be added, False if it already existed

        Raises:
            FdtException if auto_resize is False and there is not enough space
        """
        added = False
        if self._offset is None:
            # The subnode doesn't exist yet, so add it
            fdt_obj = self._fdt._fdt_obj
            if auto_resize:
                while True:
                    offset = fdt_obj.add_subnode(self.parent._offset, self.name,
                                                (libfdt.NOSPACE,))
                    if offset != -libfdt.NOSPACE:
                        break
                    fdt_obj.resize(fdt_obj.totalsize() + 1024)
            else:
                offset = fdt_obj.add_subnode(self.parent._offset, self.name)
            self._offset = offset
            added = True

        # Sync the existing subnodes first, so that we can rely on the offsets
        # being correct. As soon as we add new subnodes, it pushes all the
        # existing subnodes up.
        for node in reversed(self.subnodes):
            if node._offset is not None:
                node.Sync(auto_resize)

        # Sync subnodes in reverse so that we get the expected order. Each
        # new node goes at the start of the subnode list. This avoids an O(n^2)
        # rescan of node offsets.
        num_added = 0
        for node in reversed(self.subnodes):
            if node.Sync(auto_resize):
                num_added += 1
        if num_added:
            # Reorder our list of nodes to put the new ones first, since that's
            # what libfdt does
            old_count = len(self.subnodes) - num_added
            subnodes = self.subnodes[old_count:] + self.subnodes[:old_count]
            self.subnodes = subnodes

        # Sync properties now, whose offsets should not have been disturbed,
        # since properties come before subnodes. This is done after all the
        # subnode processing above, since updating properties can disturb the
        # offsets of those subnodes.
        # Properties are synced in reverse order, with new properties added
        # before existing properties are synced. This ensures that the offsets
        # of earlier properties are not disturbed.
        # Note that new properties will have an offset of None here, which
        # Python cannot sort against int. So use a large value instead so that
        # new properties are added first.
        prop_list = sorted(self.props.values(),
                           key=lambda prop: prop._offset or 1 << 31,
                           reverse=True)
        for prop in prop_list:
            prop.Sync(auto_resize)
        return added

    def merge_props(self, src, copy_phandles):
        """Copy missing properties (except 'phandle') from another node

        Args:
            src (Node): Node containing properties to copy
            copy_phandles (bool): True to copy phandle properties in nodes

        Adds properties which are present in src but not in this node. Any
        'phandle' property is not copied since this might result in two nodes
        with the same phandle, thus making phandle references ambiguous.
        """
        tout.debug(f'copy to {self.path}: {src.path}')
        for name, src_prop in src.props.items():
            done = False
            if name not in self.props:
                if copy_phandles or name != 'phandle':
                    self.props[name] = Prop(self, None, name, src_prop.bytes)
                    done = True
            tout.debug(f"  {name}{'' if done else '  - ignored'}")

    def copy_node(self, src, copy_phandles=False):
        """Copy a node and all its subnodes into this node

        Args:
            src (Node): Node to copy
            copy_phandles (bool): True to copy phandle properties in nodes

        Returns:
            Node: Resulting destination node

        This works recursively, with copy_phandles being set to True for the
        recursive calls

        The new node is put before all other nodes. If the node already
        exists, just its subnodes and properties are copied, placing them before
        any existing subnodes. Properties which exist in the destination node
        already are not copied.
        """
        dst = self.FindNode(src.name)
        if dst:
            dst.move_to_first()
        else:
            dst = self.insert_subnode(src.name)
        dst.merge_props(src, copy_phandles)

        # Process in reverse order so that they appear correctly in the result,
        # since copy_node() puts the node first in the list
        for node in reversed(src.subnodes):
            dst.copy_node(node, True)
        return dst

    def copy_subnodes_from_phandles(self, phandle_list):
        """Copy subnodes of a list of nodes into another node

        Args:
            phandle_list (list of int): List of phandles of nodes to copy

        For each node in the phandle list, its subnodes and their properties are
        copied recursively. Note that it does not copy the node itself, nor its
        properties.
        """
        # Process in reverse order, since new nodes are inserted at the start of
        # the destination's node list. We want them to appear in order of the
        # phandle list
        for phandle in phandle_list.__reversed__():
            parent = self.GetFdt().LookupPhandle(phandle)
            tout.debug(f'adding template {parent.path} to node {self.path}')
            for node in parent.subnodes.__reversed__():
                dst = self.copy_node(node)

            tout.debug(f'merge props from {parent.path} to {self.path}')
            self.merge_props(parent, False)


class Fdt:
    """Provides simple access to a flat device tree blob using libfdts.

    Properties:
      fname: Filename of fdt
      _root: Root of device tree (a Node object)
      name: Helpful name for this Fdt for the user (useful when creating the
        DT from data rather than a file)
    """
    def __init__(self, fname):
        self._fname = fname
        self._cached_offsets = False
        self.phandle_to_node = {}
        self.name = ''
        if self._fname:
            self.name = self._fname
            self._fname = fdt_util.EnsureCompiled(self._fname)

            with open(self._fname, 'rb') as fd:
                self._fdt_obj = libfdt.Fdt(fd.read())

    @staticmethod
    def FromData(data, name=''):
        """Create a new Fdt object from the given data

        Args:
            data: Device-tree data blob
            name: Helpful name for this Fdt for the user

        Returns:
            Fdt object containing the data
        """
        fdt = Fdt(None)
        fdt._fdt_obj = libfdt.Fdt(bytes(data))
        fdt.name = name
        return fdt

    def LookupPhandle(self, phandle):
        """Look up a phandle

        Args:
            phandle: Phandle to look up (int)

        Returns:
            Node object the phandle points to
        """
        return self.phandle_to_node.get(phandle)

    def Scan(self, root='/'):
        """Scan a device tree, building up a tree of Node objects

        This fills in the self._root property

        Args:
            root: Ignored

        TODO(sjg@chromium.org): Implement the 'root' parameter
        """
        self.phandle_to_node = {}
        self._cached_offsets = True
        self._root = self.Node(self, None, 0, '/', '/')
        self._root.Scan()

    def GetRoot(self):
        """Get the root Node of the device tree

        Returns:
            The root Node object
        """
        return self._root

    def GetNode(self, path):
        """Look up a node from its path

        Args:
            path: Path to look up, e.g. '/microcode/update@0'
        Returns:
            Node object, or None if not found
        """
        node = self._root
        parts = path.split('/')
        if len(parts) < 2:
            return None
        if len(parts) == 2 and parts[1] == '':
            return node
        for part in parts[1:]:
            node = node.FindNode(part)
            if not node:
                return None
        return node

    def Flush(self):
        """Flush device tree changes back to the file

        If the device tree has changed in memory, write it back to the file.
        """
        with open(self._fname, 'wb') as fd:
            fd.write(self._fdt_obj.as_bytearray())

    def Sync(self, auto_resize=False):
        """Make sure any DT changes are written to the blob

        Args:
            auto_resize: Resize the device tree automatically if it does not
                have enough space for the update

        Raises:
            FdtException if auto_resize is False and there is not enough space
        """
        self.CheckCache()
        self._root.Sync(auto_resize)
        self.Refresh()

    def Pack(self):
        """Pack the device tree down to its minimum size

        When nodes and properties shrink or are deleted, wasted space can
        build up in the device tree binary.
        """
        CheckErr(self._fdt_obj.pack(), 'pack')
        self.Refresh()

    def GetContents(self):
        """Get the contents of the FDT

        Returns:
            The FDT contents as a string of bytes
        """
        return bytes(self._fdt_obj.as_bytearray())

    def GetFdtObj(self):
        """Get the contents of the FDT

        Returns:
            The FDT contents as a libfdt.Fdt object
        """
        return self._fdt_obj

    def GetProps(self, node):
        """Get all properties from a node.

        Args:
            node: Full path to node name to look in.

        Returns:
            A dictionary containing all the properties, indexed by node name.
            The entries are Prop objects.

        Raises:
            ValueError: if the node does not exist.
        """
        props_dict = {}
        poffset = self._fdt_obj.first_property_offset(node._offset,
                                                      QUIET_NOTFOUND)
        while poffset >= 0:
            p = self._fdt_obj.get_property_by_offset(poffset)
            prop = Prop(node, poffset, p.name, p)
            props_dict[prop.name] = prop

            poffset = self._fdt_obj.next_property_offset(poffset,
                                                         QUIET_NOTFOUND)
        return props_dict

    def Invalidate(self):
        """Mark our offset cache as invalid"""
        self._cached_offsets = False

    def CheckCache(self):
        """Refresh the offset cache if needed"""
        if self._cached_offsets:
            return
        self.Refresh()

    def Refresh(self):
        """Refresh the offset cache"""
        self._root.Refresh(0)
        self._cached_offsets = True

    def GetStructOffset(self, offset):
        """Get the file offset of a given struct offset

        Args:
            offset: Offset within the 'struct' region of the device tree
        Returns:
            Position of @offset within the device tree binary
        """
        return self._fdt_obj.off_dt_struct() + offset

    @classmethod
    def Node(self, fdt, parent, offset, name, path):
        """Create a new node

        This is used by Fdt.Scan() to create a new node using the correct
        class.

        Args:
            fdt: Fdt object
            parent: Parent node, or None if this is the root node
            offset: Offset of node
            name: Node name
            path: Full path to node
        """
        node = Node(fdt, parent, offset, name, path)
        return node

    def GetFilename(self):
        """Get the filename of the device tree

        Returns:
            String filename
        """
        return self._fname

def FdtScan(fname):
    """Returns a new Fdt object"""
    dtb = Fdt(fname)
    dtb.Scan()
    return dtb
