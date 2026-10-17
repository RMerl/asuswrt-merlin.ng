.. SPDX-License-Identifier: GPL-2.0+
.. sectionauthor:: Simon Glass <sjg@chromium.org>

Chromebook Link
===============

First, you need the following binary blobs:

   * descriptor.bin - Intel flash descriptor
   * me.bin - Intel Management Engine
   * mrc.bin - Memory Reference Code, which sets up SDRAM
   * video ROM - sets up the display

You can get these binary blobs by::

   $ git clone http://review.coreboot.org/p/blobs.git
   $ cd blobs

Find the following files:

   * ./mainboard/google/link/descriptor.bin
   * ./mainboard/google/link/me.bin
   * ./northbridge/intel/sandybridge/systemagent-r6.bin

The 3rd one should be renamed to mrc.bin.
As for the video ROM, you can get it `here`_ and rename it to vga.bin.
Make sure all these binary blobs are put in the board directory.

Now you can build U-Boot and obtain u-boot.rom::

   $ make chromebook_link_defconfig
   $ make all

.. _here: http://www.coreboot.org/~stepan/pci8086,0166.rom
