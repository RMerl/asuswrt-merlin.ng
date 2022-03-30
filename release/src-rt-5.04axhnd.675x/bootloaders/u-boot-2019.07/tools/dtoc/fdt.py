#!/usr/bin/python
# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (C) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#

import struct
import sys

import fdt_util
import libfdt
from libfdt import QUIET_NOTFOUND

# This deals with a device tree, presenting it as an assortment of Node and
# Prop objects, representing nodes and properties, respectively. This file
# contains the base classes and defines the high-level API. You can use
# FdtScan() as a convenience function to create and scan an Fdt.

# This implementation uses a libfdt Python library to access the device tree,
# so it is fairly efficient.

# A list of types we support
(TYPE_BYTE, TYPE_INT, TYPE_STRING, TYPE_BOOL, TYPE_INT64) = range(5)

def CheckErr(errnum, msg):
    if errnum:
        raise ValueError('Error %d: %s: %s' %
            (errnum, libfdt.fdt_strerror(errnum), msg))

class Prop:
    """A device tree property

    Properties:
        name: Property name (as per the device tree)
        value: Property value as a string of bytes, or a list of strings of
            bytes
        type: Value type
    """
    def __init__(self, node, offset, name, bytes):
        self._node = node
        self._offset = offset
        self.name = name
        self.value = None
        self.bytes = str(bytes)
        self.dirty = False
        if not bytes:
            self.type = TYPE_BOOL
            self.value = True
            return
        self.type, self.value = self.BytesToValue(bytes)

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
        if newprop.type < self.type:
            self.type = newprop.type

        if type(newprop.value) == list and type(self.value) != list:
            self.value = [self.value]

        if type(self.value) == list and len(newprop.value) > len(self.value):
            val = self.GetEmpty(self.type)
            while len(self.value) < len(newprop.value):
                self.value.append(val)

    def BytesToValue(self, bytes):
        """Converts a string of bytes into a type and value

        Args:
            A string containing bytes

        Return:
            A tuple:
                Type of data
                Data, either a single element or a list of elements. Each element
                is one of:
                    TYPE_STRING: string value from the property
                    TYPE_INT: a byte-swapped integer stored as a 4-byte string
                    TYPE_BYTE: a byte stored as a single-byte string
        """
        bytes = str(bytes)
        size = len(bytes)
        strings = bytes.split('\0')
        is_string = True
        count = len(strings) - 1
        if count > 0 and not strings[-1]:
            for string in strings[:-1]:
                if not string:
                    is_string = False
                    break
                for ch in string:
                    if ch < ' ' or ch > '~':
                        is_string = False
                        break
        else:
            is_string = False
        if is_string:
            if count == 1:
                return TYPE_STRING, strings[0]
            else:
                return TYPE_STRING, strings[:-1]
        if size % 4:
            if size == 1:
                return TYPE_BYTE, bytes[0]
            else:
                return TYPE_BYTE, list(bytes)
        val = []
        for i in range(0, size, 4):
            val.append(bytes[i:i + 4])
        if size == 4:
            return TYPE_INT, val[0]
        else:
            return TYPE_INT, val

    @classmethod
    def GetEmpty(self, type):
        """Get an empty / zero value of the given type

        Returns:
            A single value of the given type
        """
        if type == TYPE_BYTE:
            return chr(0)
        elif type == TYPE_INT:
            return struct.pack('>I', 0);
        elif type == TYPE_STRING:
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
        self.type = TYPE_INT
        self.dirty = True

    def SetData(self, bytes):
        """Set the value of a property as bytes

        Args:
            bytes: New property value to set
        """
        self.bytes = str(bytes)
        self.type, self.value = self.BytesToValue(bytes)
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
        if self._offset is None or self.dirty:
            node = self._node
            fdt_obj = node._fdt._fdt_obj
            if auto_resize:
                while fdt_obj.setprop(node.Offset(), self.name, self.bytes,
                                    (libfdt.NOSPACE,)) == -libfdt.NOSPACE:
                    fdt_obj.resize(fdt_obj.totalsize() + 1024)
                    fdt_obj.setprop(node.Offset(), self.name, self.bytes)
            else:
                fdt_obj.setprop(node.Offset(), self.name, self.bytes)


