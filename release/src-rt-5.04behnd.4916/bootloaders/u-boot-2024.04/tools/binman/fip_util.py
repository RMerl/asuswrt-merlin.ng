#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0+
# Copyright 2021 Google LLC
# Written by Simon Glass <sjg@chromium.org>

"""Support for ARM's Firmware Image Package (FIP) format

FIP is a format similar to FMAP[1] but with fewer features and an obscure UUID
instead of the region name.

It consists of a header and a table of entries, each pointing to a place in the
firmware image where something can be found.

[1] https://chromium.googlesource.com/chromiumos/third_party/flashmap/+/refs/heads/master/lib/fmap.h

If ATF updates, run this program to update the FIT_TYPE_LIST.

ARM Trusted Firmware is available at:

https://github.com/ARM-software/arm-trusted-firmware.git
"""

from argparse import ArgumentParser
import collections
import io
import os
import re
import struct
import sys
from uuid import UUID

OUR_FILE = os.path.realpath(__file__)
OUR_PATH = os.path.dirname(OUR_FILE)

# Bring in the patman and dtoc libraries (but don't override the first path
# in PYTHONPATH)
sys.path.insert(2, os.path.join(OUR_PATH, '..'))

# pylint: disable=C0413
from u_boot_pylib import command
from u_boot_pylib import tools

# The TOC header, at the start of the FIP
HEADER_FORMAT = '<IIQ'
HEADER_LEN = 0x10
HEADER_MAGIC = 0xaA640001
HEADER_SERIAL = 0x12345678

# The entry header (a table of these comes after the TOC header)
UUID_LEN = 16
ENTRY_FORMAT = f'<{UUID_LEN}sQQQ'
ENTRY_SIZE = 0x28

HEADER_NAMES = (
    'name',
    'serial',
    'flags',
)

ENTRY_NAMES = (
    'uuid',
    'offset',
    'size',
    'flags',
)

# Set to True to enable output from running fiptool for debugging
VERBOSE = False

# Use a class so we can convert the bytes, making the table more readable
# pylint: disable=R0903
class FipType:
    """A FIP entry type that we understand"""
    def __init__(self, name, desc, uuid_bytes):
        """Create up a new type

        Args:
            name (str): Short name for the type
            desc (str): Longer description for the type
            uuid_bytes (bytes): List of 16 bytes for the UUID
        """
        self.name = name
        self.desc = desc
        self.uuid = bytes(uuid_bytes)

