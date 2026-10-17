.. SPDX-License-Identifier: GPL-2.0+

imx8mq_evk
==========

U-Boot for the NXP i.MX8MQ EVK board

Quick Start
-----------

- Build the ARM Trusted firmware binary
- Get ddr and hdmi fimware
- Build U-Boot
- Boot

Get and Build the ARM Trusted firmware
--------------------------------------

Note: srctree is U-Boot source directory
Get ATF from: https://github.com/nxp-imx/imx-atf
branch: imx_5.4.47_2.2.0

.. code-block:: bash

   $ make PLAT=imx8mq bl31
   $ cp build/imx8mq/release/bl31.bin $(builddir)

Get the ddr and hdmi firmware
-----------------------------

.. code-block:: bash

   $ wget https://www.nxp.com/lgfiles/NMG/MAD/YOCTO/firmware-imx-8.9.bin
   $ chmod +x firmware-imx-8.9.bin
   $ ./firmware-imx-8.9.bin
   $ cp firmware-imx-8.9/firmware/hdmi/cadence/signed_hdmi_imx8m.bin $(builddir)
   $ cp firmware-imx-8.9/firmware/ddr/synopsys/lpddr4*.bin $(builddir)

Build U-Boot
------------

.. code-block:: bash

   $ export CROSS_COMPILE=aarch64-poky-linux-
   $ make imx8mq_evk_defconfig
   $ make

Burn the flash.bin to MicroSD card offset 33KB:

.. code-block:: bash

   $sudo dd if=flash.bin of=/dev/sd[x] bs=1024 seek=33 conv=notrunc

Boot
----
Set Boot switch SW801: 1100 and Bmode: 10 to boot from Micro SD.
