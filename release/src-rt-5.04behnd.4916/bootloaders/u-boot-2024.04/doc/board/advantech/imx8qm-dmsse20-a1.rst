.. SPDX-License-Identifier: GPL-2.0+

NXP i.MX8QM DMSSE20-a1 board
============================

Quick Start
-----------

- Build the ARM Trusted firmware binary
- Get scfw_tcm.bin and ahab-container.img
- Get imx-mkimage
- Build U-Boot
- Flash the binary into the SD card
- Boot

Get and Build the ARM Trusted Firmware
--------------------------------------

.. code-block:: bash

     $ git clone https://github.com/nxp-imx/imx-atf
     $ cd imx-atf/
     $ git checkout lf-5.10.72-2.2.0 -b lf-5.10.72-2.2.0
     $ make PLAT=imx8qm bl31
     $ cp build/imx8qm/release/bl31.bin $(builddir)

Get scfw_tcm.bin and ahab-container.img
---------------------------------------

.. code-block:: bash

     $ wget https://www.nxp.com/lgfiles/NMG/MAD/YOCTO/imx-sc-firmware-1.11.0.bin
     $ chmod +x imx-sc-firmware-1.11.0.bin
     $ ./imx-sc-firmware-1.11.0.bin
     $ wget https://www.nxp.com/lgfiles/NMG/MAD/YOCTO/imx-seco-3.8.5.bin
     $ chmod +x imx-seco-3.8.5.bin
     $ ./imx-seco-3.8.5.bin

Or use this to avoid running random scripts from the internet,
but note that you must agree to the license the script displays:

.. code-block:: bash

     $ dd if=imx-sc-firmware-1.11.0.bin of=imx-sc-firmware-1.11.0.tar.bz2 bs=42757 skip=1
     $ tar -xf imx-sc-firmware-1.11.0.tar.bz2
     $ cp imx-sc-firmware-1.11.0/mx8qm-val-scfw-tcm.bin $(builddir)
     $ dd if=imx-seco-3.8.5.bin of=imx-seco-3.8.5.tar.bz2 bs=43978 skip=1
     $ tar -xf imx-seco-3.8.5.tar.bz2
     $ cp imx-seco-3.8.5/firmware/seco/mx8qmb0-ahab-container.img $(builddir)

Build U-Boot
------------
.. code-block:: bash

     $ export ATF_LOAD_ADDR=0x80000000
     $ export BL33_LOAD_ADDR=0x80020000
     $ make imx8qm_dmsse20a1_defconfig
     $ make
