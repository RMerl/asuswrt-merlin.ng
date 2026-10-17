.. SPDX-License-Identifier: GPL-2.0+

imx8qxp_mek
===========

U-Boot for the NXP i.MX8QXP EVK board

Quick Start
-----------

- Build the ARM Trusted firmware binary
- Get scfw_tcm.bin and ahab-container.img
- Build U-Boot
- Flash the binary into the SD card
- Boot

Get and Build the ARM Trusted firmware
--------------------------------------

.. code-block:: bash

   $ git clone https://github.com/nxp-imx/imx-atf
   $ cd imx-atf/
   $ git checkout origin/imx_4.19.35_1.1.0 -b imx_4.19.35_1.1.0
   $ make PLAT=imx8qx bl31

Get scfw_tcm.bin and ahab-container.img
---------------------------------------

.. code-block:: bash

   $ wget https://www.nxp.com/lgfiles/NMG/MAD/YOCTO/imx-sc-firmware-1.2.7.1.bin
   $ chmod +x imx-sc-firmware-1.2.7.1.bin
   $ ./imx-sc-firmware-1.2.7.1.bin
   $ wget https://www.nxp.com/lgfiles/NMG/MAD/YOCTO/imx-seco-2.3.1.bin
   $ chmod +x imx-seco-2.3.1.bin
   $ ./imx-seco-2.3.1.bin

Copy the following binaries to U-Boot folder:

.. code-block:: bash

  $ cp imx-atf/build/imx8qx/release/bl31.bin .
  $ cp imx-seco-2.3.1/firmware/seco/mx8qx-ahab-container.img ./ahab-container.img
  $ cp imx-sc-firmware-1.2.7.1/mx8qx-mek-scfw-tcm.bin	.

Build U-Boot
------------

.. code-block:: bash

  $ make imx8qxp_mek_defconfig
  $ make

Flash the binary into the SD card
---------------------------------

Burn the flash.bin binary to SD card offset 32KB:

.. code-block:: bash

   $ sudo dd if=flash.bin of=/dev/sd[x] bs=1024 seek=32 conv=notrunc

Boot
----
Set Boot switch SW2: 1100.
