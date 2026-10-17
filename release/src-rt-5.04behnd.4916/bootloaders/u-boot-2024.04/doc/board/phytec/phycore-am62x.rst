.. SPDX-License-Identifier: GPL-2.0+
.. sectionauthor:: Wadim Egorov <w.egorov@phytec.de>

phyCORE-AM62x
=============

The `phyCORE-AM62x <https://www.phytec.com/product/phycore-am62x>`_ is a
SoM (System on Module) featuring TI's AM62x SoC. It can be used in combination
with different carrier boards. This module can come with different sizes and
models for DDR, eMMC, SPI NOR Flash and various SoCs from the AM62x family.

A development Kit, called `phyBOARD-Lyra <https://www.phytec.com/product/phyboard-am62x>`_
is used as a carrier board reference design around the AM62x SoM.

Quickstart
----------

* Download sources and TI firmware blobs
* Build Trusted Firmware-A
* Build OP-TEE
* Build U-Boot for the R5
* Build U-Boot for the A53
* Create bootable uSD Card
* Boot

Sources
-------

.. include::  ../ti/k3.rst
    :start-after: .. k3_rst_include_start_boot_sources
    :end-before: .. k3_rst_include_end_boot_sources

Build procedure
---------------

Setup the environment variables:

.. include::  ../ti/k3.rst
    :start-after: .. k3_rst_include_start_common_env_vars_desc
    :end-before: .. k3_rst_include_end_common_env_vars_desc

.. include::  ../ti/k3.rst
    :start-after: .. k3_rst_include_start_board_env_vars_desc
    :end-before: .. k3_rst_include_end_board_env_vars_desc

Set the variables corresponding to this platform:

.. include::  ../ti/k3.rst
    :start-after: .. k3_rst_include_start_common_env_vars_defn
    :end-before: .. k3_rst_include_end_common_env_vars_defn
.. code-block:: bash

 $ export UBOOT_CFG_CORTEXR=phycore_am62x_r5_defconfig
 $ export UBOOT_CFG_CORTEXA=phycore_am62x_a53_defconfig
 $ export TFA_BOARD=lite
 $ # we don't use any extra TFA parameters
 $ unset TFA_EXTRA_ARGS
 $ export OPTEE_PLATFORM=k3-am62x
 $ export OPTEE_EXTRA_ARGS="CFG_WITH_SOFTWARE_PRNG=y"

.. include::  ../ti/am62x_sk.rst
    :start-after: .. am62x_evm_rst_include_start_build_steps
    :end-before: .. am62x_evm_rst_include_end_build_steps

uSD Card creation
-----------------

Use fdisk to partition the uSD card. The layout should look similar to:

.. code-block:: bash

 $ sudo fdisk -l /dev/mmcblk0
 Disk /dev/mmcblk0: 7.56 GiB, 8120172544 bytes, 15859712 sectors
 Units: sectors of 1 * 512 = 512 bytes
 Sector size (logical/physical): 512 bytes / 512 bytes
 I/O size (minimum/optimal): 512 bytes / 512 bytes
 Disklabel type: dos
 Disk identifier: 0x6583d9a3

 Device         Boot  Start     End Sectors   Size Id Type
 /dev/mmcblk0p1 *      2048  264191  262144   128M  c W95 FAT32 (LBA)
 /dev/mmcblk0p2      264192 1934953 1670762 815.8M 83 Linux


Once partitioned, the boot partition has to be formatted with a FAT filesystem.
Assuming the uSD card is `/dev/mmcblk0`:

.. code-block:: bash

 $ mkfs.vfat /dev/mmcblk0p1

To boot from a micro SD card on a HSFS device simply copy the following
artifacts to the FAT partition:

* tiboot3.bin from R5 build
* tispl.bin from Cortex-A build
* u-boot.img from Cortex-A build

Boot
----

Put the uSD card in the slot on the board and apply power. Check the serial
console for output.

Flash to SPI NOR
----------------

Below commands can be used to flash the SPI NOR flash; assuming
tiboot3.bin, tispl.bin and u-boot.img are stored on the uSD card.

.. code-block:: bash

  sf probe
  fatload mmc 1 ${loadaddr} tiboot3.bin
  sf update $loadaddr 0x0 $filesize
  fatload mmc 1 ${loadaddr} tispl.bin
  sf update $loadaddr 0x80000 $filesize
  fatload mmc 1 ${loadaddr} u-boot.img
  sf update $loadaddr 0x280000 $filesize


Boot Modes
----------

The phyCORE-AM62x development kit supports booting from many different
interfaces. By default, the development kit is set to boot from the micro-SD
card. To change the boot device, DIP switches S5 and S6 can be used.
Boot switches should be changed with power off.

.. list-table:: Boot Modes
   :widths: 16 16 16
   :header-rows: 1

   * - Switch Label
     - SW5: 12345678
     - SW6: 12345678

   * - uSD
     - 11000010
     - 01000000

   * - eMMC
     - 11010010
     - 00000000

   * - OSPI
     - 11010000
     - 10000000

   * - UART
     - 11011100
     - 00000000

Further Information
-------------------

Please see :doc:`../ti/am62x_sk` chapter for further AM62 SoC related documentation
and https://docs.phytec.com/phycore-am62x for vendor documentation.
