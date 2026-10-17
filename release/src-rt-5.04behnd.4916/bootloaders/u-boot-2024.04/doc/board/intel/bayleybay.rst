.. SPDX-License-Identifier: GPL-2.0+
.. sectionauthor:: Bin Meng <bmeng.cn@gmail.com>

Bayley Bay CRB
==============

This uses as FSP as with Crown Bay, except it is for the Atom E3800 series.
Download this and get the .fd file (BAYTRAIL_FSP_GOLD_003_16-SEP-2014.fd at
the time of writing). Put it in the corresponding board directory and rename
it to fsp.bin.

Obtain the VGA RAM (Vga.dat at the time of writing) and put it into the same
board directory as vga.bin.

You still need two more binary blobs. For Bayley Bay, they can be extracted
from the sample SPI image provided in the FSP (SPI.bin at the time of writing)::

   $ ./tools/ifdtool -x BayleyBay/SPI.bin
   $ cp flashregion_0_flashdescriptor.bin board/intel/bayleybay/descriptor.bin
   $ cp flashregion_2_intel_me.bin board/intel/bayleybay/me.bin

Now you can build U-Boot and obtain u-boot.rom::

   $ make bayleybay_defconfig
   $ make all

Note that the debug version of the FSP is bigger in size. If this version
is used, CONFIG_FSP_ADDR needs to be configured to 0xfffb0000 instead of
the default value 0xfffc0000.
