.. SPDX-License-Identifier: GPL-2.0+

Librem5
=======

U-Boot for the Purism Librem5 phone

Quick Start
-----------

- Build the ARM Trusted firmware binary
- Get ddr and hdmi firmware
- Build U-Boot

Get and Build the ARM Trusted firmware
--------------------------------------

Note: srctree is U-Boot source directory
Get ATF from: https://source.puri.sm/Librem5/arm-trusted-firmware
branch: librem5

.. code-block:: bash

   $ make PLAT=imx8mq CROSS_COMPILE=aarch64-linux-gnu- bl31
   $ cp build/imx8mq/release/bl31.bin $(builddir)

Get the ddr and display port firmware
-------------------------------------

.. code-block:: bash

   $ wget https://www.nxp.com/lgfiles/NMG/MAD/YOCTO/firmware-imx-8.15.bin
   $ chmod +x firmware-imx-8.15.bin
   $ ./firmware-imx-8.15.bin
   $ cp firmware-imx-8.15/firmware/hdmi/cadence/signed_dp_imx8m.bin $(builddir)
   $ cp firmware-imx-8.15/firmware/ddr/synopsys/lpddr4*.bin $(builddir)

Build U-Boot
------------

.. code-block:: bash

   $ export CROSS_COMPILE=aarch64-linux-gnu-
   $ make librem5_defconfig
   $ make ARCH=arm

Burn the flash.bin
------------------

Use uuu to burn flash.bin. Power on the phone while holding vol+ to get it
into uuu mode.

.. code-block:: bash

   $ git clone https://source.puri.sm/Librem5/librem5-devkit-tools.git
   $ cd librem5-devkit-tools
   $ cp $(builddir)/flash.bin files/u-boot-librem5.imx
   $ uuu uuu_scripts/u-boot_flash_librem5.lst

Reboot the phone.
