.. SPDX-License-Identifier: GPL-2.0

LS1021A-TSN Board Overview
==========================

 - 1GB DDR3 at 800 MHz
 - Spansion/Cypress 64 MB (Rev. A) / 32 MB (Rev. B and C) QSPI NOR flash
 - Ethernet
     - 2 SGMII 10/100/1G Ethernet ports (Atheros AR8031)
     - One SJA1105T switch with 4 Ethernet ports (Broadcom BCM5464R)
     - One internal RGMII port connected to the switch
 - SDHC
     - microSDHC/SDXC connector
 - Other I/O
    - One Serial port
    - Arduino and expansion headers
    - mPCIE slot
    - SATA port
    - USB3.0 port

LS1021A Memory map
==================

The addresses in brackets are physical addresses.

==============  ==============  ==============================  =======
Start Address   End Address     Description                     Size
==============  ==============  ==============================  =======
0x00_0000_0000  0x00_000F_FFFF  Secure Boot ROM                 1MB
0x00_0100_0000  0x00_0FFF_FFFF  CCSRBAR                         240MB
0x00_1000_0000  0x00_1000_FFFF  OCRAM0                          64KB
0x00_1001_0000  0x00_1001_FFFF  OCRAM1                          64KB
0x00_2000_0000  0x00_20FF_FFFF  DCSR                            16MB
0x00_4000_0000  0x00_5FFF_FFFF  QSPI                            512MB
0x00_6000_0000  0x00_67FF_FFFF  IFC - NOR Flash                 128MB
0x00_8000_0000  0x00_FFFF_FFFF  DRAM1                           2GB
==============  ==============  ==============================  =======

Compiling and flashing
======================

The LS1021A-TSN board comes along with a microSD card with OpenIL U-Boot that
can be used to update its internal QSPI flash (which is empty out of the
factory).

To compile and flash an SD card image::

  make ls1021atsn_sdcard_defconfig && make -j 8 && sudo cp u-boot-with-spl-pbl.bin /srv/tftpboot/
  => tftp 0x82000000 u-boot-with-spl-pbl.bin && mmc rescan && mmc erase 8 0x1100 && mmc write 0x82000000 8 0x1100

For the QSPI flash, first obtain the Reset Configuration Word binary for
bootimg from the QSPI flash from the rcw project
(https://github.com/nxp-qoriq/rcw)::

  make -j 8 && sudo cp ls1021atsn/SSR_PNS_30/rcw_1200_qspiboot.bin.swapped /srv/tftpboot/

The above RCW binary takes care of swapping the QSPI AMBA memory, so that the
U-Boot binary does not need to be swapped when flashing it.

To compile and flash a U-Boot image for QSPI::

  make ls1021atsn_qspi_defconfig && make -j 8 && sudo cp u-boot.bin /srv/tftpboot/

Then optionally create a custom uboot-env.txt file (although the default
environment already supports distro boot) and convert it to binary format::

  mkenvimage -s 2M -o /srv/tftpboot/uboot-env.bin uboot-env.txt

To program the QSPI flash with the images::

  => tftp 0x82000000 rcw_1200_qspiboot.bin.swapped && sf probe && sf erase 0x0 +${filesize} && sf write 0x82000000 0x0 ${filesize}
  => tftp 0x82000000 u-boot.bin && sf probe && sf erase 0x100000 +${filesize} && sf write 0x82000000 0x100000 ${filesize}
  => tftp 0x82000000 uboot-env.bin && sf probe && sf erase 0x400000 +${filesize} && sf write 0x82000000 0x400000 ${filesize}

The boards contain an AT24 I2C EEPROM that is supposed to hold the MAC
addresses of the Ethernet interfaces, however the EEPROM comes blank out of
the factory, and the MAC addresses are printed on a label on the bottom of
the boards.

To write the MAC addresses to the EEPROM, the following needs to be done once::

  => mac id
  => mac 0 00:1F:7B:xx:xx:xx
  => mac 1 00:1F:7B:xx:xx:xx
  => mac 2 00:1F:7B:xx:xx:xx
  => mac save

The switch ports do not have their own MAC address - they inherit it from the
master enet2 port.

Known issues and limitations
============================

- The 4 SJA1105 switch ports are not functional in U-Boot for now.
- Since the IFC pins are multiplexed with QSPI on LS1021A, currently there is
  no way to talk to the CPLD for e.g. running the "qixis_reset" command, or
  turning the fan on, etc.