# This is taken from tbbr_config.c in ARM Trusted Firmware
FIP_TYPE_LIST = [
    # ToC Entry UUIDs
    FipType('scp-fwu-cfg', 'SCP Firmware Updater Configuration FWU SCP_BL2U',
            [0x65, 0x92, 0x27, 0x03, 0x2f, 0x74, 0xe6, 0x44,
             0x8d, 0xff, 0x57, 0x9a, 0xc1, 0xff, 0x06, 0x10]),
    FipType('ap-fwu-cfg', 'AP Firmware Updater Configuration BL2U',
            [0x60, 0xb3, 0xeb, 0x37, 0xc1, 0xe5, 0xea, 0x41,
             0x9d, 0xf3, 0x19, 0xed, 0xa1, 0x1f, 0x68, 0x01]),
    FipType('fwu', 'Firmware Updater NS_BL2U',
            [0x4f, 0x51, 0x1d, 0x11, 0x2b, 0xe5, 0x4e, 0x49,
             0xb4, 0xc5, 0x83, 0xc2, 0xf7, 0x15, 0x84, 0x0a]),
    FipType('fwu-cert', 'Non-Trusted Firmware Updater certificate',
            [0x71, 0x40, 0x8a, 0xb2, 0x18, 0xd6, 0x87, 0x4c,
             0x8b, 0x2e, 0xc6, 0xdc, 0xcd, 0x50, 0xf0, 0x96]),
    FipType('tb-fw', 'Trusted Boot Firmware BL2',
            [0x5f, 0xf9, 0xec, 0x0b, 0x4d, 0x22, 0x3e, 0x4d,
             0xa5, 0x44, 0xc3, 0x9d, 0x81, 0xc7, 0x3f, 0x0a]),
    FipType('scp-fw', 'SCP Firmware SCP_BL2',
            [0x97, 0x66, 0xfd, 0x3d, 0x89, 0xbe, 0xe8, 0x49,
             0xae, 0x5d, 0x78, 0xa1, 0x40, 0x60, 0x82, 0x13]),
    FipType('soc-fw', 'EL3 Runtime Firmware BL31',
            [0x47, 0xd4, 0x08, 0x6d, 0x4c, 0xfe, 0x98, 0x46,
             0x9b, 0x95, 0x29, 0x50, 0xcb, 0xbd, 0x5a, 0x00]),
    FipType('tos-fw', 'Secure Payload BL32 (Trusted OS)',
            [0x05, 0xd0, 0xe1, 0x89, 0x53, 0xdc, 0x13, 0x47,
             0x8d, 0x2b, 0x50, 0x0a, 0x4b, 0x7a, 0x3e, 0x38]),
    FipType('tos-fw-extra1', 'Secure Payload BL32 Extra1 (Trusted OS Extra1)',
            [0x0b, 0x70, 0xc2, 0x9b, 0x2a, 0x5a, 0x78, 0x40,
             0x9f, 0x65, 0x0a, 0x56, 0x82, 0x73, 0x82, 0x88]),
    FipType('tos-fw-extra2', 'Secure Payload BL32 Extra2 (Trusted OS Extra2)',
            [0x8e, 0xa8, 0x7b, 0xb1, 0xcf, 0xa2, 0x3f, 0x4d,
             0x85, 0xfd, 0xe7, 0xbb, 0xa5, 0x02, 0x20, 0xd9]),
    FipType('nt-fw', 'Non-Trusted Firmware BL33',
            [0xd6, 0xd0, 0xee, 0xa7, 0xfc, 0xea, 0xd5, 0x4b,
             0x97, 0x82, 0x99, 0x34, 0xf2, 0x34, 0xb6, 0xe4]),
    FipType('rmm-fw', 'Realm Monitor Management Firmware',
            [0x6c, 0x07, 0x62, 0xa6, 0x12, 0xf2, 0x4b, 0x56,
             0x92, 0xcb, 0xba, 0x8f, 0x63, 0x36, 0x06, 0xd9]),
    # Key certificates
    FipType('rot-cert', 'Root Of Trust key certificate',
            [0x86, 0x2d, 0x1d, 0x72, 0xf8, 0x60, 0xe4, 0x11,
             0x92, 0x0b, 0x8b, 0xe7, 0x62, 0x16, 0x0f, 0x24]),
    FipType('trusted-key-cert', 'Trusted key certificate',
            [0x82, 0x7e, 0xe8, 0x90, 0xf8, 0x60, 0xe4, 0x11,
             0xa1, 0xb4, 0x77, 0x7a, 0x21, 0xb4, 0xf9, 0x4c]),
    FipType('scp-fw-key-cert', 'SCP Firmware key certificate',
            [0x02, 0x42, 0x21, 0xa1, 0xf8, 0x60, 0xe4, 0x11,
             0x8d, 0x9b, 0xf3, 0x3c, 0x0e, 0x15, 0xa0, 0x14]),
    FipType('soc-fw-key-cert', 'SoC Firmware key certificate',
            [0x8a, 0xb8, 0xbe, 0xcc, 0xf9, 0x60, 0xe4, 0x11,
             0x9a, 0xd0, 0xeb, 0x48, 0x22, 0xd8, 0xdc, 0xf8]),
    FipType('tos-fw-key-cert', 'Trusted OS Firmware key certificate',
            [0x94, 0x77, 0xd6, 0x03, 0xfb, 0x60, 0xe4, 0x11,
             0x85, 0xdd, 0xb7, 0x10, 0x5b, 0x8c, 0xee, 0x04]),
    FipType('nt-fw-key-cert', 'Non-Trusted Firmware key certificate',
            [0x8a, 0xd5, 0x83, 0x2a, 0xfb, 0x60, 0xe4, 0x11,
             0x8a, 0xaf, 0xdf, 0x30, 0xbb, 0xc4, 0x98, 0x59]),
    # Content certificates
    FipType('tb-fw-cert', 'Trusted Boot Firmware BL2 certificate',
            [0xd6, 0xe2, 0x69, 0xea, 0x5d, 0x63, 0xe4, 0x11,
             0x8d, 0x8c, 0x9f, 0xba, 0xbe, 0x99, 0x56, 0xa5]),
    FipType('scp-fw-cert', 'SCP Firmware content certificate',
            [0x44, 0xbe, 0x6f, 0x04, 0x5e, 0x63, 0xe4, 0x11,
             0xb2, 0x8b, 0x73, 0xd8, 0xea, 0xae, 0x96, 0x56]),
    FipType('soc-fw-cert', 'SoC Firmware content certificate',
            [0xe2, 0xb2, 0x0c, 0x20, 0x5e, 0x63, 0xe4, 0x11,
             0x9c, 0xe8, 0xab, 0xcc, 0xf9, 0x2b, 0xb6, 0x66]),
    FipType('tos-fw-cert', 'Trusted OS Firmware content certificate',
            [0xa4, 0x9f, 0x44, 0x11, 0x5e, 0x63, 0xe4, 0x11,
             0x87, 0x28, 0x3f, 0x05, 0x72, 0x2a, 0xf3, 0x3d]),
    FipType('nt-fw-cert', 'Non-Trusted Firmware content certificate',
            [0x8e, 0xc4, 0xc1, 0xf3, 0x5d, 0x63, 0xe4, 0x11,
             0xa7, 0xa9, 0x87, 0xee, 0x40, 0xb2, 0x3f, 0xa7]),
    FipType('sip-sp-cert', 'SiP owned Secure Partition content certificate',
            [0x77, 0x6d, 0xfd, 0x44, 0x86, 0x97, 0x4c, 0x3b,
             0x91, 0xeb, 0xc1, 0x3e, 0x02, 0x5a, 0x2a, 0x6f]),
    FipType('plat-sp-cert', 'Platform owned Secure Partition content certificate',
            [0xdd, 0xcb, 0xbf, 0x4a, 0xca, 0xd6, 0x11, 0xea,
             0x87, 0xd0, 0x02, 0x42, 0xac, 0x13, 0x00, 0x03]),
    # Dynamic configs
    FipType('hw-config', 'HW_CONFIG',
            [0x08, 0xb8, 0xf1, 0xd9, 0xc9, 0xcf, 0x93, 0x49,
             0xa9, 0x62, 0x6f, 0xbc, 0x6b, 0x72, 0x65, 0xcc]),
    FipType('tb-fw-config', 'TB_FW_CONFIG',
            [0x6c, 0x04, 0x58, 0xff, 0xaf, 0x6b, 0x7d, 0x4f,
             0x82, 0xed, 0xaa, 0x27, 0xbc, 0x69, 0xbf, 0xd2]),
    FipType('soc-fw-config', 'SOC_FW_CONFIG',
            [0x99, 0x79, 0x81, 0x4b, 0x03, 0x76, 0xfb, 0x46,
             0x8c, 0x8e, 0x8d, 0x26, 0x7f, 0x78, 0x59, 0xe0]),
    FipType('tos-fw-config', 'TOS_FW_CONFIG',
            [0x26, 0x25, 0x7c, 0x1a, 0xdb, 0xc6, 0x7f, 0x47,
             0x8d, 0x96, 0xc4, 0xc4, 0xb0, 0x24, 0x80, 0x21]),
    FipType('nt-fw-config', 'NT_FW_CONFIG',
            [0x28, 0xda, 0x98, 0x15, 0x93, 0xe8, 0x7e, 0x44,
             0xac, 0x66, 0x1a, 0xaf, 0x80, 0x15, 0x50, 0xf9]),
    FipType('fw-config', 'FW_CONFIG',
            [0x58, 0x07, 0xe1, 0x6a, 0x84, 0x59, 0x47, 0xbe,
             0x8e, 0xd5, 0x64, 0x8e, 0x8d, 0xdd, 0xab, 0x0e]),
    ] # end

