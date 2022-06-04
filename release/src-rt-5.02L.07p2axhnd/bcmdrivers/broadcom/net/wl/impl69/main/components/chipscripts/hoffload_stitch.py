#!/usr/bin/env python

"""Pack host offload modules into a single file."""

# $ Copyright Broadcom Corporation $
__id__ = '$Id: hoffload_stitch.py 666337 2016-10-20 20:37:28Z anchakra $'
__url__ = '$URL: svn://bcawlan-svn.lvn.broadcom.net/svn/bcawlan/components/chips/scripts/branches/CHIPSSCRIPTS_BRANCH_17_100/hoffload_stitch.py $'
__version__ = '1.0'

import ctypes
import hashlib
import logging
import os
import sys
import zlib

import hoffload_common as common

CRC32_MASK = 0xffffffff
MAX_NAME = 31
BLOCK_SIZE = 4096


class PackModules(common.HoffloadApp):
    """Pack modules into a single file."""

    def __call__(self):
        """Generate module headers, pack modules into single file."""

        def compute_signature(fin):
            """Compute sha256 signature of module."""

            crypto = hashlib.sha256()
            chunk = fin.read(BLOCK_SIZE)
            while chunk:
                crypto.update(chunk)
                chunk = fin.read(BLOCK_SIZE)
            return crypto.digest()

        # Gather module info from config and hdr files.
        cfg_parser = common.HoffloadConfigParser(self.args.config,
                                                 enum=self.args.enum_file)
        module_cnt = len(self.args.module)
        with open(self.args.result, 'wb') as fp:
            for counter, module in enumerate(self.args.module):
                module_name = os.path.splitext(
                    os.path.basename(module).strip())[0]
                logging.info('Packing module %s', module_name)
                header = Header()
                header.loc_idx = cfg_parser.get_module_index(module_name)
                header.id = cfg_parser.get_module_id(module_name)
                module_size = os.stat(module).st_size
                header.size = module_size
                pad = BLOCK_SIZE - ((module_size + ctypes.sizeof(header)) %
                                    BLOCK_SIZE)
                if counter < module_cnt - 1:
                    # Set the "next" bit (bit 16 of offset fields).
                    header.pad = (pad | 0x8000)
                else:
                    header.pad = pad
                # Truncate to size allowed by header.
                header.name = module_name[:MAX_NAME]
                # Compute checksum
                with open(module, 'rb') as fin:
                    header.digest = (zlib.crc32(fin.read(), 0) &
                                     CRC32_MASK) ^ CRC32_MASK
                    # Compute digital signature of module and write it
                    fin.seek(0)
                    sign = compute_signature(fin)
                    header.pad -= len(sign)
                    # Write module header and digital signature.
                    fp.write(header)
                    fp.write(sign)
                    pad -= len(sign)
                    # write module image
                    fin.seek(0)
                    fp.write(fin.read())
                # Fill to block size.
                if pad:
                    fp.write('\0' * pad)


class Header(ctypes.LittleEndianStructure):
    """Represents a C struct containing header fields."""

    _fields_ = (
        ('ver', ctypes.c_uint8),
        ('loc_idx', ctypes.c_uint8),
        ('pad', ctypes.c_uint16),
        ('size', ctypes.c_uint32),
        ('id', ctypes.c_uint16),
        ('rsvd', ctypes.c_uint16),
        ('name', ctypes.c_char * 32),
        ('digest', ctypes.c_uint32),
    )

    # pylint: disable=super-init-not-called
    def __init__(self):
        self.ver = self.id = self.reg = 0
        self.rsvd = self.size = self.rgn = 0
        self.name = ''
        self.digest = self.offset = self.pad = 0


class CommandLine(common.CommandLine):
    """Command-line methods."""

    def __init__(self):
        super(CommandLine, self).__init__()
        self.version = __version__
        self.idkw = __id__
        self.urlkw = __url__
        self.desc = __doc__
        self.app_class = PackModules

    def set_options(self):
        super(CommandLine, self).set_options()
        parser = self.parser
        parser.add_argument('-c', '--config', required=True,
                            help='Host offload configuration file')
        parser.add_argument('-e', '--enum-file', required=True,
                            help='Header file for enum of module types')
        parser.add_argument('module', nargs='+',
                            help='Module to be stitched')

if __name__ == '__main__':
    sys.exit(CommandLine()())

# vim: ts=4:sw=4:tw=80:et
