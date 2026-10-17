.. SPDX-License-Identifier: GPL-2.0+

U-Boot for the Congatec conga-QMX8 board
========================================

Quick Start
-----------

- Build the ARM Trusted firmware binary
- Get scfw_tcm.bin and ahab-container.img
- Get imx-mkimage
- Build U-Boot
- Build imx-mkimage
- Flash the binary into the SD card
- Boot

Get and Build the ARM Trusted firmware
--------------------------------------

.. code-block:: bash

     $ git clone https://github.com/nxp-imx/imx-atf
     $ cd imx-atf/
     $ git checkout origin/imx_4.14.78_1.0.0_ga -b imx_4.14.78_1.0.0_ga
     $ make PLAT=imx8qm bl31

Get scfw_tcm.bin and ahab-container.img
---------------------------------------

.. code-block:: bash

     $ wget https://www.nxp.com/lgfiles/NMG/MAD/YOCTO/imx-sc-firmware-1.1.bin
     $ chmod +x imx-sc-firmware-1.1.bin
     $ ./imx-sc-firmware-1.1.bin
     $ wget https://www.nxp.com/lgfiles/NMG/MAD/YOCTO/firmware-imx-8.0.bin
     $ chmod +x firmware-imx-8.0.bin
     $ ./firmware-imx-8.0.bin

Or use this to avoid running random scripts from the internet,
but note that you must agree to the license the script displays:

.. code-block:: bash

     $ dd if=imx-sc-firmware-1.1.bin of=imx-sc-firmware-1.1.tar.bz2 bs=37185 skip=1
     $ tar -xf imx-sc-firmware-1.1.tar.bz2
     $ cp imx-sc-firmware-1.1/mx8qx-val-scfw-tcm.bin $(builddir)

     $ dd if=firmware-imx-8.0.bin of=firmware-imx-8.0.tar.bz2 bs=37180 skip=1
     $ tar -xf firmware-imx-8.0.tar.bz2
     $ cp firmware-imx-8.0/firmware/seco/mx8qm-ahab-container.img $(builddir)

Build U-Boot
------------

.. code-block:: bash

     $ export ATF_LOAD_ADDR=0x80000000
     $ export BL33_LOAD_ADDR=0x80020000
     $ make cgtqmx8_defconfig
     $ make

Flash the binary into the SD card
---------------------------------

Burn the flash.bin binary to SD card offset 32KB:

.. code-block:: bash

     $ sudo dd if=flash.bin of=/dev/sd[x] bs=1k seek=32 conv=fsync