FIP_TYPES = {ftype.name: ftype for ftype in FIP_TYPE_LIST}


def get_type_uuid(fip_type_or_uuid):
    """get_type_uuid() - Convert a type or uuid into both

    This always returns a UUID, but may not return a type since it does not do
    the reverse lookup.

    Args:
        fip_type_or_uuid (str or bytes): Either a string containing the name of
            an entry (e.g. 'soc-fw') or a bytes(16) containing the UUID

    Returns:
        tuple:
            str: fip type (None if not known)
            bytes(16): uuid

    Raises:
        ValueError: An unknown type was requested
    """
    if isinstance(fip_type_or_uuid, str):
        fip_type = fip_type_or_uuid
        lookup = FIP_TYPES.get(fip_type)
        if not lookup:
            raise ValueError(f"Unknown FIP entry type '{fip_type}'")
        uuid = lookup.uuid
    else:
        fip_type = None
        uuid = fip_type_or_uuid
    return fip_type, uuid


# pylint: disable=R0903
class FipHeader:
    """Class to represent a FIP header"""
    def __init__(self, name, serial, flags):
        """Set up a new header object

        Args:
            name (str): Name, i.e. HEADER_MAGIC
            serial (str): Serial value, i.e. HEADER_SERIAL
            flags (int64): Flags value
        """
        self.name = name
        self.serial = serial
        self.flags = flags


