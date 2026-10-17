.. SPDX-License-Identifier: GPL-2.0+

ToC:
- Introduction
- Booting
- Debugging
- i2c


Introduction
============
The Embedded Artists LPC3250 Developer's Kit v2 features the LPC3250 SoC
which is based on the ARM926EJ-S CPU. The kit features a base board and
a removable OEM board which features the SoC. Details, schematics, and
documentation are available from the Embedded Artists product website:

	https://www.embeddedartists.com/products/lpc3250-developers-kit-v2/

The base board includes::
- 200 pos, 0.6mm pitch SODIMM connector for OEM Board
- LCD expansion connector with control signals for touch screen interface
- Expansion connector with all OEM Board signals
- Ethernet connector (RJ45)
- CAN interface & connector (provision for second CAN interface, but not mounted)
- MMC/SD interface & connector
- USB1: OTG or Host interface & connector
- USB2: Device or Host interface & connector
- Provision for NXP JN5148 RF module (former Jennic) interface (RF module not included)
- Full modem RS232 (cannot be fully used on 32-bit databus OEM boards)
- RS422/485 interface & connector
- Provision for IrDA transceiver interface (transceiver not mounted)
- I2S audio codec (mic in, line in, line out, headphone out)
- SWD/JTAG connector
- Trace connector and pads for ETM connector
- Serial Expansion Connector, 14-pos connector with UART/I2C/SPI/GPIO pins
- Power supply, either via USB or external 5V DC
- Optional coin cell battery for RTC and LED on ALARM output (coin cell not included)
- OEM Board current measuring
- Parallel NOR flash on external memory bus
- 16-bit register and LEDs on external memory bus
- 5-key joystick
- LM75 temperature sensor (I2C connected)
- 5 push-button keys (four via I2C and one on ISP-ENABLE)
- 9 LEDs (8 via I2C and one on ISP-ENABLE)
- Trimming potentiometer to analog input
- USB-to-serial bridge on UART #0 (FT232R) and ISP functionality
- Reset push-button and LED
- Speaker output on analog output from OEM Board, or from I2S audio codec
- 160x150 mm in size

The OEM board::
- ARMv5 ARM926EJ-S @ 266 MHz with hard-float VFPv2
- 256 KByte IRAM, 64 MByte SDRAM
- 128 MByte NAND flash
- 4 MByte NOR Flash
- Graphics Output: Parallel RGB
- Hardware 2D/3D Graphic: No
- Hardware Video: SW only
- Graphics input: No
- Audio: I2S
- Ethernet: 10/100 Mbps
- USB: 1x FS USB 2.0 OTG
- Wi-Fi: No
- FlexIO: No
- Serial: 2x I2C, 2x SPI, 7x UART
- ADC/PWM: 3 ch (10-bit) / 2 ch
- SD: MCI
- PCIe: No
- Serial ATA: No
- Size: 68 x 48 mm
- Connector: 200 pos SODIMM


Booting
=======
The processor will start its code execution from an internal ROM,
containing the boot code. This boot loader can load code from one of four
external sources to internal RAM (IRAM) at address 0x0::
- UART5
- SSP0 (in SPI mode)
- EMC Static CS0 memory
- NAND FLASH

The ROM boot loader loads code as a single contiguous block at a maximum
size of 56 kByte. Programs larger than this size must be loaded in more
steps, for example, by a secondary boot loader.

Kickstart Loader
----------------
By default the Embedded Artists LPC3250 OEM Board is programmed with the
kickstart loader in block 0 of the NAND flash. The responsibility of this
loader is to load an application stored in block 1 and onwards of the NAND
flash. The kickstart loader will load the application into internal RAM
(IRAM) at address 0x0.

Stage 1 Loader (s1l)
--------------------
By default the Embedded Artists LPC3250 OEM Board is programmed with the
stage 1 loader (s1l) in block 1 of the NAND flash. This application will be
loaded by the kickstart loader when the LPC3250 OEM Board powers up. The
S1L loader will initialize the board, such as clocks and external memory
and then start a console where you can give input commands to the loader.
S1L offers the following booting options::
- MMC/SD card
- UART5
- NAND Flash

U-Boot with kickstart+s1l
-------------------------
Out of the box, the easiest way to get U-Boot running on the EA LPC3250
DevKit v2 board is to build the ea-lpc3250devkitv2_defconfig, copy the
resulting u-boot.bin to a vfat-formatted MMC/SD card, insert the MMC/SD card
into the MMC/SD card slot on the board, reset the board (SW1), and::

	Embedded Artist 3250 Board (S1L 2.0)
	Build date: Oct 31 2016 13:00:37

	EA3250>load blk u-boot.bin raw 0x83000000
	File loaded successfully

	EA3250>exec 0x83000000


Debugging
=========
JTAG debugging of the Embedded Artists LPC3250 Developer's Kit v2 board is
easy thanks to the included/populated 20-pin JTAG port on the main board (J8).
openocd 0.11 has been used with this board along with the ARM-USB-OCD-H JTAG
dongle from Olimex successfully as follows:

	# openocd \
		-f interface/ftdi/olimex-arm-usb-ocd-h.cfg \
		-f board/phytec_lpc3250.cfg


i2c
===
Some of the LEDs on the board are connected via an I/O Expander (PCA9532) that
is attached to the i2c1 bus. Here is a sample session of toggling some of
these LEDs via i2c in U-Boot:

show the existing i2c busses:
	EA-LPC3250v2=> i2c bus
	Bus 0:  i2c@300
	Bus 1:  i2c@400a0000
	Bus 2:  i2c@400a8000

set i2c1 as the current bus:
	EA-LPC3250v2=> i2c dev 1
	Setting bus to 1

see what potential devices are found with rudimentary probing on i2c1:
	EA-LPC3250v2=> i2c probe
	Valid chip addresses: 1A 1D 48 50 57 60 66 6E

According to the schematics the i2c slave address of the PCA9532 is 0x60.

dump all of the 10 registers from the I/O Expander; NOTE that the 0x10 in the
command specifies the self-incrementing mode of the PCA9532; also NOTE that
the values repeat themseves to fill out a full 16 bytes:
	EA-LPC3250v2=> i2c md 0x60 0x10 10
	0010: 00 ff 00 80 00 80 00 00 00 00 4f ff 00 80 00 80    ..........O.....

turn on LEDs 23, 25, 27, and 29 (green):
	EA-LPC3250v2=> i2c mw 0x60 9 0x55

turn on LEDs 22, 24, 26, and 28 (red):
	EA-LPC3250v2=> i2c mw 0x60 8 0x55

dim the green LEDs (23, 25, 27, 29):
	EA-LPC3250v2=> i2c mw 0x60 3 0x20
	EA-LPC3250v2=> i2c mw 0x60 9 0xaa

turn off all LEDs (23-29):
	EA-LPC3250v2=> i2c mw 0x60 8 0
	EA-LPC3250v2=> i2c mw 0x60 9 0

read value of switches (input):
	EA-LPC3250v2=> i2c md 0x60 0 1
	0000: 4f    O
[none are pressed]

press and hold SW2 while running the following:
	EA-LPC3250v2=> i2c md 0x60 0 1
	0000: 4e    N
[SW2 is pressed]
