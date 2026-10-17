.. SPDX-License-Identifier: GPL-2.0+

LS1046ARDB
==========

The LS1046A Reference Design Board (RDB) is a high-performance computing,
evaluation, and development platform that supports the QorIQ LS1046A
LayerScape Architecture processor. The LS1046ARDB provides SW development
platform for the Freescale LS1046A processor series, with a complete
debugging environment. The LS1046A RDB is lead-free and RoHS-compliant.

LS1046A SoC Overview
--------------------
Please refer arch/arm/cpu/armv8/fsl-layerscape/doc/README.soc for LS1046A
SoC overview.

LS1046ARDB board Overview
-------------------------
- SERDES1 Connections, 4 lanes supporting:

  - Lane0: 10GBase-R with x1 RJ45 connector
  - Lane1: 10GBase-R Cage
  - Lane2: SGMII.5
  - Lane3: SGMII.6

- SERDES2 Connections, 4 lanes supporting:

  - Lane0: PCIe1 with miniPCIe slot
  - Lane1: PCIe2 with PCIe x2 slot
  - Lane2: PCIe3 with PCIe x4 slot
  - Lane3: SATA

- DDR Controller

  - 8GB 64bits DDR4 SDRAM. Support rates of up to 2133MT/s

- IFC/Local Bus

  - One 512 MB NAND flash with ECC support
  - CPLD connection

- USB 3.0

  - one Type A port, one Micro-AB port

- SDHC: connects directly to a full SD/MMC slot
- DSPI: 64 MB high-speed flash Memory for boot code and storage (up to 108MHz)
- 4 I2C controllers
- UART

  - Two 4-pin serial ports at up to 115.2 Kbit/s
  - Two DB9 D-Type connectors supporting one Serial port each

- ARM JTAG support

Memory map from core's view
---------------------------

================== ================== ================ =====
Start Address      End Address        Description      Size
================== ================== ================ =====
``0x00_0000_0000`` ``0x00_000F_FFFF`` Secure Boot ROM  1M
``0x00_0100_0000`` ``0x00_0FFF_FFFF`` CCSRBAR          240M
``0x00_1000_0000`` ``0x00_1000_FFFF`` OCRAM0           64K
``0x00_1001_0000`` ``0x00_1001_FFFF`` OCRAM1           64K
``0x00_2000_0000`` ``0x00_20FF_FFFF`` DCSR             16M
``0x00_7E80_0000`` ``0x00_7E80_FFFF`` IFC - NAND Flash 64K
``0x00_7FB0_0000`` ``0x00_7FB0_0FFF`` IFC - CPLD       4K
``0x00_8000_0000`` ``0x00_FFFF_FFFF`` DRAM1            2G
``0x05_0000_0000`` ``0x05_07FF_FFFF`` QMAN S/W Portal  128M
``0x05_0800_0000`` ``0x05_0FFF_FFFF`` BMAN S/W Portal  128M
``0x08_8000_0000`` ``0x09_FFFF_FFFF`` DRAM2            6G
``0x40_0000_0000`` ``0x47_FFFF_FFFF`` PCI Express1     32G
``0x48_0000_0000`` ``0x4F_FFFF_FFFF`` PCI Express2     32G
``0x50_0000_0000`` ``0x57_FFFF_FFFF`` PCI Express3     32G
================== ================== ================ =====

QSPI flash map
--------------

================== ================== ================== =====
Start Address      End Address        Description        Size
================== ================== ================== =====
``0x00_4000_0000`` ``0x00_400F_FFFF`` RCW + PBI          1M
``0x00_4010_0000`` ``0x00_402F_FFFF`` U-Boot             2M
``0x00_4030_0000`` ``0x00_403F_FFFF`` U-Boot Env         1M
``0x00_4040_0000`` ``0x00_405F_FFFF`` PPA                2M
``0x00_4060_0000`` ``0x00_408F_FFFF`` Secure boot header 3M
                                      + bootscript