# pylint: disable=R0903
class FipEntry:
    """Class to represent a single FIP entry

    This is used to hold the information about an entry, including its contents.
    Use the get_data() method to obtain the raw output for writing to the FIP
    file.
    """
    def __init__(self, uuid, offset, size, flags):
        self.uuid = uuid
        self.offset = offset
        self.size = size
        self.flags = flags
        self.fip_type = None
        self.data = None
        self.valid = uuid != tools.get_bytes(0, UUID_LEN)
        if self.valid:
            # Look up the friendly name
            matches = {val for (key, val) in FIP_TYPES.items()
                       if val.uuid == uuid}
            if len(matches) == 1:
                self.fip_type = matches.pop().name

    @classmethod
    def from_type(cls, fip_type_or_uuid, data, flags):
        """Create a FipEntry from a type name

        Args:
            cls (class): This class
            fip_type_or_uuid (str or bytes): Name of the type to create, or
                bytes(16) uuid
            data (bytes): Contents of entry
            flags (int64): Flags value

        Returns:
            FipEntry: Created 241
        """
        fip_type, uuid = get_type_uuid(fip_type_or_uuid)
        fent = FipEntry(uuid, None, len(data), flags)
        fent.fip_type = fip_type
        fent.data = data
        return fent


def decode_fip(data):
    """Decode a FIP into a header and list of FIP entries

    Args:
        data (bytes): Data block containing the FMAP

    Returns:
        Tuple:
            header: FipHeader object
            List of FipArea objects
    """
    fields = list(struct.unpack(HEADER_FORMAT, data[:HEADER_LEN]))
    header = FipHeader(*fields)
    fents = []
    pos = HEADER_LEN
    while True:
        fields = list(struct.unpack(ENTRY_FORMAT, data[pos:pos + ENTRY_SIZE]))
        fent = FipEntry(*fields)
        if not fent.valid:
            break
        fent.data = data[fent.offset:fent.offset + fent.size]
        fents.append(fent)
        pos += ENTRY_SIZE
    return header, fents


class FipWriter:
    """Class to handle writing a ARM Trusted Firmware's Firmware Image Package

    Usage is something like:

        fip = FipWriter(size)
        fip.add_entry('scp-fwu-cfg', tools.read_file('something.bin'))
        ...
        data = cbw.get_data()

    Attributes:
    """
    def __init__(self, flags, align):
        self._fip_entries = []
        self._flags = flags
        self._align = align

    def add_entry(self, fip_type, data, flags):
        """Add a new entry to the FIP

        Args:
            fip_type (str): Type to add, e.g. 'tos-fw-config'
            data (bytes): Contents of entry
            flags (int64): Entry flags

        Returns:
            FipEntry: entry that was added
        """
        fent = FipEntry.from_type(fip_type, data, flags)
        self._fip_entries.append(fent)
        return fent

    def get_data(self):
        """Obtain the full contents of the FIP

        Thhis builds the FIP with headers and all required FIP entries.

        Returns:
            bytes: data resulting from building the FIP
        """
        buf = io.BytesIO()
        hdr = struct.pack(HEADER_FORMAT, HEADER_MAGIC, HEADER_SERIAL,
                          self._flags)
        buf.write(hdr)

        # Calculate the position fo the first entry
        offset = len(hdr)
        offset += len(self._fip_entries) * ENTRY_SIZE
        offset += ENTRY_SIZE   # terminating entry

        for fent in self._fip_entries:
            offset = tools.align(offset, self._align)
            fent.offset = offset
            offset += fent.size

        # Write out the TOC
        for fent in self._fip_entries:
            hdr = struct.pack(ENTRY_FORMAT, fent.uuid, fent.offset, fent.size,
                              fent.flags)
            buf.write(hdr)

        # Write out the entries
        for fent in self._fip_entries:
            buf.seek(fent.offset)
            buf.write(fent.data)

        return buf.getvalue()


