.. SPDX-License-Identifier: GPL-2.0+

imx8mm_evk
==========

U-Boot for the NXP i.MX8MM EVK board

Quick Start
-----------

- Build the ARM Trusted firmware binary
- Get ddr firmware
- Build U-Boot
- Boot

Get and Build the ARM Trusted firmware
--------------------------------------

Note: builddir is U-Boot build directory (source directory for in-tree builds)
Get ATF from: https://github.com/nxp-imx/imx-atf
branch: imx_5.4.47_2.2.0

.. code-block:: bash

   $ make PLAT=imx8mm bl31
   $ cp build/imx8mm/release/bl31.bin $(builddir)

Get the ddr firmware
--------------------

.. code-block:: bash

   $ wget https://www.nxp.com/lgfiles/NMG/MAD/YOCTO/firmware-imx-8.9.bin
   $ chmod +x firmware-imx-8.9.bin
   $ ./firmware-imx-8.9
   $ cp firmware-imx-8.9/firmware/ddr/synopsys/lpddr4*.bin $(builddir)

Build U-Boot for sd card
------------------------

.. code-block:: bash

   $ export CROSS_COMPILE=aarch64-poky-linux-
   $ make imx8mm_evk_defconfig
   $ make

Burn the flash.bin to MicroSD card offset 33KB:

.. code-block:: bash

   $sudo dd if=flash.bin of=/dev/sd[x] bs=1024 seek=33 conv=notrunc

Boot
----
Set Boot switch to SD boot

Build U-Boot for qspi flash card
--------------------------------

.. code-block:: bash

   $ export CROSS_COMPILE=aarch64-poky-linux-
   $ make imx8mm_evk_fspi_defconfig
   $ make

Currently, there is no direct support to write to QSPI Flash.
Copy flash.bin to ${loadaddr} either from sd card or over network and then copy to
qspi flash

From sd card to memory

.. code-block:: bash

    $mmc dev 1
    $mmc read ${loadaddr} 0x00 <size_of_flash.bin/512>

.. code-block:: bash

   $ sf probe
   $ sf erase 0 <size_of_flash.bin_in_hex>
   $ sf write $loadaddr 0x00 <size_of_flash.bin_in_hex>

Boot from QSPI Flash
--------------------

Set Boot Switch to QSPI Flash

Pin configuration for imx8mm_revC evk to boot from qspi flash
SW1101: 0110xxxxxx
SW1102: 00100x0010
