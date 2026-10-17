.. SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
.. sectionauthor:: Neha Malcom Francis <n-francis@ti.com>

AM43xx Generation
=================

Secure Boot
-----------

.. include:: am335x_evm.rst
   :start-after: .. secure_boot_include_start_config_ti_secure_device
   :end-before: .. secure_boot_include_end_config_ti_secure_device

.. include:: am335x_evm.rst
   :start-after: .. secure_boot_include_start_spl_boot
   :end-before: .. secure_boot_include_end_spl_boot

<IMAGE_FLAG> is a value that specifies the type of the image to
generate OR the action the image generation tool will take. Valid
values are:

.. list-table::
   :widths: 25 25
   :header-rows: 0

   * - SPI_X-LOADER
     - Generates an image for SPI flash (byte swapped)
   * - XIP_X-LOADER
     - Generates a single stage u-boot for NOR/QSPI XiP
   * - ISSW
     - Generates an image for all other boot modes

<INPUT_FILE> is the full path and filename of the public world boot
loaderbinary file (depending on the boot media, this is usually
either u-boot-spl.bin or u-boot.bin).

<OUTPUT_FILE> is the full path and filename of the final secure
image. The output binary images should be used in place of the standard
non-secure binary images (see the platform-specific user's guides and
releases notes for how the non-secure images are typically used)

.. list-table::
   :widths: 25 25
   :header-rows: 0

   * - u-boot-spl_HS_SPI_X-LOADER
     - byte swapped boot image for SPI flash
   * - u-boot_HS_XIP_X-LOADER
     - boot image for NOR or QSPI Xip flash
   * - u-boot-spl_HS_ISSW
     - boot image for all other boot media


<SPL_LOAD_ADDR> is the address at which SOC ROM should load the
<INPUT_FILE>

.. include:: am335x_evm.rst
   :start-after: .. secure_boot_include_start_primary_u_boot
   :end-before: .. secure_boot_include_end_primary_u_boot

.. qspi_boot_support_include_start

QSPI U-Boot support
-------------------

Host processor is connected to serial flash device via qpsi
interface. QSPI is a kind of spi module that allows single,
dual and quad read access to external spi devices. The module
has a memory mapped interface which provide direct interface
for accessing data form external spi devices.

The one QSPI in the device is primarily intended for fast booting
from Quad SPI flash devices.

Usecase
^^^^^^^

MLO/u-boot.img will be flashed from SD/MMC to the flash device
using serial flash erase and write commands. Then, switch settings
will be changed to qspi boot. Then, the ROM code will read MLO
from the predefined location in the flash, where it was flashed and
execute it after storing it in SDRAM. Then, the MLO will read
u-boot.img from flash and execute it from SDRAM.

SPI mode
^^^^^^^^

SPI mode uses mtd spi framework for transfer and reception of data.
Can be used in:

 #. Normal mode: use single pin for transfers
 #. Dual Mode: use two pins for transfers.
 #. Quad mode: use four pin for transfer

Memory mapped read mode
^^^^^^^^^^^^^^^^^^^^^^^

In this, SPI controller is configured using configuration port and then
controller is switched to memory mapped port for data read.

Driver
^^^^^^

drivers/qspi/ti_qspi.c
    - File which is responsible for configuring the
      qspi controller and also for providing the low level api which
      is responsible for transferring the datas from host controller
      to flash device and vice versa.

.. qspi_boot_support_include_end

Testing
^^^^^^^

These are the testing details of qspi flash driver with Macronix M25L51235
flash device.

The test includes
 - probing the flash device
 - erasing the flash device
 - Writing to flash
 - Reading the contents of the flash.

Test Log

.. code-block:: bash

  Hit any key to stop autoboot:  0
  => sf probe 0
  SF: Detected MX25L51235F with page size 256 Bytes, erase size 64 KiB, total 64 MiB, mapped at 30000000
  => sf erase 0 0x80000
  SF: 524288 bytes @ 0x0 Erased: OK
  => mw 81000000 0xdededede 0x40000
  => sf write 81000000 0 0x40000
  SF: 262144 bytes @ 0x0 Written: OK
  => sf read 82000000 0 0x40000
  SF: 262144 bytes @ 0x0 Read: OK
  => md 0x82000000
  82000000: dededede dededede dededede dededede    ................
  82000010: dededede dededede dededede dededede    ................
  82000020: dededede dededede dededede dededede    ................
  82000030: dededede dededede dededede dededede    ................
  82000040: dededede dededede dededede dededede    ................
  82000050: dededede dededede dededede dededede    ................
  82000060: dededede dededede dededede dededede    ................
  82000070: dededede dededede dededede dededede    ................
  82000080: dededede dededede dededede dededede    ................
  82000090: dededede dededede dededede dededede    ................
  820000a0: dededede dededede dededede dededede    ................
  820000b0: dededede dededede dededede dededede    ................
  820000c0: dededede dededede dededede dededede    ................
  820000d0: dededede dededede dededede dededede    ................
  820000e0: dededede dededede dededede dededede    ................
  820000f0: dededede dededede dededede dededede    ................
  => md 0x82010000
  82010000: dededede dededede dededede dededede    ................
  82010010: dededede dededede dededede dededede    ................
  82010020: dededede dededede dededede dededede    ................
  82010030: dededede dededede dededede dededede    ................
  82010040: dededede dededede dededede dededede    ................
  82010050: dededede dededede dededede dededede    ................
  82010060: dededede dededede dededede dededede    ................
  82010070: dededede dededede dededede dededede    ................
  82010080: dededede dededede dededede dededede    ................
  82010090: dededede dededede dededede dededede    ................
  820100a0: dededede dededede dededede dededede    ................
  820100b0: dededede dededede dededede dededede    ................
  820100c0: dededede dededede dededede dededede    ................
  820100d0: dededede dededede dededede dededede    ................
  820100e0: dededede dededede dededede dededede    ................
  820100f0: dededede dededede dededede dededede    ................
  => md 0x82030000
  82030000: dededede dededede dededede dededede    ................
  82030010: dededede dededede dededede dededede    ................
  82030020: dededede dededede dededede dededede    ................
  82030030: dededede dededede dededede dededede    ................
  82030040: dededede dededede dededede dededede    ................
  82030050: dededede dededede dededede dededede    ................
  82030060: dededede dededede dededede dededede    ................
  82030070: dededede dededede dededede dededede    ................
  82030080: dededede dededede dededede dededede    ................
  82030090: dededede dededede dededede dededede    ................
  820300a0: dededede dededede dededede dededede    ................
  820300b0: dededede dededede dededede dededede    ................
  820300c0: dededede dededede dededede dededede    ................
  820300d0: dededede dededede dededede dededede    ................
  820300e0: dededede dededede dededede dededede    ................
  820300f0: dededede dededede dededede dededede    ................