class FipReader():
    """Class to handle reading a Firmware Image Package (FIP)

    Usage is something like:
        fip = fip_util.FipReader(data)
        fent = fip.get_entry('fwu')
        self.WriteFile('ufwu.bin', fent.data)
        blob = fip.get_entry(
            bytes([0xe3, 0xb7, 0x8d, 0x9e, 0x4a, 0x64, 0x11, 0xec,
                   0xb4, 0x5c, 0xfb, 0xa2, 0xb9, 0xb4, 0x97, 0x88]))
        self.WriteFile('blob.bin', blob.data)
    """
    def __init__(self, data, read=True):
        """Set up a new FitReader

        Args:
            data (bytes): data to read
            read (bool): True to read the data now
        """
        self.fents = collections.OrderedDict()
        self.data = data
        if read:
            self.read()

    def read(self):
        """Read all the files in the FIP and add them to self.files"""
        self.header, self.fents = decode_fip(self.data)

    def get_entry(self, fip_type_or_uuid):
        """get_entry() - Find an entry by type or UUID

        Args:
            fip_type_or_uuid (str or bytes): Name of the type to create, or
                    bytes(16) uuid

        Returns:
            FipEntry: if found

        Raises:
            ValueError: entry type not found
        """
        fip_type, uuid = get_type_uuid(fip_type_or_uuid)
        for fent in self.fents:
            if fent.uuid == uuid:
                return fent
        label = fip_type
        if not label:
            label = UUID(bytes=uuid)
        raise ValueError(f"Cannot find FIP entry '{label}'")


def parse_macros(srcdir):
    """parse_macros: Parse the firmware_image_package.h file

    Args:
        srcdir (str): 'arm-trusted-firmware' source directory

    Returns:
        dict:
            key: UUID macro name, e.g. 'UUID_TRUSTED_FWU_CERT'
            value: list:
                file comment, e.g. 'ToC Entry UUIDs'
                macro name, e.g. 'UUID_TRUSTED_FWU_CERT'
                uuid as bytes(16)

    Raises:
        ValueError: a line cannot be parsed
    """
    re_uuid = re.compile('0x[0-9a-fA-F]{2}')
    re_comment = re.compile(r'^/\* (.*) \*/$')
    fname = os.path.join(srcdir, 'include/tools_share/firmware_image_package.h')
    data = tools.read_file(fname, binary=False)
    macros = collections.OrderedDict()
    comment = None
    for linenum, line in enumerate(data.splitlines()):
        if line.startswith('/*'):
            mat = re_comment.match(line)
            if mat:
                comment = mat.group(1)
        else:
            # Example: #define UUID_TOS_FW_CONFIG \
            if 'UUID' in line:
                macro = line.split()[1]
            elif '{{' in line:
                mat = re_uuid.findall(line)
                if not mat or len(mat) != 16:
                    raise ValueError(
                        f'{fname}: Cannot parse UUID line {linenum + 1}: Got matches: {mat}')

                uuid = bytes([int(val, 16) for val in mat])
                macros[macro] = comment, macro, uuid
    if not macros:
        raise ValueError(f'{fname}: Cannot parse file')
    return macros


def parse_names(srcdir):
    """parse_names: Parse the tbbr_config.c file

    Args:
        srcdir (str): 'arm-trusted-firmware' source directory

    Returns:
        tuple: dict of entries:
            key: UUID macro, e.g. 'UUID_NON_TRUSTED_FIRMWARE_BL33'
            tuple: entry information
                Description of entry, e.g. 'Non-Trusted Firmware BL33'
                UUID macro, e.g. 'UUID_NON_TRUSTED_FIRMWARE_BL33'
                Name of entry, e.g. 'nt-fw'

    Raises:
        ValueError: the file cannot be parsed
    """
    # Extract the .name, .uuid and .cmdline_name values
    re_data = re.compile(r'\.name = "([^"]*)",\s*\.uuid = (UUID_\w*),\s*\.cmdline_name = "([^"]+)"',
                         re.S)
    fname = os.path.join(srcdir, 'tools/fiptool/tbbr_config.c')
    data = tools.read_file(fname, binary=False)

    # Example entry:
    #   {
    #       .name = "Secure Payload BL32 Extra2 (Trusted OS Extra2)",
    #       .uuid = UUID_SECURE_PAYLOAD_BL32_EXTRA2,
    #       .cmdline_name = "tos-fw-extra2"
    #   },
    mat = re_data.findall(data)
    if not mat:
        raise ValueError(f'{fname}: Cannot parse file')
    names = {uuid: (desc, uuid, name) for desc, uuid, name in mat}
    return names


