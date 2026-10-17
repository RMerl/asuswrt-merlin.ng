.. SPDX-License-Identifier: GPL-2.0+
.. sectionauthor:: Bin Meng <bmeng.cn@gmail.com>

Cherry Hill CRB
===============

This uses Intel FSP for Braswell platform. Download it from Intel FSP website,
put the .fd file to the board directory and rename it to fsp.bin.

Extract descriptor.bin and me.bin from the original BIOS on the board using
ifdtool and put them to the board directory as well.

Note the FSP package for Braswell does not ship a traditional legacy VGA BIOS
image for the integrated graphics device. Instead a new binary called Video
BIOS Table (VBT) is shipped. Put it to the board directory and rename it to
vbt.bin if you want graphics support in U-Boot.

Now you can build U-Boot and obtain u-boot.rom::

   $ make cherryhill_defconfig
   $ make all

An important note for programming u-boot.rom to the on-board SPI flash is that
you need make sure the SPI flash's 'quad enable' bit in its status register
matches the settings in the descriptor.bin, otherwise the board won't boot.

For the on-board SPI flash MX25U6435F, this can be done by writing 0x40 to the
status register by DediProg in: Config > Modify Status Register > Write Status
Register(s) > Register1 Value(Hex). This is is a one-time change. Once set, it
persists in SPI flash part regardless of the u-boot.rom image burned.
