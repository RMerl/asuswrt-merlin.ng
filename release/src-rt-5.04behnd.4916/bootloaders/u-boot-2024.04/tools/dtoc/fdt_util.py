#!/usr/bin/python
# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (C) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#

# Utility functions for reading from a device tree. Once the upstream pylibfdt
# implementation advances far enough, we should be able to drop these.

import os
import struct
import sys
import tempfile

from u_boot_pylib import command
from u_boot_pylib import tools

def fdt32_to_cpu(val):
    """Convert a device tree cell to an integer

    Args:
        Value to convert (4-character string representing the cell value)

    Return:
        A native-endian integer value
    """
    return struct.unpack('>I', val)[0]

def fdt64_to_cpu(val):
    """Convert a device tree cell to an integer

    Args:
        val (list): Value to convert (list of 2 4-character strings representing
            the cell value)

    Return:
        int: A native-endian integer value
    """
    return fdt32_to_cpu(val[0]) << 32 | fdt32_to_cpu(val[1])

def fdt_cells_to_cpu(val, cells):
    """Convert one or two cells to a long integer

    Args:
        Value to convert (array of one or more 4-character strings)

    Return:
        A native-endian integer value
    """
    if not cells:
        return 0
    out = int(fdt32_to_cpu(val[0]))
    if cells == 2:
        out = out << 32 | fdt32_to_cpu(val[1])
    return out

def EnsureCompiled(fname, tmpdir=None, capture_stderr=False):
    """Compile an fdt .dts source file into a .dtb binary blob if needed.

    Args:
        fname: Filename (if .dts it will be compiled). It not it will be
            left alone
        tmpdir: Temporary directory for output files, or None to use the
            tools-module output directory

    Returns:
        Filename of resulting .dtb file
    """
    _, ext = os.path.splitext(fname)
    if ext != '.dts':
        return fname

    if tmpdir:
        dts_input = os.path.join(tmpdir, 'source.dts')
        dtb_output = os.path.join(tmpdir, 'source.dtb')
    else:
        dts_input = tools.get_output_filename('source.dts')
        dtb_output = tools.get_output_filename('source.dtb')

    search_paths = [os.path.join(os.getcwd(), 'include')]
    root, _ = os.path.splitext(fname)
    cc, args = tools.get_target_compile_tool('cc')
    args += ['-E', '-P', '-x', 'assembler-with-cpp', '-D__ASSEMBLY__']
    args += ['-Ulinux']
    for path in search_paths:
        args.extend(['-I', path])
    args += ['-o', dts_input, fname]
    command.run(cc, *args)

    # If we don't have a directory, put it in the tools tempdir
    search_list = []
    for path in search_paths:
        search_list.extend(['-i', path])
    dtc, args = tools.get_target_compile_tool('dtc')
    args += ['-I', 'dts', '-o', dtb_output, '-O', 'dtb',
            '-W', 'no-unit_address_vs_reg']
    args.extend(search_list)
    args.append(dts_input)
    command.run(dtc, *args, capture_stderr=capture_stderr)
    return dtb_output

def GetInt(node, propname, default=None):
    """Get an integer from a property

    Args:
        node: Node object to read from
        propname: property name to read
        default: Default value to use if the node/property do not exist

    Returns:
        Integer value read, or default if none
    """
    prop = node.props.get(propname)
    if not prop:
        return default
    if isinstance(prop.value, list):
        raise ValueError("Node '%s' property '%s' has list value: expecting "
                         "a single integer" % (node.name, propname))
    value = fdt32_to_cpu(prop.value)
    return value

def GetInt64(node, propname, default=None):
    """Get a 64-bit integer from a property

    Args:
        node (Node): Node object to read from
        propname (str): property name to read
        default (int): Default value to use if the node/property do not exist

    Returns:
        int: value read, or default if none

    Raises:
        ValueError: Property is not of the correct size
    """
    prop = node.props.get(propname)
    if not prop:
        return default
    if not isinstance(prop.value, list) or len(prop.value) != 2:
        raise ValueError("Node '%s' property '%s' should be a list with 2 items for 64-bit values" %
                         (node.name, propname))
    value = fdt64_to_cpu(prop.value)
    return value

