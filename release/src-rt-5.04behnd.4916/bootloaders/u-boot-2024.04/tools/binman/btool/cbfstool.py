# SPDX-License-Identifier: GPL-2.0+
# Copyright 2022 Google LLC
#
"""Bintool implementation for cbfstool

cfstool provides a number of features useful with Coreboot Filesystem binaries.

Documentation is at https://www.coreboot.org/CBFS

Source code is at https://github.com/coreboot/coreboot/blob/master/util/cbfstool/cbfstool.c

Here is the help:

cbfstool: Management utility for CBFS formatted ROM images

USAGE:
 cbfstool [-h]
 cbfstool FILE COMMAND [-v] [PARAMETERS]...

OPTIONs:
  -H header_offset Do not search for header; use this offset*
  -T               Output top-aligned memory address
  -u               Accept short data; fill upward/from bottom
  -d               Accept short data; fill downward/from top
  -F               Force action
  -g               Generate position and alignment arguments
  -U               Unprocessed; don't decompress or make ELF
  -v               Provide verbose output
  -h               Display this help message

COMMANDs:
 add [-r image,regions] -f FILE -n NAME -t TYPE [-A hash] \
        [-c compression] [-b base-address | -a alignment] \
        [-p padding size] [-y|--xip if TYPE is FSP]       \
        [-j topswap-size] (Intel CPUs only) [--ibb]
        Add a component
        -j valid size: 0x10000 0x20000 0x40000 0x80000 0x100000
 add-payload [-r image,regions] -f FILE -n NAME [-A hash] \
        [-c compression] [-b base-address] \
        (linux specific: [-C cmdline] [-I initrd])
        Add a payload to the ROM
 add-stage [-r image,regions] -f FILE -n NAME [-A hash] \
        [-c compression] [-b base] [-S section-to-ignore] \
        [-a alignment] [-y|--xip] [-P page-size] [--ibb]
        Add a stage to the ROM
 add-flat-binary [-r image,regions] -f FILE -n NAME \
        [-A hash] -l load-address -e entry-point \
        [-c compression] [-b base]
        Add a 32bit flat mode binary
 add-int [-r image,regions] -i INTEGER -n NAME [-b base]
 Add a raw 64-bit integer value
 add-master-header [-r image,regions] \
        [-j topswap-size] (Intel CPUs only)
        Add a legacy CBFS master header
 remove [-r image,regions] -n NAME
 Remove a component
 compact -r image,regions
 Defragment CBFS image.
 copy -r image,regions -R source-region
 Create a copy (duplicate) cbfs instance in fmap
 create -m ARCH -s size [-b bootblock offset] \
        [-o CBFS offset] [-H header offset] [-B bootblock]
        Create a legacy ROM file with CBFS master header*
 create -M flashmap [-r list,of,regions,containing,cbfses]
 Create a new-style partitioned firmware image
 locate [-r image,regions] -f FILE -n NAME [-P page-size] \
        [-a align] [-T]
        Find a place for a file of that size
 layout [-w]
 List mutable (or, with -w, readable) image regions
 print [-r image,regions]
 Show the contents of the ROM
 extract [-r image,regions] [-m ARCH] -n NAME -f FILE [-U]
 Extracts a file from ROM
 write [-F] -r image,regions -f file [-u | -d] [-i int]
 Write file into same-size [or larger] raw region
 read [-r fmap-region] -f file
 Extract raw region contents into binary file
 truncate [-r fmap-region]
 Truncate CBFS and print new size on stdout
 expand [-r fmap-region]
 Expand CBFS to span entire region
OFFSETs:
  Numbers accompanying -b, -H, and -o switches* may be provided
  in two possible formats: if their value is greater than
  0x80000000, they are interpreted as a top-aligned x86 memory
  address; otherwise, they are treated as an offset into flash.
ARCHes:
  arm64, arm, mips, ppc64, power8, riscv, x86, unknown
TYPEs:
 bootblock, cbfs header, stage, simple elf, fit, optionrom, bootsplash, raw,
 vsa, mbi, microcode, fsp, mrc, cmos_default, cmos_layout, spd,
 mrc_cache, mma, efi, struct, deleted, null

* Note that these actions and switches are only valid when
  working with legacy images whose structure is described
  primarily by a CBFS master header. New-style images, in
  contrast, exclusively make use of an FMAP to describe their
  layout: this must minimally contain an 'FMAP' section
  specifying the location of this FMAP itself and a 'COREBOOT'
  section describing the primary CBFS. It should also be noted
  that, when working with such images, the -F and -r switches
  default to 'COREBOOT' for convenience, and both the -b switch to
  CBFS operations and the output of the locate action become
  relative to the selected CBFS region's lowest address.
  The one exception to this rule is the top-aligned address,
  which is always relative to the end of the entire image
  rather than relative to the local region; this is true for
  for both input (sufficiently large) and output (-T) data.


Since binman has a native implementation of CBFS (see cbfs_util.py), we don't
actually need this tool, except for sanity checks in the tests.
"""

