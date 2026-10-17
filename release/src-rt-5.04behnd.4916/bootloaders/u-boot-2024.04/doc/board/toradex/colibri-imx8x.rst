.. SPDX-License-Identifier: GPL-2.0-or-later
.. sectionauthor:: Marcel Ziswiler <marcel.ziswiler@toradex.com>

Colibri iMX8X Module
====================

- SoM: https://www.toradex.com/computer-on-modules/colibri-arm-family/nxp-imx-8x
- Carrier board: https://www.toradex.com/products/carrier-board/colibri-evaluation-board

Quick Start
-----------

- Build the ARM trusted firmware binary
- Get scfw_tcm.bin and ahab-container.img
- Build U-Boot
- Load U-Boot binary using uuu
- Flash U-Boot binary into the eMMC
- Boot

Get and Build the ARM Trusted Firmware
--------------------------------------

Download the imx-atf repository:

.. code-block:: bash

    $ git clone -b lf_v2.6 https://github.com/nxp-imx/imx-atf.git

Compile it with an aarch64 toolchain:

.. code-block:: bash

    $ make PLAT=imx8qx bl31 -C imx-atf

Get scfw_tcm.bin and ahab-container.img
---------------------------------------

Download imx-seco firmware and extract it:

.. code-block:: bash

    $ wget https://www.nxp.com/lgfiles/NMG/MAD/YOCTO/imx-seco-5.8.7.bin
    $ sh imx-seco-5.8.7.bin --auto-accept

Copy the following firmware to the U-Boot folder:

.. code-block:: bash

    $ wget https://github.com/toradex/i.MX-System-Controller-Firmware/raw/master/src/scfw_export_mx8qx_b0/build_mx8qx_b0/mx8qx-colibri-scfw-tcm.bin
    $ cp ../imx-atf/build/imx8qx/release/bl31.bin .
    $ cp ../imx-seco-5.8.7/firmware/seco/mx8qxc0-ahab-container.img mx8qx-ahab-container.img

Build U-Boot
------------

.. code-block:: bash

   $ make colibri-imx8x_defconfig
   $ make u-boot-dtb.imx

Load the U-Boot Binary Using UUU
--------------------------------

Get the latest version of the universal update utility (uuu) aka ``mfgtools 3.0``:

https://github.com/nxp-imx/mfgtools/releases

Put the module into USB recovery aka serial downloader mode, connect the USB
device to your host and execute ``uuu``:

.. code-block:: bash

    sudo ./uuu u-boot/u-boot-dtb.imx

Flash the U-Boot Binary into the eMMC
-------------------------------------

Burn the ``u-boot-dtb.imx`` binary to the primary eMMC hardware boot area
partition and boot:

.. code-block:: bash

    load mmc 1:1 $loadaddr u-boot-dtb.imx
    setexpr blkcnt ${filesize} + 0x1ff && setexpr blkcnt ${blkcnt} / 0x200
    mmc dev 0 1
    mmc write ${loadaddr} 0x0 ${blkcnt}

As a convenience, instead of the last three commands, one may also use the
update U-Boot wrapper:

.. code-block:: bash

    > run update_uboot
