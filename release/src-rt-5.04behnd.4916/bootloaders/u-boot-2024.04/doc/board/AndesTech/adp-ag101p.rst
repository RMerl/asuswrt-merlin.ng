.. SPDX-License-Identifier: GPL-2.0+

ADP-AG101P
==========

ADP-AG101P is the SoC with AG101 hardcore CPU.

AG101P SoC
----------

AG101P is the mainline SoC produced by Andes Technology using N1213 CPU core
with FPU and DDR contoller support.
AG101P has integrated both AHB and APB bus and many periphals for application
and product development.


Configurations
--------------

CONFIG_MEM_REMAP:
	Doing memory remap is essential for preparing some non-OS or RTOS
	applications.

CONFIG_SKIP_LOWLEVEL_INIT:
	If you want to boot this system from SPI ROM and bypass e-bios (the
	other boot loader on ROM). You should enable CONFIG_SKIP_LOWLEVEL_INIT
	when running menuconfig or similar.

Build and boot steps
--------------------

Build:

1. Prepare the toolchains and make sure the $PATH to toolchains is correct.
2. Use `make adp-ag101p_defconfig` in u-boot root to build the image.

Burn U-Boot to SPI ROM
----------------------

This section will be added later.
