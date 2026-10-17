.. SPDX-License-Identifier: GPL-2.0+

Sipeed's Lichee PI 4A based on T-HEAD TH1520 SoC
================================================

The LicheePi4A is a high-performance RISC-V SBC based on TH1520(4xC910@1.85GHz),
comes with 4/8/16 GB RAM, and up to 128GB eMMC, and rich peripherals.

 - SoC            T-HEAD TH1520 SoC
 - System Memory  4GB, 8GB, or 16GB LPDDR4X
 - Storage        eMMC flash with 8/32/128 GB
 -                external microSD slot
 - Networking     2x Gigabit Ethernet
 -                WiFi+BT
 - Display        HDMI2.0, 4-lane MIPI DSI
 - Camera         4-lane MIPI CSI + 2x2-lane MIPI CSI
 - Audio          Onboard Speaker, 2xMEMS MIC, 3.5mm headphone jack
 - USB            4xUSB3.0 Type-A, 1xUSB2.0 Type-C
 - GPIO           2x10Pin breakout, UART/IIC/SPI
 - Power          DC 12V/2A, POE 5V/2.4A, USB Type-C 5V/2A

TH1520 RISC-V SoC
-----------------

The TH1520 SoC consist of quad-core RISC-V Xuantie C910 (RV64GCV) processor,
Xuantie C906 audio DSP, low power Xuantie E902 core, it also integrate
Imagination GPU for graphics, and 4 TOPS NPU for AI acceleration.

Mainline support
----------------

The support for following drivers are already enabled:

1. ns16550 UART Driver.

Building
~~~~~~~~

1. Add the RISC-V toolchain to your PATH.
2. Setup ARCH & cross compilation environment variable:

.. code-block:: none

   export CROSS_COMPILE=<riscv64 toolchain prefix>

The U-Boot is capable of running in M-Mode, so we can directly build it.

.. code-block:: console

	cd <U-Boot-dir>
	make th1520_lpi4a_defconfig
	make

This will generate u-boot-dtb.bin

Booting
~~~~~~~

Currently, we rely on vendor u-boot to initialize the clock, pinctrl subsystem,
and chain load the mainline u-boot image either via tftp or emmc storage,
then bootup from it.

Sample boot log from Lichee PI 4A board via tftp
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: none

	brom_ver 8
	[APP][E] protocol_connect failed, exit.

	U-Boot SPL 2020.01-00016-g8c870a6be8 (May 20 2023 - 01:04:49 +0000)
	FM[1] lpddr4x dualrank freq=3733 64bit dbi_off=n sdram init
	ddr initialized, jump to uboot
	image has no header


	U-Boot 2020.01-00016-g8c870a6be8 (May 20 2023 - 01:04:49 +0000)

	CPU:   rv64imafdcvsu
	Model: T-HEAD c910 light
	DRAM:  8 GiB
	C910 CPU FREQ: 750MHz
	AHB2_CPUSYS_HCLK FREQ: 250MHz
	AHB3_CPUSYS_PCLK FREQ: 125MHz
	PERISYS_AHB_HCLK FREQ: 250MHz
	PERISYS_APB_PCLK FREQ: 62MHz
	GMAC PLL POSTDIV FREQ: 1000MHZ
	DPU0 PLL POSTDIV FREQ: 1188MHZ
	DPU1 PLL POSTDIV FREQ: 1188MHZ
	MMC:   sdhci@ffe7080000: 0, sd@ffe7090000: 1
	Loading Environment from MMC... OK
	Error reading output register
	Warning: cannot get lcd-en GPIO
	LCD panel cannot be found : -121
	splash screen startup cost 16 ms
	In:    serial
	Out:   serial
	Err:   serial
	Net:
	Warning: ethernet@ffe7070000 using MAC address from ROM
	eth0: ethernet@ffe7070000ethernet@ffe7070000:0 is connected to ethernet@ffe7070000.  Reconnecting to ethernet@ffe7060000

	Warning: ethernet@ffe7060000 (eth1) using random MAC address - 42:25:d4:16:5f:fc
	, eth1: ethernet@ffe7060000
	Hit any key to stop autoboot:  2
	ethernet@ffe7060000 Waiting for PHY auto negotiation to complete.. done
	Speed: 1000, full duplex
	Using ethernet@ffe7070000 device
	TFTP from server 192.168.8.50; our IP address is 192.168.8.45
	Filename 'u-boot-dtb.bin'.
	Load address: 0x1c00000
	Loading: * #########################
		 8 MiB/s
	done
	Bytes transferred = 376686 (5bf6e hex)
	## Starting application at 0x01C00000 ...

        U-Boot 2023.07-rc2-00004-g1befbe31c1 (May 23 2023 - 18:40:01 +0800)

        CPU:   rv64imafdc
        Model: Sipeed Lichee Pi 4A
        DRAM:  8 GiB
        Core:  13 devices, 6 uclasses, devicetree: separate
        Loading Environment from <NULL>... OK
        In:    serial@ffe7014000
        Out:   serial@ffe7014000
        Err:   serial@ffe7014000
        Model: Sipeed Lichee Pi 4A
        LPI4A=>
