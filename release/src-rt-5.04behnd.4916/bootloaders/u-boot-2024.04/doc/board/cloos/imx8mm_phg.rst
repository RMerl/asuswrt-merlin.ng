.. SPDX-License-Identifier: GPL-2.0+

Cloos i.MX8MM PHG board
=======================

U-Boot for the Cloos i.MX8MM PHG board

Quick Start
-----------

- Get and Build the ARM Trusted firmware
- Get the DDR firmware
- Build U-Boot
- Flash U-Boot into the eMMC

Get and Build the ARM Trusted firmware
--------------------------------------

Note: builddir is U-Boot build directory (source directory for in-tree builds)
Get ATF from: https://github.com/nxp-imx/imx-atf
branch: lf_v2.6

.. code-block:: bash

   $ make PLAT=imx8mm bl31
   $ cp build/imx8mm/release/bl31.bin $(builddir)

Get the DDR firmware
--------------------

.. code-block:: bash

   $ wget https://www.nxp.com/lgfiles/NMG/MAD/YOCTO/firmware-imx-8.9.bin
   $ chmod +x firmware-imx-8.9.bin
   $ ./firmware-imx-8.9
   $ cp firmware-imx-8.9/firmware/ddr/synopsys/lpddr4*.bin $(builddir)

Build U-Boot
------------

.. code-block:: bash

   $ export CROSS_COMPILE=aarch64-poky-linux-
   $ make imx8mm_phg_defconfig
   $ make

Flash U-Boot into the eMMC
--------------------------

Program flash.bin to the eMMC at offset 33KB:

.. code-block:: bash

   $ ums 0 mmc 0
   $ sudo dd if=flash.bin of=/dev/sd[x] bs=1K seek=33; sync
