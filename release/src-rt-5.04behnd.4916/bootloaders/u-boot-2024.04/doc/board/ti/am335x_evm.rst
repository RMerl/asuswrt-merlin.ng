.. SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
.. sectionauthor:: Tom Rini <trini@konsulko.com>

AM335x Generation
=================

Summary
-------

This document covers various features of the `am335x_evm` default
configuration, some of the related defconfigs, and how to enable hardware
features not present by default in the defconfigs.

Hardware
--------

The binary produced by this board supports, based on parsing of the EEPROM
documented in TI's reference designs:
* AM335x GP EVM
* AM335x EVM SK
* The Beaglebone family of designs

Customization
-------------

Given that all of the above boards are reference platforms (and the
Beaglebone platforms are OSHA), it is likely that this platform code and
configuration will be used as the basis of a custom platform.  It is
worth noting that aside from things such as NAND or MMC only being
required if a custom platform makes use of these blocks, the following
are required, depending on design:

* GPIO is only required if DDR3 power is controlled in a way similar to EVM SK
* SPI is only required for SPI flash, or exposing the SPI bus.

The following blocks are required:

* I2C, to talk with the PMIC and ensure that we do not run afoul of
  errata 1.0.24.

When removing options as part of customization, note that you will likely need
to look at both `include/configs/am335x_evm.h`,
`include/configs/ti_am335x_common.h` and `include/configs/am335x_evm.h` as the
migration to Kconfig is not yet complete.

Secure Boot
-----------

.. secure_boot_include_start_config_ti_secure_device

Secure TI devices require a boot image that is authenticated by ROM
code to function. Without this, even JTAG remains locked and the
device is essentially useless. In order to create a valid boot image for
a secure device from TI, the initial public software image must be signed
and combined with various headers, certificates, and other binary images.

Information on the details on the complete boot image format can be obtained
from Texas Instruments. The tools used to generate boot images for secure
devices are part of a secure development package (SECDEV) that can be
downloaded from:

	https://www.ti.com/mysecuresoftware (login required)

The secure development package is access controlled due to NDA and export
control restrictions. Access must be requested and granted by TI before the
package is viewable and downloadable. Contact TI, either online or by way
of a local TI representative, to request access.

.. secure_boot_include_end_config_ti_secure_device

.. secure_boot_include_start_spl_boot

1. Booting of U-Boot SPL
^^^^^^^^^^^^^^^^^^^^^^^^

When CONFIG_TI_SECURE_DEVICE is set, the U-Boot SPL build process
requires the presence and use of these tools in order to create a
viable boot image. The build process will look for the environment
variable TI_SECURE_DEV_PKG, which should be the path of the installed
SECDEV package. If the TI_SECURE_DEV_PKG variable is not defined or
if it is defined but doesn't point to a valid SECDEV package, a
warning is issued during the build to indicate that a final secure
bootable image was not created.

Within the SECDEV package exists an image creation script:

.. prompt:: bash $

   ${TI_SECURE_DEV_PKG}/scripts/create-boot-image.sh

This is called as part of the SPL/u-boot build process. As the secure
boot image formats and requirements differ between secure SOC from TI,
the purpose of this script is to abstract these details as much as
possible.

The script is basically the only required interface to the TI SECDEV
package for creating a bootable SPL image for secure TI devices.

.. prompt:: bash $

   create-boot-image.sh \
		<IMAGE_FLAG> <INPUT_FILE> <OUTPUT_FILE> <SPL_LOAD_ADDR>

.. secure_boot_include_end_spl_boot

<IMAGE_FLAG> is a value that specifies the type of the image to
generate OR the action the image generation tool will take. Valid
values are:

.. list-table::
   :widths: 25 25
   :header-rows: 0

   * - PI_X-LOADER
     - Generates an image for SPI flash (byte swapped)
   * - X-LOADER
     - Generates an image for non-XIP flash
   * - MLO
     - Generates an image for SD/MMC/eMMC media
   * - 2ND
     - Generates an image for USB, UART and Ethernet
   * - XIP_X-LOADER
     - Generates a single stage u-boot for NOR/QSPI XiP

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
   * - u-boot-spl_HS_X-LOADER
     - boot image for NAND or SD/MMC/eMMC rawmode
   * - u-boot-spl_HS_MLO
     - boot image for SD/MMC/eMMC media
   * - u-boot-spl_HS_2ND
     - boot image for USB, UART and Ethernet
   * - u-boot_HS_XIP_X-LOADER
     - boot image for NOR or QSPI Xip flash

<SPL_LOAD_ADDR> is the address at which SOC ROM should load the
<INPUT_FILE>

.. secure_boot_include_start_primary_u_boot

2. Booting of Primary U-Boot (u-boot.img)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The SPL image is responsible for loading the next stage boot loader,
which is the main u-boot image. For secure TI devices, the SPL will
be authenticated, as described above, as part of the particular
device's ROM boot process. In order to continue the secure boot
process, the authenticated SPL must authenticate the main u-boot
image that it loads.

The configurations for secure TI platforms are written to make the boot
process use the FIT image format for the u-boot.img (CONFIG_SPL_FRAMEWORK
and CONFIG_SPL_LOAD_FIT). With these configurations the binary
components that the SPL loads include a specific DTB image and u-boot
image. These DTB image may be one of many available to the boot
process. In order to secure these components so that they can be
authenticated by the SPL as they are loaded from the FIT image,	the
build procedure for secure TI devices will secure these images before
they are integrated into the FIT image. When those images are extracted
from the FIT image at boot time, they are post-processed to verify that
they are still secure. The outlined security-related SPL post-processing
is enabled through the CONFIG_SPL_FIT_IMAGE_POST_PROCESS option which
must be enabled for the secure boot scheme to work. In order to allow
verifying proper operation of the secure boot chain in case of successful
authentication messages like "Authentication passed" are output by the
SPL to the console for each blob that got extracted from the FIT image.

The exact details of the how the images are secured is handled by the
SECDEV package. Within the SECDEV package exists a script to process
an input binary image:

.. prompt:: bash $

   ${TI_SECURE_DEV_PKG}/scripts/secure-binary-image.sh

This is called as part of the u-boot build process. As the secure
image formats and requirements can differ between the various secure
SOCs from TI, this script in the SECDEV package abstracts these
details. This script is essentially the only required interface to the
TI SECDEV package for creating a u-boot.img image for secure TI
devices.

The SPL/u-boot code contains calls to dedicated secure ROM functions
to perform the validation on the secured images. The details of the
interface to those functions is shown in the code. The summary
is that they are accessed by invoking an ARM secure monitor call to
the device's secure ROM (fixed read-only-memory that is secure and
only accessible when the ARM core is operating in the secure mode).

Invoking the secure-binary-image script for Secure Devices
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. prompt:: bash $

   secure-binary-image.sh <INPUT_FILE> <OUTPUT_FILE>

<INPUT_FILE> is the full path and filename of the input binary image

<OUTPUT_FILE> is the full path and filename of the output secure image.

.. secure_boot_include_end_primary_u_boot

NAND
----

The AM335x GP EVM ships with a 256MiB NAND available in most profiles.  In
this example to program the NAND we assume that an SD card has been
inserted with the files to write in the first SD slot and that mtdparts
have been configured correctly for the board. All images are first loaded
into memory, then written to NAND.

1. Building u-boot for NAND boot

.. list-table:: CONFIGxx options for NAND device
   :widths: 25 25
   :header-rows: 1

   * - Config
     - Description
   * - CONFIG_SYS_NAND_PAGE_SIZE
     - number of main bytes in NAND page
   * - CONFIG_SYS_NAND_OOBSIZE
     - number of OOB bytes in NAND page
   * - CONFIG_SYS_NAND_BLOCK_SIZE
     - number of bytes in NAND erase-block
   * - CFG_SYS_NAND_ECCPOS
     - ECC map for NAND page
   * - CONFIG_NAND_OMAP_ECCSCHEME
     - (refer doc/README.nand)

