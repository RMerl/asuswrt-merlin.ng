.. SPDX-License-Identifier: GPL-2.0+
.. Copyright (C) 2020 Sean Anderson <seanga2@gmail.com>

MAIX
====

Several of the Sipeed Maix series of boards contain the Kendryte K210 processor,
a 64-bit RISC-V CPU produced by Canaan Inc. This processor contains several
peripherals to accelerate neural network processing and other "ai" tasks. This
includes a "KPU" neural network processor, an audio processor supporting
beamforming reception, and a digital video port supporting capture and output at
VGA resolution. Other peripherals include 8M of SRAM (accessible with and
without caching); remappable pins, including 40 GPIOs; AES, FFT, and SHA256
accelerators; a DMA controller; and I2C, I2S, and SPI controllers. Maix
peripherals vary, but include spi flash; on-board usb-serial bridges; ports for
cameras, displays, and sd cards; and ESP32 chips.

Currently, only the Sipeed MAIX BiT V2.0 (bitm) and Sipeed MAIXDUINO are
supported, but the boards are fairly similar.

Documentation for Maix boards is available from
`Sipeed's website <http://dl.sipeed.com/MAIX/HDK/>`_.
Documentation for the Kendryte K210 is available from
`Kendryte's website <https://kendryte.com/downloads/>`_. However, hardware
details are rather lacking, so most technical reference has been taken from the
`standalone sdk <https://github.com/kendryte/kendryte-standalone-sdk>`_.

Build and boot steps
--------------------

To build U-Boot, run

.. code-block:: none

    make <defconfig>
    make CROSS_COMPILE=<your cross compile prefix>

To flash U-Boot, run

.. code-block:: none

    kflash -tp /dev/<your tty here> -B <board_id> u-boot-dtb.bin

The board provides two serial devices, e.g.

* /dev/serial/by-id/usb-Kongou_Hikari_Sipeed-Debug_12345678AB-if00-port0
* /dev/serial/by-id/usb-Kongou_Hikari_Sipeed-Debug_12345678AB-if01-port0

Which one is used for flashing depends on the board.

Currently only a small subset of the board features are supported. So we can
use the same default configuration and device tree. In the long run we may need
separate settings.

======================== ========================== ========== ==========
Board                    defconfig                  board_id   TTY device
======================== ========================== ========== ==========
Sipeed MAIX BiT          sipeed_maix_bitm_defconfig bit        first
Sipeed MAIX BiT with Mic sipeed_maix_bitm_defconfig bit_mic    first
Sipeed MAIXDUINO         sipeed_maix_bitm_defconfig maixduino  first
Sipeed MAIX GO                                      goE        second
Sipeed MAIX ONE DOCK                                dan        first
======================== ========================== ========== ==========

Flashing causes a reboot of the device. Parameter -t specifies that the serial
console shall be opened immediately. Boot output should look like the following:

.. code-block:: none

    U-Boot 2020.04-rc2-00087-g2221cc09c1-dirty (Feb 28 2020 - 13:53:09 -0500)

    DRAM:  8 MiB
    MMC:   spi@53000000:slot@0: 0
    In:    serial@38000000
    Out:   serial@38000000
    Err:   serial@38000000
    =>

OpenSBI
^^^^^^^

OpenSBI is an open source supervisor execution environment implementing the
RISC-V Supervisor Binary Interface Specification [1]. One of its features is
to intercept run-time exceptions, e.g. for unaligned access or illegal
instructions, and to emulate the failing instructions.

The OpenSBI source can be downloaded via:

.. code-block:: bash

    git clone https://github.com/riscv/opensbi

As OpenSBI will be loaded at 0x80000000 we have to adjust the U-Boot text base.
Furthermore we have to enable building U-Boot for S-mode::

    CONFIG_TEXT_BASE=0x80020000
    CONFIG_RISCV_SMODE=y

Both settings are contained in sipeed_maix_smode_defconfig so we can build
U-Boot with:

