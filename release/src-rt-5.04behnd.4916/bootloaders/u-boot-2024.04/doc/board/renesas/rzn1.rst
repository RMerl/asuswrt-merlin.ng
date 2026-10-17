.. SPDX-License-Identifier: GPL-2.0+

Renesas RZ/N1
=============

Building
--------

This document describes how to build and flash U-Boot for the RZ/N1.

U-Boot
^^^^^^

Clone the U-Boot repository and build it as follows:

.. code-block:: bash

        git clone --depth 1 https://source.denx.de/u-boot/u-boot.git
        cd u-boot
        make rzn1_snarc_defconfig
        make CROSS_COMPILE=arm-linux-gnu-

This produces `u-boot` which is an ELF executable, suitable for use with `gdb`
and JTAG debugging tools.

It also produceds `u-boot.bin` which is a raw binary.

Binman
^^^^^^

The BootROM in the RZ/N1 SoC expects to find the boot image in SPKG format.
This format is documented in Chapter 7.4 of the RZ/N1 User Manual.

The `binman` tool may be used to generate the SPKG format for booting.
See tools/binman/binman.rst for details on this tool and its pre-requisites.

.. code-block:: bash

    binman -d arch/arm/dts/r9a06g032-rzn1-snarc.dtb -o <OUT>

This will produce `u-boot.bin.spkg` in the specified <OUT> directory. It can
then be flashed into QSPI, NAND, or loaded via USB-DFU mode.

SPKG image
^^^^^^^^^^

Alternatively, the same SPKG image can be built by calling `mkimage` as follows:

.. code-block:: bash

    tools/mkimage -n board/schneider/rzn1-snarc/spkgimage.cfg \
            -T spkgimage -a 0x20040000 -e 0x20040000 \
            -d u-boot.bin u-boot.bin.spkg

This produces `u-boot.bin.spkg` which can be flashed into QSPI, NAND, or loaded
via USB-DFU mode.

Take note of the load and execution address, which are encoded into the SPKG
headers. For development convenience, mkimage computes the execution offset
(part of the SPKG header) by subtracting the supplied load address from the
supplied execution address.

Also note there are other parameters, notably ECC configuration in the case of
boot from NAND, specified in the `spkgimage.cfg` configuration file.

Flashing
--------

The RZ/N1 is able to boot from QSPI, NAND, or via USB (DFU). In all cases the
on-board BootROM expects for the binary to be wrapped with a "SPKG" header.

It is possible to recover a bricked unit by using the USB (DFU) boot mode. This
allows uploading U-Boot into the internal RAM. Thereafter U-Boot can be used to
program the QSPI and/or NAND, making use of U-Boot dfu mode.

Otherwise the only other option for recovery is via JTAG.
