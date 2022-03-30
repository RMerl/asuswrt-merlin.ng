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

import command
import tools

VERSION3 = sys.version_info > (3, 0)

def get_plain_bytes(val):
    """Handle Python 3 strings"""
    if isinstance(val, bytes):
        val = val.decode('utf-8')
    return val.encode('raw_unicode_escape')

def fdt32_to_cpu(val):
    """Convert a device tree cell to an integer

    Args:
        Value to convert (4-character string representing the cell value)

    Return:
        A native-endian integer value
    """
    if VERSION3:
        # This code is not reached in Python 2
        val = get_plain_bytes(val)  # pragma: no cover
    return struct.unpack('>I', val)[0]

def fdt_cells_to_cpu(val, cells):
    """Convert one or two cells to a long integer

    Args:
        Value to convert (array of one or more 4-character strings)

    Return:
        A native-endian long value
    """
    if not cells:
        return 0
    out = long(fdt32_to_cpu(val[0]))
    if cells == 2:
        out = out << 32 | fdt32_to_cpu(val[1])
    return out

def EnsureCompiled(fname, capture_stderr=False):
    """Compile an fdt .dts source file into a .dtb binary blob if needed.

    Args:
        fname: Filename (if .dts it will be compiled). It not it will be
            left alone

    Returns:
        Filename of resulting .dtb file
    """
    _, ext = os.path.splitext(fname)
    if ext != '.dts':
        return fname

    dts_input = tools.GetOutputFilename('source.dts')
    dtb_output = tools.GetOutputFilename('source.dtb')

    search_paths = [os.path.join(os.getcwd(), 'include')]
    root, _ = os.path.splitext(fname)
    args = ['-E', '-P', '-x', 'assembler-with-cpp', '-D__ASSEMBLY__']
    args += ['-Ulinux']
    for path in search_paths:
        args.extend(['-I', path])
    args += ['-o', dts_input, fname]
    command.Run('cc', *args)

    # If we don't have a directory, put it in the tools tempdir
    search_list = []
    for path in search_paths:
        search_list.extend(['-i', path])
    args = ['-I', 'dts', '-o', dtb_output, '-O', 'dtb',
            '-W', 'no-unit_address_vs_reg']
    args.extend(search_list)
    args.append(dts_input)
    dtc = os.environ.get('DTC') or 'dtc'
    command.Run(dtc, *args, capture_stderr=capture_stderr)
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
    if isinstance(value, list):
        raise ValueError("Node '%s' property '%s' has list value: expecting "
                         "a single string" % (node.name, propname))
    return value

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
