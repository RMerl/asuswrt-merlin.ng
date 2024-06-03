# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# To run a single test, change to this directory, and:
#
#    python -m unittest func_test.TestFunctional.testHelp

import hashlib
from optparse import OptionParser
import os
import shutil
import struct
import sys
import tempfile
import unittest

import binman
import cmdline
import command
import control
import elf
import fdt
import fdt_util
import fmap_util
import test_util
import state
import tools
import tout

# Contents of test files, corresponding to different entry types
U_BOOT_DATA           = '1234'
U_BOOT_IMG_DATA       = 'img'
U_BOOT_SPL_DATA       = '56780123456789abcde'
U_BOOT_TPL_DATA       = 'tpl'
BLOB_DATA             = '89'
ME_DATA               = '0abcd'
VGA_DATA              = 'vga'
U_BOOT_DTB_DATA       = 'udtb'
U_BOOT_SPL_DTB_DATA   = 'spldtb'
U_BOOT_TPL_DTB_DATA   = 'tpldtb'
X86_START16_DATA      = 'start16'
X86_START16_SPL_DATA  = 'start16spl'
X86_START16_TPL_DATA  = 'start16tpl'
PPC_MPC85XX_BR_DATA   = 'ppcmpc85xxbr'
U_BOOT_NODTB_DATA     = 'nodtb with microcode pointer somewhere in here'
U_BOOT_SPL_NODTB_DATA = 'splnodtb with microcode pointer somewhere in here'
U_BOOT_TPL_NODTB_DATA = 'tplnodtb with microcode pointer somewhere in here'
FSP_DATA              = 'fsp'
CMC_DATA              = 'cmc'
VBT_DATA              = 'vbt'
MRC_DATA              = 'mrc'
TEXT_DATA             = 'text'
TEXT_DATA2            = 'text2'
TEXT_DATA3            = 'text3'
CROS_EC_RW_DATA       = 'ecrw'
GBB_DATA              = 'gbbd'
BMPBLK_DATA           = 'bmp'
VBLOCK_DATA           = 'vblk'
FILES_DATA            = ("sorry I'm late\nOh, don't bother apologising, I'm " +
                         "sorry you're alive\n")
COMPRESS_DATA         = 'data to compress'
REFCODE_DATA          = 'refcode'


