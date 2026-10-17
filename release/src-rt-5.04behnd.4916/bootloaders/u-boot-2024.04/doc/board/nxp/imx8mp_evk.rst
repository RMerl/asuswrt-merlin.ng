.. SPDX-License-Identifier: GPL-2.0+

imx8mp_evk
==========

U-Boot for the NXP i.MX8MP EVK board

Quick Start
-----------

- Build the ARM Trusted firmware binary
- Get the firmware-imx package
- Build U-Boot
- Boot

Get and Build the ARM Trusted firmware
--------------------------------------

Get ATF from: https://github.com/nxp-imx/imx-atf
branch: imx_5.4.70_2.3.0

.. code-block:: bash

   $ make PLAT=imx8mp bl31

Get the ddr firmware
--------------------

.. code-block:: bash

   $ wget https://www.nxp.com/lgfiles/NMG/MAD/YOCTO/firmware-imx-8.10.bin
   $ chmod +x firmware-imx-8.10.bin
   $ ./firmware-imx-8.10.bin

Build U-Boot
------------

.. code-block:: bash

Note: builddir is U-Boot build directory (source directory for in-tree builds).

   $ export CROSS_COMPILE=aarch64-poky-linux-
   $ make O=build imx8mp_evk_defconfig
   $ cp ../imx-atf/build/imx8mp/release/bl31.bin $(builddir)
   $ cp ../firmware-imx-8.10/firmware/ddr/synopsys/lpddr4_pmu_train_1d_dmem_202006.bin $(builddir)
   $ cp ../firmware-imx-8.10/firmware/ddr/synopsys/lpddr4_pmu_train_1d_imem_202006.bin $(builddir)
   $ cp ../firmware-imx-8.10/firmware/ddr/synopsys/lpddr4_pmu_train_2d_dmem_202006.bin $(builddir)
   $ cp ../firmware-imx-8.10/firmware/ddr/synopsys/lpddr4_pmu_train_2d_imem_202006.bin $(builddir)
   $ make

Burn the flash.bin to the MicroSD card at offset 32KB:

.. code-block:: bash

   $ sudo dd if=flash.bin of=/dev/sd[x] bs=1K seek=32 conv=notrunc; sync

Boot
----

Set Boot switch to SD boot
Use /dev/ttyUSB2 for U-Boot console
