.. SPDX-License-Identifier: GPL-2.0+
.. sectionauthor:: Simon Glass <sjg@chromium.org>

Minnowboard MAX
===============

This uses as FSP as with Crown Bay, except it is for the Atom E3800 series.
Download this and get the .fd file (BAYTRAIL_FSP_GOLD_003_16-SEP-2014.fd at
the time of writing). Put it in the corresponding board directory and rename
it to fsp.bin.

Obtain the VGA RAM (Vga.dat at the time of writing) and put it into the same
board directory as vga.bin.

You still need two more binary blobs. For Minnowboard MAX, we can reuse the
same ME firmware above, but for flash descriptor, we need get that somewhere
else, as the one above does not seem to work, probably because it is not
designed for the Minnowboard MAX. Now download the original firmware image
for this board from:

   * http://firmware.intel.com/sites/default/files/2014-WW42.4-MinnowBoardMax.73-64-bit.bin_Release.zip

Unzip it::

   $ unzip 2014-WW42.4-MinnowBoardMax.73-64-bit.bin_Release.zip

Use ifdtool in the U-Boot tools directory to extract the images from that
file, for example::

   $ ./tools/ifdtool -x MNW2MAX1.X64.0073.R02.1409160934.bin

This will provide the descriptor file - copy this into the correct place::

   $ cp flashregion_0_flashdescriptor.bin board/intel/minnowmax/descriptor.bin

Now you can build U-Boot and obtain u-boot.rom::

   $ make minnowmax_defconfig
   $ make all

Checksums are as follows (but note that newer versions will invalidate this)::

   $ md5sum -b board/intel/minnowmax/*.bin
   ffda9a3b94df5b74323afb328d51e6b4  board/intel/minnowmax/descriptor.bin
   69f65b9a580246291d20d08cbef9d7c5  board/intel/minnowmax/fsp.bin
   894a97d371544ec21de9c3e8e1716c4b  board/intel/minnowmax/me.bin
   a2588537da387da592a27219d56e9962  board/intel/minnowmax/vga.bin

The ROM image is broken up into these parts:

======   ==================  ============================
Offset   Description         Controlling config
======   ==================  ============================
000000   descriptor.bin      Hard-coded to 0 in ifdtool
001000   me.bin              Set by the descriptor
500000   <spare>
5f0000   MRC cache           CONFIG_ENABLE_MRC_CACHE
600000   u-boot-dtb.bin      CONFIG_TEXT_BASE
6ef000   Environment         CONFIG_ENV_OFFSET
7b0000   vga.bin             CONFIG_VGA_BIOS_ADDR
7c0000   fsp.bin             CONFIG_FSP_ADDR
7f8000   <spare>             (depends on size of fsp.bin)
7ff800   U-Boot 16-bit boot  CONFIG_SYS_X86_START16
======   ==================  ============================

Overall ROM image size is controlled by CONFIG_ROM_SIZE.

Note that the debug version of the FSP is bigger in size. If this version
is used, CONFIG_FSP_ADDR needs to be configured to 0xfffb0000 instead of
the default value 0xfffc0000.

If you want to change CONFIG_TEXT_BASE from the current value of ffe00000
you need to check a few other things. CONFIG_SYS_MONITOR_BASE should
automatically update to be the same as CONFIG_TEXT_BASE but
CONFIG_SYS_MONITOR_LEN may need to be adjusted too. It must cover the space
from the start of U-Boot to the end of the RAM, since the 16-bit boot needs to
be able to jump to U-Boot. See the end of arch/x86/lib/fsp1/fsp_car.S which
has these values.

Also check the MRC cache address in the devicetree ("rw-mrc-cache"). It must
not overlap with U-Boot.
