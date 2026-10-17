# SPDX-License-Identifier: GPL-2.0+
# Copyright 2020 Google LLC
#

"""Tests for the src_scan module

This includes unit tests for scanning of the source code
"""

import copy
import os
import shutil
import tempfile
import unittest
from unittest import mock

from dtoc import src_scan
from u_boot_pylib import test_util
from u_boot_pylib import tools

OUR_PATH = os.path.dirname(os.path.realpath(__file__))

EXPECT_WARN = {'rockchip_rk3288_grf':
                   {'WARNING: the driver rockchip_rk3288_grf was not found in the driver list'}}

class FakeNode:
    """Fake Node object for testing"""
    def __init__(self):
        self.name = None
        self.props = {}

class FakeProp:
    """Fake Prop object for testing"""
    def __init__(self):
        self.name = None
        self.value = None

# This is a test so is allowed to access private things in the module it is
# testing
# pylint: disable=W0212

class TestSrcScan(unittest.TestCase):
    """Tests for src_scan"""
    @classmethod
    def setUpClass(cls):
        tools.prepare_output_dir(None)

    @classmethod
    def tearDownClass(cls):
        tools.finalise_output_dir()

    def test_simple(self):
        """Simple test of scanning drivers"""
        scan = src_scan.Scanner(None, None)
        scan.scan_drivers()
        self.assertIn('sandbox_gpio', scan._drivers)
        self.assertIn('sandbox_gpio_alias', scan._driver_aliases)
        self.assertEqual('sandbox_gpio',
                         scan._driver_aliases['sandbox_gpio_alias'])
        self.assertNotIn('sandbox_gpio_alias2', scan._driver_aliases)

    def test_additional(self):
        """Test with additional drivers to scan"""
        scan = src_scan.Scanner(
            None, [None, '', 'tools/dtoc/test/dtoc_test_scan_drivers.cxx'])
        scan.scan_drivers()
        self.assertIn('sandbox_gpio_alias2', scan._driver_aliases)
        self.assertEqual('sandbox_gpio',
                         scan._driver_aliases['sandbox_gpio_alias2'])

    def test_unicode_error(self):
        """Test running dtoc with an invalid unicode file

        To be able to perform this test without adding a weird text file which
        would produce issues when using checkpatch.pl or patman, generate the
        file at runtime and then process it.
        """
        driver_fn = '/tmp/' + next(tempfile._get_candidate_names())
        with open(driver_fn, 'wb+') as fout:
            fout.write(b'\x81')

        scan = src_scan.Scanner(None, [driver_fn])
        with test_util.capture_sys_output() as (stdout, _):
            scan.scan_drivers()
        self.assertRegex(stdout.getvalue(),
                         r"Skipping file '.*' due to unicode error\s*")

    def test_driver(self):
        """Test the Driver class"""
        i2c = 'I2C_UCLASS'
        compat = {'rockchip,rk3288-grf': 'ROCKCHIP_SYSCON_GRF',
                  'rockchip,rk3288-srf': None}
        drv1 = src_scan.Driver('fred', 'fred.c')
        drv2 = src_scan.Driver('mary', 'mary.c')
        drv3 = src_scan.Driver('fred', 'fred.c')
        drv1.uclass_id = i2c
        drv1.compat = compat
        drv2.uclass_id = i2c
        drv2.compat = compat
        drv3.uclass_id = i2c
        drv3.compat = compat
        self.assertEqual(
            "Driver(name='fred', used=False, uclass_id='I2C_UCLASS', "
            "compat={'rockchip,rk3288-grf': 'ROCKCHIP_SYSCON_GRF', "
            "'rockchip,rk3288-srf': None}, priv=)", str(drv1))
        self.assertEqual(drv1, drv3)
        self.assertNotEqual(drv1, drv2)
        self.assertNotEqual(drv2, drv3)

    def test_scan_dirs(self):
        """Test scanning of source directories"""
        def add_file(fname):
            pathname = os.path.join(indir, fname)
            dirname = os.path.dirname(pathname)
            os.makedirs(dirname, exist_ok=True)
            tools.write_file(pathname, '', binary=False)
            fname_list.append(pathname)

        try:
            indir = tempfile.mkdtemp(prefix='dtoc.')

            fname_list = []
            add_file('fname.c')
            add_file('.git/ignoreme.c')
            add_file('dir/fname2.c')
            add_file('build-sandbox/ignoreme2.c')

            # Mock out scan_driver and check that it is called with the
            # expected files
            with mock.patch.object(src_scan.Scanner, "scan_driver")  as mocked:
                scan = src_scan.Scanner(indir, None)
                scan.scan_drivers()
            self.assertEqual(2, len(mocked.mock_calls))
            self.assertEqual(mock.call(fname_list[0]),
                             mocked.mock_calls[0])
            # .git file should be ignored
            self.assertEqual(mock.call(fname_list[2]),
                             mocked.mock_calls[1])
        finally:
            shutil.rmtree(indir)

    def test_scan(self):
        """Test scanning of a driver"""
        fname = os.path.join(OUR_PATH, '..', '..', 'drivers/i2c/tegra_i2c.c')
        buff = tools.read_file(fname, False)
        scan = src_scan.Scanner(None, None)
        scan._parse_driver(fname, buff)
        self.assertIn('i2c_tegra', scan._drivers)
        drv = scan._drivers['i2c_tegra']
        self.assertEqual('i2c_tegra', drv.name)
        self.assertEqual('UCLASS_I2C', drv.uclass_id)
        self.assertEqual(
            {'nvidia,tegra114-i2c': 'TYPE_114',
             'nvidia,tegra124-i2c': 'TYPE_114',
             'nvidia,tegra20-i2c': 'TYPE_STD',
             'nvidia,tegra20-i2c-dvc': 'TYPE_DVC'}, drv.compat)
        self.assertEqual('i2c_bus', drv.priv)
        self.assertEqual(1, len(scan._drivers))
        self.assertEqual({}, scan._warnings)

    def test_normalized_name(self):
        """Test operation of get_normalized_compat_name()"""
        prop = FakeProp()
        prop.name = 'compatible'
        prop.value = 'rockchip,rk3288-grf'
        node = FakeNode()
        node.props = {'compatible': prop}

        # get_normalized_compat_name() uses this to check for root node
        node.parent = FakeNode()

        scan = src_scan.Scanner(None, None)
        with test_util.capture_sys_output() as (stdout, _):
            name, aliases = scan.get_normalized_compat_name(node)
        self.assertEqual('rockchip_rk3288_grf', name)
        self.assertEqual([], aliases)
        self.assertEqual(1, len(scan._missing_drivers))
        self.assertEqual({'rockchip_rk3288_grf'}, scan._missing_drivers)
        self.assertEqual('', stdout.getvalue().strip())
        self.assertEqual(EXPECT_WARN, scan._warnings)

        i2c = 'I2C_UCLASS'
        compat = {'rockchip,rk3288-grf': 'ROCKCHIP_SYSCON_GRF',
                  'rockchip,rk3288-srf': None}
        drv = src_scan.Driver('fred', 'fred.c')
        drv.uclass_id = i2c
        drv.compat = compat
        scan._drivers['rockchip_rk3288_grf'] = drv

        scan._driver_aliases['rockchip_rk3288_srf'] = 'rockchip_rk3288_grf'

        with test_util.capture_sys_output() as (stdout, _):
            name, aliases = scan.get_normalized_compat_name(node)
        self.assertEqual('', stdout.getvalue().strip())
        self.assertEqual('rockchip_rk3288_grf', name)
        self.assertEqual([], aliases)
        self.assertEqual(EXPECT_WARN, scan._warnings)

        prop.value = 'rockchip,rk3288-srf'
        with test_util.capture_sys_output() as (stdout, _):
            name, aliases = scan.get_normalized_compat_name(node)
        self.assertEqual('', stdout.getvalue().strip())
        self.assertEqual('rockchip_rk3288_grf', name)
        self.assertEqual(['rockchip_rk3288_srf'], aliases)
        self.assertEqual(EXPECT_WARN, scan._warnings)

    def test_scan_errors(self):
        """Test detection of scanning errors"""
        buff = '''
static const struct udevice_id tegra_i2c_ids2[] = {
	{ .compatible = "nvidia,tegra114-i2c", .data = TYPE_114 },
	{ }
};

U_BOOT_DRIVER(i2c_tegra) = {
	.name	= "i2c_tegra",
	.id	= UCLASS_I2C,
	.of_match = tegra_i2c_ids,
};
'''
        scan = src_scan.Scanner(None, None)
        with self.assertRaises(ValueError) as exc:
            scan._parse_driver('file.c', buff)
        self.assertIn(
            "file.c: Unknown compatible var 'tegra_i2c_ids' (found: tegra_i2c_ids2)",
            str(exc.exception))

    def test_of_match(self):
        """Test detection of of_match_ptr() member"""
        buff = '''
static const struct udevice_id tegra_i2c_ids[] = {
	{ .compatible = "nvidia,tegra114-i2c", .data = TYPE_114 },
	{ }
};

U_BOOT_DRIVER(i2c_tegra) = {
	.name	= "i2c_tegra",
	.id	= UCLASS_I2C,
	.of_match = of_match_ptr(tegra_i2c_ids),
};
'''
        scan = src_scan.Scanner(None, None)
        scan._parse_driver('file.c', buff)
        self.assertIn('i2c_tegra', scan._drivers)
        drv = scan._drivers['i2c_tegra']
        self.assertEqual('i2c_tegra', drv.name)
        self.assertEqual('', drv.phase)
        self.assertEqual([], drv.headers)

    def test_priv(self):
        """Test collection of struct info from drivers"""
        buff = '''
static const struct udevice_id test_ids[] = {
	{ .compatible = "nvidia,tegra114-i2c", .data = TYPE_114 },
	{ }
};

U_BOOT_DRIVER(testing) = {
	.name	= "testing",
	.id	= UCLASS_I2C,
	.of_match = test_ids,
	.priv_auto	= sizeof(struct some_priv),
	.plat_auto = sizeof(struct some_plat),
	.per_child_auto	= sizeof(struct some_cpriv),
	.per_child_plat_auto = sizeof(struct some_cplat),
	DM_PHASE(tpl)
	DM_HEADER(<i2c.h>)
	DM_HEADER(<asm/clk.h>)
};
'''
        scan = src_scan.Scanner(None, None)
        scan._parse_driver('file.c', buff)
        self.assertIn('testing', scan._drivers)
        drv = scan._drivers['testing']
        self.assertEqual('testing', drv.name)
        self.assertEqual('UCLASS_I2C', drv.uclass_id)
        self.assertEqual(
            {'nvidia,tegra114-i2c': 'TYPE_114'}, drv.compat)
        self.assertEqual('some_priv', drv.priv)
        self.assertEqual('some_plat', drv.plat)
        self.assertEqual('some_cpriv', drv.child_priv)
        self.assertEqual('some_cplat', drv.child_plat)
        self.assertEqual('tpl', drv.phase)
        self.assertEqual(['<i2c.h>', '<asm/clk.h>'], drv.headers)
        self.assertEqual(1, len(scan._drivers))

    def test_uclass_scan(self):
        """Test collection of uclass-driver info"""
        buff = '''
UCLASS_DRIVER(i2c) = {
	.id		= UCLASS_I2C,
	.name		= "i2c",
	.flags		= DM_UC_FLAG_SEQ_ALIAS,
	.priv_auto	= sizeof(struct some_priv),
	.per_device_auto	= sizeof(struct per_dev_priv),
	.per_device_plat_auto	= sizeof(struct per_dev_plat),
	.per_child_auto	= sizeof(struct per_child_priv),
	.per_child_plat_auto	= sizeof(struct per_child_plat),
	.child_post_bind = i2c_child_post_bind,
};

'''
        scan = src_scan.Scanner(None, None)
        scan._parse_uclass_driver('file.c', buff)
        self.assertIn('UCLASS_I2C', scan._uclass)
        drv = scan._uclass['UCLASS_I2C']
        self.assertEqual('i2c', drv.name)
        self.assertEqual('UCLASS_I2C', drv.uclass_id)
        self.assertEqual('some_priv', drv.priv)
        self.assertEqual('per_dev_priv', drv.per_dev_priv)
        self.assertEqual('per_dev_plat', drv.per_dev_plat)
        self.assertEqual('per_child_priv', drv.per_child_priv)
        self.assertEqual('per_child_plat', drv.per_child_plat)
        self.assertEqual(1, len(scan._uclass))

        drv2 = copy.deepcopy(drv)
        self.assertEqual(drv, drv2)
        drv2.priv = 'other_priv'
        self.assertNotEqual(drv, drv2)

        # The hashes only depend on the uclass ID, so should be equal
        self.assertEqual(drv.__hash__(), drv2.__hash__())

        self.assertEqual("UclassDriver(name='i2c', uclass_id='UCLASS_I2C')",
                         str(drv))

    def test_uclass_scan_errors(self):
        """Test detection of uclass scanning errors"""
        buff = '''
UCLASS_DRIVER(i2c) = {
	.name		= "i2c",
};

'''
        scan = src_scan.Scanner(None, None)
        with self.assertRaises(ValueError) as exc:
            scan._parse_uclass_driver('file.c', buff)
        self.assertIn("file.c: Cannot parse uclass ID in driver 'i2c'",
                      str(exc.exception))

    def test_struct_scan(self):
        """Test collection of struct info"""
        buff = '''
/* some comment */
struct some_struct1 {
	struct i2c_msg *msgs;
	uint nmsgs;
};
'''
        scan = src_scan.Scanner(None, None)
        scan._basedir = os.path.join(OUR_PATH, '..', '..')
        scan._parse_structs('arch/arm/include/asm/file.h', buff)
        self.assertIn('some_struct1', scan._structs)
        struc = scan._structs['some_struct1']
        self.assertEqual('some_struct1', struc.name)
        self.assertEqual('asm/file.h', struc.fname)

        buff = '''
/* another comment */
struct another_struct {
	int speed_hz;
	int max_transaction_bytes;
};
'''
        scan._parse_structs('include/file2.h', buff)
        self.assertIn('another_struct', scan._structs)
        struc = scan._structs['another_struct']
        self.assertEqual('another_struct', struc.name)
        self.assertEqual('file2.h', struc.fname)

        self.assertEqual(2, len(scan._structs))

        self.assertEqual("Struct(name='another_struct', fname='file2.h')",
                         str(struc))

    def test_struct_scan_errors(self):
        """Test scanning a header file with an invalid unicode file"""
        output = tools.get_output_filename('output.h')
        tools.write_file(output, b'struct this is a test \x81 of bad unicode')

        scan = src_scan.Scanner(None, None)
        with test_util.capture_sys_output() as (stdout, _):
            scan.scan_header(output)
        self.assertIn('due to unicode error', stdout.getvalue())

    def setup_dup_drivers(self, name, phase=''):
        """Set up for a duplcate test

        Returns:
            tuple:
                Scanner to use
                Driver record for first driver
                Text of second driver declaration
                Node for driver 1
        """
        driver1 = '''
static const struct udevice_id test_ids[] = {
	{ .compatible = "nvidia,tegra114-i2c", .data = TYPE_114 },
	{ }
};

U_BOOT_DRIVER(%s) = {
	.name	= "testing",
	.id	= UCLASS_I2C,
	.of_match = test_ids,
	%s
};
''' % (name, 'DM_PHASE(%s)' % phase if phase else '')
        driver2 = '''
static const struct udevice_id test_ids[] = {
	{ .compatible = "nvidia,tegra114-dvc" },
	{ }
};

U_BOOT_DRIVER(%s) = {
	.name	= "testing",
	.id	= UCLASS_RAM,
	.of_match = test_ids,
};
''' % name
        scan = src_scan.Scanner(None, None, phase)
        scan._parse_driver('file1.c', driver1)
        self.assertIn(name, scan._drivers)
        drv1 = scan._drivers[name]

        prop = FakeProp()
        prop.name = 'compatible'
        prop.value = 'nvidia,tegra114-i2c'
        node = FakeNode()
        node.name = 'testing'
        node.props = {'compatible': prop}

        # get_normalized_compat_name() uses this to check for root node
        node.parent = FakeNode()

        return scan, drv1, driver2, node

    def test_dup_drivers(self):
        """Test handling of duplicate drivers"""
        name = 'nvidia_tegra114_i2c'
        scan, drv1, driver2, node = self.setup_dup_drivers(name)
        self.assertEqual('', drv1.phase)

        # The driver should not have a duplicate yet
        self.assertEqual([], drv1.dups)

        scan._parse_driver('file2.c', driver2)

        # The first driver should now be a duplicate of the second
        drv2 = scan._drivers[name]
        self.assertEqual('', drv2.phase)
        self.assertEqual(1, len(drv2.dups))
        self.assertEqual([drv1], drv2.dups)

        # There is no way to distinguish them, so we should expect a warning
        self.assertTrue(drv2.warn_dups)

        # We should see a warning
        with test_util.capture_sys_output() as (stdout, _):
            scan.mark_used([node])
        self.assertEqual(
            "Warning: Duplicate driver name 'nvidia_tegra114_i2c' (orig=file2.c, dups=file1.c)",
            stdout.getvalue().strip())

    def test_dup_drivers_phase(self):
        """Test handling of duplicate drivers but with different phases"""
        name = 'nvidia_tegra114_i2c'
        scan, drv1, driver2, node = self.setup_dup_drivers(name, 'spl')
        scan._parse_driver('file2.c', driver2)
        self.assertEqual('spl', drv1.phase)

        # The second driver should now be a duplicate of the second
        self.assertEqual(1, len(drv1.dups))
        drv2 = drv1.dups[0]

        # The phase is different, so we should not warn of dups
        self.assertFalse(drv1.warn_dups)

        # We should not see a warning
        with test_util.capture_sys_output() as (stdout, _):
            scan.mark_used([node])
        self.assertEqual('', stdout.getvalue().strip())

    def test_sequence(self):
        """Test assignment of sequence numnbers"""
        scan = src_scan.Scanner(None, None, '')
        node = FakeNode()
        uc = src_scan.UclassDriver('UCLASS_I2C')
        node.uclass = uc
        node.driver = True
        node.seq = -1
        node.path = 'mypath'
        uc.alias_num_to_node[2] = node

        # This should assign 3 (after the 2 that exists)
        seq = scan.assign_seq(node)
        self.assertEqual(3, seq)
        self.assertEqual({'mypath': 3}, uc.alias_path_to_num)
        self.assertEqual({2: node, 3: node}, uc.alias_num_to_node)

    def test_scan_warnings(self):
        """Test detection of scanning warnings"""
        buff = '''
static const struct udevice_id tegra_i2c_ids[] = {
	{ .compatible = "nvidia,tegra114-i2c", .data = TYPE_114 },
	{ }
};

U_BOOT_DRIVER(i2c_tegra) = {
	.name	= "i2c_tegra",
	.id	= UCLASS_I2C,
	.of_match = tegra_i2c_ids + 1,
};
'''
        # The '+ 1' above should generate a warning

        prop = FakeProp()
        prop.name = 'compatible'
        prop.value = 'rockchip,rk3288-grf'
        node = FakeNode()
        node.props = {'compatible': prop}

        # get_normalized_compat_name() uses this to check for root node
        node.parent = FakeNode()

        scan = src_scan.Scanner(None, None)
        scan._parse_driver('file.c', buff)
        self.assertEqual(
            {'i2c_tegra':
                 {"file.c: Warning: unexpected suffix ' + 1' on .of_match line for compat 'tegra_i2c_ids'"}},
            scan._warnings)

        tprop = FakeProp()
        tprop.name = 'compatible'
        tprop.value = 'nvidia,tegra114-i2c'
        tnode = FakeNode()
        tnode.props = {'compatible': tprop}

        # get_normalized_compat_name() uses this to check for root node
        tnode.parent = FakeNode()

        with test_util.capture_sys_output() as (stdout, _):
            scan.get_normalized_compat_name(node)
            scan.get_normalized_compat_name(tnode)
        self.assertEqual('', stdout.getvalue().strip())

        self.assertEqual(2, len(scan._missing_drivers))
        self.assertEqual({'rockchip_rk3288_grf', 'nvidia_tegra114_i2c'},
                         scan._missing_drivers)
        with test_util.capture_sys_output() as (stdout, _):
            scan.show_warnings()
        self.assertIn('rockchip_rk3288_grf', stdout.getvalue())

        # This should show just the rockchip warning, since the tegra driver
        # is not in self._missing_drivers
        scan._missing_drivers.remove('nvidia_tegra114_i2c')
        with test_util.capture_sys_output() as (stdout, _):
            scan.show_warnings()
        self.assertIn('rockchip_rk3288_grf', stdout.getvalue())
        self.assertNotIn('tegra_i2c_ids', stdout.getvalue())

        # Do a similar thing with used drivers. By marking the tegra driver as
        # used, the warning related to that driver will be shown
        drv = scan._drivers['i2c_tegra']
        drv.used = True
        with test_util.capture_sys_output() as (stdout, _):
            scan.show_warnings()
        self.assertIn('rockchip_rk3288_grf', stdout.getvalue())
        self.assertIn('tegra_i2c_ids', stdout.getvalue())

        # Add a warning to make sure multiple warnings are shown
        scan._warnings['i2c_tegra'].update(
            scan._warnings['nvidia_tegra114_i2c'])
        del scan._warnings['nvidia_tegra114_i2c']
        with test_util.capture_sys_output() as (stdout, _):
            scan.show_warnings()
        self.assertEqual('''i2c_tegra: WARNING: the driver nvidia_tegra114_i2c was not found in the driver list
         : file.c: Warning: unexpected suffix ' + 1' on .of_match line for compat 'tegra_i2c_ids'

rockchip_rk3288_grf: WARNING: the driver rockchip_rk3288_grf was not found in the driver list

''',
            stdout.getvalue())
        self.assertIn('tegra_i2c_ids', stdout.getvalue())

    def scan_uclass_warning(self):
        """Test a missing .uclass in the driver"""
        buff = '''
static const struct udevice_id tegra_i2c_ids[] = {
	{ .compatible = "nvidia,tegra114-i2c", .data = TYPE_114 },
	{ }
};

U_BOOT_DRIVER(i2c_tegra) = {
	.name	= "i2c_tegra",
	.of_match = tegra_i2c_ids,
};
'''
        scan = src_scan.Scanner(None, None)
        scan._parse_driver('file.c', buff)
        self.assertEqual(
            {'i2c_tegra': {'Missing .uclass in file.c'}},
            scan._warnings)

    def scan_compat_warning(self):
        """Test a missing .compatible in the driver"""
        buff = '''
static const struct udevice_id tegra_i2c_ids[] = {
	{ .compatible = "nvidia,tegra114-i2c", .data = TYPE_114 },
	{ }
};

U_BOOT_DRIVER(i2c_tegra) = {
	.name	= "i2c_tegra",
	.id	= UCLASS_I2C,
};
'''
        scan = src_scan.Scanner(None, None)
        scan._parse_driver('file.c', buff)
        self.assertEqual(
            {'i2c_tegra': {'Missing .compatible in file.c'}},
            scan._warnings)