2. Flashing NAND via MMC/SD

.. prompt:: bash =>

   # select BOOTSEL to MMC/SD boot and boot from MMC/SD card
   mmc rescan
   # erase flash
   nand erase.chip
   env default -f -a
   saveenv
   # flash MLO. Redundant copies of MLO are kept for failsafe
   load mmc 0 0x82000000 MLO
   nand write 0x82000000 0x00000 0x20000
   nand write 0x82000000 0x20000 0x20000
   nand write 0x82000000 0x40000 0x20000
   nand write 0x82000000 0x60000 0x20000
   # flash u-boot.img
   load mmc 0 0x82000000 u-boot.img
   nand write 0x82000000 0x80000 0x60000
   # flash kernel image
   load mmc 0 0x82000000 uImage
   nand write 0x82000000 ${nandsrcaddr} ${nandimgsize}
   # flash filesystem image
   load mmc 0 0x82000000 filesystem.img
   nand write 0x82000000 ${loadaddress} 0x300000

3. Set BOOTSEL pin to select NAND boot, and POR the device.
	The device should boot from images flashed on NAND device.


Falcon Mode
-----------

The default build includes "Falcon Mode" (see doc/README.falcon) via NAND,
eMMC (or raw SD cards) and FAT SD cards.  Our default behavior currently is
to read a 'c' on the console while in SPL at any point prior to loading the
OS payload (so as soon as possible) to opt to booting full U-Boot.  Also
note that while one can program Falcon Mode "in place" great care needs to
be taken by the user to not 'brick' their setup.  As these are all eval
boards with multiple boot methods, recovery should not be an issue in this
worst-case however.

Falcon Mode: eMMC
-----------------

The recommended layout in this case is:

.. list-table:: eMMC Recommended Layout
   :widths: 25 25 50
   :header-rows: 1

   * - MMC Blocks
     - Description
     - Location in bytes
   * - 0x0000 - 0x007F
     - MBR or GPT table
     - 0x000000 - 0x020000
   * - 0x0080 - 0x00FF
     - ARGS or FDT file
     - 0x010000 - 0x020000
   * - 0x0100 - 0x01FF
     - SPL.backup1 (first copy used)
     - 0x020000 - 0x040000
   * - 0x0200 - 0x02FF
     - SPL.backup2 (second copy used)
     - 0x040000 - 0x060000
   * - 0x0300 - 0x06FF
     - U-Boot
     - 0x060000 - 0x0e0000
   * - 0x0700 - 0x08FF
     - U-Boot Env + Redundant
     - 0x0e0000 - 0x120000
   * - 0x0900 - 0x28FF
     - Kernel
     - 0x120000 - 0x520000

Note that when we run 'spl export' it will prepare to boot the kernel.
This includes relocation of the uImage from where we loaded it to the entry
point defined in the header.  As these locations overlap by default, it
would leave us with an image that if written to MMC will not boot, so
instead of using the loadaddr variable we use 0x81000000 in the following
example.  In this example we are loading from the network, for simplicity,
and assume a valid partition table already exists and 'mmc dev' has already
been run to select the correct device.  Also note that if you previously
had a FAT partition (such as on a Beaglebone Black) it is not enough to
write garbage into the area, you must delete it from the partition table
first.

.. prompt:: bash =>

   # Ensure we are able to talk with this mmc device
   mmc rescan
   tftp 81000000 am335x/MLO
   # Write to two of the backup locations ROM uses
   mmc write 81000000 100 100
   mmc write 81000000 200 100
   # Write U-Boot to the location set in the config
   tftp 81000000 am335x/u-boot.img
   mmc write 81000000 300 400
   # Load kernel and device tree into memory, perform export
   tftp 81000000 am335x/uImage
   run findfdt
   tftp ${fdtaddr} am335x/${fdtfile}
   run mmcargs
   spl export fdt 81000000 - ${fdtaddr}
   # Write the updated device tree to MMC
   mmc write ${fdtaddr} 80 80
   # Write the uImage to MMC
   mmc write 81000000 900 2000