.. code-block:: bash

    make sipeed_maix_smode_defconfig
    make

To build OpenSBI with U-Boot as a payload:

.. code-block:: bash

    cd opensbi
    make \
    PLATFORM=kendryte/k210 \
    FW_PAYLOAD=y \
    FW_PAYLOAD_OFFSET=0x20000 \
    FW_PAYLOAD_PATH=<path to U-Boot>/u-boot-dtb.bin

The value of FW_PAYLOAD_OFFSET must match CONFIG_TEXT_BASE - 0x80000000.

The file to flash is build/platform/kendryte/k210/firmware/fw_payload.bin.

Booting
^^^^^^^

The default boot process is to load and boot the files ``/uImage`` and
``/k210.dtb`` off of the first partition of the MMC. For Linux, this will result
in an output like

.. code-block:: none

    U-Boot 2020.10-00691-gd1d651d988-dirty (Oct 16 2020 - 17:05:24 -0400)

    DRAM:  8 MiB
    MMC:   spi@53000000:slot@0: 0
    Loading Environment from SPIFlash... SF: Detected w25q128fw with page size 256 Bytes, erase size 4 KiB, total 16 MiB
    OK
    In:    serial@38000000
    Out:   serial@38000000
    Err:   serial@38000000
    Hit any key to stop autoboot:  0
    1827380 bytes read in 1044 ms (1.7 MiB/s)
    13428 bytes read in 10 ms (1.3 MiB/s)
    ## Booting kernel from Legacy Image at 80060000 ...
       Image Name:   linux
       Image Type:   RISC-V Linux Kernel Image (uncompressed)
       Data Size:    1827316 Bytes = 1.7 MiB
       Load Address: 80000000
       Entry Point:  80000000
       Verifying Checksum ... OK
    ## Flattened Device Tree blob at 80400000
       Booting using the fdt blob at 0x80400000
       Loading Kernel Image
       Loading Device Tree to 00000000803f9000, end 00000000803ff473 ... OK

    Starting kernel ...

    [    0.000000] Linux version 5.9.0-00021-g6dcc2f0814c6-dirty (sean@godwin) (riscv64-linux-gnu-gcc (GCC) 10.2.0, GNU ld (GNU Binutils) 2.35) #34 SMP Fri Oct 16 14:40:57 EDT 2020
    [    0.000000] earlycon: sifive0 at MMIO 0x0000000038000000 (options '115200n8')
    [    0.000000] printk: bootconsole [sifive0] enabled
    [    0.000000] Zone ranges:
    [    0.000000]   DMA32    [mem 0x0000000080000000-0x00000000807fffff]
    [    0.000000]   Normal   empty
    [    0.000000] Movable zone start for each node
    [    0.000000] Early memory node ranges
    [    0.000000]   node   0: [mem 0x0000000080000000-0x00000000807fffff]
    [    0.000000] Initmem setup node 0 [mem 0x0000000080000000-0x00000000807fffff]
    [    0.000000] riscv: ISA extensions acdfgim
    [    0.000000] riscv: ELF capabilities acdfim
    [    0.000000] percpu: max_distance=0x18000 too large for vmalloc space 0x0
    [    0.000000] percpu: Embedded 12 pages/cpu s18848 r0 d30304 u49152
    [    0.000000] Built 1 zonelists, mobility grouping off.  Total pages: 2020
    [    0.000000] Kernel command line: earlycon console=ttySIF0
    [    0.000000] Dentry cache hash table entries: 1024 (order: 1, 8192 bytes, linear)
    [    0.000000] Inode-cache hash table entries: 512 (order: 0, 4096 bytes, linear)
    [    0.000000] Sorting __ex_table...
    [    0.000000] mem auto-init: stack:off, heap alloc:off, heap free:off
    [    0.000000] Memory: 6004K/8192K available (1139K kernel code, 126K rwdata, 198K rodata, 90K init, 81K bss, 2188K reserved, 0K cma-reserved)
    [    0.000000] rcu: Hierarchical RCU implementation.
    [    0.000000] rcu: RCU calculated value of scheduler-enlistment delay is 25 jiffies.
    [    0.000000] NR_IRQS: 64, nr_irqs: 64, preallocated irqs: 0
    [    0.000000] riscv-intc: 64 local interrupts mapped
    [    0.000000] plic: interrupt-controller@C000000: mapped 65 interrupts with 2 handlers for 2 contexts.
    [    0.000000] random: get_random_bytes called from 0x00000000800019a8 with crng_init=0
    [    0.000000] k210-clk: clock-controller
    [    0.000000] k210-clk: clock-controller: fixed-rate 26 MHz osc base clock
    [    0.000000] clint: clint@2000000: timer running at 7800000 Hz
    [    0.000000] clocksource: clint_clocksource: mask: 0xffffffffffffffff max_cycles: 0x3990be68b, max_idle_ns: 881590404272 ns
    [    0.000014] sched_clock: 64 bits at 7MHz, resolution 128ns, wraps every 4398046511054ns
    [    0.008450] Console: colour dummy device 80x25
    [    0.012494] Calibrating delay loop (skipped), value calculated using timer frequency.. 15.60 BogoMIPS (lpj=31200)
    [    0.022693] pid_max: default: 4096 minimum: 301
    [    0.027352] Mount-cache hash table entries: 512 (order: 0, 4096 bytes, linear)
    [    0.034428] Mountpoint-cache hash table entries: 512 (order: 0, 4096 bytes, linear)
    [    0.045099] rcu: Hierarchical SRCU implementation.
    [    0.050048] smp: Bringing up secondary CPUs ...
    [    0.055417] smp: Brought up 1 node, 2 CPUs
    [    0.059602] devtmpfs: initialized
    [    0.082796] clocksource: jiffies: mask: 0xffffffff max_cycles: 0xffffffff, max_idle_ns: 7645041785100000 ns
    [    0.091820] futex hash table entries: 16 (order: -2, 1024 bytes, linear)
    [    0.098507] pinctrl core: initialized pinctrl subsystem
    [    0.140938] clocksource: Switched to clocksource clint_clocksource
    [    0.247216] workingset: timestamp_bits=62 max_order=11 bucket_order=0
    [    0.277392] k210-fpioa 502b0000.pinmux: K210 FPIOA pin controller
    [    0.291724] k210-sysctl 50440000.syscon: K210 system controller
    [    0.305317] k210-rst 50440000.syscon:reset-controller: K210 reset controller
    [    0.313808] 38000000.serial: ttySIF0 at MMIO 0x38000000 (irq = 1, base_baud = 115200) is a SiFive UART v0
    [    0.322712] printk: console [ttySIF0] enabled
    [    0.322712] printk: console [ttySIF0] enabled
    [    0.331328] printk: bootconsole [sifive0] disabled
    [    0.331328] printk: bootconsole [sifive0] disabled
    [    0.353347] Freeing unused kernel memory: 88K
    [    0.357004] This architecture does not have kernel memory protection.
    [    0.363397] Run /init as init process

