.. SPDX-License-Identifier: GPL-2.0+
.. Copyright (C) 2023 Linus Walleij <linus.walleij@linaro.org>

Broadcom Northstar Boards
=========================

This document describes how to use U-Boot on the Broadcom Northstar
boards, comprised of the Cortex A9 ARM-based BCM470x and BCM5301x SoCs. These
were introduced in 2012-2013 and some of them are also called StrataGX.

Northstar is part of the iProc SoC family.

A good overview of these boards can be found in Jon Mason's presentation
"Enabling New Hardware in U-Boot" where the difference between Northstar
and Northstar Plus and Northstar 2 (Aarch64) is addressed.

The ROM in the Northstar SoC will typically look into NOR flash memory
for a boot loader, and the way this works is undocumented. It should be
possible to execute U-Boot as the first binary from the NOR flash but
this usage path is unexplored. Please add information if you know more.

D-Link Boards
-------------

When we use U-Boot with D-Link routers, the NOR flash has a boot loader
and web server that can re-flash the bigger NAND flash memory for object
code in the SEAMA format, so on these platforms U-Boot is converted into
a SEAMA binary and installed in the SoC using the flash tool resident in
the NOR flash. Details can be found in the OpenWrt project codebase.

Configure
---------

.. code-block:: console

	$ make CROSS_COMPILE=${CROSS_COMPILE} bcmns_defconfig

Build
-----

.. code-block:: console

	$ make CROSS_COMPILE=${CROSS_COMPILE}
	$ ${CROSS_COMPILE}strip u-boot
