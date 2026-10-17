.. SPDX-License-Identifier: GPL-2.0

Arm Versatile Express
=====================

The vexpress_* board configuration supports the following platforms:

 * FVP_Base_RevC-2xAEMvA
 * FVP_BaseR_AEMv8R
 * Juno development board

Fixed Virtual Platforms
-----------------------

The Fixed Virtual Platforms (FVP) are complete simulations of an Arm system,
including processor, memory and peripherals. They are set out in a "programmer's
view", which gives a comprehensive model on which to build and test software.

The supported FVPs are available free of charge and can be downloaded from the
Arm developer site [1]_ (user registration might be required).

Supported features:

 * GICv3
 * Generic timer
 * PL011 UART

The default configuration assumes that U-Boot is bootstrapped using a suitable
bootloader, such as Trusted Firmware-A [4]_. The u-boot binary can be passed
into the TF-A build: ``make PLAT=<platform> all fip BL33=u-boot.bin``

The FVPs can be debugged using Arm Development Studio [2]_.

Juno
----

Juno is an Arm development board with the following features:

 * Arm Cortex-A72/A57 and Arm Cortex-A53 in a "big.LITTLE" configuration
 * A PCIe Gen2.0 bus with 4 lanes
 * 8GB of DRAM
 * GICv2

More details can be found in the board documentation [3]_.

References
----------

.. [1] https://developer.arm.com/tools-and-software/simulation-models/fixed-virtual-platforms
.. [2] https://developer.arm.com/tools-and-software/embedded/arm-development-studio
.. [3] https://developer.arm.com/tools-and-software/development-boards/juno-development-board
.. [4] https://trustedfirmware-a.readthedocs.io/