Loading, Booting, and Storing Images
------------------------------------

.. _loading:

Loading Images
^^^^^^^^^^^^^^

Serial
""""""

Use the ``loady`` command to load images over serial.

.. code-block:: none

    => loady $loadaddr 1500000
    ## Switch baudrate to 1500000 bps and press ENTER ...

    *** baud: 1500000

    *** baud: 1500000 ***
    ## Ready for binary (ymodem) download to 0x80000000 at 1500000 bps...
    C
    *** file: loader.bin
    $ sz -vv loader.bin
    Sending: loader.bin
    Bytes Sent:2478208   BPS:72937
    Sending:
    Ymodem sectors/kbytes sent:   0/ 0k
    Transfer complete

    *** exit status: 0 ***
    ## Total Size      = 0x0025d052 = 2478162 Bytes
    ## Switch baudrate to 115200 bps and press ESC ...

    *** baud: 115200

    *** baud: 115200 ***
    =>

This command does not set ``$filesize``, so it may need to be set manually.

SPI Flash
"""""""""

To load an image off of SPI flash, first set up a partition as described in
:ref:`k210_partitions`. Then, use ``mtd`` to load that partition

.. code-block:: none

    => sf probe
    SF: Detected w25q128fw with page size 256 Bytes, erase size 4 KiB, total 16 MiB
    => mtd read linux $loadaddr
    Reading 2097152 byte(s) at offset 0x00000000

This command does not set ``$filesize``, so it may need to be set manually.

MMC
"""