from binman import bintool

class Bintoolcbfstool(bintool.Bintool):
    """Coreboot filesystem (CBFS) tool

    This bintool supports creating new CBFS images and adding files to an
    existing image, i.e. the features needed by binman.

    It also supports fetching a binary cbfstool, since building it from source
    is fairly slow.

    Documentation about CBFS is at https://www.coreboot.org/CBFS
    """
    def __init__(self, name):
        super().__init__(name, 'Manipulate CBFS files')

    def create_new(self, cbfs_fname, size, arch='x86'):
        """Create a new CBFS

        Args:
            cbfs_fname (str): Filename of CBFS to create
            size (int): Size of CBFS in bytes
            arch (str): Architecture for which this CBFS is intended

        Returns:
            str: Tool output
        """
        args = [cbfs_fname, 'create', '-s', f'{size:#x}', '-m', arch]
        return self.run_cmd(*args)

    # pylint: disable=R0913
    def add_raw(self, cbfs_fname, name, fname, compress=None, base=None):
        """Add a raw file to the CBFS

        Args:
            cbfs_fname (str): Filename of CBFS to create
            name (str): Name to use inside the CBFS
            fname (str): Filename of file to add
            compress (str): Compression to use (cbfs_util.COMPRESS_NAMES) or
                None for None
            base (int): Address to place the file, or None for anywhere

        Returns:
            str: Tool output
        """
        args = [cbfs_fname,
                'add',
                '-n', name,
                '-t', 'raw',
                '-f', fname,
                '-c', compress or 'none']
        if base:
            args += ['-b', f'{base:#x}']
        return self.run_cmd(*args)

    def add_stage(self, cbfs_fname, name, fname):
        """Add a stage file to the CBFS

        Args:
            cbfs_fname (str): Filename of CBFS to create
            name (str): Name to use inside the CBFS
            fname (str): Filename of file to add

        Returns:
            str: Tool output
        """
        args = [cbfs_fname,
                'add-stage',
                '-n', name,
                '-f', fname
            ]
        return self.run_cmd(*args)

    def fail(self):
        """Run cbfstool with invalid arguments to check it reports failure

        This is really just a sanity check

        Returns:
            CommandResult: Result from running the bad command
        """
        args = ['missing-file', 'bad-command']
        return self.run_cmd_result(*args)

    def fetch(self, method):
        """Fetch handler for cbfstool

        This installs cbfstool by downloading from Google Drive.

        Args:
            method (FETCH_...): Method to use

        Returns:
            True if the file was fetched and now installed, None if a method
            other than FETCH_BIN was requested

        Raises:
            Valuerror: Fetching could not be completed
        """
        if method != bintool.FETCH_BIN:
            return None
        fname, tmpdir = self.fetch_from_drive(
            '1IOnE0Qvy97d-0WOCwF64xBGpKSY2sMtJ')
        return fname, tmpdir
