.. SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
.. sectionauthor:: Neha Malcom Francis <n-francis@ti.com>

DRA7xx Generation
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

   * - X-LOADER
     - Generates an image for NOR or QSPI boot modes
   * - MLO
     - Generates an image for NOR or QSPI boot modes
   * - ULO
     - Generates an image for USB/UART peripheral boot modes

<INPUT_FILE> is the full path and filename of the public world boot
loaderbinary file (for this platform, this is always u-boot-spl.bin).

<OUTPUT_FILE> is the full path and filename of the final secure image.
The output binary images should be used in place of the standard
non-secure binary images (see the platform-specific user's guides
and releases notes for how the non-secure images are typically used)

.. list-table::
   :widths: 25 25
   :header-rows: 0

   * - u-boot-spl_HS_SPI_X-LOADER
     - boot image for SD/MMC/eMMC. This image is
       copied to a file named MLO, which is the name that
       the device ROM bootloader requires for loading from
       the FAT partition of an SD card (same as on
       non-secure devices)
   * - u-boot-spl_HS_ULO
     - boot image for USB/UART peripheral boot modes
   * - u-boot-spl_HS_X-LOADER
     - boot image for all other flash memories
       including QSPI and NOR flash

<SPL_LOAD_ADDR> is the address at which SOC ROM should load the
<INPUT_FILE>

.. include:: am335x_evm.rst
   :start-after: .. secure_boot_include_start_primary_u_boot
   :end-before: .. secure_boot_include_end_primary_u_boot

eMMC Boot Partition Use
-----------------------

It is possible, depending on SYSBOOT configuration to boot from the eMMC
boot partitions using (name depending on documentation referenced)
Alternative Boot operation mode or Boot Sequence Option 1/2.  In this
example we load MLO and u-boot.img from the build into DDR and then use
'mmc bootbus' to set the required rate (see TRM) and 'mmc partconfig' to
set boot0 as the boot device.

.. prompt:: bash =>

   setenv autoload no
   usb start
   dhcp
   mmc dev 1 1
   tftp ${loadaddr} dra7xx/MLO
   mmc write ${loadaddr} 0 100
   tftp ${loadaddr} dra7xx/u-boot.img
   mmc write ${loadaddr} 300 400
   mmc bootbus 1 2 0 2
   mmc partconf 1 1 1 0
   mmc rst-function 1 1

.. include:: am43xx_evm.rst
   :start-after: qspi_boot_support_include_start
   :end-before: qspi_boot_support_include_end

Testing
^^^^^^^

Build the patched U-Boot and load MLO/u-boot.img.

Boot from another medium like MMC

.. prompt:: bash

  => mmc dev 0
  mmc0 is current device
  => fatload mmc 0 0x82000000 MLO
  reading MLO
  55872 bytes read in 8 ms (6.7 MiB/s)
  => fatload mmc 0 0x83000000 u-boot.img
  reading u-boot.img
  248600 bytes read in 19 ms (12.5 MiB/s)

Commands to erase/write u-boot/MLO to flash device

.. prompt:: bash

  => sf probe 0
  SF: Detected S25FL256S_64K with page size 256 Bytes, erase size 64 KiB, total 32 MiB, mapped at 5c000000
  => sf erase 0 0x10000
  SF: 65536 bytes @ 0x0 Erased: OK
  => sf erase 0x20000 0x10000
  SF: 65536 bytes @ 0x20000 Erased: OK
  => sf erase 0x30000 0x10000
  SF: 65536 bytes @ 0x30000 Erased: OK
  => sf erase 0x40000 0x10000
  SF: 65536 bytes @ 0x40000 Erased: OK
  => sf erase 0x50000 0x10000
  SF: 65536 bytes @ 0x50000 Erased: OK
  => sf erase 0x60000 0x10000
  SF: 65536 bytes @ 0x60000 Erased: OK
  => sf write 82000000 0 0x10000
  SF: 65536 bytes @ 0x0 Written: OK
  => sf write 83000000 0x20000 0x60000
  SF: 393216 bytes @ 0x20000 Written: OK

Next, set sysboot to QSPI-1 boot mode(SYSBOOT[5:0] = 100110) and power
on. ROM should find the GP header at offset 0 and load/execute SPL. SPL
then detects that ROM was in QSPI-1 mode (boot code 10) and attempts to
find a U-Boot image header at offset 0x20000 (set in the config file)
and proceeds to load that image using the U-Boot image payload offset/size
from the header. It will then start U-Boot.
