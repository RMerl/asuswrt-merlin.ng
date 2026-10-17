.. SPDX-License-Identifier: GPL-2.0+

Kontron pitx-imx8m
==================

The Kontron pitx-imx8m is an embedded board with an i.MX8MQ in the pITX
form factor.

The board has two Ethernet ports, USB, HDMI/LVDS, m.2 slot, SD card, CAN,
RS232 and much more.

Quick Start
-----------

- Get and build the ARM Trusted firmware binary
- Get DDR and HDMI firmware
- Build U-Boot
- Install on SD card
- Boot

Get and build the ARM Trusted firmware binary
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Note: builddir is U-Boot build directory (source directory for in-tree builds)

.. code-block:: bash

    $ git clone https://github.com/ARM-software/arm-trusted-firmware.git
    $ git checkout v2.5
    $ make PLAT=imx8mq ARCH=aarch64 CROSS_COMPILE=aarch64-linux-gnu- bl31
    $ cp build/imx8mq/release/bl31.bin $(builddir)

Get DDR and HDMI firmware
^^^^^^^^^^^^^^^^^^^^^^^^^

Note: builddir is U-Boot build directory (source directory for in-tree builds)

.. code-block:: bash

    $ wget https://www.nxp.com/lgfiles/NMG/MAD/YOCTO/firmware-imx-8.11.bin
    $ chmod +x firmware-imx-8.11.bin
    $ ./firmware-imx-8.11
    $ cp firmware-imx-8.11/firmware/ddr/synopsys/lpddr4*.bin $(builddir)
    $ cp firmware-imx-8.11/firmware/hdmi/cadence/signed_hdmi_imx8m.bin $(builddir)

Build U-Boot
^^^^^^^^^^^^

.. code-block:: bash

    $ make kontron_pitx_imx8m_defconfig
    $ make CROSS_COMPILE=aarch64-linux-gnu-

Install on SD card
^^^^^^^^^^^^^^^^^^


Burn the flash.bin to SD card at an offset of 33 KiB:

.. code-block:: bash

    $ sudo dd if=flash.bin of=/dev/sd[x] bs=1024 seek=33

Boot
^^^^

Set the boot source selection to SD card boot and power on the board.
