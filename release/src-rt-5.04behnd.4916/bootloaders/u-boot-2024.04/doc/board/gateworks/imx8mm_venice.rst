.. SPDX-License-Identifier: GPL-2.0+

imx8mm_venice
=============

U-Boot for the Gateworks i.MX8M Mini Venice Development Kit boards

Quick Start
-----------
- Build the ARM Trusted firmware binary
- Get DDR firmware
- Build U-Boot
- Flash to eMMC
- Boot

Get and Build the ARM Trusted firmware
--------------------------------------

.. code-block:: bash

   $ git clone https://github.com/nxp-imx/imx-atf.git -b lf_v2.4
   $ make PLAT=imx8mm bl31 CROSS_COMPILE=aarch64-linux-gnu-
   $ cp build/imx8mm/release/bl31.bin .

Get the DDR Firmware
--------------------

.. code-block:: bash

   $ wget https://www.nxp.com/lgfiles/NMG/MAD/YOCTO/firmware-imx-8.9.bin
   $ chmod +x firmware-imx-8.9.bin
   $ ./firmware-imx-8.9.bin
   $ cp firmware-imx-8.9/firmware/ddr/synopsys/lpddr4*.bin .

Build U-Boot
------------

.. code-block:: bash

   $ make imx8mm_venice_defconfig
   $ make CROSS_COMPILE=aarch64-linux-gnu-

Update eMMC
-----------

.. code-block:: bash

   => tftpboot $loadaddr flash.bin
   => setexpr blkcnt $filesize + 0x1ff && setexpr blkcnt $blkcnt / 0x200
   => mmc dev 2 0 && mmc write $loadaddr 0x42 $blkcnt # emmc user hw part
   => mmc dev 2 1 && mmc write $loadaddr 0x42 $blkcnt # or emmc boot0 hw part
   => mmc dev 2 2 && mmc write $loadaddr 0x42 $blkcnt # or emmc boot1 hw part