``0x00_4090_0000`` ``0x00_4093_FFFF`` FMan ucode         256K
``0x00_4094_0000`` ``0x00_4097_FFFF`` QE/uQE firmware    256K
``0x00_4098_0000`` ``0x00_40FF_FFFF`` Reserved           6M
``0x00_4100_0000`` ``0x00_43FF_FFFF`` FIT Image          48M
================== ================== ================== =====

Booting Options
---------------

NB: The reference manual documents the RCW source with the *least-significant
bit first*.

QSPI boot
^^^^^^^^^

This is the default. ``{ SW5[0:8], SW4[0] }`` should be ``0010_0010_0``.

SD boot and eMMC boot
^^^^^^^^^^^^^^^^^^^^^

``{ SW5[0:8], SW4[0] }`` should be ``0010_0000_0``. eMMC is selected only if
there is no SD card in the slot.

.. _ls1046ardb_jtag:

JTAG boot
^^^^^^^^^

To recover a bricked board, or to perform initial programming, the ls1046
supports using two hard-coded Reset Configuration Words (RCWs). Unfortunately,
this configuration disables most functionality, including the uarts and ethernet.
However, the SD/MMC and flash controllers are still functional. To get around
the lack of a serial console, we will use ARM semihosting instead. When
enabled, OpenOCD will interpret certain instructions as calls to the host
operating system. This allows U-Boot to use the console, read/write files, or
run arbitrary commands (!).

When configuring U-Boot, ensure that ``CONFIG_SEMIHOSTING``,
``CONFIG_SPL_SEMIHOSTING``, and ``CONFIG_SEMIHOSTING_SERIAL`` are enabled.
``{ SW5[0:8], SW4[0] }`` should be ``0100_1111_0``. Additionally, ``SW4[7]``
should be set to ``0``. Connect to the "console" USB connector on the front of
the enclosure.

Create a new file called ``u-boot.tcl`` (or whatever you choose) with the
following contents::

    # Load the configuration for the LS1046ARDB
    source [find board/nxp_rdb-ls1046a.cfg]
    # Initialize the scan chain
    init
    # Stop the processor
    halt
    # Enable semihosting
    arm semihosting enable
    # Load U-Boot SPL
    load_image spl/u-boot-spl 0 elf
    # Start executing SPL at the beginning of OCRAM
    resume 0x10000000

Then, launch openocd like::

    openocd -f u-boot.tcl

You should see the U-Boot SPL banner followed by the banner for U-Boot proper
in the output of openocd. The CMSIS-DAP adapter is slow, so this can take a
long time. If you don't see it, something has gone wrong. After a while, you
should see the prompt. You can load an image using semihosting by running::

    => load hostfs - $loadaddr <name of file>

Note that openocd's terminal is "cooked," so commands will only be sent to
U-Boot when you press enter, and all commands will be echoed twice.
Additionally, openocd will block when waiting for input, ignoring gdb, JTAG
events, and Ctrl-Cs. To make openocd process these events, just hit enter.

Using an external JTAG adapter
""""""""""""""""""""""""""""""

The CMSIS-DAP adapter can be rather slow. To speed up booting, use an external
JTAG adapter. The following examples assume you are using a J-Link, though any
adapter supported by OpenOCD will do. Ensure that ``SW4[7]`` is ``1``. Attach
your jtag adapter to J22. Modify ``u-boot.tcl`` and replace the first two lines
with the following::

    # Load the J-Link configuration (or whatever your adapter is)
    source [find interface/jlink.cfg]
    # Use JTAG, since the J-Link also supports SWD
    transport select jtag
    # The reset pin resets the whole CPU
    reset_config srst_only
    # Load the LS1046A config
    source [find target/ls1046a.cfg]

You can proceed as normal through the rest of the steps above. I got a speedup
of around 100x by using a J-Link.

Debug UART
----------

To enable the debug UART, enable the following config options::

    CONFIG_DEBUG_UART_NS16550=y
    CONFIG_DEBUG_UART_BASE=0x21c0500
    CONFIG_DEBUG_UART_CLOCK=300000000
