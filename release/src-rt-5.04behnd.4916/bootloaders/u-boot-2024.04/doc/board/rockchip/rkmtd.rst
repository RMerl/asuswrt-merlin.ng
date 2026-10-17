.. SPDX-License-Identifier: GPL-2.0+
.. Copyright (C) 2023 Johan Jonker <jbx6244@gmail.com>

RKMTD
=====

Info
----

The command rkmtd creates a virtual block device to transfer
Rockchip boot block data to and from NAND with block orientated
tools like "ums" and "rockusb".

It uses the Rockchip MTD driver to scan for boot blocks and copies
data from the first block in a GPT formatted virtual disk.
Data must be written in U-boot "idbloader.img" format and start at
partition "loader1" offset 64. The data header is parsed
for length and offset. When the last sector is received
it erases up to 5 erase blocks on NAND and writes boot blocks
in a pattern depending on the NAND ID. Data is then verified.
When a block turns out bad the block header is discarded.

Limitations
-----------

- Support with CONFIG_ROCKCHIP_NAND MTD driver only.
- Support for Rockchip boot block header type 1 only.
- Pattern for listed NAND IDs only. (Logic still not disclosed by Rockchip)
- The MTD framework driver data and NAND ID must be extracted at a lower level.

Available rkmtd commands
------------------------

.. code-block:: bash

        rkmtd bind <label>      - bind RKMTD device
        rkmtd unbind <label>    - unbind RKMTD device
        rkmtd info [<label>]    - show all available RKMTD devices
        rkmtd dev [<label>]     - show or set current RKMTD device

U-boot settings
---------------

Config to enable Rockchip MTD support:

.. code-block:: bash

        CONFIG_MTD=y
        CONFIG_MTD_RAW_NAND=y
        CONFIG_SYS_NAND_DRIVER_ECC_LAYOUT=y
        CONFIG_SYS_NAND_USE_FLASH_BBT=y
        CONFIG_ROCKCHIP_NAND=y

Option to keep existing NAND data unchanged:

.. code-block:: bash

        CONFIG_ROCKCHIP_NAND_SKIP_BBTSCAN=y

Commands to enable:

.. code-block:: bash

        CONFIG_CMD_USB=y
        CONFIG_CMD_RKMTD=y
        CONFIG_CMD_ROCKUSB=y
        CONFIG_CMD_USB_MASS_STORAGE=y

Linux Host (PC) tool commands combinations that work
----------------------------------------------------

.. table::
   :widths: 20 44

   ==================== ============================================
   U-boot               Linux
   ==================== ============================================
   rkmtd bind 0
   rockusb 0 rkmtd 0
                        upgrade_tool pl

                        upgrade_tool rl 64 512 idbloader_backup.img

                        upgrade_tool wl 64 idbloader.img

                        upgrade_tool rd

                        rkdeveloptool ppt

                        rkdeveloptool rl 64 512 idbloader_backup.img

                        rkdeveloptool wlx loader1 idbloader.img

                        rkdeveloptool wl 64 idbloader.img

                        rkdeveloptool rd

                        rkflashtool r 64 512 > idbloader_backup.img

                        rkflashtool w 64 512 < idbloader.img
   ums 0 rkmtd 0
                        dd if=/dev/sda1 of=idbloader_backup.img

                        dd if=idbloader.img of=/dev/sda1
   ==================== ============================================