class Node:
    """A device tree node

    Properties:
        offset: Integer offset in the device tree
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
        offset = fdt_obj.first_subnode(self._offset, QUIET_NOTFOUND)
        for subnode in self.subnodes:
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
                raise ValueError("Internal error, property '%s' missing, "
                                 'offset %d' % (p.name, poffset))
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
        self.props[prop_name] = Prop(self, None, prop_name, '\0' * 4)

    def AddEmptyProp(self, prop_name, len):
        """Add a property with a fixed data size, for filling in later

        The device tree is marked dirty so that the value will be written to
        the blob on the next sync.

        Args:
            prop_name: Name of property
            len: Length of data in property
        """
        value = chr(0) * len
        self.props[prop_name] = Prop(self, None, prop_name, value)

    def SetInt(self, prop_name, val):
        """Update an integer property int the device tree.

        This is not allowed to change the size of the FDT.

        The device tree is marked dirty so that the value will be written to
        the blob on the next sync.

        Args:
            prop_name: Name of property
            val: Value to set
        """
        self.props[prop_name].SetInt(val)

    def SetData(self, prop_name, val):
        """Set the data value of a property

        The device tree is marked dirty so that the value will be written to
        the blob on the next sync.

        Args:
            prop_name: Name of property to set
            val: Data value to set
        """
        self.props[prop_name].SetData(val)

    def SetString(self, prop_name, val):
        """Set the string value of a property

        The device tree is marked dirty so that the value will be written to
        the blob on the next sync.

        Args:
            prop_name: Name of property to set
            val: String value to set (will be \0-terminated in DT)
        """
        self.props[prop_name].SetData(val + chr(0))

    def AddString(self, prop_name, val):
        """Add a new string property to a node

        The device tree is marked dirty so that the value will be written to
        the blob on the next sync.

        Args:
            prop_name: Name of property to add
            val: String value of property
        """
        self.props[prop_name] = Prop(self, None, prop_name, val + chr(0))

    def AddSubnode(self, name):
        """Add a new subnode to the node

        Args:
            name: name of node to add

        Returns:
            New subnode that was created
        """
        path = self.path + '/' + name
        subnode = Node(self._fdt, self, None, name, path)
        self.subnodes.append(subnode)
        return subnode

    def Sync(self, auto_resize=False):
        """Sync node changes back to the device tree

        This updates the device tree blob with any changes to this node and its
        subnodes since the last sync.

        Args:
            auto_resize: Resize the device tree automatically if it does not
                have enough space for the update

        Raises:
            FdtException if auto_resize is False and there is not enough space
        """
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

        # Sync subnodes in reverse so that we don't disturb node offsets for
        # nodes that are earlier in the DT. This avoids an O(n^2) rescan of
        # node offsets.
        for node in reversed(self.subnodes):
            node.Sync(auto_resize)

        # Sync properties now, whose offsets should not have been disturbed.
        # We do this after subnodes, since this disturbs the offsets of these
        # properties.
        prop_list = sorted(self.props.values(), key=lambda prop: prop._offset,
                           reverse=True)
        for prop in prop_list:
            prop.Sync(auto_resize)


class Fdt:
    """Provides simple access to a flat device tree blob using libfdts.

    Properties:
      fname: Filename of fdt
      _root: Root of device tree (a Node object)
    """
    def __init__(self, fname):
        self._fname = fname
        self._cached_offsets = False
        self.phandle_to_node = {}
        if self._fname:
            self._fname = fdt_util.EnsureCompiled(self._fname)

            with open(self._fname) as fd:
                self._fdt_obj = libfdt.Fdt(fd.read())

    @staticmethod
    def FromData(data):
        """Create a new Fdt object from the given data

        Args:
            data: Device-tree data blob

        Returns:
            Fdt object containing the data
        """
        fdt = Fdt(None)
        fdt._fdt_obj = libfdt.Fdt(bytearray(data))
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
        self._root.Sync(auto_resize)
        self.Invalidate()

    def Pack(self):
        """Pack the device tree down to its minimum size

        When nodes and properties shrink or are deleted, wasted space can
        build up in the device tree binary.
        """
        CheckErr(self._fdt_obj.pack(), 'pack')
        self.Invalidate()

    def GetContents(self):
        """Get the contents of the FDT

        Returns:
            The FDT contents as a string of bytes
        """
        return self._fdt_obj.as_bytearray()

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
        self._cached_offsets = True

    def Refresh(self):
        """Refresh the offset cache"""
        self._root.Refresh(0)

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

def FdtScan(fname):
    """Returns a new Fdt object"""
    dtb = Fdt(fname)
    dtb.Scan()
    return dtb
