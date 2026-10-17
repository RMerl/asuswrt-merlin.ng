.. SPDX-License-Identifier: GPL-2.0+
.. sectionauthor:: Bin Meng <bmeng.cn@gmail.com>

Cougar Canyon 2 CRB
===================

This uses Intel FSP for 3rd generation Intel Core and Intel Celeron processors
with mobile Intel HM76 and QM77 chipsets platform. Download it from Intel FSP
website and put the .fd file (CHIEFRIVER_FSP_GOLD_001_09-OCTOBER-2013.fd at the
time of writing) in the board directory and rename it to fsp.bin.

Now build U-Boot and obtain u-boot.rom::

   $ make cougarcanyon2_defconfig
   $ make all

The board has two 8MB SPI flashes mounted, which are called SPI-0 and SPI-1 in
the board manual. The SPI-0 flash should have flash descriptor plus ME firmware
and SPI-1 flash is used to store U-Boot. For convenience, the complete 8MB SPI-0
flash image is included in the FSP package (named Rom00_8M_MB_PPT.bin). Program
this image to the SPI-0 flash according to the board manual just once and we are
all set. For programming U-Boot we just need to program SPI-1 flash. Since the
default u-boot.rom image for this board is set to 2MB, it should be programmed
to the last 2MB of the 8MB chip, address range [600000, 7FFFFF].
