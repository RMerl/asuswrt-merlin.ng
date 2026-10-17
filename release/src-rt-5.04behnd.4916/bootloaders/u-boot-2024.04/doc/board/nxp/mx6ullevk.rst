.. SPDX-License-Identifier: GPL-2.0+

mx6ullevk
=========

How to use U-Boot on Freescale MX6ULL 14x14 EVK
-----------------------------------------------

- First make sure you have installed the dtc package (device tree compiler):

.. code-block:: bash

   $ sudo apt-get install device-tree-compiler

- Build U-Boot for MX6ULL 14x14 EVK:

.. code-block:: bash

   $ make mrproper
   $ make mx6ull_14x14_evk_defconfig
   $ make

This generates the u-boot-dtb.imx image in the current directory.

- Flash the u-boot-dtb.imx image into the micro SD card:

.. code-block:: bash

   $ sudo dd if=u-boot-dtb.imx of=/dev/sdb bs=1K seek=1 conv=notrunc && sync

- Jumper settings::

   SW601: 0 0 1 0
   Sw602: 1 0

Where 0 means bottom position and 1 means top position (from the switch label
numbers reference).

Connect the USB cable between the EVK and the PC for the console.
(The USB console connector is the one close the push buttons)

Insert the micro SD card in the board, power it up and U-Boot messages should
come up.

The link for the board: http://www.nxp.com/products/microcontrollers-and- \
processors/arm-processors/i.mx-applications-processors/i.mx-6-processors/ \
i.mx6qp/evaluation-kit-for-the-i.mx-6ull-applications-processor:MCIMX6ULL-EVK
