.. SPDX-License-Identifier: GPL-2.0+
.. sectionauthor:: Stephan Gerhold <stephan@gerhold.net>

ST-Ericsson U8500 Samsung "stemmy" board
========================================

The "stemmy" board supports Samsung smartphones released with
the ST-Ericsson NovaThor U8500 SoC, e.g.

    +---------------------------+----------+--------------+----------------+
    | Device                    | Model    | Codename     | U-Boot         |
    +===========================+==========+==============+================+
    | Samsung Galaxy Ace 2      | GT-I8160 | codina       | ``u-boot.bin`` |
    +---------------------------+----------+--------------+----------------+
    | Samsung Galaxy Amp        | SGH-I407 | kyle         | ``u-boot.img`` |
    +---------------------------+----------+--------------+----------------+
    | Samsung Galaxy Beam       | GT-I8530 | gavini       | ``u-boot.bin`` |
    +---------------------------+----------+--------------+----------------+
    | Samsung Galaxy Exhibit    | SGH-T599 | codina (TMO) | ``u-boot.bin`` |
    +---------------------------+----------+--------------+----------------+
    | Samsung Galaxy S Advance  | GT-I9070 | janice       | ``u-boot.bin`` |
    +---------------------------+----------+--------------+----------------+
    | Samsung Galaxy S III mini | GT-I8190 | golden       | ``u-boot.img`` |
    +---------------------------+----------+--------------+----------------+
    | Samsung Galaxy Xcover 2   | GT-S7710 | skomer       | ``u-boot.img`` |
    +---------------------------+----------+--------------+----------------+

At the moment, U-Boot is intended to be chain-loaded from
the original Samsung bootloader, not replacing it entirely.

Installation
------------
First, setup ``CROSS_COMPILE`` for ARMv7. Then, build U-Boot for ``stemmy``::

  $ export CROSS_COMPILE=arm-none-eabi-
  $ make stemmy_defconfig
  $ make

This will build ``u-boot.bin`` in the configured output directory.

For newer devices (check ``u-boot.img`` in the table above), the U-Boot binary
has to be packed into an Android boot image. Devices with ``u-boot.bin`` boot
the raw U-Boot binary from the boot partition. You can build the Android boot
image with ``mkbootimg``, e.g. from from android-7.1.2_r37_::

  $ mkbootimg \
    --kernel=u-boot.bin \
    --base=0x00000000 \
    --kernel_offset=0x00100000 \
    --ramdisk_offset=0x02000000 \
    --tags_offset=0x00000100 \
    --output=u-boot.img

.. _android-7.1.2_r37: https://android.googlesource.com/platform/system/core/+/refs/tags/android-7.1.2_r37/mkbootimg/mkbootimg

To flash the U-Boot binary, enter the Samsung download mode
(press Power + Home + Volume Down). Use Heimdall_ to flash the U-Boot image to
the Android boot partition::

  $ heimdall flash --Kernel u-boot.(bin|img)

If this is not working but there are messages like ``Android recovery image`` in
the UART console, you can try flashing to the recovery partition instead::

  $ heimdall flash --Kernel2 u-boot.(bin|img)

.. _Heimdall: https://gitlab.com/BenjaminDobell/Heimdall

After a reboot the U-Boot prompt should appear via UART. Unless interrupted it
automatically boots to USB Fastboot mode where Android boot images can be booted
via ``fastboot boot boot.img``. It is mainly intended to boot mainline Linux,
but booting original Samsung Android boot images is also supported (e.g. for
charging).

UART
----
UART is available through the micro USB port, similar to the Carkit standard.
With a ~619kOhm resistor between ID and GND, 1.8V RX/TX is available at D+/D-.

.. note::
  Make sure to connect the UART cable **before** turning on the phone.
