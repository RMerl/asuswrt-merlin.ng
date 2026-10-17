.. SPDX-License-Identifier: GPL-2.0+
.. sectionauthor:: Stephan Gerhold <stephan@gerhold.net>

DragonBoard 410c
================

The DragonBoard 410c is a development board based on the Qualcomm APQ8016E SoC.
More information can be found on the `96Boards product page`_.

U-Boot can be used as a replacement for Qualcomm's original Android bootloader
(a fork of Little Kernel/LK). Like LK, it is installed directly into the ``aboot``
partition. Note that the U-Boot port used to be loaded as an Android boot image
through LK. This is no longer the case, now U-Boot can replace LK entirely.

.. _96Boards product page: https://www.96boards.org/product/dragonboard410c/

Installation
------------
First, setup ``CROSS_COMPILE`` for aarch64. Then, build U-Boot for ``dragonboard410c``::

  $ export CROSS_COMPILE=<aarch64 toolchain prefix>
  $ make dragonboard410c_defconfig
  $ make

This will build ``u-boot.elf`` in the configured output directory.

Although the DragonBoard 410c does not have secure boot set up by default,
the firmware still expects firmware ELF images to be "signed". The signature
does not provide any security in this case, but it provides the firmware with
some required metadata.

To "sign" ``u-boot.elf`` you can use e.g. `qtestsign`_::

  $ ./qtestsign.py aboot u-boot.elf

Then install the resulting ``u-boot-test-signed.mbn`` to the ``aboot`` partition
on your device, e.g. with ``fastboot flash aboot u-boot-test-signed.mbn``.

U-Boot should be running after a reboot (``fastboot reboot``).

.. _qtestsign: https://github.com/msm8916-mainline/qtestsign

Usage
-----
Press Volume Down during boot to enter Fastboot mode.