The MMC device number is 0. To list partitions on the device, use ``part``:

.. code-block:: none

    => part list mmc 0

    Partition Map for MMC device 0  --   Partition Type: EFI

    Part    Start LBA       End LBA          Name
            Attributes
            Type GUID
            Partition GUID
      1     0x00000800      0x039effde      "boot"
            attrs:  0x0000000000000000
            type:   c12a7328-f81f-11d2-ba4b-00a0c93ec93b
            guid:   96161f7d-7113-4cc7-9a24-08ab7fc5cb72

To list files, use ``ls``:

.. code-block:: none

    => ls mmc 0:1
    <DIR>       4096 .
    <DIR>       4096 ..
    <DIR>      16384 lost+found
               13428 k210.dtb
             1827380 uImage

To load a file, use ``load``:

.. code-block:: none

    => load mmc 0:1 $loadaddr uImage
    1827380 bytes read in 1049 ms (1.7 MiB/s)

Running Programs
^^^^^^^^^^^^^^^^

Binaries
""""""""

To run a bare binary, use the ``go`` command:

.. code-block:: none

    => go 80000000
    ## Starting application at 0x80000000 ...
    Example expects ABI version 9
    Actual U-Boot ABI version 9
    Hello World
    argc = 1
    argv[0] = "80000000"
    argv[1] = "<NULL>"
    Hit any key to exit ...

Note that this will only start a program on one hart. As-of this writing it is
only possible to start a program on multiple harts using the ``bootm`` command.

Legacy Images
"""""""""""""

To create a legacy image, use ``tools/mkimage``:

.. code-block:: none

    $ tools/mkimage -A riscv -O linux -T kernel -C none -a 0x80000000 -e 0x80000000 -n linux -d ../linux-git/arch/riscv/boot/Image uImage
    Image Name:   linux
    Created:      Fri Oct 16 17:36:32 2020
    Image Type:   RISC-V Linux Kernel Image (uncompressed)
    Data Size:    1827316 Bytes = 1784.49 KiB = 1.74 MiB
    Load Address: 80000000
    Entry Point:  80000000

The ``bootm`` command also requires an FDT, even if the image doesn't require
one. After loading the image to ``$loadaddr`` and the FDT to ``$fdt_addr_r``,
boot with:

.. code-block:: none

    => bootm $loadaddr - $fdt_addr_r
    ## Booting kernel from Legacy Image at 80060000 ...
       Image Name:   linux
       Image Type:   RISC-V Linux Kernel Image (uncompressed)
       Data Size:    1827316 Bytes = 1.7 MiB
       Load Address: 80000000
       Entry Point:  80000000
       Verifying Checksum ... OK
    ## Flattened Device Tree blob at 80400000
       Booting using the fdt blob at 0x80400000
       Loading Kernel Image
       Loading Device Tree to 00000000803f9000, end 00000000803ff473 ... OK

    Starting kernel ...

The FDT is verified after the kernel is relocated, so it must be loaded high
enough so that it won't be overwritten. The default values for ``$loadaddr``
and ``$fdt_addr_r`` should provide ample headroom for most use-cases.

Flashing Images
^^^^^^^^^^^^^^^