def create_code_output(macros, names):
    """create_code_output() - Create the new version of this Python file

    Args:
        macros (dict):
            key (str): UUID macro name, e.g. 'UUID_TRUSTED_FWU_CERT'
            value: list:
                file comment, e.g. 'ToC Entry UUIDs'
                macro name, e.g. 'UUID_TRUSTED_FWU_CERT'
                uuid as bytes(16)

        names (dict): list of entries, each
            tuple: entry information
                Description of entry, e.g. 'Non-Trusted Firmware BL33'
                UUID macro, e.g. 'UUID_NON_TRUSTED_FIRMWARE_BL33'
                Name of entry, e.g. 'nt-fw'

    Returns:
        str: Table of FipType() entries
    """
    def _to_hex_list(data):
        """Convert bytes into C code

        Args:
            bytes to convert

        Returns:
            str: in the format '0x12, 0x34, 0x56...'
        """
        # Use 0x instead of %# since the latter ignores the 0 modifier in
        # Python 3.8.10
        return ', '.join(['0x%02x' % byte for byte in data])

    out = ''
    last_comment = None
    for comment, macro, uuid in macros.values():
        name_entry = names.get(macro)
        if not name_entry:
            print(f"Warning: UUID '{macro}' is not mentioned in tbbr_config.c file")
            continue
        desc, _, name = name_entry
        if last_comment != comment:
            out += f'    # {comment}\n'
            last_comment = comment
        out += """    FipType('%s', '%s',
            [%s,
             %s]),
""" % (name, desc, _to_hex_list(uuid[:8]), _to_hex_list(uuid[8:]))
    return out


def parse_atf_source(srcdir, dstfile, oldfile):
    """parse_atf_source(): Parse the ATF source tree and update this file

    Args:
        srcdir (str): Path to 'arm-trusted-firmware' directory. Get this from:
            https://github.com/ARM-software/arm-trusted-firmware.git
        dstfile (str): File to write new code to, if an update is needed
        oldfile (str): Python source file to compare against

    Raises:
        ValueError: srcdir readme.rst is missing or the first line does not
            match what is expected
    """
    # We expect a readme file
    readme_fname = os.path.join(srcdir, 'readme.rst')
    if not os.path.exists(readme_fname):
        raise ValueError(
            f"Expected file '{readme_fname}' - try using -s to specify the "
            'arm-trusted-firmware directory')
    readme = tools.read_file(readme_fname, binary=False)
    first_line = 'Trusted Firmware-A'
    if readme.splitlines()[0] != first_line:
        raise ValueError(f"'{readme_fname}' does not start with '{first_line}'")
    macros = parse_macros(srcdir)
    names = parse_names(srcdir)
    output = create_code_output(macros, names)
    orig = tools.read_file(oldfile, binary=False)
    re_fip_list = re.compile(r'(.*FIP_TYPE_LIST = \[).*?(    ] # end.*)', re.S)
    mat = re_fip_list.match(orig)
    new_code = mat.group(1) + '\n' + output + mat.group(2) if mat else output
    if new_code == orig:
        print(f"Existing code in '{oldfile}' is up-to-date")
    else:
        tools.write_file(dstfile, new_code, binary=False)
        print(f'Needs update, try:\n\tmeld {dstfile} {oldfile}')


def main(argv, oldfile):
    """Main program for this tool

    Args:
        argv (list): List of str command-line arguments
        oldfile (str): Python source file to compare against

    Returns:
        int: 0 (exit code)
    """
    parser = ArgumentParser(epilog='''Creates an updated version of this code,
with a table of FIP-entry types parsed from the arm-trusted-firmware source
directory''')
    parser.add_argument(
        '-D', '--debug', action='store_true',
        help='Enabling debugging (provides a full traceback on error)')
    parser.add_argument(
        '-o', '--outfile', type=str, default='fip_util.py.out',
        help='Output file to write new fip_util.py file to')
    parser.add_argument(
        '-s', '--src', type=str, default='.',
        help='Directory containing the arm-trusted-firmware source')
    args = parser.parse_args(argv)

    if not args.debug:
        sys.tracebacklimit = 0

    parse_atf_source(args.src, args.outfile, oldfile)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:], OUR_FILE))  # pragma: no cover
