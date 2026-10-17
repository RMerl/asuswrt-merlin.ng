.. SPDX-License-Identifier: GPL-2.0+

U-Boot for Anbernic RGxx3 Devices
=================================

This allows U-Boot to boot the following Anbernic devices:

 - Anbernic RG-ARC-D
 - Anbernic RG-ARC-S
 - Anbernic RG353M
 - Anbernic RG353P
 - Anbernic RG353PS
 - Anbernic RG353V
 - Anbernic RG353VS
 - Anbernic RG503

Additionally, the following very similar non-Anbernic devices are also
supported:

 - Powkiddy RGB30
 - Powkiddy RK2023

The correct device is detected automatically by comparing ADC values
from ADC channel 1. In the event of an RG353V or RG353P, an attempt
is then made to probe for an eMMC and if it fails the device is assumed
to be an RG353VS or RG353PS. Based on the detected device, the
environment variables "board", "board_name", and "fdtfile" are set to
the correct values corresponding to the board which can be read by a
boot script to boot with the correct device tree. If a board is defined
as requiring panel detection, a panel detect is then performed by
probing a "dummy" display on the DSI bus and then querying the display
ID. The display ID is then compared to a table to get the known
compatible string for use in Linux, and this string is saved as an
environment variable of "panel".

FDT fixups are performed in the event of an RG353M to change the device
name, or in the event the panel detected does not match the devicetree.
This allows Linux to load the correct panel driver without having to
know exactly which panel is used (as there is no user distingushable
way to tell).

Building U-Boot
---------------

.. code-block:: bash

    $ export CROSS_COMPILE=aarch64-linux-gnu-
    $ export BL31=../rkbin/bin/rk35/rk3568_bl31_v1.34.elf
    $ export ROCKCHIP_TPL=../rkbin/bin/rk35/rk3568_ddr_1056MHz_v1.13.bin
    $ make anbernic-rgxx3_defconfig
    $ make

This will build ``u-boot-rockchip.bin`` which can be written to an SD
card.

Image installation
------------------

Write the ``u-boot-rockchip.bin`` to an SD card offset 32kb from the
start. Please note that eMMC booting has not been tested at this time.

.. code-block:: bash

    $ dd if=u-boot-rockchip.bin of=/dev/mmcblk0 bs=512 seek=64
