.. SPDX-License-Identifier: GPL-2.0+

NVM XIP Block Storage Emulation Driver
======================================

Summary
-------

Non-Volatile Memory devices with addressable memory (e.g: QSPI NOR flash) could
be used for block storage needs (e.g: parsing a GPT layout in a raw QSPI NOR flash).

The NVMXIP Uclass provides this functionality and can be used for any 64-bit platform.

The NVMXIP Uclass provides the following drivers:

      nvmxip-blk block driver:

        A generic block driver allowing to read from the XIP flash.
	The driver belongs to UCLASS_BLK.
	The driver implemented by drivers/mtd/nvmxip/nvmxip.c

      nvmxip Uclass driver:

        When a device is described in the DT and associated with UCLASS_NVMXIP,
        the Uclass creates a block device and binds it with the nvmxip-blk.
	The Uclass driver implemented by drivers/mtd/nvmxip/nvmxip-uclass.c

      nvmxip_qspi driver :

        The driver probed with the DT and is the parent of the blk#<id> device.
        nvmxip_qspi can be reused by other platforms. If the platform
        has custom settings to apply before using the flash, then the platform
        can provide its own parent driver belonging to UCLASS_NVMXIP and reuse
        nvmxip-blk. The custom driver can be implemented like nvmxip_qspi in
        addition to the platform custom settings.
	The nvmxip_qspi driver belongs to UCLASS_NVMXIP.
	The driver implemented by drivers/mtd/nvmxip/nvmxip_qspi.c

	For example, if we have two NVMXIP devices described in the DT
	The devices hierarchy is as follows:

::

   => dm tree

        Class     Index  Probed  Driver                Name
    -----------------------------------------------------------
    ...
     nvmxip        0  [ + ]   nvmxip_qspi           |-- nvmxip-qspi1@08000000
     blk           3  [ + ]   nvmxip-blk                    |   `-- nvmxip-qspi1@08000000.blk#1
     nvmxip        1  [ + ]   nvmxip_qspi           |-- nvmxip-qspi2@08200000
     blk           4  [ + ]   nvmxip-blk                    |   `-- nvmxip-qspi2@08200000.blk#2

The implementation is generic and can be used by different platforms.

Supported hardware
------------------

Any plaform supporting readq().

Configuration
-------------

config NVMXIP
	  This option allows the emulation of a block storage device
	  on top of a direct access non volatile memory XIP flash devices.
	  This support provides the read operation.
	  This option provides the block storage driver nvmxip-blk which
	  handles the read operation. This driver is HW agnostic and can support
	  multiple flash devices at the same time.

config NVMXIP_QSPI
	  This option allows the emulation of a block storage device on top of a QSPI XIP flash.
	  Any platform that needs to emulate one or multiple QSPI XIP flash devices can turn this
	  option on to enable the functionality. NVMXIP config is selected automatically.
	  Platforms that need to add custom treatments before accessing to the flash, can
	  write their own driver (same as nvmxip_qspi in addition to the custom settings).

Device Tree nodes
-----------------

Multiple QSPI XIP flash devices can be used at the same time by describing them through DT
nodes.

Please refer to the documentation of the DT binding at:

doc/device-tree-bindings/nvmxip/nvmxip_qspi.txt

Contributors
------------
   * Abdellatif El Khlifi <abdellatif.elkhlifi@arm.com>