class TestFunctional(unittest.TestCase):
    """Functional tests for binman

    Most of these use a sample .dts file to build an image and then check
    that it looks correct. The sample files are in the test/ subdirectory
    and are numbered.

    For each entry type a very small test file is created using fixed
    string contents. This makes it easy to test that things look right, and
    debug problems.

    In some cases a 'real' file must be used - these are also supplied in
    the test/ diurectory.
    """
    @classmethod
    def setUpClass(self):
        global entry
        import entry

        # Handle the case where argv[0] is 'python'
        self._binman_dir = os.path.dirname(os.path.realpath(sys.argv[0]))
        self._binman_pathname = os.path.join(self._binman_dir, 'binman')

        # Create a temporary directory for input files
        self._indir = tempfile.mkdtemp(prefix='binmant.')

        # Create some test files
        TestFunctional._MakeInputFile('u-boot.bin', U_BOOT_DATA)
        TestFunctional._MakeInputFile('u-boot.img', U_BOOT_IMG_DATA)
        TestFunctional._MakeInputFile('spl/u-boot-spl.bin', U_BOOT_SPL_DATA)
        TestFunctional._MakeInputFile('tpl/u-boot-tpl.bin', U_BOOT_TPL_DATA)
        TestFunctional._MakeInputFile('blobfile', BLOB_DATA)
        TestFunctional._MakeInputFile('me.bin', ME_DATA)
        TestFunctional._MakeInputFile('vga.bin', VGA_DATA)
        self._ResetDtbs()
        TestFunctional._MakeInputFile('u-boot-x86-16bit.bin', X86_START16_DATA)
        TestFunctional._MakeInputFile('u-boot-br.bin', PPC_MPC85XX_BR_DATA)
        TestFunctional._MakeInputFile('spl/u-boot-x86-16bit-spl.bin',
                                      X86_START16_SPL_DATA)
        TestFunctional._MakeInputFile('tpl/u-boot-x86-16bit-tpl.bin',
                                      X86_START16_TPL_DATA)
        TestFunctional._MakeInputFile('u-boot-nodtb.bin', U_BOOT_NODTB_DATA)
        TestFunctional._MakeInputFile('spl/u-boot-spl-nodtb.bin',
                                      U_BOOT_SPL_NODTB_DATA)
        TestFunctional._MakeInputFile('tpl/u-boot-tpl-nodtb.bin',
                                      U_BOOT_TPL_NODTB_DATA)
        TestFunctional._MakeInputFile('fsp.bin', FSP_DATA)
        TestFunctional._MakeInputFile('cmc.bin', CMC_DATA)
        TestFunctional._MakeInputFile('vbt.bin', VBT_DATA)
        TestFunctional._MakeInputFile('mrc.bin', MRC_DATA)
        TestFunctional._MakeInputFile('ecrw.bin', CROS_EC_RW_DATA)
        TestFunctional._MakeInputDir('devkeys')
        TestFunctional._MakeInputFile('bmpblk.bin', BMPBLK_DATA)
        TestFunctional._MakeInputFile('refcode.bin', REFCODE_DATA)

        # ELF file with a '_dt_ucode_base_size' symbol
        with open(self.TestFile('u_boot_ucode_ptr')) as fd:
            TestFunctional._MakeInputFile('u-boot', fd.read())

        # Intel flash descriptor file
        with open(self.TestFile('descriptor.bin')) as fd:
            TestFunctional._MakeInputFile('descriptor.bin', fd.read())

        shutil.copytree(self.TestFile('files'),
                        os.path.join(self._indir, 'files'))

        TestFunctional._MakeInputFile('compress', COMPRESS_DATA)

    @classmethod
    def tearDownClass(self):
        """Remove the temporary input directory and its contents"""
        if self._indir:
            shutil.rmtree(self._indir)
        self._indir = None

    def setUp(self):
        # Enable this to turn on debugging output
        # tout.Init(tout.DEBUG)
        command.test_result = None

    def tearDown(self):
        """Remove the temporary output directory"""
        tools._FinaliseForTest()

    @classmethod
    def _ResetDtbs(self):
        TestFunctional._MakeInputFile('u-boot.dtb', U_BOOT_DTB_DATA)
        TestFunctional._MakeInputFile('spl/u-boot-spl.dtb', U_BOOT_SPL_DTB_DATA)
        TestFunctional._MakeInputFile('tpl/u-boot-tpl.dtb', U_BOOT_TPL_DTB_DATA)

    def _RunBinman(self, *args, **kwargs):
        """Run binman using the command line

        Args:
            Arguments to pass, as a list of strings
            kwargs: Arguments to pass to Command.RunPipe()
        """
        result = command.RunPipe([[self._binman_pathname] + list(args)],
                capture=True, capture_stderr=True, raise_on_error=False)
        if result.return_code and kwargs.get('raise_on_error', True):
            raise Exception("Error running '%s': %s" % (' '.join(args),
                            result.stdout + result.stderr))
        return result

    def _DoBinman(self, *args):
        """Run binman using directly (in the same process)

        Args:
            Arguments to pass, as a list of strings
        Returns:
            Return value (0 for success)
        """
        args = list(args)
        if '-D' in sys.argv:
            args = args + ['-D']
        (options, args) = cmdline.ParseArgs(args)
        options.pager = 'binman-invalid-pager'
        options.build_dir = self._indir

        # For testing, you can force an increase in verbosity here
        # options.verbosity = tout.DEBUG
        return control.Binman(options, args)

    def _DoTestFile(self, fname, debug=False, map=False, update_dtb=False,
                    entry_args=None, images=None, use_real_dtb=False,
                    verbosity=None):
        """Run binman with a given test file

        Args:
            fname: Device-tree source filename to use (e.g. 005_simple.dts)
            debug: True to enable debugging output
            map: True to output map files for the images
            update_dtb: Update the offset and size of each entry in the device
                tree before packing it into the image
            entry_args: Dict of entry args to supply to binman
                key: arg name
                value: value of that arg
            images: List of image names to build
        """
        args = ['-p', '-I', self._indir, '-d', self.TestFile(fname)]
        if debug:
            args.append('-D')
        if map:
            args.append('-m')
        if update_dtb:
            args.append('-up')
        if not use_real_dtb:
            args.append('--fake-dtb')
        if verbosity is not None:
            args.append('-v%d' % verbosity)
        if entry_args:
            for arg, value in entry_args.iteritems():
                args.append('-a%s=%s' % (arg, value))
        if images:
            for image in images:
                args += ['-i', image]
        return self._DoBinman(*args)

    def _SetupDtb(self, fname, outfile='u-boot.dtb'):
        """Set up a new test device-tree file

        The given file is compiled and set up as the device tree to be used
        for ths test.

        Args:
            fname: Filename of .dts file to read
            outfile: Output filename for compiled device-tree binary

        Returns:
            Contents of device-tree binary
        """
        tools.PrepareOutputDir(None)
        dtb = fdt_util.EnsureCompiled(self.TestFile(fname))
        with open(dtb) as fd:
            data = fd.read()
            TestFunctional._MakeInputFile(outfile, data)
        tools.FinaliseOutputDir()
        return data

    def _GetDtbContentsForSplTpl(self, dtb_data, name):
        """Create a version of the main DTB for SPL or SPL

        For testing we don't actually have different versions of the DTB. With
        U-Boot we normally run fdtgrep to remove unwanted nodes, but for tests
        we don't normally have any unwanted nodes.

        We still want the DTBs for SPL and TPL to be different though, since
        otherwise it is confusing to know which one we are looking at. So add
        an 'spl' or 'tpl' property to the top-level node.
        """
        dtb = fdt.Fdt.FromData(dtb_data)
        dtb.Scan()
        dtb.GetNode('/binman').AddZeroProp(name)
        dtb.Sync(auto_resize=True)
        dtb.Pack()
        return dtb.GetContents()

    def _DoReadFileDtb(self, fname, use_real_dtb=False, map=False,
                       update_dtb=False, entry_args=None, reset_dtbs=True):
        """Run binman and return the resulting image

        This runs binman with a given test file and then reads the resulting
        output file. It is a shortcut function since most tests need to do
        these steps.

        Raises an assertion failure if binman returns a non-zero exit code.

        Args:
            fname: Device-tree source filename to use (e.g. 005_simple.dts)
            use_real_dtb: True to use the test file as the contents of
                the u-boot-dtb entry. Normally this is not needed and the
                test contents (the U_BOOT_DTB_DATA string) can be used.
                But in some test we need the real contents.
            map: True to output map files for the images
            update_dtb: Update the offset and size of each entry in the device
                tree before packing it into the image

        Returns:
            Tuple:
                Resulting image contents
                Device tree contents
                Map data showing contents of image (or None if none)
                Output device tree binary filename ('u-boot.dtb' path)
        """
        dtb_data = None
        # Use the compiled test file as the u-boot-dtb input
        if use_real_dtb:
            dtb_data = self._SetupDtb(fname)
            infile = os.path.join(self._indir, 'u-boot.dtb')

            # For testing purposes, make a copy of the DT for SPL and TPL. Add
            # a node indicating which it is, so aid verification.
            for name in ['spl', 'tpl']:
                dtb_fname = '%s/u-boot-%s.dtb' % (name, name)
                outfile = os.path.join(self._indir, dtb_fname)
                TestFunctional._MakeInputFile(dtb_fname,
                        self._GetDtbContentsForSplTpl(dtb_data, name))

        try:
            retcode = self._DoTestFile(fname, map=map, update_dtb=update_dtb,
                    entry_args=entry_args, use_real_dtb=use_real_dtb)
            self.assertEqual(0, retcode)
            out_dtb_fname = tools.GetOutputFilename('u-boot.dtb.out')

            # Find the (only) image, read it and return its contents
            image = control.images['image']
            image_fname = tools.GetOutputFilename('image.bin')
            self.assertTrue(os.path.exists(image_fname))
            if map:
                map_fname = tools.GetOutputFilename('image.map')
                with open(map_fname) as fd:
                    map_data = fd.read()
            else:
                map_data = None
            with open(image_fname) as fd:
                return fd.read(), dtb_data, map_data, out_dtb_fname
        finally:
            # Put the test file back
            if reset_dtbs and use_real_dtb:
                self._ResetDtbs()

    def _DoReadFile(self, fname, use_real_dtb=False):
        """Helper function which discards the device-tree binary

        Args:
            fname: Device-tree source filename to use (e.g. 005_simple.dts)
            use_real_dtb: True to use the test file as the contents of
                the u-boot-dtb entry. Normally this is not needed and the
                test contents (the U_BOOT_DTB_DATA string) can be used.
                But in some test we need the real contents.

        Returns:
            Resulting image contents
        """
        return self._DoReadFileDtb(fname, use_real_dtb)[0]

    @classmethod
    def _MakeInputFile(self, fname, contents):
        """Create a new test input file, creating directories as needed

        Args:
            fname: Filename to create
            contents: File contents to write in to the file
        Returns:
            Full pathname of file created
        """
        pathname = os.path.join(self._indir, fname)
        dirname = os.path.dirname(pathname)
        if dirname and not os.path.exists(dirname):
            os.makedirs(dirname)
        with open(pathname, 'wb') as fd:
            fd.write(contents)
        return pathname

    @classmethod
    def _MakeInputDir(self, dirname):
        """Create a new test input directory, creating directories as needed

        Args:
            dirname: Directory name to create

        Returns:
            Full pathname of directory created
        """
        pathname = os.path.join(self._indir, dirname)
        if not os.path.exists(pathname):
            os.makedirs(pathname)
        return pathname

    @classmethod
    def _SetupSplElf(self, src_fname='bss_data'):
        """Set up an ELF file with a '_dt_ucode_base_size' symbol

        Args:
            Filename of ELF file to use as SPL
        """
        with open(self.TestFile(src_fname)) as fd:
            TestFunctional._MakeInputFile('spl/u-boot-spl', fd.read())

    @classmethod
    def TestFile(self, fname):
        return os.path.join(self._binman_dir, 'test', fname)

    def AssertInList(self, grep_list, target):
        """Assert that at least one of a list of things is in a target

        Args:
            grep_list: List of strings to check
            target: Target string
        """
        for grep in grep_list:
            if grep in target:
                return
        self.fail("Error: '%' not found in '%s'" % (grep_list, target))

    def CheckNoGaps(self, entries):
        """Check that all entries fit together without gaps

        Args:
            entries: List of entries to check
        """
        offset = 0
        for entry in entries.values():
            self.assertEqual(offset, entry.offset)
            offset += entry.size

    def GetFdtLen(self, dtb):
        """Get the totalsize field from a device-tree binary

        Args:
            dtb: Device-tree binary contents

        Returns:
            Total size of device-tree binary, from the header
        """
        return struct.unpack('>L', dtb[4:8])[0]

    def _GetPropTree(self, dtb, prop_names):
        def AddNode(node, path):
            if node.name != '/':
                path += '/' + node.name
            for subnode in node.subnodes:
                for prop in subnode.props.values():
                    if prop.name in prop_names:
                        prop_path = path + '/' + subnode.name + ':' + prop.name
                        tree[prop_path[len('/binman/'):]] = fdt_util.fdt32_to_cpu(
                            prop.value)
                AddNode(subnode, path)

        tree = {}
        AddNode(dtb.GetRoot(), '')
        return tree

    def testRun(self):
        """Test a basic run with valid args"""
        result = self._RunBinman('-h')

    def testFullHelp(self):
        """Test that the full help is displayed with -H"""
        result = self._RunBinman('-H')
        help_file = os.path.join(self._binman_dir, 'README')
        # Remove possible extraneous strings
        extra = '::::::::::::::\n' + help_file + '\n::::::::::::::\n'
        gothelp = result.stdout.replace(extra, '')
        self.assertEqual(len(gothelp), os.path.getsize(help_file))
        self.assertEqual(0, len(result.stderr))
        self.assertEqual(0, result.return_code)

    def testFullHelpInternal(self):
        """Test that the full help is displayed with -H"""
        try:
            command.test_result = command.CommandResult()
            result = self._DoBinman('-H')
            help_file = os.path.join(self._binman_dir, 'README')
        finally:
            command.test_result = None

    def testHelp(self):
        """Test that the basic help is displayed with -h"""
        result = self._RunBinman('-h')
        self.assertTrue(len(result.stdout) > 200)
        self.assertEqual(0, len(result.stderr))
        self.assertEqual(0, result.return_code)

    def testBoard(self):
        """Test that we can run it with a specific board"""
        self._SetupDtb('005_simple.dts', 'sandbox/u-boot.dtb')
        TestFunctional._MakeInputFile('sandbox/u-boot.bin', U_BOOT_DATA)
        result = self._DoBinman('-b', 'sandbox')
        self.assertEqual(0, result)

    def testNeedBoard(self):
        """Test that we get an error when no board ius supplied"""
        with self.assertRaises(ValueError) as e:
            result = self._DoBinman()
        self.assertIn("Must provide a board to process (use -b <board>)",
                str(e.exception))

    def testMissingDt(self):
        """Test that an invalid device-tree file generates an error"""
        with self.assertRaises(Exception) as e:
            self._RunBinman('-d', 'missing_file')
        # We get one error from libfdt, and a different one from fdtget.
        self.AssertInList(["Couldn't open blob from 'missing_file'",
                           'No such file or directory'], str(e.exception))

    def testBrokenDt(self):
        """Test that an invalid device-tree source file generates an error

        Since this is a source file it should be compiled and the error
        will come from the device-tree compiler (dtc).
        """
        with self.assertRaises(Exception) as e:
            self._RunBinman('-d', self.TestFile('001_invalid.dts'))
        self.assertIn("FATAL ERROR: Unable to parse input tree",
                str(e.exception))

    def testMissingNode(self):
        """Test that a device tree without a 'binman' node generates an error"""
        with self.assertRaises(Exception) as e:
            self._DoBinman('-d', self.TestFile('002_missing_node.dts'))
        self.assertIn("does not have a 'binman' node", str(e.exception))

    def testEmpty(self):
        """Test that an empty binman node works OK (i.e. does nothing)"""
        result = self._RunBinman('-d', self.TestFile('003_empty.dts'))
        self.assertEqual(0, len(result.stderr))
        self.assertEqual(0, result.return_code)

    def testInvalidEntry(self):
        """Test that an invalid entry is flagged"""
        with self.assertRaises(Exception) as e:
            result = self._RunBinman('-d',
                                     self.TestFile('004_invalid_entry.dts'))
        self.assertIn("Unknown entry type 'not-a-valid-type' in node "
                "'/binman/not-a-valid-type'", str(e.exception))

    def testSimple(self):
        """Test a simple binman with a single file"""
        data = self._DoReadFile('005_simple.dts')
        self.assertEqual(U_BOOT_DATA, data)

    def testSimpleDebug(self):
        """Test a simple binman run with debugging enabled"""
        data = self._DoTestFile('005_simple.dts', debug=True)

    def testDual(self):
        """Test that we can handle creating two images

        This also tests image padding.
        """
        retcode = self._DoTestFile('006_dual_image.dts')
        self.assertEqual(0, retcode)

        image = control.images['image1']
        self.assertEqual(len(U_BOOT_DATA), image._size)
        fname = tools.GetOutputFilename('image1.bin')
        self.assertTrue(os.path.exists(fname))
        with open(fname) as fd:
            data = fd.read()
            self.assertEqual(U_BOOT_DATA, data)

        image = control.images['image2']
        self.assertEqual(3 + len(U_BOOT_DATA) + 5, image._size)
        fname = tools.GetOutputFilename('image2.bin')
        self.assertTrue(os.path.exists(fname))
        with open(fname) as fd:
            data = fd.read()
            self.assertEqual(U_BOOT_DATA, data[3:7])
            self.assertEqual(chr(0) * 3, data[:3])
            self.assertEqual(chr(0) * 5, data[7:])

    def testBadAlign(self):
        """Test that an invalid alignment value is detected"""
        with self.assertRaises(ValueError) as e:
            self._DoTestFile('007_bad_align.dts')
        self.assertIn("Node '/binman/u-boot': Alignment 23 must be a power "
                      "of two", str(e.exception))

    def testPackSimple(self):
        """Test that packing works as expected"""
        retcode = self._DoTestFile('008_pack.dts')
        self.assertEqual(0, retcode)
        self.assertIn('image', control.images)
        image = control.images['image']
        entries = image.GetEntries()
        self.assertEqual(5, len(entries))

        # First u-boot
        self.assertIn('u-boot', entries)
        entry = entries['u-boot']
        self.assertEqual(0, entry.offset)
        self.assertEqual(len(U_BOOT_DATA), entry.size)

        # Second u-boot, aligned to 16-byte boundary
        self.assertIn('u-boot-align', entries)
        entry = entries['u-boot-align']
        self.assertEqual(16, entry.offset)
        self.assertEqual(len(U_BOOT_DATA), entry.size)

        # Third u-boot, size 23 bytes
        self.assertIn('u-boot-size', entries)
        entry = entries['u-boot-size']
        self.assertEqual(20, entry.offset)
        self.assertEqual(len(U_BOOT_DATA), entry.contents_size)
        self.assertEqual(23, entry.size)

        # Fourth u-boot, placed immediate after the above
        self.assertIn('u-boot-next', entries)
        entry = entries['u-boot-next']
        self.assertEqual(43, entry.offset)
        self.assertEqual(len(U_BOOT_DATA), entry.size)

        # Fifth u-boot, placed at a fixed offset
        self.assertIn('u-boot-fixed', entries)
        entry = entries['u-boot-fixed']
        self.assertEqual(61, entry.offset)
        self.assertEqual(len(U_BOOT_DATA), entry.size)

        self.assertEqual(65, image._size)

    def testPackExtra(self):
        """Test that extra packing feature works as expected"""
        retcode = self._DoTestFile('009_pack_extra.dts')

        self.assertEqual(0, retcode)
        self.assertIn('image', control.images)
        image = control.images['image']
        entries = image.GetEntries()
        self.assertEqual(5, len(entries))

        # First u-boot with padding before and after
        self.assertIn('u-boot', entries)
        entry = entries['u-boot']
        self.assertEqual(0, entry.offset)
        self.assertEqual(3, entry.pad_before)
        self.assertEqual(3 + 5 + len(U_BOOT_DATA), entry.size)

        # Second u-boot has an aligned size, but it has no effect
        self.assertIn('u-boot-align-size-nop', entries)
        entry = entries['u-boot-align-size-nop']
        self.assertEqual(12, entry.offset)
        self.assertEqual(4, entry.size)

        # Third u-boot has an aligned size too
        self.assertIn('u-boot-align-size', entries)
        entry = entries['u-boot-align-size']
        self.assertEqual(16, entry.offset)
        self.assertEqual(32, entry.size)

        # Fourth u-boot has an aligned end
        self.assertIn('u-boot-align-end', entries)
        entry = entries['u-boot-align-end']
        self.assertEqual(48, entry.offset)
        self.assertEqual(16, entry.size)

        # Fifth u-boot immediately afterwards
        self.assertIn('u-boot-align-both', entries)
        entry = entries['u-boot-align-both']
        self.assertEqual(64, entry.offset)
        self.assertEqual(64, entry.size)

        self.CheckNoGaps(entries)
        self.assertEqual(128, image._size)

    def testPackAlignPowerOf2(self):
        """Test that invalid entry alignment is detected"""
        with self.assertRaises(ValueError) as e:
            self._DoTestFile('010_pack_align_power2.dts')
        self.assertIn("Node '/binman/u-boot': Alignment 5 must be a power "
                      "of two", str(e.exception))

    def testPackAlignSizePowerOf2(self):
        """Test that invalid entry size alignment is detected"""
        with self.assertRaises(ValueError) as e:
            self._DoTestFile('011_pack_align_size_power2.dts')
        self.assertIn("Node '/binman/u-boot': Alignment size 55 must be a "
                      "power of two", str(e.exception))

    def testPackInvalidAlign(self):
        """Test detection of an offset that does not match its alignment"""
        with self.assertRaises(ValueError) as e:
            self._DoTestFile('012_pack_inv_align.dts')
        self.assertIn("Node '/binman/u-boot': Offset 0x5 (5) does not match "
                      "align 0x4 (4)", str(e.exception))

    def testPackInvalidSizeAlign(self):
        """Test that invalid entry size alignment is detected"""
        with self.assertRaises(ValueError) as e:
            self._DoTestFile('013_pack_inv_size_align.dts')
        self.assertIn("Node '/binman/u-boot': Size 0x5 (5) does not match "
                      "align-size 0x4 (4)", str(e.exception))

    def testPackOverlap(self):
        """Test that overlapping regions are detected"""
        with self.assertRaises(ValueError) as e:
            self._DoTestFile('014_pack_overlap.dts')
        self.assertIn("Node '/binman/u-boot-align': Offset 0x3 (3) overlaps "
                      "with previous entry '/binman/u-boot' ending at 0x4 (4)",
                      str(e.exception))

    def testPackEntryOverflow(self):
        """Test that entries that overflow their size are detected"""
        with self.assertRaises(ValueError) as e:
            self._DoTestFile('015_pack_overflow.dts')
        self.assertIn("Node '/binman/u-boot': Entry contents size is 0x4 (4) "
                      "but entry size is 0x3 (3)", str(e.exception))

    def testPackImageOverflow(self):
        """Test that entries which overflow the image size are detected"""
        with self.assertRaises(ValueError) as e:
            self._DoTestFile('016_pack_image_overflow.dts')
        self.assertIn("Section '/binman': contents size 0x4 (4) exceeds section "
                      "size 0x3 (3)", str(e.exception))

    def testPackImageSize(self):
        """Test that the image size can be set"""
        retcode = self._DoTestFile('017_pack_image_size.dts')
        self.assertEqual(0, retcode)
        self.assertIn('image', control.images)
        image = control.images['image']
        self.assertEqual(7, image._size)

    def testPackImageSizeAlign(self):
        """Test that image size alignemnt works as expected"""
        retcode = self._DoTestFile('018_pack_image_align.dts')
        self.assertEqual(0, retcode)
        self.assertIn('image', control.images)
        image = control.images['image']
        self.assertEqual(16, image._size)

    def testPackInvalidImageAlign(self):
        """Test that invalid image alignment is detected"""
        with self.assertRaises(ValueError) as e:
            self._DoTestFile('019_pack_inv_image_align.dts')
        self.assertIn("Section '/binman': Size 0x7 (7) does not match "
                      "align-size 0x8 (8)", str(e.exception))

    def testPackAlignPowerOf2(self):
        """Test that invalid image alignment is detected"""
        with self.assertRaises(ValueError) as e:
            self._DoTestFile('020_pack_inv_image_align_power2.dts')
        self.assertIn("Section '/binman': Alignment size 131 must be a power of "
                      "two", str(e.exception))

    def testImagePadByte(self):
        """Test that the image pad byte can be specified"""
        self._SetupSplElf()
        data = self._DoReadFile('021_image_pad.dts')
        self.assertEqual(U_BOOT_SPL_DATA + (chr(0xff) * 1) + U_BOOT_DATA, data)

    def testImageName(self):
        """Test that image files can be named"""
        retcode = self._DoTestFile('022_image_name.dts')
        self.assertEqual(0, retcode)
        image = control.images['image1']
        fname = tools.GetOutputFilename('test-name')
        self.assertTrue(os.path.exists(fname))

        image = control.images['image2']
        fname = tools.GetOutputFilename('test-name.xx')
        self.assertTrue(os.path.exists(fname))

    def testBlobFilename(self):
        """Test that generic blobs can be provided by filename"""
        data = self._DoReadFile('023_blob.dts')
        self.assertEqual(BLOB_DATA, data)

    def testPackSorted(self):
        """Test that entries can be sorted"""
        self._SetupSplElf()
        data = self._DoReadFile('024_sorted.dts')
        self.assertEqual(chr(0) * 1 + U_BOOT_SPL_DATA + chr(0) * 2 +
                         U_BOOT_DATA, data)

    def testPackZeroOffset(self):
        """Test that an entry at offset 0 is not given a new offset"""
        with self.assertRaises(ValueError) as e:
            self._DoTestFile('025_pack_zero_size.dts')
        self.assertIn("Node '/binman/u-boot-spl': Offset 0x0 (0) overlaps "
                      "with previous entry '/binman/u-boot' ending at 0x4 (4)",
                      str(e.exception))

    def testPackUbootDtb(self):
        """Test that a device tree can be added to U-Boot"""
        data = self._DoReadFile('026_pack_u_boot_dtb.dts')
        self.assertEqual(U_BOOT_NODTB_DATA + U_BOOT_DTB_DATA, data)

    def testPackX86RomNoSize(self):
        """Test that the end-at-4gb property requires a size property"""
        with self.assertRaises(ValueError) as e:
            self._DoTestFile('027_pack_4gb_no_size.dts')
        self.assertIn("Section '/binman': Section size must be provided when "
                      "using end-at-4gb", str(e.exception))

    def test4gbAndSkipAtStartTogether(self):
        """Test that the end-at-4gb and skip-at-size property can't be used
        together"""
        with self.assertRaises(ValueError) as e:
            self._DoTestFile('80_4gb_and_skip_at_start_together.dts')
        self.assertIn("Section '/binman': Provide either 'end-at-4gb' or "
                      "'skip-at-start'", str(e.exception))

    def testPackX86RomOutside(self):
        """Test that the end-at-4gb property checks for offset boundaries"""
        with self.assertRaises(ValueError) as e:
            self._DoTestFile('028_pack_4gb_outside.dts')
        self.assertIn("Node '/binman/u-boot': Offset 0x0 (0) is outside "
                      "the section starting at 0xffffffe0 (4294967264)",
                      str(e.exception))

    def testPackX86Rom(self):
        """Test that a basic x86 ROM can be created"""
        self._SetupSplElf()
        data = self._DoReadFile('029_x86-rom.dts')
        self.assertEqual(U_BOOT_DATA + chr(0) * 7 + U_BOOT_SPL_DATA +
                         chr(0) * 2, data)

    def testPackX86RomMeNoDesc(self):
        """Test that an invalid Intel descriptor entry is detected"""
        TestFunctional._MakeInputFile('descriptor.bin', '')
        with self.assertRaises(ValueError) as e:
            self._DoTestFile('031_x86-rom-me.dts')
        self.assertIn("Node '/binman/intel-descriptor': Cannot find FD "
                      "signature", str(e.exception))

    def testPackX86RomBadDesc(self):
        """Test that the Intel requires a descriptor entry"""
        with self.assertRaises(ValueError) as e:
            self._DoTestFile('030_x86-rom-me-no-desc.dts')
        self.assertIn("Node '/binman/intel-me': No offset set with "
                      "offset-unset: should another entry provide this correct "
                      "offset?", str(e.exception))

    def testPackX86RomMe(self):
        """Test that an x86 ROM with an ME region can be created"""
        data = self._DoReadFile('031_x86-rom-me.dts')
        self.assertEqual(ME_DATA, data[0x1000:0x1000 + len(ME_DATA)])

    def testPackVga(self):
        """Test that an image with a VGA binary can be created"""
        data = self._DoReadFile('032_intel-vga.dts')
        self.assertEqual(VGA_DATA, data[:len(VGA_DATA)])

    def testPackStart16(self):
        """Test that an image with an x86 start16 region can be created"""
        data = self._DoReadFile('033_x86-start16.dts')
        self.assertEqual(X86_START16_DATA, data[:len(X86_START16_DATA)])

    def testPackPowerpcMpc85xxBootpgResetvec(self):
        """Test that an image with powerpc-mpc85xx-bootpg-resetvec can be
        created"""
        data = self._DoReadFile('81_powerpc_mpc85xx_bootpg_resetvec.dts')
        self.assertEqual(PPC_MPC85XX_BR_DATA, data[:len(PPC_MPC85XX_BR_DATA)])

    def _RunMicrocodeTest(self, dts_fname, nodtb_data, ucode_second=False):
        """Handle running a test for insertion of microcode

        Args:
            dts_fname: Name of test .dts file
            nodtb_data: Data that we expect in the first section
            ucode_second: True if the microsecond entry is second instead of
                third

        Returns:
            Tuple:
                Contents of first region (U-Boot or SPL)
                Offset and size components of microcode pointer, as inserted
                    in the above (two 4-byte words)
        """
        data = self._DoReadFile(dts_fname, True)

        # Now check the device tree has no microcode
        if ucode_second:
            ucode_content = data[len(nodtb_data):]
            ucode_pos = len(nodtb_data)
            dtb_with_ucode = ucode_content[16:]
            fdt_len = self.GetFdtLen(dtb_with_ucode)
        else:
            dtb_with_ucode = data[len(nodtb_data):]
            fdt_len = self.GetFdtLen(dtb_with_ucode)
            ucode_content = dtb_with_ucode[fdt_len:]
            ucode_pos = len(nodtb_data) + fdt_len
        fname = tools.GetOutputFilename('test.dtb')
        with open(fname, 'wb') as fd:
            fd.write(dtb_with_ucode)
        dtb = fdt.FdtScan(fname)
        ucode = dtb.GetNode('/microcode')
        self.assertTrue(ucode)
        for node in ucode.subnodes:
            self.assertFalse(node.props.get('data'))

        # Check that the microcode appears immediately after the Fdt
        # This matches the concatenation of the data properties in
        # the /microcode/update@xxx nodes in 34_x86_ucode.dts.
        ucode_data = struct.pack('>4L', 0x12345678, 0x12345679, 0xabcd0000,
                                 0x78235609)
        self.assertEqual(ucode_data, ucode_content[:len(ucode_data)])

        # Check that the microcode pointer was inserted. It should match the
        # expected offset and size
        pos_and_size = struct.pack('<2L', 0xfffffe00 + ucode_pos,
                                   len(ucode_data))
        u_boot = data[:len(nodtb_data)]
        return u_boot, pos_and_size

    def testPackUbootMicrocode(self):
        """Test that x86 microcode can be handled correctly

        We expect to see the following in the image, in order:
            u-boot-nodtb.bin with a microcode pointer inserted at the correct
                place
            u-boot.dtb with the microcode removed
            the microcode
        """
        first, pos_and_size = self._RunMicrocodeTest('034_x86_ucode.dts',
                                                     U_BOOT_NODTB_DATA)
        self.assertEqual('nodtb with microcode' + pos_and_size +
                         ' somewhere in here', first)

    def _RunPackUbootSingleMicrocode(self):
        """Test that x86 microcode can be handled correctly

        We expect to see the following in the image, in order:
            u-boot-nodtb.bin with a microcode pointer inserted at the correct
                place
            u-boot.dtb with the microcode
            an empty microcode region
        """
        # We need the libfdt library to run this test since only that allows
        # finding the offset of a property. This is required by
        # Entry_u_boot_dtb_with_ucode.ObtainContents().
        data = self._DoReadFile('035_x86_single_ucode.dts', True)

        second = data[len(U_BOOT_NODTB_DATA):]

        fdt_len = self.GetFdtLen(second)
        third = second[fdt_len:]
        second = second[:fdt_len]

        ucode_data = struct.pack('>2L', 0x12345678, 0x12345679)
        self.assertIn(ucode_data, second)
        ucode_pos = second.find(ucode_data) + len(U_BOOT_NODTB_DATA)

        # Check that the microcode pointer was inserted. It should match the
        # expected offset and size
        pos_and_size = struct.pack('<2L', 0xfffffe00 + ucode_pos,
                                   len(ucode_data))
        first = data[:len(U_BOOT_NODTB_DATA)]
        self.assertEqual('nodtb with microcode' + pos_and_size +
                         ' somewhere in here', first)

    def testPackUbootSingleMicrocode(self):
        """Test that x86 microcode can be handled correctly with fdt_normal.
        """
        self._RunPackUbootSingleMicrocode()

    def testUBootImg(self):
        """Test that u-boot.img can be put in a file"""
        data = self._DoReadFile('036_u_boot_img.dts')
        self.assertEqual(U_BOOT_IMG_DATA, data)

    def testNoMicrocode(self):
        """Test that a missing microcode region is detected"""
        with self.assertRaises(ValueError) as e:
            self._DoReadFile('037_x86_no_ucode.dts', True)
        self.assertIn("Node '/binman/u-boot-dtb-with-ucode': No /microcode "
                      "node found in ", str(e.exception))

    def testMicrocodeWithoutNode(self):
        """Test that a missing u-boot-dtb-with-ucode node is detected"""
        with self.assertRaises(ValueError) as e:
            self._DoReadFile('038_x86_ucode_missing_node.dts', True)
        self.assertIn("Node '/binman/u-boot-with-ucode-ptr': Cannot find "
                "microcode region u-boot-dtb-with-ucode", str(e.exception))

    def testMicrocodeWithoutNode2(self):
        """Test that a missing u-boot-ucode node is detected"""
        with self.assertRaises(ValueError) as e:
            self._DoReadFile('039_x86_ucode_missing_node2.dts', True)
        self.assertIn("Node '/binman/u-boot-with-ucode-ptr': Cannot find "
            "microcode region u-boot-ucode", str(e.exception))

    def testMicrocodeWithoutPtrInElf(self):
        """Test that a U-Boot binary without the microcode symbol is detected"""
        # ELF file without a '_dt_ucode_base_size' symbol
        try:
            with open(self.TestFile('u_boot_no_ucode_ptr')) as fd:
                TestFunctional._MakeInputFile('u-boot', fd.read())

            with self.assertRaises(ValueError) as e:
                self._RunPackUbootSingleMicrocode()
            self.assertIn("Node '/binman/u-boot-with-ucode-ptr': Cannot locate "
                    "_dt_ucode_base_size symbol in u-boot", str(e.exception))

        finally:
            # Put the original file back
            with open(self.TestFile('u_boot_ucode_ptr')) as fd:
                TestFunctional._MakeInputFile('u-boot', fd.read())

    def testMicrocodeNotInImage(self):
        """Test that microcode must be placed within the image"""
        with self.assertRaises(ValueError) as e:
            self._DoReadFile('040_x86_ucode_not_in_image.dts', True)
        self.assertIn("Node '/binman/u-boot-with-ucode-ptr': Microcode "
                "pointer _dt_ucode_base_size at fffffe14 is outside the "
                "section ranging from 00000000 to 0000002e", str(e.exception))

    def testWithoutMicrocode(self):
        """Test that we can cope with an image without microcode (e.g. qemu)"""
        with open(self.TestFile('u_boot_no_ucode_ptr')) as fd:
            TestFunctional._MakeInputFile('u-boot', fd.read())
        data, dtb, _, _ = self._DoReadFileDtb('044_x86_optional_ucode.dts', True)

        # Now check the device tree has no microcode
        self.assertEqual(U_BOOT_NODTB_DATA, data[:len(U_BOOT_NODTB_DATA)])
        second = data[len(U_BOOT_NODTB_DATA):]

        fdt_len = self.GetFdtLen(second)
        self.assertEqual(dtb, second[:fdt_len])

        used_len = len(U_BOOT_NODTB_DATA) + fdt_len
        third = data[used_len:]
        self.assertEqual(chr(0) * (0x200 - used_len), third)

    def testUnknownPosSize(self):
        """Test that microcode must be placed within the image"""
        with self.assertRaises(ValueError) as e:
            self._DoReadFile('041_unknown_pos_size.dts', True)
        self.assertIn("Section '/binman': Unable to set offset/size for unknown "
                "entry 'invalid-entry'", str(e.exception))

    def testPackFsp(self):
        """Test that an image with a FSP binary can be created"""
        data = self._DoReadFile('042_intel-fsp.dts')
        self.assertEqual(FSP_DATA, data[:len(FSP_DATA)])

    def testPackCmc(self):
        """Test that an image with a CMC binary can be created"""
        data = self._DoReadFile('043_intel-cmc.dts')
        self.assertEqual(CMC_DATA, data[:len(CMC_DATA)])

    def testPackVbt(self):
        """Test that an image with a VBT binary can be created"""
        data = self._DoReadFile('046_intel-vbt.dts')
        self.assertEqual(VBT_DATA, data[:len(VBT_DATA)])

    def testSplBssPad(self):
        """Test that we can pad SPL's BSS with zeros"""
        # ELF file with a '__bss_size' symbol
        self._SetupSplElf()
        data = self._DoReadFile('047_spl_bss_pad.dts')
        self.assertEqual(U_BOOT_SPL_DATA + (chr(0) * 10) + U_BOOT_DATA, data)

    def testSplBssPadMissing(self):
        """Test that a missing symbol is detected"""
        self._SetupSplElf('u_boot_ucode_ptr')
        with self.assertRaises(ValueError) as e:
            self._DoReadFile('047_spl_bss_pad.dts')
        self.assertIn('Expected __bss_size symbol in spl/u-boot-spl',
                      str(e.exception))

    def testPackStart16Spl(self):
        """Test that an image with an x86 start16 SPL region can be created"""
        data = self._DoReadFile('048_x86-start16-spl.dts')
        self.assertEqual(X86_START16_SPL_DATA, data[:len(X86_START16_SPL_DATA)])

    def _PackUbootSplMicrocode(self, dts, ucode_second=False):
        """Helper function for microcode tests

        We expect to see the following in the image, in order:
            u-boot-spl-nodtb.bin with a microcode pointer inserted at the
                correct place
            u-boot.dtb with the microcode removed
            the microcode

        Args:
            dts: Device tree file to use for test
            ucode_second: True if the microsecond entry is second instead of
                third
        """
        self._SetupSplElf('u_boot_ucode_ptr')
        first, pos_and_size = self._RunMicrocodeTest(dts, U_BOOT_SPL_NODTB_DATA,
                                                     ucode_second=ucode_second)
        self.assertEqual('splnodtb with microc' + pos_and_size +
                         'ter somewhere in here', first)

    def testPackUbootSplMicrocode(self):
        """Test that x86 microcode can be handled correctly in SPL"""
        self._PackUbootSplMicrocode('049_x86_ucode_spl.dts')

    def testPackUbootSplMicrocodeReorder(self):
        """Test that order doesn't matter for microcode entries

        This is the same as testPackUbootSplMicrocode but when we process the
        u-boot-ucode entry we have not yet seen the u-boot-dtb-with-ucode
        entry, so we reply on binman to try later.
        """
        self._PackUbootSplMicrocode('058_x86_ucode_spl_needs_retry.dts',
                                    ucode_second=True)

    def testPackMrc(self):
        """Test that an image with an MRC binary can be created"""
        data = self._DoReadFile('050_intel_mrc.dts')
        self.assertEqual(MRC_DATA, data[:len(MRC_DATA)])

    def testSplDtb(self):
        """Test that an image with spl/u-boot-spl.dtb can be created"""
        data = self._DoReadFile('051_u_boot_spl_dtb.dts')
        self.assertEqual(U_BOOT_SPL_DTB_DATA, data[:len(U_BOOT_SPL_DTB_DATA)])

    def testSplNoDtb(self):
        """Test that an image with spl/u-boot-spl-nodtb.bin can be created"""
        data = self._DoReadFile('052_u_boot_spl_nodtb.dts')
        self.assertEqual(U_BOOT_SPL_NODTB_DATA, data[:len(U_BOOT_SPL_NODTB_DATA)])

    def testSymbols(self):
        """Test binman can assign symbols embedded in U-Boot"""
        elf_fname = self.TestFile('u_boot_binman_syms')
        syms = elf.GetSymbols(elf_fname, ['binman', 'image'])
        addr = elf.GetSymbolAddress(elf_fname, '__image_copy_start')
        self.assertEqual(syms['_binman_u_boot_spl_prop_offset'].address, addr)

        self._SetupSplElf('u_boot_binman_syms')
        data = self._DoReadFile('053_symbols.dts')
        sym_values = struct.pack('<LQL', 0x24 + 0, 0x24 + 24, 0x24 + 20)
        expected = (sym_values + U_BOOT_SPL_DATA[16:] + chr(0xff) +
                    U_BOOT_DATA +
                    sym_values + U_BOOT_SPL_DATA[16:])
        self.assertEqual(expected, data)

    def testPackUnitAddress(self):
        """Test that we support multiple binaries with the same name"""
        data = self._DoReadFile('054_unit_address.dts')
        self.assertEqual(U_BOOT_DATA + U_BOOT_DATA, data)

    def testSections(self):
        """Basic test of sections"""
        data = self._DoReadFile('055_sections.dts')
        expected = (U_BOOT_DATA + '!' * 12 + U_BOOT_DATA + 'a' * 12 +
                    U_BOOT_DATA + '&' * 4)
        self.assertEqual(expected, data)

    def testMap(self):
        """Tests outputting a map of the images"""
        _, _, map_data, _ = self._DoReadFileDtb('055_sections.dts', map=True)
        self.assertEqual('''ImagePos    Offset      Size  Name
00000000  00000000  00000028  main-section
00000000   00000000  00000010  section@0
00000000    00000000  00000004  u-boot
00000010   00000010  00000010  section@1
00000010    00000000  00000004  u-boot
00000020   00000020  00000004  section@2
00000020    00000000  00000004  u-boot
''', map_data)

    def testNamePrefix(self):
        """Tests that name prefixes are used"""
        _, _, map_data, _ = self._DoReadFileDtb('056_name_prefix.dts', map=True)
        self.assertEqual('''ImagePos    Offset      Size  Name
00000000  00000000  00000028  main-section
00000000   00000000  00000010  section@0
00000000    00000000  00000004  ro-u-boot
00000010   00000010  00000010  section@1
00000010    00000000  00000004  rw-u-boot
''', map_data)

    def testUnknownContents(self):
        """Test that obtaining the contents works as expected"""
        with self.assertRaises(ValueError) as e:
            self._DoReadFile('057_unknown_contents.dts', True)
        self.assertIn("Section '/binman': Internal error: Could not complete "
                "processing of contents: remaining [<_testing.Entry__testing ",
                str(e.exception))

    def testBadChangeSize(self):
        """Test that trying to change the size of an entry fails"""
        with self.assertRaises(ValueError) as e:
            self._DoReadFile('059_change_size.dts', True)
        self.assertIn("Node '/binman/_testing': Cannot update entry size from "
                      '2 to 1', str(e.exception))

    def testUpdateFdt(self):
        """Test that we can update the device tree with offset/size info"""
        _, _, _, out_dtb_fname = self._DoReadFileDtb('060_fdt_update.dts',
                                                     update_dtb=True)
        dtb = fdt.Fdt(out_dtb_fname)
        dtb.Scan()
        props = self._GetPropTree(dtb, ['offset', 'size', 'image-pos'])
        self.assertEqual({
            'image-pos': 0,
            'offset': 0,
            '_testing:offset': 32,
            '_testing:size': 1,
            '_testing:image-pos': 32,
            'section@0/u-boot:offset': 0,
            'section@0/u-boot:size': len(U_BOOT_DATA),
            'section@0/u-boot:image-pos': 0,
            'section@0:offset': 0,
            'section@0:size': 16,
            'section@0:image-pos': 0,

            'section@1/u-boot:offset': 0,
            'section@1/u-boot:size': len(U_BOOT_DATA),
            'section@1/u-boot:image-pos': 16,
            'section@1:offset': 16,
            'section@1:size': 16,
            'section@1:image-pos': 16,
            'size': 40
        }, props)

    def testUpdateFdtBad(self):
        """Test that we detect when ProcessFdt never completes"""
        with self.assertRaises(ValueError) as e:
            self._DoReadFileDtb('061_fdt_update_bad.dts', update_dtb=True)
        self.assertIn('Could not complete processing of Fdt: remaining '
                      '[<_testing.Entry__testing', str(e.exception))

    def testEntryArgs(self):
        """Test passing arguments to entries from the command line"""
        entry_args = {
            'test-str-arg': 'test1',
            'test-int-arg': '456',
        }
        self._DoReadFileDtb('062_entry_args.dts', entry_args=entry_args)
        self.assertIn('image', control.images)
        entry = control.images['image'].GetEntries()['_testing']
        self.assertEqual('test0', entry.test_str_fdt)
        self.assertEqual('test1', entry.test_str_arg)
        self.assertEqual(123, entry.test_int_fdt)
        self.assertEqual(456, entry.test_int_arg)

    def testEntryArgsMissing(self):
        """Test missing arguments and properties"""
        entry_args = {
            'test-int-arg': '456',
        }
        self._DoReadFileDtb('063_entry_args_missing.dts', entry_args=entry_args)
        entry = control.images['image'].GetEntries()['_testing']
        self.assertEqual('test0', entry.test_str_fdt)
        self.assertEqual(None, entry.test_str_arg)
        self.assertEqual(None, entry.test_int_fdt)
        self.assertEqual(456, entry.test_int_arg)

    def testEntryArgsRequired(self):
        """Test missing arguments and properties"""
        entry_args = {
            'test-int-arg': '456',
        }
        with self.assertRaises(ValueError) as e:
            self._DoReadFileDtb('064_entry_args_required.dts')
        self.assertIn("Node '/binman/_testing': Missing required "
            'properties/entry args: test-str-arg, test-int-fdt, test-int-arg',
            str(e.exception))

    def testEntryArgsInvalidFormat(self):
        """Test that an invalid entry-argument format is detected"""
        args = ['-d', self.TestFile('064_entry_args_required.dts'), '-ano-value']
        with self.assertRaises(ValueError) as e:
            self._DoBinman(*args)
        self.assertIn("Invalid entry arguemnt 'no-value'", str(e.exception))

    def testEntryArgsInvalidInteger(self):
        """Test that an invalid entry-argument integer is detected"""
        entry_args = {
            'test-int-arg': 'abc',
        }
        with self.assertRaises(ValueError) as e:
            self._DoReadFileDtb('062_entry_args.dts', entry_args=entry_args)
        self.assertIn("Node '/binman/_testing': Cannot convert entry arg "
                      "'test-int-arg' (value 'abc') to integer",
            str(e.exception))

    def testEntryArgsInvalidDatatype(self):
        """Test that an invalid entry-argument datatype is detected

        This test could be written in entry_test.py except that it needs
        access to control.entry_args, which seems more than that module should
        be able to see.
        """
        entry_args = {
            'test-bad-datatype-arg': '12',
        }
        with self.assertRaises(ValueError) as e:
            self._DoReadFileDtb('065_entry_args_unknown_datatype.dts',
                                entry_args=entry_args)
        self.assertIn('GetArg() internal error: Unknown data type ',
                      str(e.exception))

    def testText(self):
        """Test for a text entry type"""
        entry_args = {
            'test-id': TEXT_DATA,
            'test-id2': TEXT_DATA2,
            'test-id3': TEXT_DATA3,
        }
        data, _, _, _ = self._DoReadFileDtb('066_text.dts',
                                            entry_args=entry_args)
        expected = (TEXT_DATA + chr(0) * (8 - len(TEXT_DATA)) + TEXT_DATA2 +
                    TEXT_DATA3 + 'some text')
        self.assertEqual(expected, data)

    def testEntryDocs(self):
        """Test for creation of entry documentation"""
        with test_util.capture_sys_output() as (stdout, stderr):
            control.WriteEntryDocs(binman.GetEntryModules())
        self.assertTrue(len(stdout.getvalue()) > 0)

    def testEntryDocsMissing(self):
        """Test handling of missing entry documentation"""
        with self.assertRaises(ValueError) as e:
            with test_util.capture_sys_output() as (stdout, stderr):
                control.WriteEntryDocs(binman.GetEntryModules(), 'u_boot')
        self.assertIn('Documentation is missing for modules: u_boot',
                      str(e.exception))

    def testFmap(self):
        """Basic test of generation of a flashrom fmap"""
        data = self._DoReadFile('067_fmap.dts')
        fhdr, fentries = fmap_util.DecodeFmap(data[32:])
        expected = U_BOOT_DATA + '!' * 12 + U_BOOT_DATA + 'a' * 12
        self.assertEqual(expected, data[:32])
        self.assertEqual('__FMAP__', fhdr.signature)
        self.assertEqual(1, fhdr.ver_major)
        self.assertEqual(0, fhdr.ver_minor)
        self.assertEqual(0, fhdr.base)
        self.assertEqual(16 + 16 +
                         fmap_util.FMAP_HEADER_LEN +
                         fmap_util.FMAP_AREA_LEN * 3, fhdr.image_size)
        self.assertEqual('FMAP', fhdr.name)
        self.assertEqual(3, fhdr.nareas)
        for fentry in fentries:
            self.assertEqual(0, fentry.flags)

        self.assertEqual(0, fentries[0].offset)
        self.assertEqual(4, fentries[0].size)
        self.assertEqual('RO_U_BOOT', fentries[0].name)

        self.assertEqual(16, fentries[1].offset)
        self.assertEqual(4, fentries[1].size)
        self.assertEqual('RW_U_BOOT', fentries[1].name)

        self.assertEqual(32, fentries[2].offset)
        self.assertEqual(fmap_util.FMAP_HEADER_LEN +
                         fmap_util.FMAP_AREA_LEN * 3, fentries[2].size)
        self.assertEqual('FMAP', fentries[2].name)

    def testBlobNamedByArg(self):
        """Test we can add a blob with the filename coming from an entry arg"""
        entry_args = {
            'cros-ec-rw-path': 'ecrw.bin',
        }
        data, _, _, _ = self._DoReadFileDtb('068_blob_named_by_arg.dts',
                                            entry_args=entry_args)

    def testFill(self):
        """Test for an fill entry type"""
        data = self._DoReadFile('069_fill.dts')
        expected = 8 * chr(0xff) + 8 * chr(0)
        self.assertEqual(expected, data)

    def testFillNoSize(self):
        """Test for an fill entry type with no size"""
        with self.assertRaises(ValueError) as e:
            self._DoReadFile('070_fill_no_size.dts')
        self.assertIn("'fill' entry must have a size property",
                      str(e.exception))

    def _HandleGbbCommand(self, pipe_list):
        """Fake calls to the futility utility"""
        if pipe_list[0][0] == 'futility':
            fname = pipe_list[0][-1]
            # Append our GBB data to the file, which will happen every time the
            # futility command is called.
            with open(fname, 'a') as fd:
                fd.write(GBB_DATA)
            return command.CommandResult()

    def testGbb(self):
        """Test for the Chromium OS Google Binary Block"""
        command.test_result = self._HandleGbbCommand
        entry_args = {
            'keydir': 'devkeys',
            'bmpblk': 'bmpblk.bin',
        }
        data, _, _, _ = self._DoReadFileDtb('071_gbb.dts', entry_args=entry_args)

        # Since futility
        expected = GBB_DATA + GBB_DATA + 8 * chr(0) + (0x2180 - 16) * chr(0)
        self.assertEqual(expected, data)

    def testGbbTooSmall(self):
        """Test for the Chromium OS Google Binary Block being large enough"""
        with self.assertRaises(ValueError) as e:
            self._DoReadFileDtb('072_gbb_too_small.dts')
        self.assertIn("Node '/binman/gbb': GBB is too small",
                      str(e.exception))

    def testGbbNoSize(self):
        """Test for the Chromium OS Google Binary Block having a size"""
        with self.assertRaises(ValueError) as e:
            self._DoReadFileDtb('073_gbb_no_size.dts')
        self.assertIn("Node '/binman/gbb': GBB must have a fixed size",
                      str(e.exception))

    def _HandleVblockCommand(self, pipe_list):
        """Fake calls to the futility utility"""
        if pipe_list[0][0] == 'futility':
            fname = pipe_list[0][3]
            with open(fname, 'wb') as fd:
                fd.write(VBLOCK_DATA)
            return command.CommandResult()

    def testVblock(self):
        """Test for the Chromium OS Verified Boot Block"""
        command.test_result = self._HandleVblockCommand
        entry_args = {
            'keydir': 'devkeys',
        }
        data, _, _, _ = self._DoReadFileDtb('074_vblock.dts',
                                            entry_args=entry_args)
        expected = U_BOOT_DATA + VBLOCK_DATA + U_BOOT_DTB_DATA
        self.assertEqual(expected, data)

    def testVblockNoContent(self):
        """Test we detect a vblock which has no content to sign"""
        with self.assertRaises(ValueError) as e:
            self._DoReadFile('075_vblock_no_content.dts')
        self.assertIn("Node '/binman/vblock': Vblock must have a 'content' "
                      'property', str(e.exception))

    def testVblockBadPhandle(self):
        """Test that we detect a vblock with an invalid phandle in contents"""
        with self.assertRaises(ValueError) as e:
            self._DoReadFile('076_vblock_bad_phandle.dts')
        self.assertIn("Node '/binman/vblock': Cannot find node for phandle "
                      '1000', str(e.exception))

    def testVblockBadEntry(self):
        """Test that we detect an entry that points to a non-entry"""
        with self.assertRaises(ValueError) as e:
            self._DoReadFile('077_vblock_bad_entry.dts')
        self.assertIn("Node '/binman/vblock': Cannot find entry for node "
                      "'other'", str(e.exception))

    def testTpl(self):
        """Test that an image with TPL and ots device tree can be created"""
        # ELF file with a '__bss_size' symbol
        with open(self.TestFile('bss_data')) as fd:
            TestFunctional._MakeInputFile('tpl/u-boot-tpl', fd.read())
        data = self._DoReadFile('078_u_boot_tpl.dts')
        self.assertEqual(U_BOOT_TPL_DATA + U_BOOT_TPL_DTB_DATA, data)

    def testUsesPos(self):
        """Test that the 'pos' property cannot be used anymore"""
        with self.assertRaises(ValueError) as e:
           data = self._DoReadFile('079_uses_pos.dts')
        self.assertIn("Node '/binman/u-boot': Please use 'offset' instead of "
                      "'pos'", str(e.exception))

    def testFillZero(self):
        """Test for an fill entry type with a size of 0"""
        data = self._DoReadFile('080_fill_empty.dts')
        self.assertEqual(chr(0) * 16, data)

    def testTextMissing(self):
        """Test for a text entry type where there is no text"""
        with self.assertRaises(ValueError) as e:
            self._DoReadFileDtb('066_text.dts',)
        self.assertIn("Node '/binman/text': No value provided for text label "
                      "'test-id'", str(e.exception))

    def testPackStart16Tpl(self):
        """Test that an image with an x86 start16 TPL region can be created"""
        data = self._DoReadFile('081_x86-start16-tpl.dts')
        self.assertEqual(X86_START16_TPL_DATA, data[:len(X86_START16_TPL_DATA)])

    def testSelectImage(self):
        """Test that we can select which images to build"""
        expected = 'Skipping images: image1'

        # We should only get the expected message in verbose mode
        for verbosity in (None, 2):
            with test_util.capture_sys_output() as (stdout, stderr):
                retcode = self._DoTestFile('006_dual_image.dts',
                                           verbosity=verbosity,
                                           images=['image2'])
            self.assertEqual(0, retcode)
            if verbosity:
                self.assertIn(expected, stdout.getvalue())
            else:
                self.assertNotIn(expected, stdout.getvalue())

            self.assertFalse(os.path.exists(tools.GetOutputFilename('image1.bin')))
            self.assertTrue(os.path.exists(tools.GetOutputFilename('image2.bin')))

    def testUpdateFdtAll(self):
        """Test that all device trees are updated with offset/size info"""
        data, _, _, _ = self._DoReadFileDtb('082_fdt_update_all.dts',
                                            use_real_dtb=True, update_dtb=True)

        base_expected = {
            'section:image-pos': 0,
            'u-boot-tpl-dtb:size': 513,
            'u-boot-spl-dtb:size': 513,
            'u-boot-spl-dtb:offset': 493,
            'image-pos': 0,
            'section/u-boot-dtb:image-pos': 0,
            'u-boot-spl-dtb:image-pos': 493,
            'section/u-boot-dtb:size': 493,
            'u-boot-tpl-dtb:image-pos': 1006,
            'section/u-boot-dtb:offset': 0,
            'section:size': 493,
            'offset': 0,
            'section:offset': 0,
            'u-boot-tpl-dtb:offset': 1006,
            'size': 1519
        }

        # We expect three device-tree files in the output, one after the other.
        # Read them in sequence. We look for an 'spl' property in the SPL tree,
        # and 'tpl' in the TPL tree, to make sure they are distinct from the
        # main U-Boot tree. All three should have the same postions and offset.
        start = 0
        for item in ['', 'spl', 'tpl']:
            dtb = fdt.Fdt.FromData(data[start:])
            dtb.Scan()
            props = self._GetPropTree(dtb, ['offset', 'size', 'image-pos',
                                            'spl', 'tpl'])
            expected = dict(base_expected)
            if item:
                expected[item] = 0
            self.assertEqual(expected, props)
            start += dtb._fdt_obj.totalsize()

    def testUpdateFdtOutput(self):
        """Test that output DTB files are updated"""
        try:
            data, dtb_data, _, _ = self._DoReadFileDtb('082_fdt_update_all.dts',
                    use_real_dtb=True, update_dtb=True, reset_dtbs=False)

            # Unfortunately, compiling a source file always results in a file
            # called source.dtb (see fdt_util.EnsureCompiled()). The test
            # source file (e.g. test/075_fdt_update_all.dts) thus does not enter
            # binman as a file called u-boot.dtb. To fix this, copy the file
            # over to the expected place.
            #tools.WriteFile(os.path.join(self._indir, 'u-boot.dtb'),
                    #tools.ReadFile(tools.GetOutputFilename('source.dtb')))
            start = 0
            for fname in ['u-boot.dtb.out', 'spl/u-boot-spl.dtb.out',
                          'tpl/u-boot-tpl.dtb.out']:
                dtb = fdt.Fdt.FromData(data[start:])
                size = dtb._fdt_obj.totalsize()
                pathname = tools.GetOutputFilename(os.path.split(fname)[1])
                outdata = tools.ReadFile(pathname)
                name = os.path.split(fname)[0]

                if name:
                    orig_indata = self._GetDtbContentsForSplTpl(dtb_data, name)
                else:
                    orig_indata = dtb_data
                self.assertNotEqual(outdata, orig_indata,
                        "Expected output file '%s' be updated" % pathname)
                self.assertEqual(outdata, data[start:start + size],
                        "Expected output file '%s' to match output image" %
                        pathname)
                start += size
        finally:
            self._ResetDtbs()

    def _decompress(self, data):
        out = os.path.join(self._indir, 'lz4.tmp')
        with open(out, 'wb') as fd:
            fd.write(data)
        return tools.Run('lz4', '-dc', out)
        '''
        try:
            orig = lz4.frame.decompress(data)
        except AttributeError:
            orig = lz4.decompress(data)
        '''

    def testCompress(self):
        """Test compression of blobs"""
        data, _, _, out_dtb_fname = self._DoReadFileDtb('083_compress.dts',
                                            use_real_dtb=True, update_dtb=True)
        dtb = fdt.Fdt(out_dtb_fname)
        dtb.Scan()
        props = self._GetPropTree(dtb, ['size', 'uncomp-size'])
        orig = self._decompress(data)
        self.assertEquals(COMPRESS_DATA, orig)
        expected = {
            'blob:uncomp-size': len(COMPRESS_DATA),
            'blob:size': len(data),
            'size': len(data),
            }
        self.assertEqual(expected, props)

    def testFiles(self):
        """Test bringing in multiple files"""
        data = self._DoReadFile('084_files.dts')
        self.assertEqual(FILES_DATA, data)

    def testFilesCompress(self):
        """Test bringing in multiple files and compressing them"""
        data = self._DoReadFile('085_files_compress.dts')

        image = control.images['image']
        entries = image.GetEntries()
        files = entries['files']
        entries = files._section._entries

        orig = ''
        for i in range(1, 3):
            key = '%d.dat' % i
            start = entries[key].image_pos
            len = entries[key].size
            chunk = data[start:start + len]
            orig += self._decompress(chunk)

        self.assertEqual(FILES_DATA, orig)

    def testFilesMissing(self):
        """Test missing files"""
        with self.assertRaises(ValueError) as e:
            data = self._DoReadFile('086_files_none.dts')
        self.assertIn("Node '/binman/files': Pattern \'files/*.none\' matched "
                      'no files', str(e.exception))

    def testFilesNoPattern(self):
        """Test missing files"""
        with self.assertRaises(ValueError) as e:
            data = self._DoReadFile('087_files_no_pattern.dts')
        self.assertIn("Node '/binman/files': Missing 'pattern' property",
                      str(e.exception))

    def testExpandSize(self):
        """Test an expanding entry"""
        data, _, map_data, _ = self._DoReadFileDtb('088_expand_size.dts',
                                                   map=True)
        expect = ('a' * 8 + U_BOOT_DATA +
                  MRC_DATA + 'b' * 1 + U_BOOT_DATA +
                  'c' * 8 + U_BOOT_DATA +
                  'd' * 8)
        self.assertEqual(expect, data)
        self.assertEqual('''ImagePos    Offset      Size  Name
00000000  00000000  00000028  main-section
00000000   00000000  00000008  fill
00000008   00000008  00000004  u-boot
0000000c   0000000c  00000004  section
0000000c    00000000  00000003  intel-mrc
00000010   00000010  00000004  u-boot2
00000014   00000014  0000000c  section2
00000014    00000000  00000008  fill
0000001c    00000008  00000004  u-boot
00000020   00000020  00000008  fill2
''', map_data)

    def testExpandSizeBad(self):
        """Test an expanding entry which fails to provide contents"""
        with test_util.capture_sys_output() as (stdout, stderr):
            with self.assertRaises(ValueError) as e:
                self._DoReadFileDtb('089_expand_size_bad.dts', map=True)
        self.assertIn("Node '/binman/_testing': Cannot obtain contents when "
                      'expanding entry', str(e.exception))

    def testHash(self):
        """Test hashing of the contents of an entry"""
        _, _, _, out_dtb_fname = self._DoReadFileDtb('090_hash.dts',
                use_real_dtb=True, update_dtb=True)
        dtb = fdt.Fdt(out_dtb_fname)
        dtb.Scan()
        hash_node = dtb.GetNode('/binman/u-boot/hash').props['value']
        m = hashlib.sha256()
        m.update(U_BOOT_DATA)
        self.assertEqual(m.digest(), ''.join(hash_node.value))

    def testHashNoAlgo(self):
        with self.assertRaises(ValueError) as e:
            self._DoReadFileDtb('091_hash_no_algo.dts', update_dtb=True)
        self.assertIn("Node \'/binman/u-boot\': Missing \'algo\' property for "
                      'hash node', str(e.exception))

    def testHashBadAlgo(self):
        with self.assertRaises(ValueError) as e:
            self._DoReadFileDtb('092_hash_bad_algo.dts', update_dtb=True)
        self.assertIn("Node '/binman/u-boot': Unknown hash algorithm",
                      str(e.exception))

    def testHashSection(self):
        """Test hashing of the contents of an entry"""
        _, _, _, out_dtb_fname = self._DoReadFileDtb('099_hash_section.dts',
                use_real_dtb=True, update_dtb=True)
        dtb = fdt.Fdt(out_dtb_fname)
        dtb.Scan()
        hash_node = dtb.GetNode('/binman/section/hash').props['value']
        m = hashlib.sha256()
        m.update(U_BOOT_DATA)
        m.update(16 * 'a')
        self.assertEqual(m.digest(), ''.join(hash_node.value))

    def testPackUBootTplMicrocode(self):
        """Test that x86 microcode can be handled correctly in TPL

        We expect to see the following in the image, in order:
            u-boot-tpl-nodtb.bin with a microcode pointer inserted at the correct
                place
            u-boot-tpl.dtb with the microcode removed
            the microcode
        """
        with open(self.TestFile('u_boot_ucode_ptr')) as fd:
            TestFunctional._MakeInputFile('tpl/u-boot-tpl', fd.read())
        first, pos_and_size = self._RunMicrocodeTest('093_x86_tpl_ucode.dts',
                                                     U_BOOT_TPL_NODTB_DATA)
        self.assertEqual('tplnodtb with microc' + pos_and_size +
                         'ter somewhere in here', first)

    def testFmapX86(self):
        """Basic test of generation of a flashrom fmap"""
        data = self._DoReadFile('094_fmap_x86.dts')
        fhdr, fentries = fmap_util.DecodeFmap(data[32:])
        expected = U_BOOT_DATA + MRC_DATA + 'a' * (32 - 7)
        self.assertEqual(expected, data[:32])
        fhdr, fentries = fmap_util.DecodeFmap(data[32:])

        self.assertEqual(0x100, fhdr.image_size)

        self.assertEqual(0, fentries[0].offset)
        self.assertEqual(4, fentries[0].size)
        self.assertEqual('U_BOOT', fentries[0].name)

        self.assertEqual(4, fentries[1].offset)
        self.assertEqual(3, fentries[1].size)
        self.assertEqual('INTEL_MRC', fentries[1].name)

        self.assertEqual(32, fentries[2].offset)
        self.assertEqual(fmap_util.FMAP_HEADER_LEN +
                         fmap_util.FMAP_AREA_LEN * 3, fentries[2].size)
        self.assertEqual('FMAP', fentries[2].name)

    def testFmapX86Section(self):
        """Basic test of generation of a flashrom fmap"""
        data = self._DoReadFile('095_fmap_x86_section.dts')
        expected = U_BOOT_DATA + MRC_DATA + 'b' * (32 - 7)
        self.assertEqual(expected, data[:32])
        fhdr, fentries = fmap_util.DecodeFmap(data[36:])

        self.assertEqual(0x100, fhdr.image_size)

        self.assertEqual(0, fentries[0].offset)
        self.assertEqual(4, fentries[0].size)
        self.assertEqual('U_BOOT', fentries[0].name)

        self.assertEqual(4, fentries[1].offset)
        self.assertEqual(3, fentries[1].size)
        self.assertEqual('INTEL_MRC', fentries[1].name)

        self.assertEqual(36, fentries[2].offset)
        self.assertEqual(fmap_util.FMAP_HEADER_LEN +
                         fmap_util.FMAP_AREA_LEN * 3, fentries[2].size)
        self.assertEqual('FMAP', fentries[2].name)

    def testElf(self):
        """Basic test of ELF entries"""
        self._SetupSplElf()
        with open(self.TestFile('bss_data')) as fd:
            TestFunctional._MakeInputFile('-boot', fd.read())
        data = self._DoReadFile('096_elf.dts')

    def testElfStripg(self):
        """Basic test of ELF entries"""
        self._SetupSplElf()
        with open(self.TestFile('bss_data')) as fd:
            TestFunctional._MakeInputFile('-boot', fd.read())
        data = self._DoReadFile('097_elf_strip.dts')

    def testPackOverlapMap(self):
        """Test that overlapping regions are detected"""
        with test_util.capture_sys_output() as (stdout, stderr):
            with self.assertRaises(ValueError) as e:
                self._DoTestFile('014_pack_overlap.dts', map=True)
        map_fname = tools.GetOutputFilename('image.map')
        self.assertEqual("Wrote map file '%s' to show errors\n" % map_fname,
                         stdout.getvalue())

        # We should not get an inmage, but there should be a map file
        self.assertFalse(os.path.exists(tools.GetOutputFilename('image.bin')))
        self.assertTrue(os.path.exists(map_fname))
        map_data = tools.ReadFile(map_fname)
        self.assertEqual('''ImagePos    Offset      Size  Name
<none>    00000000  00000007  main-section
<none>     00000000  00000004  u-boot
<none>     00000003  00000004  u-boot-align
''', map_data)

    def testPacRefCode(self):
        """Test that an image with an Intel Reference code binary works"""
        data = self._DoReadFile('100_intel_refcode.dts')
        self.assertEqual(REFCODE_DATA, data[:len(REFCODE_DATA)])

    def testSectionOffset(self):
        """Tests use of a section with an offset"""
        data, _, map_data, _ = self._DoReadFileDtb('101_sections_offset.dts',
                                                   map=True)
        self.assertEqual('''ImagePos    Offset      Size  Name
00000000  00000000  00000038  main-section
00000004   00000004  00000010  section@0
00000004    00000000  00000004  u-boot
00000018   00000018  00000010  section@1
00000018    00000000  00000004  u-boot
0000002c   0000002c  00000004  section@2
0000002c    00000000  00000004  u-boot
''', map_data)
        self.assertEqual(data,
                         4 * chr(0x26) + U_BOOT_DATA + 12 * chr(0x21) +
                         4 * chr(0x26) + U_BOOT_DATA + 12 * chr(0x61) +
                         4 * chr(0x26) + U_BOOT_DATA + 8 * chr(0x26))


if __name__ == "__main__":
    unittest.main()