Falcon Mode: FAT SD cards
-------------------------

In this case the additional file is written to the filesystem.  In this
example we assume that the uImage and device tree to be used are already on
the FAT filesystem (only the uImage MUST be for this to function
afterwards) along with a Falcon Mode aware MLO and the FAT partition has
already been created and marked bootable:

.. prompt:: bash =>

   mmc rescan
   # Load kernel and device tree into memory, perform export
   load mmc 0:1 ${loadaddr} uImage
   run findfdt
   load mmc 0:1 ${fdtaddr} ${fdtfile}
   run mmcargs
   spl export fdt ${loadaddr} - ${fdtaddr}

This will print a number of lines and then end with something like:

.. code-block:: bash

   Using Device Tree in place at 80f80000, end 80f85928
   Using Device Tree in place at 80f80000, end 80f88928

So then you:

.. prompt:: bash =>

   fatwrite mmc 0:1 0x80f80000 args 8928

Falcon Mode: NAND
-----------------

In this case the additional data is written to another partition of the
NAND.  In this example we assume that the uImage and device tree to be are
already located on the NAND somewhere (such as filesystem or mtd partition)
along with a Falcon Mode aware MLO written to the correct locations for
booting and mtdparts have been configured correctly for the board:

.. prompt:: bash =>

   nand read ${loadaddr} kernel
   load nand rootfs ${fdtaddr} /boot/am335x-evm.dtb
   run nandargs
   spl export fdt ${loadaddr} - ${fdtaddr}
   nand erase.part u-boot-spl-os
   nand write ${fdtaddr} u-boot-spl-os

USB device
----------

The platform code for am33xx based designs is legacy in the sense that
it is not fully compliant with the driver model in its management of the
various resources. This is particularly true for the USB Ethernet gadget
which will automatically be bound to the first USB Device Controller
(UDC). This make the USB Ethernet gadget work out of the box on common
boards like the Beagle Bone Blacks and by default will prevents other
gadgets to be used.

The output of the 'dm tree' command shows which driver is bound to which
device, so the user can easily configure their platform differently from
the command line:

.. prompt:: bash =>

   dm tree

.. code-block:: text

   Class     Index  Probed  Driver                Name
  -----------------------------------------------------------
  [...]
  misc          0  [ + ]   ti-musb-wrapper       |   |-- usb@47400000
  usb           0  [ + ]   ti-musb-peripheral    |   |   |-- usb@47401000
  ethernet      1  [ + ]   usb_ether             |   |   |   `-- usb_ether
  bootdev       3  [   ]   eth_bootdev           |   |   |       `-- usb_ether.bootdev
  usb           0  [   ]   ti-musb-host          |   |   `-- usb@47401800

Typically here any network command performed using the usb_ether
interface would work, while using other gadgets would fail:

.. prompt:: bash =>

   fastboot usb 0

.. code-block:: text

  All UDC in use (1 available), use the unbind command
  g_dnl_register: failed!, error: -19
  exit not allowed from main input shell.

As hinted by the primary error message, the only controller available
(usb@47401000) is currently bound to the usb_ether driver, which makes
it impossible for the fastboot command to bind with this device (at
least from a bootloader point of view). The solution here would be to
use the unbind command specifying the class and index parameters (as
shown above in the 'dm tree' output) to target the driver to unbind:

.. prompt:: bash =>

   unbind ethernet 1

The output of the 'dm tree' command now shows the availability of the
first USB device controller, the fastboot gadget will now be able to
bind with it:

.. prompt:: bash =>

   dm tree

.. code-block:: text

  Class     Index  Probed  Driver                Name
  -----------------------------------------------------------
  [...]
  misc          0  [ + ]   ti-musb-wrapper       |   |-- usb@47400000
  usb           0  [   ]   ti-musb-peripheral    |   |   |-- usb@47401000
  usb           0  [   ]   ti-musb-host          |   |   `-- usb@47401800
