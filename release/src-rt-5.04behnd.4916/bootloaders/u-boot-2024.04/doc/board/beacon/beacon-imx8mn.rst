.. SPDX-License-Identifier: GPL-2.0+

U-Boot for the Beacon EmbeddedWorks i.MX8M Nano Devkit
======================================================

Quick Start
-----------

- Build the ARM Trusted firmware binary
- Get DDR firmware
- Build U-Boot
- Burn U-Boot to microSD Card
- Boot

Get and Build the ARM Trusted firmware
--------------------------------------

.. code-block:: bash

    $ git clone https://github.com/nxp-imx/imx-atf.git -b v2.6
    $ make PLAT=imx8mn bl31 CROSS_COMPILE=aarch64-linux-gnu-
    $ cp build/imx8mn/release/bl31.bin ../

Get the DDR firmware
--------------------

.. code-block:: bash

    $ wget https://www.nxp.com/lgfiles/NMG/MAD/YOCTO/firmware-imx-8.15.bin
    $ chmod +x firmware-imx-8.15.bin
    $ ./firmware-imx-8.15
    $ cp firmware-imx-8.15/firmware/ddr/synopsys/lpddr4*.bin .

Build U-Boot
------------

.. code-block:: bash

    $ make imx8mn_beacon_defconfig
    $ make CROSS_COMPILE=aarch64-linux-gnu-

Burn U-Boot to microSD Card
---------------------------

.. code-block:: bash

    $ sudo dd if=flash.bin of=/dev/sd[x] bs=1024 seek=32

Boot
----

Set baseboard DIP switches for micoSD Card:
S17 (1:8): 1100xxxx
