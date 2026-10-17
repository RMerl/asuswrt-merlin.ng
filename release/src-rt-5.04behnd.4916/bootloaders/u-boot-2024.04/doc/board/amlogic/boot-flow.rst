.. SPDX-License-Identifier: GPL-2.0+

Amlogic SoC Boot Flow
=====================

Amlogic SoCs follow a pre-defined boot sequence stored in SoC ROM code. The possible boot
sequences of the different SoC families are:

GX* & AXG Family
----------------

+----------+-------------------+---------+---------+---------+---------+
|          |   1               | 2       | 3       | 4       | 5       |
+==========+===================+=========+=========+=========+=========+
| S905     | POC=0: SPI NOR    | eMMC    | NAND    | SD      | USB     |
| S905D    |                   |         |         |         |         |
| S905L    |                   |         |         |         |         |
| S905W    |                   |         |         |         |         |
| S905X    |                   |         |         |         |         |
| S905Y    |                   |         |         |         |         |
| S912     |                   |         |         |         |         |
+----------+-------------------+---------+---------+---------+---------+
| S805X    | POC=0: SPI NOR    | eMMC    | NAND    | USB     | -       |
| A113D    |                   |         |         |         |         |
| A113X    |                   |         |         |         |         |
+----------+-------------------+---------+---------+---------+---------+

POC pin: `NAND_CLE`

Some boards provide a button to force USB boot by disabling the eMMC clock signal and
allowing the eMMC step to be bypassed. Others have removable eMMC modules; removing an
eMMC module and SD card will allow boot from USB.

An exception is the Libre Computer AML-S805X-XX (LaFrite) board which has no SD card
slot and boots from SPI. Booting a LaFrite board from USB requires either:

 - Erasing the first sectors of SPI NOR flash
 - Inserting an HDMI boot plug forcing boot over USB

The VIM1 and initial VIM2 boards provide a test point on the eMMC signals to block the
storage from answering, allowing boot to continue with the next boot step.

USB boot uses the first USB interface. On some boards this port is only available on a
USB-A type connector and requires a special Type-A to Type-A cable to communicate with
the BootROM.

G12* & SM1 Family
-----------------

+-------+-------+-------+------------+------------+------------+-----------+
| POC0  | POC1  | POC2  | 1          | 2          | 3          | 4         |
+=======+=======+=======+============+============+============+===========+
| 0     | 0     | 0     | USB        | SPI-NOR    | NAND/eMMC  | SD        |
+-------+-------+-------+------------+------------+-------------+----------+
| 0     | 0     | 1     | USB        | NAND/eMMC  | SD         | -         |
+-------+-------+-------+------------+------------+------------+-----------+
| 0     | 1     | 0     | SPI-NOR    | NAND/eMMC  | SD         | USB       |
+-------+-------+-------+------------+------------+------------+-----------+
| 0     | 1     | 1     | SPI-NAND   | NAND/eMMC  | USB        | -         |
+-------+-------+-------+------------+------------+------------+-----------+
| 1     | 0     | 0     | USB        | SPI-NOR    | NAND/eMMC  | SD        |
+-------+-------+-------+------------+------------+------------+-----------+
| 1     | 0     | 1     | USB        | NAND/eMMC  | SD         | -         |
+-------+-------+-------+------------+------------+------------+-----------+
| 1     | 1     | 0     | SPI-NOR    | NAND/eMMC  | SD         | USB       |
+-------+-------+-------+------------+------------+------------+-----------+
| 1     | 1     | 1     | NAND/eMMC  | SD         | USB        | -         |
+-------+-------+-------+------------+------------+------------+-----------+

The last option (1/1/1) is the normal default seen on production devices:

 * POC0 pin: `BOOT_4` (0 and all other 1 means SPI NAND boot first)
 * POC1 pin: `BOOT_5` (0 and all other 1 means USB Device boot first
 * POC2 pin: `BOOT_6` (0 and all other 1 means SPI NOR boot first)

Most boards provide a button to force USB BOOT which lowers `BOOT_5` to 0. Some boards
provide a test point on eMMC or SPI NOR clock signals to block storage from answering
and allowing boot to continue from the next boot step.

The Khadas VIM3/3L boards embed a microcontroller which sets POC signals according to
its configuration or a specific key press sequence to either boot from SPI NOR or eMMC
then SD card, or boot as a USB device.

The Odroid N2/N2+ has a hardware switch to select between SPI NOR or eMMC boot. The
Odroid HC4 has a button to disable SPI-NOR allowing boot from SD card.

Boot Modes
----------

 * SD

The BootROM fetches the first SD card sectors in one sequence then checks the content of
the data. It expects to find the FIP binary in sector 1, 512 bytes offset from the start.

 * eMMC

The BootROM fetches the first sectors of the main partition in one sequence then checks
the content of the data. On GXL and newer boards it expects to find the FIP binary in
sector 1, 512 bytes offset from the start. If not found it checks the boot0 partition,
then the boot1 partition. On GXBB it expects to find the FIP binary at an offset that
conflicts with MBR partition tables, but this has been worked around (thus avoiding the
need for a partition scheme that relocates the MBR). For a more detailed explanation
please see: https://github.com/LibreELEC/amlogic-boot-fip/pull/8

 * SPI-NOR

The BootROM fetches the first SPI NOR sectors in one sequence then checks the content of
the data. It expects to find the FIP binary in sector 1, 512 bytes offset from the start.

 * NAND & SPI-NAND

These modes are rarely used in open platforms and no details are available.

 * USB

The BootROM supports a custom USB protocol and sets the USB Gadget interface to use the
USB ID 1b8e:c003. The Amlogic `update` utility uses this protocol. It is also supported
in the Amlogic vendor U-Boot sources.

The `pyamlboot` utility https://github.com/superna9999/pyamlboot is open-source and also
implements the USB protocol. It can load U-Boot into memory to start the SoC without the
storage being attached, or to recover the device from a failed/incorrect image flash.

HDMI Recovery Dongle
--------------------

The BootROM also reads 8 bytes at address I2C 0x52 offset 0xf8 (248) on the HDMI DDC bus
during startup. The content `boot@USB` forces USB boot. The content `boot@SDC` forces SD
card boot. The content `boot@SPI` forces SPI-NOT boot. If an SD card or USB device does
not enumerate the BootROM continues with the normal boot sequence.

HDMI boot dongles can be created by connecting a 256bytes EEPROM set to answer on address
0x52, with `boot@USB` or `boot@SDC` or `boot@SPI` programmed at offset 0xf8 (248).

If the SoC is booted with USB Device forced at first step, it will retain the forced boot
order on warm reboot. Only cold reboot (removing power) will reset the boot order.
