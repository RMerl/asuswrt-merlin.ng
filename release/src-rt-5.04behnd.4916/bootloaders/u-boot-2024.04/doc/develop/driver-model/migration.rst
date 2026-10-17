.. SPDX-License-Identifier: GPL-2.0+

Migration Schedule
==================

U-Boot has been migrating to a new driver model since its introduction in
2014. This file describes the schedule for deprecation of pre-driver-model
features.

CONFIG_DM
---------

* Status: In progress
* Deadline: 2020.01

Starting with the 2020.01 release CONFIG_DM will be enabled for all boards.
This does not concern CONFIG_DM_SPL and CONFIG_DM_TPL. The conversion date for
these configuration items still needs to be defined.

CONFIG_DM_MMC
-------------

* Status: In progress
* Deadline: 2019.04

The subsystem itself has been converted and maintainers should submit patches
switching over to using CONFIG_DM_MMC and other base driver model options in
time for inclusion in the 2019.04 rerelease.

CONFIG_DM_USB
-------------

* Status: In progress
* Deadline: 2019.07

The subsystem itself has been converted along with many of the host controller
and maintainers should submit patches switching over to using CONFIG_DM_USB and
other base driver model options in time for inclusion in the 2019.07 rerelease.

CONFIG_SATA
-----------

* Status: In progress
* Deadline: 2019.07

The subsystem itself has been converted along with many of the host controller
and maintainers should submit patches switching over to using CONFIG_AHCI and
other base driver model options in time for inclusion in the 2019.07 rerelease.

CONFIG_BLK
----------

* Status: In progress
* Deadline: 2019.07

In concert with maintainers migrating their block device usage to the
appropriate DM driver, CONFIG_BLK needs to be set as well.  The final deadline
here coincides with the final deadline for migration of the various block
subsystems.  At this point we will be able to audit and correct the logic in
Kconfig around using CONFIG_PARTITIONS and CONFIG_SPL_LEGACY_BLOCK and make
use of CONFIG_BLK / CONFIG_SPL_BLK as needed.

CONFIG_DM_SPI / CONFIG_DM_SPI_FLASH
-----------------------------------

Board Maintainers should submit the patches for enabling DM_SPI and DM_SPI_FLASH
to move the migration with in the deadline.

Partially converted::

	drivers/spi/fsl_espi.c
	drivers/spi/mxc_spi.c
	drivers/spi/sh_qspi.c

* Status: In progress
* Deadline: 2019.07

CONFIG_DM_VIDEO
---------------
Deadline: 2019.07

The video subsystem has supported driver model since early 2016. Maintainers
should submit patches switching over to using CONFIG_VIDEO and other base
driver model options in time for inclusion in the 2019.07 release.

CONFIG_DM_ETH
-------------
Deadline: 2020.07

The network subsystem has supported the driver model since early 2015.
Maintainers should submit patches switching over to using CONFIG_DM_ETH and
other base driver model options in time for inclusion in the 2020.07 release.

CONFIG_DM_I2C
-------------
Deadline: 2021.10

The I2C subsystem has supported the driver model since early 2015.
Maintainers should submit patches switching over to using CONFIG_DM_I2C and
other base driver model options in time for inclusion in the 2021.10 release.

CFG_SYS_TIMER_RATE and CFG_SYS_TIMER_COUNTER
--------------------------------------------
Deadline: 2023.01

These are legacy options which have been replaced by driver model.
Maintainers should submit patches switching over to using CONFIG_TIMER and
other base driver model options in time for inclusion in the 2022.10 release.

There is only one method to implement, unless you want to support bootstage,
in which case you need an early timer also. For example drivers, see
sandbox_timer.c and rockchip_timer.c

CONFIG_DM_SERIAL
----------------
Deadline: 2023.04

The serial subsystem has supported the driver model since late 2014.
Maintainers should submit patches switching over to using CONFIG_DM_SERIAL and
other base driver model options in time for inclusion in the 2022.10 release.
