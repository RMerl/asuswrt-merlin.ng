#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0+
# Copyright 2021 Google LLC
# Written by Simon Glass <sjg@chromium.org>

"""Tests for fip_util

This tests a few features of fip_util which are not covered by binman's ftest.py
"""

import os
import shutil
import sys
import tempfile
import unittest

# Bring in the patman and dtoc libraries (but don't override the first path
# in PYTHONPATH)
OUR_PATH = os.path.dirname(os.path.realpath(__file__))
sys.path.insert(2, os.path.join(OUR_PATH, '..'))

# pylint: disable=C0413
from binman import bintool
from binman import fip_util
from u_boot_pylib import test_util
from u_boot_pylib import tools

FIPTOOL = bintool.Bintool.create('fiptool')
HAVE_FIPTOOL = FIPTOOL.is_present()

# pylint: disable=R0902,R0904
class TestFip(unittest.TestCase):
    """Test of fip_util classes"""
    #pylint: disable=W0212
    def setUp(self):
        # Create a temporary directory for test files
        self._indir = tempfile.mkdtemp(prefix='fip_util.')
        tools.set_input_dirs([self._indir])

        # Set up a temporary output directory, used by the tools library when
        # compressing files
        tools.prepare_output_dir(None)

        self.src_file = os.path.join(self._indir, 'orig.py')
        self.outname = tools.get_output_filename('out.py')
        self.args = ['-D', '-s', self._indir, '-o', self.outname]
        self.readme = os.path.join(self._indir, 'readme.rst')
        self.macro_dir = os.path.join(self._indir, 'include/tools_share')
        self.macro_fname = os.path.join(self.macro_dir,
                                        'firmware_image_package.h')
        self.name_dir = os.path.join(self._indir, 'tools/fiptool')
        self.name_fname = os.path.join(self.name_dir, 'tbbr_config.c')

    macro_contents = '''

/* ToC Entry UUIDs */
#define UUID_TRUSTED_UPDATE_FIRMWARE_SCP_BL2U \\
	{{0x65, 0x92, 0x27, 0x03}, {0x2f, 0x74}, {0xe6, 0x44}, 0x8d, 0xff, {0x57, 0x9a, 0xc1, 0xff, 0x06, 0x10} }
#define UUID_TRUSTED_UPDATE_FIRMWARE_BL2U \\
	{{0x60, 0xb3, 0xeb, 0x37}, {0xc1, 0xe5}, {0xea, 0x41}, 0x9d, 0xf3, {0x19, 0xed, 0xa1, 0x1f, 0x68, 0x01} }

'''

    name_contents = '''

toc_entry_t toc_entries[] = {
	{
		.name = "SCP Firmware Updater Configuration FWU SCP_BL2U",
		.uuid = UUID_TRUSTED_UPDATE_FIRMWARE_SCP_BL2U,
		.cmdline_name = "scp-fwu-cfg"
	},
	{
		.name = "AP Firmware Updater Configuration BL2U",
		.uuid = UUID_TRUSTED_UPDATE_FIRMWARE_BL2U,
		.cmdline_name = "ap-fwu-cfg"
	},
'''

    def setup_readme(self):
        """Set up the readme.txt file"""
        tools.write_file(self.readme, 'Trusted Firmware-A\n==================',
                        binary=False)

    def setup_macro(self, data=macro_contents):
        """Set up the tbbr_config.c file"""
        os.makedirs(self.macro_dir)
        tools.write_file(self.macro_fname, data, binary=False)

    def setup_name(self, data=name_contents):
        """Set up the firmware_image_package.h file"""
        os.makedirs(self.name_dir)
        tools.write_file(self.name_fname, data, binary=False)

    def tearDown(self):
        """Remove the temporary input directory and its contents"""
        if self._indir:
            shutil.rmtree(self._indir)
        self._indir = None
        tools.finalise_output_dir()

    def test_no_readme(self):
        """Test handling of a missing readme.rst"""
        with self.assertRaises(Exception) as err:
            fip_util.main(self.args, self.src_file)
        self.assertIn('Expected file', str(err.exception))

    def test_invalid_readme(self):
        """Test that an invalid readme.rst is detected"""
        tools.write_file(self.readme, 'blah', binary=False)
        with self.assertRaises(Exception) as err:
            fip_util.main(self.args, self.src_file)
        self.assertIn('does not start with', str(err.exception))

    def test_no_fip_h(self):
        """Check handling of missing firmware_image_package.h"""
        self.setup_readme()
        with self.assertRaises(Exception) as err:
            fip_util.main(self.args, self.src_file)
        self.assertIn('No such file or directory', str(err.exception))

    def test_invalid_fip_h(self):
        """Check failure to parse firmware_image_package.h"""
        self.setup_readme()
        self.setup_macro('blah')
        with self.assertRaises(Exception) as err:
            fip_util.main(self.args, self.src_file)
        self.assertIn('Cannot parse file', str(err.exception))

    def test_parse_fip_h(self):
        """Check parsing of firmware_image_package.h"""
        self.setup_readme()
        # Check parsing the header file
        self.setup_macro()
        macros = fip_util.parse_macros(self._indir)
        expected_macros = {
            'UUID_TRUSTED_UPDATE_FIRMWARE_SCP_BL2U':
                ('ToC Entry UUIDs', 'UUID_TRUSTED_UPDATE_FIRMWARE_SCP_BL2U',
                 bytes([0x65, 0x92, 0x27, 0x03, 0x2f, 0x74, 0xe6, 0x44,
                        0x8d, 0xff, 0x57, 0x9a, 0xc1, 0xff, 0x06, 0x10])),
            'UUID_TRUSTED_UPDATE_FIRMWARE_BL2U':
                ('ToC Entry UUIDs', 'UUID_TRUSTED_UPDATE_FIRMWARE_BL2U',
                 bytes([0x60, 0xb3, 0xeb, 0x37, 0xc1, 0xe5, 0xea, 0x41,
                        0x9d, 0xf3, 0x19, 0xed, 0xa1, 0x1f, 0x68, 0x01])),
            }
        self.assertEqual(expected_macros, macros)

    def test_missing_tbbr_c(self):
        """Check handlinh of missing tbbr_config.c"""
        self.setup_readme()
        self.setup_macro()

        # Still need the .c file
        with self.assertRaises(Exception) as err:
            fip_util.main(self.args, self.src_file)
        self.assertIn('tbbr_config.c', str(err.exception))

    def test_invalid_tbbr_c(self):
        """Check failure to parse tbbr_config.c"""
        self.setup_readme()
        self.setup_macro()
        # Check invalid format for C file
        self.setup_name('blah')
        with self.assertRaises(Exception) as err:
            fip_util.main(self.args, self.src_file)
        self.assertIn('Cannot parse file', str(err.exception))

    def test_inconsistent_tbbr_c(self):
        """Check tbbr_config.c in a format we don't expect"""
        self.setup_readme()
        # This is missing a hex value
        self.setup_macro('''

/* ToC Entry UUIDs */
#define UUID_TRUSTED_UPDATE_FIRMWARE_SCP_BL2U \\
	{{0x65, 0x92, 0x27,}, {0x2f, 0x74}, {0xe6, 0x44}, 0x8d, 0xff, {0x57, 0x9a, 0xc1, 0xff, 0x06, 0x10} }
#define UUID_TRUSTED_UPDATE_FIRMWARE_BL2U \\
	{{0x60, 0xb3, 0xeb, 0x37}, {0xc1, 0xe5}, {0xea, 0x41}, 0x9d, 0xf3, {0x19, 0xed, 0xa1, 0x1f, 0x68, 0x01} }

''')
        # Check invalid format for C file
        self.setup_name('blah')
        with self.assertRaises(Exception) as err:
            fip_util.main(self.args, self.src_file)
        self.assertIn('Cannot parse UUID line 5', str(err.exception))

    def test_parse_tbbr_c(self):
        """Check parsing tbbr_config.c"""
        self.setup_readme()
        self.setup_macro()
        self.setup_name()

        names = fip_util.parse_names(self._indir)

        expected_names = {
            'UUID_TRUSTED_UPDATE_FIRMWARE_SCP_BL2U': (
                'SCP Firmware Updater Configuration FWU SCP_BL2U',
                'UUID_TRUSTED_UPDATE_FIRMWARE_SCP_BL2U',
                'scp-fwu-cfg'),
            'UUID_TRUSTED_UPDATE_FIRMWARE_BL2U': (
                'AP Firmware Updater Configuration BL2U',
                'UUID_TRUSTED_UPDATE_FIRMWARE_BL2U',
                'ap-fwu-cfg'),
            }
        self.assertEqual(expected_names, names)

    def test_uuid_not_in_tbbr_config_c(self):
        """Check handling a UUID in the header file that's not in the .c file"""
        self.setup_readme()
        self.setup_macro(self.macro_contents + '''
#define UUID_TRUSTED_OS_FW_KEY_CERT \\
	{{0x94,  0x77, 0xd6, 0x03}, {0xfb, 0x60}, {0xe4, 0x11}, 0x85, 0xdd, {0xb7, 0x10, 0x5b, 0x8c, 0xee, 0x04} }

''')
        self.setup_name()

        macros = fip_util.parse_macros(self._indir)
        names = fip_util.parse_names(self._indir)
        with test_util.capture_sys_output() as (stdout, _):
            fip_util.create_code_output(macros, names)
        self.assertIn(
            "UUID 'UUID_TRUSTED_OS_FW_KEY_CERT' is not mentioned in tbbr_config.c file",
            stdout.getvalue())

    def test_changes(self):
        """Check handling of a source file that does/doesn't need changes"""
        self.setup_readme()
        self.setup_macro()
        self.setup_name()

        # Check generating the file when changes are needed
        tools.write_file(self.src_file, '''

# This is taken from tbbr_config.c in ARM Trusted Firmware
FIP_TYPE_LIST = [
    # ToC Entry UUIDs
    FipType('scp-fwu-cfg', 'SCP Firmware Updater Configuration FWU SCP_BL2U',
            [0x65, 0x92, 0x27, 0x03, 0x2f, 0x74, 0xe6, 0x44,
             0x8d, 0xff, 0x57, 0x9a, 0xc1, 0xff, 0x06, 0x10]),
    ] # end
blah de blah
                        ''', binary=False)
        with test_util.capture_sys_output() as (stdout, _):
            fip_util.main(self.args, self.src_file)
        self.assertIn('Needs update', stdout.getvalue())

        # Check generating the file when no changes are needed
        tools.write_file(self.src_file, '''
# This is taken from tbbr_config.c in ARM Trusted Firmware
FIP_TYPE_LIST = [
    # ToC Entry UUIDs
    FipType('scp-fwu-cfg', 'SCP Firmware Updater Configuration FWU SCP_BL2U',
            [0x65, 0x92, 0x27, 0x03, 0x2f, 0x74, 0xe6, 0x44,
             0x8d, 0xff, 0x57, 0x9a, 0xc1, 0xff, 0x06, 0x10]),
    FipType('ap-fwu-cfg', 'AP Firmware Updater Configuration BL2U',
            [0x60, 0xb3, 0xeb, 0x37, 0xc1, 0xe5, 0xea, 0x41,
             0x9d, 0xf3, 0x19, 0xed, 0xa1, 0x1f, 0x68, 0x01]),
    ] # end
blah blah''', binary=False)
        with test_util.capture_sys_output() as (stdout, _):
            fip_util.main(self.args, self.src_file)
        self.assertIn('is up-to-date', stdout.getvalue())

    def test_no_debug(self):
        """Test running without the -D flag"""
        self.setup_readme()
        self.setup_macro()
        self.setup_name()

        args = self.args.copy()
        args.remove('-D')
        tools.write_file(self.src_file, '', binary=False)
        with test_util.capture_sys_output():
            fip_util.main(args, self.src_file)

    @unittest.skipIf(not HAVE_FIPTOOL, 'No fiptool available')
    def test_fiptool_list(self):
        """Create a FIP and check that fiptool can read it"""
        fwu = b'my data'
        tb_fw = b'some more data'
        fip = fip_util.FipWriter(0x123, 0x10)
        fip.add_entry('fwu', fwu, 0x456)
        fip.add_entry('tb-fw', tb_fw, 0)
        fip.add_entry(bytes(range(16)), tb_fw, 0)
        data = fip.get_data()
        fname = tools.get_output_filename('data.fip')
        tools.write_file(fname, data)
        result = FIPTOOL.info(fname)
        self.assertEqual(
            '''Firmware Updater NS_BL2U: offset=0xB0, size=0x7, cmdline="--fwu"
Trusted Boot Firmware BL2: offset=0xC0, size=0xE, cmdline="--tb-fw"
00010203-0405-0607-0809-0A0B0C0D0E0F: offset=0xD0, size=0xE, cmdline="--blob"
''',
            result)

    fwu_data = b'my data'
    tb_fw_data = b'some more data'
    other_fw_data = b'even more'

    def create_fiptool_image(self):
        """Create an image with fiptool which we can use for testing

        Returns:
            FipReader: reader for the image
        """
        fwu = os.path.join(self._indir, 'fwu')
        tools.write_file(fwu, self.fwu_data)

        tb_fw = os.path.join(self._indir, 'tb_fw')
        tools.write_file(tb_fw, self.tb_fw_data)

        other_fw = os.path.join(self._indir, 'other_fw')
        tools.write_file(other_fw, self.other_fw_data)

        fname = tools.get_output_filename('data.fip')
        uuid = 'e3b78d9e-4a64-11ec-b45c-fba2b9b49788'
        FIPTOOL.create_new(fname, 8, 0x123, fwu, tb_fw, uuid, other_fw)

        return fip_util.FipReader(tools.read_file(fname))

    @unittest.skipIf(not HAVE_FIPTOOL, 'No fiptool available')
    def test_fiptool_create(self):
        """Create a FIP with fiptool and check that fip_util can read it"""
        reader = self.create_fiptool_image()

        header = reader.header
        fents = reader.fents

        self.assertEqual(0x123 << 32, header.flags)
        self.assertEqual(fip_util.HEADER_MAGIC, header.name)
        self.assertEqual(fip_util.HEADER_SERIAL, header.serial)

        self.assertEqual(3, len(fents))
        fent = fents[0]
        self.assertEqual(
            bytes([0x4f, 0x51, 0x1d, 0x11, 0x2b, 0xe5, 0x4e, 0x49,
                   0xb4, 0xc5, 0x83, 0xc2, 0xf7, 0x15, 0x84, 0x0a]), fent.uuid)
        self.assertEqual(0xb0, fent.offset)
        self.assertEqual(len(self.fwu_data), fent.size)
        self.assertEqual(0, fent.flags)
        self.assertEqual(self.fwu_data, fent.data)

        fent = fents[1]
        self.assertEqual(
            bytes([0x5f, 0xf9, 0xec, 0x0b, 0x4d, 0x22, 0x3e, 0x4d,
             0xa5, 0x44, 0xc3, 0x9d, 0x81, 0xc7, 0x3f, 0x0a]), fent.uuid)
        self.assertEqual(0xb8, fent.offset)
        self.assertEqual(len(self.tb_fw_data), fent.size)
        self.assertEqual(0, fent.flags)
        self.assertEqual(self.tb_fw_data, fent.data)

        fent = fents[2]
        self.assertEqual(
            bytes([0xe3, 0xb7, 0x8d, 0x9e, 0x4a, 0x64, 0x11, 0xec,
                   0xb4, 0x5c, 0xfb, 0xa2, 0xb9, 0xb4, 0x97, 0x88]), fent.uuid)
        self.assertEqual(0xc8, fent.offset)
        self.assertEqual(len(self.other_fw_data), fent.size)
        self.assertEqual(0, fent.flags)
        self.assertEqual(self.other_fw_data, fent.data)

    @unittest.skipIf(not HAVE_FIPTOOL, 'No fiptool available')
    def test_reader_get_entry(self):
        """Test get_entry() by name and UUID"""
        reader = self.create_fiptool_image()
        fents = reader.fents
        fent = reader.get_entry('fwu')
        self.assertEqual(fent, fents[0])

        fent = reader.get_entry(
            bytes([0x5f, 0xf9, 0xec, 0x0b, 0x4d, 0x22, 0x3e, 0x4d,
                   0xa5, 0x44, 0xc3, 0x9d, 0x81, 0xc7, 0x3f, 0x0a]))
        self.assertEqual(fent, fents[1])

        # Try finding entries that don't exist
        with self.assertRaises(Exception) as err:
            fent = reader.get_entry('scp-fwu-cfg')
        self.assertIn("Cannot find FIP entry 'scp-fwu-cfg'", str(err.exception))

        with self.assertRaises(Exception) as err:
            fent = reader.get_entry(bytes(list(range(16))))
        self.assertIn(
            "Cannot find FIP entry '00010203-0405-0607-0809-0a0b0c0d0e0f'",
            str(err.exception))

        with self.assertRaises(Exception) as err:
            fent = reader.get_entry('blah')
        self.assertIn("Unknown FIP entry type 'blah'", str(err.exception))

    @unittest.skipIf(not HAVE_FIPTOOL, 'No fiptool available')
    def test_fiptool_errors(self):
        """Check some error reporting from fiptool"""
        with self.assertRaises(Exception) as err:
            with test_util.capture_sys_output():
                FIPTOOL.create_bad()
        self.assertIn("unrecognized option '--fred'", str(err.exception))


if __name__ == '__main__':
    unittest.main()