SPI Flash
"""""""""

To flash data to SPI flash, first load it using one of the methods in
:ref:`loading`. Addiotionally, create some partitions as described in
:ref:`partitions`. Then use the ``mtd`` command:

.. code-block:: none

    => sf probe
    SF: Detected w25q128fw with page size 256 Bytes, erase size 4 KiB, total 16 MiB
    => mtd write linux $loadaddr 0 $filesize
    Writing 2478162 byte(s) at offset 0x00000000

Note that in order to write a bootable image, a header and tailer must be added.

MMC
"""

MMC writes are unsupported for now.

SPI Flash
^^^^^^^^^

Sipeed MAIX boards typically provide around 16 MiB of SPI NOR flash. U-Boot is
stored in the first 1 MiB or so of this flash. U-Boot's environment is stored at
the end of flash.

.. _k210_partitions:

Partitions
""""""""""

There is no set data layout. The default partition layout only allocates
partitions for U-Boot and its default environment

.. code-block:: none

    => mtd list
    List of MTD devices:
    * nor0
      - type: NOR flash
      - block size: 0x1000 bytes
      - min I/O: 0x1 bytes
      - 0x000000000000-0x000001000000 : "nor0"
          - 0x000000000000-0x000000100000 : "u-boot"
          - 0x000000fff000-0x000001000000 : "env"

As an example, to allocate 2MiB for Linux and (almost) 13 MiB for other data,
set the ``mtdparts`` like:

.. code-block:: none

    => env set mtdparts nor0:1M(u-boot),2M(linux),0xcff000(data),0x1000@0xfff000(env)
    => mtd list
    List of MTD devices:
    * nor0
      - type: NOR flash
      - block size: 0x1000 bytes
      - min I/O: 0x1 bytes
      - 0x000000000000-0x000001000000 : "nor0"
          - 0x000000000000-0x000000100000 : "u-boot"
          - 0x000000100000-0x000000300000 : "linux"
          - 0x000000300000-0x000000fff000 : "data"
          - 0x000000fff000-0x000001000000 : "env"

To make these changes permanent, save the environment:

.. code-block:: none

    => env save
    Saving Environment to SPIFlash... Erasing SPI flash...Writing to SPI flash...done
    OK

U-Boot will always load the environment from the last 4 KiB of flash.

Pin Assignment
--------------

The K210 contains a Fully Programmable I/O Array (FPIOA), which can remap any of
its 256 input functions to any any of 48 output pins. The following table has
the default pin assignments for the BitM.

===== ========== =======
Pin   Function   Comment
===== ========== =======
IO_0  JTAG_TCLK
IO_1  JTAG_TDI
IO_2  JTAG_TMS
IO_3  JTAG_TDO
IO_4  UARTHS_RX
IO_5  UARTHS_TX
IO_6             Not set
IO_7             Not set
IO_8  GPIO_0
IO_9  GPIO_1
IO_10 GPIO_2
IO_11 GPIO_3
IO_12 GPIO_4     Green LED
IO_13 GPIO_5     Red LED
IO_14 GPIO_6     Blue LED
IO_15 GPIO_7
IO_16 GPIOHS_0   ISP
IO_17 GPIOHS_1
IO_18 I2S0_SCLK  MIC CLK
IO_19 I2S0_WS    MIC WS
IO_20 I2S0_IN_D0 MIC SD
IO_21 GPIOHS_5
IO_22 GPIOHS_6
IO_23 GPIOHS_7
IO_24 GPIOHS_8
IO_25 GPIOHS_9
IO_26 SPI1_D1    MMC MISO
IO_27 SPI1_SCLK  MMC CLK
IO_28 SPI1_D0    MMC MOSI
IO_29 GPIOHS_13  MMC CS
IO_30 GPIOHS_14
IO_31 GPIOHS_15
IO_32 GPIOHS_16
IO_33 GPIOHS_17
IO_34 GPIOHS_18
IO_35 GPIOHS_19
IO_36 GPIOHS_20  Panel CS
IO_37 GPIOHS_21  Panel RST
IO_38 GPIOHS_22  Panel DC
IO_39 SPI0_SCK   Panel WR
IO_40 SCCP_SDA
IO_41 SCCP_SCLK
IO_42 DVP_RST
IO_43 DVP_VSYNC
IO_44 DVP_PWDN
IO_45 DVP_HSYNC
IO_46 DVP_XCLK
IO_47 DVP_PCLK
===== ========== =======

Over- and Under-clocking
------------------------

To change the clock speed of the K210, you will need to enable
``CONFIG_CLK_K210_SET_RATE`` and edit the board's device tree. To do this, add a
section to ``arch/riscv/arch/riscv/dts/k210-maix-bit.dts`` like the following:

.. code-block:: none

    &sysclk {
	assigned-clocks = <&sysclk K210_CLK_PLL0>;
	assigned-clock-rates = <800000000>;
    };

There are three PLLs on the K210: PLL0 is the parent of most of the components,
including the CPU and RAM. PLL1 is the parent of the neural network coprocessor.
PLL2 is the parent of the sound processing devices. Note that child clocks of
PLL0 and PLL2 run at *half* the speed of the PLLs. For example, if PLL0 is
running at 800 MHz, then the CPU will run at 400 MHz. This is the example given
above. The CPU can be overclocked to around 600 MHz, and underclocked to 26 MHz.

It is possible to set PLL2's parent to PLL0. The plls are more accurate when
converting between similar frequencies. This makes it easier to get an accurate
frequency for I2S. As an example, consider sampling an I2S device at 44.1 kHz.
On this device, the I2S serial clock runs at 64 times the sample rate.
Therefore, we would like to run PLL2 at an even multiple of 2.8224 MHz. If
PLL2's parent is IN0, we could use a frequency of 390 MHz (the same as the CPU's
default speed).  Dividing by 138 yields a serial clock of about 2.8261 MHz. This
results in a sample rate of 44.158 kHz---around 50 Hz or .1% too fast. If,
instead, we set PLL2's parent to PLL1 running at 390 MHz, and request a rate of
2.8224 * 136 = 383.8464 MHz, the achieved rate is 383.90625 MHz. Dividing by 136
yields a serial clock of about 2.8228 MHz. This results in a sample rate of
44.107 kHz---just 7 Hz or .02% too fast. This configuration is shown in the
following example:

.. code-block:: none

    &sysclk {
	assigned-clocks = <&sysclk K210_CLK_PLL1>, <&sysclk K210_CLK_PLL2>;
	assigned-clock-parents = <0>, <&sysclk K210_CLK_PLL1>;
	assigned-clock-rates = <390000000>, <383846400>;
    };

There are a couple of quirks to the PLLs. First, there are more frequency ratios
just above and below 1.0, but there is a small gap around 1.0. To be explicit,
if the input frequency is 100 MHz, it would be impossible to have an output of
99 or 101 MHz. In addition, there is a maximum frequency for the internal VCO,
so higher input/output frequencies will be less accurate than lower ones.

Technical Details
-----------------

Boot Sequence
^^^^^^^^^^^^^

1. ``RESET`` pin is deasserted. The pin is connected to the ``RESET`` button. It
   can also be set to low via either the ``DTR`` or the ``RTS`` line of the
   serial interface (depending on the board).
2. Both harts begin executing at ``0x00001000``.
3. Both harts jump to firmware at ``0x88000000``.
4. One hart is chosen as a boot hart.
5. Firmware reads the value of pin ``IO_16`` (ISP). This pin is connected to the
   ``BOOT`` button. The pin can equally be set to low via either the ``DTR`` or
   ``RTS`` line of the serial interface (depending on the board).

   * If the pin is low, enter ISP mode. This mode allows loading data to ram,
     writing it to flash, and booting from specific addresses.
   * If the pin is high, continue boot.
6. Firmware reads the next stage from flash (SPI3) to address ``0x80000000``.

   * If byte 0 is 1, the next stage is decrypted using the built-in AES
     accelerator and the one-time programmable, 128-bit AES key.
   * Bytes 1 to 4 hold the length of the next stage.
   * The SHA-256 sum of the next stage is automatically calculated, and verified
     against the 32 bytes following the next stage.
7. The boot hart sends an IPI to the other hart telling it to jump to the next
   stage.
8. The boot hart jumps to ``0x80000000``.

Debug UART
^^^^^^^^^^

The Debug UART is provided with the following settings::

    CONFIG_DEBUG_UART=y
    CONFIG_DEBUG_UART_SIFIVE=y
    CONFIG_DEBUG_UART_BASE=0x38000000
    CONFIG_DEBUG_UART_CLOCK=390000000

Resetting the board
^^^^^^^^^^^^^^^^^^^

The MAIX boards can be reset using the DTR and RTS lines of the serial console.
How the lines are used depends on the specific board. See the code of kflash.py
for details.

This is the reset sequence for the MAXDUINO and MAIX BiT with Mic:

.. code-block:: python

   def reset(self):
        self.device.setDTR(False)
        self.device.setRTS(False)
        time.sleep(0.1)
        self.device.setDTR(True)
        time.sleep(0.1)
        self.device.setDTR(False)
        time.sleep(0.1)

and this for the MAIX Bit:

.. code-block:: python

   def reset(self):
        self.device.setDTR(False)
        self.device.setRTS(False)
        time.sleep(0.1)
        self.device.setRTS(True)
        time.sleep(0.1)
        self.device.setRTS(False)
        time.sleep(0.1)

Memory Map
^^^^^^^^^^

========== ========= ===========
Address    Size      Description
========== ========= ===========
0x00000000 0x1000    debug
0x00001000 0x1000    rom
0x02000000 0xC000    clint
0x0C000000 0x4000000 plic
0x38000000 0x1000    uarths
0x38001000 0x1000    gpiohs
0x40000000 0x400000  sram0 (non-cached)
0x40400000 0x200000  sram1 (non-cached)
0x40600000 0x200000  airam (non-cached)
0x40800000 0xC00000  kpu
0x42000000 0x400000  fft
0x50000000 0x1000    dmac
0x50200000 0x200000  apb0
0x50200000 0x80      gpio
0x50210000 0x100     uart0
0x50220000 0x100     uart1
0x50230000 0x100     uart2
0x50240000 0x100     spi slave
0x50250000 0x200     i2s0
0x50250200 0x200     apu
0x50260000 0x200     i2s1
0x50270000 0x200     i2s2
0x50280000 0x100     i2c0
0x50290000 0x100     i2c1
0x502A0000 0x100     i2c2
0x502B0000 0x100     fpioa
0x502C0000 0x100     sha256
0x502D0000 0x100     timer0
0x502E0000 0x100     timer1
0x502F0000 0x100     timer2
0x50400000 0x200000  apb1
0x50400000 0x100     wdt0
0x50410000 0x100     wdt1
0x50420000 0x100     otp control
0x50430000 0x100     dvp
0x50440000 0x100     sysctl
0x50450000 0x100     aes
0x50460000 0x100     rtc
0x52000000 0x4000000 apb2
0x52000000 0x100     spi0
0x53000000 0x100     spi1
0x54000000 0x200     spi3
0x80000000 0x400000  sram0 (cached)
0x80400000 0x200000  sram1 (cached)
0x80600000 0x200000  airam (cached)
0x88000000 0x20000   otp
0x88000000 0xC200    firmware
0x8801C000 0x1000    riscv priv spec 1.9 config
0x8801D000 0x2000    flattened device tree (contains only addresses and
                     interrupts)
0x8801F000 0x1000    credits
========== ========= ===========

Links
-----

[1] https://github.com/riscv/riscv-sbi-doc
    RISC-V Supervisor Binary Interface Specification