def GetString(node, propname, default=None):
    """Get a string from a property

    Args:
        node: Node object to read from
        propname: property name to read
        default: Default value to use if the node/property do not exist

    Returns:
        String value read, or default if none
    """
    prop = node.props.get(propname)
    if not prop:
        return default
    value = prop.value
    if not prop.bytes:
        return ''
    if isinstance(value, list):
        raise ValueError("Node '%s' property '%s' has list value: expecting "
                         "a single string" % (node.name, propname))
    return value

def GetStringList(node, propname, default=None):
    """Get a string list from a property

    Args:
        node (Node): Node object to read from
        propname (str): property name to read
        default (list of str): Default value to use if the node/property do not
            exist, or None

    Returns:
        String value read, or default if none
    """
    prop = node.props.get(propname)
    if not prop:
        return default
    value = prop.value
    if not prop.bytes:
        return []
    if not isinstance(value, list):
        strval = GetString(node, propname)
        return [strval]
    return value

def GetArgs(node, propname):
    prop = node.props.get(propname)
    if not prop:
        raise ValueError(f"Node '{node.path}': Expected property '{propname}'")
    if prop.bytes:
        value = GetStringList(node, propname)
    else:
        value = []
    if not value:
        args = []
    elif len(value) == 1:
        args = value[0].split()
    else:
        args = value
    return args

def GetBool(node, propname, default=False):
    """Get an boolean from a property

    Args:
        node: Node object to read from
        propname: property name to read
        default: Default value to use if the node/property do not exist

    Returns:
        Boolean value read, or default if none (if you set this to True the
            function will always return True)
    """
    if propname in node.props:
        return True
    return default

def GetByte(node, propname, default=None):
    """Get an byte from a property

    Args:
        node: Node object to read from
        propname: property name to read
        default: Default value to use if the node/property do not exist

    Returns:
        Byte value read, or default if none
    """
    prop = node.props.get(propname)
    if not prop:
        return default
    value = prop.value
    if isinstance(value, list):
        raise ValueError("Node '%s' property '%s' has list value: expecting "
                         "a single byte" % (node.name, propname))
    if len(value) != 1:
        raise ValueError("Node '%s' property '%s' has length %d, expecting %d" %
                         (node.name, propname, len(value), 1))
    return ord(value[0])

def GetBytes(node, propname, size, default=None):
    """Get a set of bytes from a property

    Args:
        node (Node): Node object to read from
        propname (str): property name to read
        size (int): Number of bytes to expect
        default (bytes): Default value or None

    Returns:
        bytes: Bytes value read, or default if none
    """
    prop = node.props.get(propname)
    if not prop:
        return default
    if len(prop.bytes) != size:
        raise ValueError("Node '%s' property '%s' has length %d, expecting %d" %
                         (node.name, propname, len(prop.bytes), size))
    return prop.bytes

def GetPhandleList(node, propname):
    """Get a list of phandles from a property

    Args:
        node: Node object to read from
        propname: property name to read

    Returns:
        List of phandles read, each an integer
    """
    prop = node.props.get(propname)
    if not prop:
        return None
    value = prop.value
    if not isinstance(value, list):
        value = [value]
    return [fdt32_to_cpu(v) for v in value]

def GetPhandleNameOffset(node, propname):
    """Get a <&phandle>, "string", <offset> value from a property

    Args:
        node: Node object to read from
        propname: property name to read

    Returns:
        tuple:
            Node object
            str
            int
        or None if the property does not exist
    """
    prop = node.props.get(propname)
    if not prop:
        return None
    value = prop.bytes
    phandle = fdt32_to_cpu(value[:4])
    node = node.GetFdt().LookupPhandle(phandle)
    name = ''
    for byte in value[4:]:
        if not byte:
            break
        name += chr(byte)
    val = fdt32_to_cpu(value[4 + len(name) + 1:])
    return node, name, val

def GetDatatype(node, propname, datatype):
    """Get a value of a given type from a property

    Args:
        node: Node object to read from
        propname: property name to read
        datatype: Type to read (str or int)

    Returns:
        value read, or None if none

    Raises:
        ValueError if datatype is not str or int
    """
    if datatype == str:
        return GetString(node, propname)
    elif datatype == int:
        return GetInt(node, propname)
    raise ValueError("fdt_util internal error: Unknown data type '%s'" %
                     datatype)
