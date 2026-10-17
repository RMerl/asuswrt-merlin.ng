.. SPDX-License-Identifier: GPL-2.0+

Microchip PolarFire SoC Icicle Kit
==================================

RISC-V PolarFire SoC
--------------------

The PolarFire SoC is the 4+1 64-bit RISC-V SoC from Microchip.

The Icicle Kit development platform is based on PolarFire SoC and capable
of running Linux.

Mainline support
----------------

The support for following drivers are already enabled:

1. NS16550 UART Driver.
2. Microchip Clock Driver.
3. Cadence MACB ethernet driver for networking support.
4. Cadence MMC Driver for eMMC/SD support.
5. Microchip I2C Driver.

Booting from eMMC using HSS
---------------------------

Building U-Boot
~~~~~~~~~~~~~~~

1. Add the RISC-V toolchain to your PATH.
2. Setup ARCH & cross compilation environment variable:

.. code-block:: none

   export CROSS_COMPILE=<riscv64 toolchain prefix>

3. make microchip_mpfs_icicle_defconfig
4. make

Flashing
~~~~~~~~

The current U-Boot port is supported in S-mode only and loaded from DRAM.

A prior stage M-mode firmware/bootloader (e.g HSS with OpenSBI) is required to
boot the u-boot.bin in S-mode.

Currently, the u-boot.bin is used as a payload of the HSS firmware (Microchip
boot-flow) and OpenSBI generic platform fw_payload.bin (with u-boot.bin embedded)
as HSS payload (Custom boot-flow)

Microchip boot-flow
~~~~~~~~~~~~~~~~~~~

HSS with OpenSBI (M-Mode) -> U-Boot (S-Mode) -> Linux (S-Mode)

Build the HSS (Hart Software Services) - Microchip boot-flow
''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''

(Note: HSS git repo is at https://github.com/polarfire-soc/hart-software-services)

1. Configure

.. code-block:: none

   make BOARD=icicle-kit-es config

Alternatively, copy the default config for Microchip boot-flow.

.. code-block:: none

   cp boards/icicle-kit-es/def_config .config

2. make BOARD=icicle-kit-es
3. In the Default subdirectory, the standard build will create hss.elf and
   various binary formats (hss.hex and hss.bin).

The FPGA design will use the hss.hex or hss.bin.

FPGA design with HSS programming file
'''''''''''''''''''''''''''''''''''''

https://github.com/polarfire-soc/polarfire-soc-documentation/blob/master/boards/mpfs-icicle-kit-es/updating-icicle-kit/updating-icicle-kit-design-and-linux.md

The HSS firmware runs from the PolarFire SoC eNVM on reset.

Creating the HSS payload - Microchip boot-flow
''''''''''''''''''''''''''''''''''''''''''''''

1. You will be creating a payload from `u-boot-dtb.bin`.
   Copy this file to the HSS/tools/hss-payload-generator/test directory.
2. Go to hss-payload-generator source directory.

.. code-block:: none

   cd hart-software-services/tools/hss-payload-generator

3. Edit test/uboot.yaml file for hart entry points and correct name of the binary file.

	hart-entry-points: {u54_1: '0x80200000', u54_2: '0x80200000', u54_3: '0x80200000', u54_4: '0x80200000'}

	payloads:
	test/u-boot-dtb.bin: {exec-addr: '0x80200000', owner-hart: u54_1, secondary-hart: u54_2, secondary-hart: u54_3, secondary-hart: u54_4, priv-mode: prv_s}

4. Generate payload

.. code-block:: none

   ./hss-payload-generator -c test/uboot.yaml payload.bin

Once the payload binary is generated, it should be copied to the eMMC.

Please refer to HSS documenation to build the HSS firmware for payload.
(Note: HSS git repo is at https://github.com/polarfire-soc/hart-software-services/blob/master/tools/hss-payload-generator/README.md)

Custom boot-flow
~~~~~~~~~~~~~~~~

HSS without OpenSBI (M-Mode) -> OpenSBI (M-Mode) -> U-Boot (S-Mode) -> Linux (S-Mode)

Build OpenSBI
'''''''''''''

1. Get the OpenSBI source

.. code-block:: none

   git clone https://github.com/riscv/opensbi.git
   cd opensbi

2. Build

.. code-block:: none

   make PLATFORM=generic FW_PAYLOAD_PATH=<u-boot-directory>/u-boot.bin
   FW_FDT_PATH=<u-boot-directory>/arch/riscv/dts/mpfs-icicle-kit-.dtb

3. Output "fw_payload.bin" file available at
   "<opensbi-directory>/build/platform/generic/firmware/fw_payload.bin"

Build the HSS (Hart Software Services)- Custom boot-flow
''''''''''''''''''''''''''''''''''''''''''''''''''''''''

(Note: HSS git repo is at https://github.com/polarfire-soc/hart-software-services)

1. Configure

.. code-block:: none

   make BOARD=icicle-kit-es config

Alternatively, copy the default custom config for Custom boot-flow.

.. code-block:: none

   cp boards/icicle-kit-es/def_config_custom .config

2. make BOARD=icicle-kit-es
3. In the Default subdirectory, the standard build will create hss.elf and
   various binary formats (hss.hex and hss.bin).

The FPGA design will use the hss.hex or hss.bin.

Creating the HSS payload - Custom boot-flow
'''''''''''''''''''''''''''''''''''''''''''

1. You will be creating a payload from `fw_payload.bin`.
   Copy this file to the HSS/tools/hss-payload-generator/test directory.
2. Go to hss-payload-generator source directory.

.. code-block:: none

   cd hart-software-services/tools/hss-payload-generator

3. Edit test/uboot.yaml file for hart entry points and correct name of the binary file.

	hart-entry-points: {u54_1: '0x80000000', u54_2: '0x80000000', u54_3: '0x80000000', u54_4: '0x80000000'}

	payloads:
	test/fw_payload.bin: {exec-addr: '0x80000000', owner-hart: u54_1, secondary-hart: u54_2, secondary-hart: u54_3, secondary-hart: u54_4, priv-mode: prv_m}

4. Generate payload

.. code-block:: none

   ./hss-payload-generator -c test/uboot.yaml payload.bin

Once the payload binary is generated, it should be copied to the eMMC.

Please refer to HSS documenation to build the HSS firmware for payload.
(Note: HSS git repo is at https://github.com/polarfire-soc/hart-software-services/blob/master/tools/hss-payload-generator/README.md
and also refer the HSS payload generator at https://github.com/polarfire-soc/polarfire-soc-documentation/blob/master/software-development/hss-payloads.md)

eMMC
~~~~

Program eMMC with payload binary is explained in the PolarFire SoC documentation.
(Note: PolarFire SoC Documentation git repo is at https://github.com/polarfire-soc/polarfire-soc-documentation/blob/master/boards/mpfs-icicle-kit-es/updating-icicle-kit/updating-icicle-kit-design-and-linux.md#eMMC)

Once the payload image is copied to the eMMC, press CTRL+C in the HSS command
line interface, then type 'boot' and enter to boot the newly copied image.

.. code-block:: none

    sudo dd if=<payload_binary> of=/dev/sdX bs=512

GUID type
~~~~~~~~~

The HSS always picks up HSS payload from a GPT partition with
GUID type "21686148-6449-6E6F-744E-656564454649" or sector '0' of the eMMC if no
GPT partition.

Booting
~~~~~~~

You should see the U-Boot prompt on UART1.
(Note: UART0 is reserved for HSS)

Sample boot log from MPFS Icicle Kit
''''''''''''''''''''''''''''''''''''

.. code-block:: none

   U-Boot 2021.01-00314-g7303332537-dirty (Jan 14 2021 - 10:09:43 +0530)

   CPU:   rv64imafdc
   Model: Microchip MPFS Icicle Kit
   DRAM:  1 GiB
   MMC:   sdhc@20008000: 0
   In:    serial@20100000
   Out:   serial@20100000
   Err:   serial@20100000
   Net:   eth0: ethernet@20112000
   Hit any key to stop autoboot:  0

Now you can configure your networking, tftp server and use tftp boot method to
load uImage (with initramfs).

.. code-block:: none

   RISC-V # setenv kernel_addr_r 0x80200000
   RISC-V # setenv fdt_addr_r 0x82200000

   RISC-V # setenv ipaddr 192.168.1.5
   RISC-V # setenv netmask 255.255.255.0
   RISC-V # setenv serverip 192.168.1.3
   RISC-V # setenv gateway 192.168.1.1

   RISC-V # tftpboot ${kernel_addr_r} uImage
   ethernet@20112000: PHY present at 9
   ethernet@20112000: Starting autonegotiation...
   ethernet@20112000: Autonegotiation complete
   ethernet@20112000: link up, 1000Mbps full-duplex (lpa: 0x7800)
   Using ethernet@20112000 device
   TFTP from server 192.168.1.3; our IP address is 192.168.1.5
   Filename 'uImage'.
   Load address: 0x80200000
   Loading: #################################################################
	    #################################################################
	    #################################################################
	    #################################################################
	    #################################################################
	    #################################################################
	    #################################################################
	    #################################################################
	    #################################################################
	    #################################################################
	    #################################################################
	    #################################################################
	    #################################################################
	    #################################################################
	    #################################################################
	    ############
	    6.4 MiB/s
   done
   Bytes transferred = 14482480 (dcfc30 hex)

   RISC-V # tftpboot ${fdt_addr_r} mpfs-icicle-kit.dtb
   ethernet@20112000: PHY present at 9
   ethernet@20112000: Starting autonegotiation...
   ethernet@20112000: Autonegotiation complete
   ethernet@20112000: link up, 1000Mbps full-duplex (lpa: 0x7800)
   Using ethernet@20112000 device
   TFTP from server 192.168.1.3; our IP address is 192.168.1.5
   Filename 'mpfs-icicle-kit.dtb'.
   Load address: 0x82200000
   Loading: #
			2.5 MiB/s
   done
   Bytes transferred = 10282 (282a hex)

   RISC-V # bootm ${kernel_addr_r} - ${fdt_addr_r}
   ## Booting kernel from Legacy Image at 80200000 ...
		Image Name:   Linux
		Image Type:   RISC-V Linux Kernel Image (uncompressed)
		Data Size:    14482416 Bytes = 13.8 MiB
		Load Address: 80200000
		Entry Point:  80200000
		Verifying Checksum ... OK
   ## Flattened Device Tree blob at 82200000
		Booting using the fdt blob at 0x82200000
		Loading Kernel Image
		Using Device Tree in place at 000000008fffa000, end 000000008ffff829 ... OK

   Starting kernel ...

   [    0.000000] OF: fdt: Ignoring memory range 0x80000000 - 0x80200000
   [    0.000000] Linux version 5.6.17 (padmarao@padmarao-VirtualBox) (gcc version 7.2.0 (GCC)) #2 SMP Tue Jun 16 21:27:50 IST 2020
   [    0.000000] initrd not found or empty - disabling initrd
   [    0.000000] Zone ranges:
   [    0.000000]   DMA32    [mem 0x0000000080200000-0x00000000bfffffff]
   [    0.000000]   Normal   empty
   [    0.000000] Movable zone start for each node
   [    0.000000] Early memory node ranges
   [    0.000000]   node   0: [mem 0x0000000080200000-0x00000000bfffffff]
   [    0.000000] Initmem setup node 0 [mem 0x0000000080200000-0x00000000bfffffff]
   [    0.000000] software IO TLB: mapped [mem 0xbb1f5000-0xbf1f5000] (64MB)
   [    0.000000] elf_hwcap is 0x112d
   [    0.000000] percpu: Embedded 14 pages/cpu s24856 r0 d32488 u57344
   [    0.000000] Built 1 zonelists, mobility grouping on.  Total pages: 258055
   [    0.000000] Kernel command line: console=ttyS0,115200n8
   [    0.000000] Dentry cache hash table entries: 131072 (order: 8, 1048576 bytes, linear)
   [    0.000000] Inode-cache hash table entries: 65536 (order: 7, 524288 bytes, linear)
   [    0.000000] Sorting __ex_table...
   [    0.000000] mem auto-init: stack:off, heap alloc:off, heap free:off
   [    0.000000] Memory: 950308K/1046528K available (3289K kernel code, 212K rwdata, 900K rodata, 9476K init, 250K bss, 96220K reserved, 0K cma-reserved)
   [    0.000000] SLUB: HWalign=64, Order=0-3, MinObjects=0, CPUs=4, Nodes=1
   [    0.000000] rcu: Hierarchical RCU implementation.
   [    0.000000] rcu: 	RCU event tracing is enabled.
   [    0.000000] rcu: 	RCU restricting CPUs from NR_CPUS=8 to nr_cpu_ids=4.
   [    0.000000] rcu: RCU calculated value of scheduler-enlistment delay is 10 jiffies.
   [    0.000000] rcu: Adjusting geometry for rcu_fanout_leaf=16, nr_cpu_ids=4
   [    0.000000] NR_IRQS: 0, nr_irqs: 0, preallocated irqs: 0
   [    0.000000] plic: mapped 186 interrupts with 4 handlers for 9 contexts.
   [    0.000000] riscv_timer_init_dt: Registering clocksource cpuid [0] hartid [1]
   [    0.000000] clocksource: riscv_clocksource: mask: 0xffffffffffffffff max_cycles: 0x1d854df40, max_idle_ns: 3526361616960 ns
   [    0.000015] sched_clock: 64 bits at 1000kHz, resolution 1000ns, wraps every 2199023255500ns
   [    0.000311] Calibrating delay loop (skipped), value calculated using timer frequency.. 2.00 BogoMIPS (lpj=10000)
   [    0.000349] pid_max: default: 32768 minimum: 301
   [    0.000846] Mount-cache hash table entries: 2048 (order: 2, 16384 bytes, linear)
   [    0.000964] Mountpoint-cache hash table entries: 2048 (order: 2, 16384 bytes, linear)
   [    0.005630] rcu: Hierarchical SRCU implementation.
   [    0.006901] smp: Bringing up secondary CPUs ...
   [    0.012545] smp: Brought up 1 node, 4 CPUs
   [    0.014431] devtmpfs: initialized
   [    0.020526] random: get_random_bytes called from setup_net+0x36/0x192 with crng_init=0
   [    0.020928] clocksource: jiffies: mask: 0xffffffff max_cycles: 0xffffffff, max_idle_ns: 19112604462750000 ns
   [    0.020999] futex hash table entries: 1024 (order: 4, 65536 bytes, linear)
   [    0.022768] NET: Registered protocol family 16
   [    0.035478] microchip-pfsoc-clkcfg 20002000.clkcfg: Registered PFSOC core clocks
   [    0.048429] SCSI subsystem initialized
   [    0.049694] pps_core: LinuxPPS API ver. 1 registered
   [    0.049719] pps_core: Software ver. 5.3.6 - Copyright 2005-2007 Rodolfo Giometti <giometti@linux.it>
   [    0.049780] PTP clock support registered
   [    0.051781] clocksource: Switched to clocksource riscv_clocksource
   [    0.055326] NET: Registered protocol family 2
   [    0.056922] tcp_listen_portaddr_hash hash table entries: 512 (order: 1, 8192 bytes, linear)
   [    0.057053] TCP established hash table entries: 8192 (order: 4, 65536 bytes, linear)
   [    0.057648] TCP bind hash table entries: 8192 (order: 5, 131072 bytes, linear)
   [    0.058579] TCP: Hash tables configured (established 8192 bind 8192)
   [    0.059648] UDP hash table entries: 512 (order: 2, 16384 bytes, linear)
   [    0.059837] UDP-Lite hash table entries: 512 (order: 2, 16384 bytes, linear)
   [    0.060707] NET: Registered protocol family 1
   [    0.266229] workingset: timestamp_bits=62 max_order=18 bucket_order=0
   [    0.287107] io scheduler mq-deadline registered
   [    0.287140] io scheduler kyber registered
   [    0.429601] Serial: 8250/16550 driver, 4 ports, IRQ sharing disabled
   [    0.433979] printk: console [ttyS0] disabled
   [    0.434154] 20000000.serial: ttyS0 at MMIO 0x20000000 (irq = 18, base_baud = 9375000) is a 16550A
   [    0.928039] printk: console [ttyS0] enabled
   [    0.939804] libphy: Fixed MDIO Bus: probed
   [    0.948702] libphy: MACB_mii_bus: probed
   [    0.993698] macb 20112000.ethernet eth0: Cadence GEM rev 0x0107010c at 0x20112000 irq 21 (56:34:12:00:fc:00)
   [    1.006751] mousedev: PS/2 mouse device common for all mice
   [    1.013803] i2c /dev entries driver
   [    1.019451] sdhci: Secure Digital Host Controller Interface driver
   [    1.027242] sdhci: Copyright(c) Pierre Ossman
   [    1.032731] sdhci-pltfm: SDHCI platform and OF driver helper
   [    1.091826] mmc0: SDHCI controller on 20008000.sdhc [20008000.sdhc] using ADMA 64-bit
   [    1.102738] NET: Registered protocol family 17
   [    1.170326] Freeing unused kernel memory: 9476K
   [    1.176067] This architecture does not have kernel memory protection.
   [    1.184157] Run /init as init process
   Starting logging: OK
   Starting mdev...
   /etc/init.d/S10mdev: line 21: can't create /proc/sys/kernel/hotplug: nonexiste[    1.331981] mmc0: mmc_select_hs200 failed, error -74
   nt directory
   [    1.355011] mmc0: new MMC card at address 0001
   [    1.363981] mmcblk0: mmc0:0001 DG4008 7.28 GiB
   [    1.372248] mmcblk0boot0: mmc0:0001 DG4008 partition 1 4.00 MiB
   [    1.382292] mmcblk0boot1: mmc0:0001 DG4008 partition 2 4.00 MiB
   [    1.390265] mmcblk0rpmb: mmc0:0001 DG4008 partition 3 4.00 MiB, chardev (251:0)
   [    1.425234] GPT:Primary header thinks Alt. header is not at the end of the disk.
   [    1.434656] GPT:2255809 != 15273599
   [    1.439038] GPT:Alternate GPT header not at the end of the disk.
   [    1.446671] GPT:2255809 != 15273599
   [    1.451048] GPT: Use GNU Parted to correct GPT errors.
   [    1.457755]  mmcblk0: p1 p2 p3
   sort: /sys/devices/platform/Fixed: No such file or directory
   modprobe: can't change directory to '/lib/modules': No such file or directory
   Initializing random number generator... [    2.830198] random: dd: uninitialized urandom read (512 bytes read)
   done.
   Starting network...
   [    3.061867] macb 20112000.ethernet eth0: PHY [20112000.ethernet-ffffffff:09] driver [Vitesse VSC8662] (irq=POLL)
   [    3.074674] macb 20112000.ethernet eth0: configuring for phy/sgmii link mode
   [    3.084263] pps pps0: new PPS source ptp0
   [    3.089710] macb 20112000.ethernet: gem-ptp-timer ptp clock registered.
   udhcpc (v1.24.2) started
   Sending discover...
   Sending discover...
   [    6.380169] macb 20112000.ethernet eth0: Link is Up - 1Gbps/Full - flow control tx
   Sending discover...
   Sending select for 192.168.1.2...
   Lease of 192.168.1.2 obtained, lease time 86400
   deleting routers
   adding dns 192.168.1.1
   Starting dropbear sshd: [   11.385619] random: dropbear: uninitialized urandom read (32 bytes read)
   OK

   Welcome to Buildroot
   buildroot login: root
   Password:
   #

Booting U-Boot and Linux from eMMC
----------------------------------

FPGA design with HSS programming file and Linux Image
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

https://github.com/polarfire-soc/polarfire-soc-documentation/blob/master/boards/mpfs-icicle-kit-es/updating-icicle-kit/updating-icicle-kit-design-and-linux.md

The HSS firmware runs from the PolarFire SoC eNVM on reset.

eMMC
~~~~

Program eMMC with payload binary and Linux image is explained in the
PolarFire SoC documentation.
The payload binary should be copied to partition 2 of the eMMC.

(Note: PolarFire SoC Documentation git repo is at https://github.com/polarfire-soc/polarfire-soc-documentation/blob/master/boards/mpfs-icicle-kit-es/updating-icicle-kit/updating-icicle-kit-design-and-linux.md#eMMC)

Once the Linux image and payload binary is copied to the eMMC, press CTRL+C
in the HSS command line interface, then type 'boot' and enter to boot the newly
copied payload and Linux image.

.. code-block:: none

    zcat <linux-image>.wic.gz | sudo dd of=/dev/sdX bs=4096 iflag=fullblock oflag=direct conv=fsync status=progress

    sudo dd if=<payload_binary> of=/dev/sdX2 bs=512

You should see the U-Boot prompt on UART1.
(Note: UART0 is reserved for HSS)

GUID type
~~~~~~~~~

The HSS always picks up the HSS payload from a GPT partition with
GUID type "21686148-6449-6E6F-744E-656564454649" or sector '0' of the eMMC if no
GPT partition.

Sample boot log from MPFS Icicle Kit
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: none

   U-Boot 2021.01-00314-g7303332537-dirty (Jan 14 2021 - 10:09:43 +0530)

   CPU:   rv64imafdc
   Model: Microchip MPFS Icicle Kit
   DRAM:  1 GiB
   MMC:   sdhc@20008000: 0
   In:    serial@20100000
   Out:   serial@20100000
   Err:   serial@20100000
   Net:   eth0: ethernet@20112000
   Hit any key to stop autoboot:  0

   RISC-V # mmc info
   Device: sdhc@20008000
   Manufacturer ID: 45
   OEM: 100
   Name: DG400
   Bus Speed: 52000000
   Mode: MMC High Speed (52MHz)
   Rd Block Len: 512
   MMC version 5.1
   High Capacity: Yes
   Capacity: 7.3 GiB
   Bus Width: 4-bit
   Erase Group Size: 512 KiB
   HC WP Group Size: 8 MiB
   User Capacity: 7.3 GiB WRREL
   Boot Capacity: 4 MiB ENH
   RPMB Capacity: 4 MiB ENH

   RISC-V # mmc part
   Partition Map for MMC device 0  --   Partition Type: EFI

   Part	Start LBA	End LBA		Name
		Attributes
		Type GUID
		Partition GUID
	1	0x00002000	0x0000b031	"boot"
		attrs:	0x0000000000000004
		type:	ebd0a0a2-b9e5-4433-87c0-68b6b72699c7
		guid:	99ff6a94-f2e7-44dd-a7df-f3a2da106ef9
	2	0x0000b032	0x0000f031	"primary"
		attrs:	0x0000000000000000
		type:	21686148-6449-6e6f-744e-656564454649
		guid:	12006052-e64b-4423-beb0-b956ea00f1ba
	3	0x00010000	0x00226b9f	"root"
		attrs:	0x0000000000000000
		type:	0fc63daf-8483-4772-8e79-3d69d8477de4
		guid:	dd2c5619-2272-4c3c-8dc2-e21942e17ce6

   RISC-V # load mmc 0 ${ramdisk_addr_r} fitimage
   RISC-V # bootm ${ramdisk_addr_r}
   ## Loading kernel from FIT Image at 88300000 ...
   Using 'conf@microchip_icicle-kit-es-a000-microchip.dtb' configuration
   Trying 'kernel@1' kernel subimage
     Description:  Linux kernel
     Type:         Kernel Image
     Compression:  gzip compressed
     Data Start:   0x883000fc
     Data Size:    3574555 Bytes = 3.4 MiB
     Architecture: RISC-V
     OS:           Linux
     Load Address: 0x80200000
     Entry Point:  0x80200000
     Hash algo:    sha256
     Hash value:   21f18d72cf2f0a7192220abb577ad25c77c26960052d779aa02bf55dbf0a6403
   Verifying Hash Integrity ... sha256+ OK
   ## Loading fdt from FIT Image at 88300000 ...
   Using 'conf@microchip_icicle-kit-es-a000-microchip.dtb' configuration
   Trying 'fdt@microchip_icicle-kit-es-a000-microchip.dtb' fdt subimage
     Description:  Flattened Device Tree blob
     Type:         Flat Device Tree
     Compression:  uncompressed
     Data Start:   0x88668d44
     Data Size:    9760 Bytes = 9.5 KiB
     Architecture: RISC-V
     Load Address: 0x82200000
     Hash algo:    sha256
     Hash value:   5c3a9f30d41b6b8e53b47916e1f339b3a4d454006554d1f7e1f552ed62409f4b
   Verifying Hash Integrity ... sha256+ OK
   Loading fdt from 0x88668d48 to 0x82200000
   Booting using the fdt blob at 0x82200000
   Uncompressing Kernel Image
   Loading Device Tree to 000000008fffa000, end 000000008ffff61f ... OK

   Starting kernel ...

   [    0.000000] OF: fdt: Ignoring memory range 0x80000000 - 0x80200000
   [    0.000000] Linux version 5.6.16 (oe-user@oe-host) (gcc version 9.3.0 (GCC)) #1 SMP Fri Oct 9 11:49:47 UTC 2020
   [    0.000000] earlycon: sbi0 at I/O port 0x0 (options '')
   [    0.000000] printk: bootconsole [sbi0] enabled
   [    0.000000] Zone ranges:
   [    0.000000]   DMA32    [mem 0x0000000080200000-0x00000000bfffffff]
   [    0.000000]   Normal   empty
   [    0.000000] Movable zone start for each node
   [    0.000000] Early memory node ranges
   [    0.000000]   node   0: [mem 0x0000000080200000-0x00000000bfffffff]
   [    0.000000] Zeroed struct page in unavailable ranges: 512 pages
   [    0.000000] Initmem setup node 0 [mem 0x0000000080200000-0x00000000bfffffff]
   [    0.000000] software IO TLB: mapped [mem 0xb9e00000-0xbde00000] (64MB)
   [    0.000000] CPU with hartid=0 is not available
   [    0.000000] CPU with hartid=0 is not available
   [    0.000000] elf_hwcap is 0x112d
   [    0.000000] percpu: Embedded 17 pages/cpu s29784 r8192 d31656 u69632
   [    0.000000] Built 1 zonelists, mobility grouping on.  Total pages: 258055
   [    0.000000] Kernel command line: earlycon=sbi root=/dev/mmcblk0p3 rootwait console=ttyS0,115200n8 uio_pdrv_genirq.of_id=generic-uio
   [    0.000000] Dentry cache hash table entries: 131072 (order: 8, 1048576 bytes, linear)
   [    0.000000] Inode-cache hash table entries: 65536 (order: 7, 524288 bytes, linear)
   [    0.000000] Sorting __ex_table...
   [    0.000000] mem auto-init: stack:off, heap alloc:off, heap free:off
   [    0.000000] Memory: 941440K/1046528K available (4118K kernel code, 280K rwdata, 1687K rodata, 169K init, 273K bss, 105088K reserved, 0K cma-reserved)
   [    0.000000] SLUB: HWalign=64, Order=0-3, MinObjects=0, CPUs=4, Nodes=1
   [    0.000000] rcu: Hierarchical RCU implementation.
   [    0.000000] rcu: 	RCU event tracing is enabled.
   [    0.000000] rcu: 	RCU restricting CPUs from NR_CPUS=5 to nr_cpu_ids=4.
   [    0.000000] rcu: RCU calculated value of scheduler-enlistment delay is 10 jiffies.
   [    0.000000] rcu: Adjusting geometry for rcu_fanout_leaf=16, nr_cpu_ids=4
   [    0.000000] NR_IRQS: 0, nr_irqs: 0, preallocated irqs: 0
   [    0.000000] plic: mapped 53 interrupts with 4 handlers for 9 contexts.
   [    0.000000] riscv_timer_init_dt: Registering clocksource cpuid [0] hartid [1]
   [    0.000000] clocksource: riscv_clocksource: mask: 0xffffffffffffffff max_cycles: 0x1d854df40, max_idle_ns: 3526361616960 ns
   [    0.000015] sched_clock: 64 bits at 1000kHz, resolution 1000ns, wraps every 2199023255500ns
   [    0.008679] Console: colour dummy device 80x25
   [    0.013112] Calibrating delay loop (skipped), value calculated using timer frequency.. 2.00 BogoMIPS (lpj=10000)
   [    0.023368] pid_max: default: 32768 minimum: 301
   [    0.028314] Mount-cache hash table entries: 2048 (order: 2, 16384 bytes, linear)
   [    0.035766] Mountpoint-cache hash table entries: 2048 (order: 2, 16384 bytes, linear)
   [    0.047099] rcu: Hierarchical SRCU implementation.
   [    0.052813] smp: Bringing up secondary CPUs ...
   [    0.061581] smp: Brought up 1 node, 4 CPUs
   [    0.067069] devtmpfs: initialized
   [    0.073621] random: get_random_u32 called from bucket_table_alloc.isra.0+0x4e/0x150 with crng_init=0
   [    0.074409] clocksource: jiffies: mask: 0xffffffff max_cycles: 0xffffffff, max_idle_ns: 19112604462750000 ns
   [    0.093399] futex hash table entries: 1024 (order: 4, 65536 bytes, linear)
   [    0.101879] NET: Registered protocol family 16
   [    0.110336] microchip-pfsoc-clkcfg 20002000.clkcfg: Registered PFSOC core clocks
   [    0.132717] usbcore: registered new interface driver usbfs
   [    0.138225] usbcore: registered new interface driver hub
   [    0.143813] usbcore: registered new device driver usb
   [    0.148939] pps_core: LinuxPPS API ver. 1 registered
   [    0.153929] pps_core: Software ver. 5.3.6 - Copyright 2005-2007 Rodolfo Giometti <giometti@linux.it>
   [    0.163071] PTP clock support registered
   [    0.168521] clocksource: Switched to clocksource riscv_clocksource
   [    0.174927] VFS: Disk quotas dquot_6.6.0
   [    0.179016] VFS: Dquot-cache hash table entries: 512 (order 0, 4096 bytes)
   [    0.205536] NET: Registered protocol family 2
   [    0.210944] tcp_listen_portaddr_hash hash table entries: 512 (order: 1, 8192 bytes, linear)
   [    0.219393] TCP established hash table entries: 8192 (order: 4, 65536 bytes, linear)
   [    0.227497] TCP bind hash table entries: 8192 (order: 5, 131072 bytes, linear)
   [    0.235440] TCP: Hash tables configured (established 8192 bind 8192)
   [    0.242537] UDP hash table entries: 512 (order: 2, 16384 bytes, linear)
   [    0.249285] UDP-Lite hash table entries: 512 (order: 2, 16384 bytes, linear)
   [    0.256690] NET: Registered protocol family 1
   [    0.262585] workingset: timestamp_bits=62 max_order=18 bucket_order=0
   [    0.281036] Block layer SCSI generic (bsg) driver version 0.4 loaded (major 249)
   [    0.288481] io scheduler mq-deadline registered
   [    0.292983] io scheduler kyber registered
   [    0.298895] microsemi,mss-gpio 20122000.gpio: Microsemi MSS GPIO registered 32 GPIOs
   [    0.453723] Serial: 8250/16550 driver, 4 ports, IRQ sharing disabled
   [    0.462911] printk: console [ttyS0] disabled
   [    0.467216] 20100000.serial: ttyS0 at MMIO 0x20100000 (irq = 12, base_baud = 9375000) is a 16550A
   [    0.476201] printk: console [ttyS0] enabled
   [    0.476201] printk: console [ttyS0] enabled
   [    0.484576] printk: bootconsole [sbi0] disabled
   [    0.484576] printk: bootconsole [sbi0] disabled
   [    0.494920] 20102000.serial: ttyS1 at MMIO 0x20102000 (irq = 13, base_baud = 9375000) is a 16550A
   [    0.505068] 20104000.serial: ttyS2 at MMIO 0x20104000 (irq = 14, base_baud = 9375000) is a 16550A
   [    0.533336] loop: module loaded
   [    0.572284] Rounding down aligned max_sectors from 4294967295 to 4294967288
   [    0.580000] db_root: cannot open: /etc/target
   [    0.585413] libphy: Fixed MDIO Bus: probed
   [    0.591526] libphy: MACB_mii_bus: probed
   [    0.598060] macb 20112000.ethernet eth0: Cadence GEM rev 0x0107010c at 0x20112000 irq 17 (56:34:12:00:fc:00)
   [    0.608352] ehci_hcd: USB 2.0 'Enhanced' Host Controller (EHCI) Driver
   [    0.615001] ehci-platform: EHCI generic platform driver
   [    0.620446] ohci_hcd: USB 1.1 'Open' Host Controller (OHCI) Driver
   [    0.626632] ohci-platform: OHCI generic platform driver
   [    0.632326] usbcore: registered new interface driver cdc_acm
   [    0.637996] cdc_acm: USB Abstract Control Model driver for USB modems and ISDN adapters
   [    0.646459] i2c /dev entries driver
   [    0.650852] microsemi-mss-i2c 2010b000.i2c: Microsemi I2C Probe Complete
   [    0.658010] sdhci: Secure Digital Host Controller Interface driver
   [    0.664326] sdhci: Copyright(c) Pierre Ossman
   [    0.668754] sdhci-pltfm: SDHCI platform and OF driver helper
   [    0.706845] mmc0: SDHCI controller on 20008000.sdhc [20008000.sdhc] using ADMA 64-bit
   [    0.715052] usbcore: registered new interface driver usbhid
   [    0.720722] usbhid: USB HID core driver
   [    0.725174] pac193x 0-0010: Chip revision: 0x03
   [    0.733339] pac193x 0-0010: :pac193x_prep_iio_channels: Channel 0 active
   [    0.740127] pac193x 0-0010: :pac193x_prep_iio_channels: Channel 1 active
   [    0.746881] pac193x 0-0010: :pac193x_prep_iio_channels: Channel 2 active
   [    0.753686] pac193x 0-0010: :pac193x_prep_iio_channels: Channel 3 active
   [    0.760495] pac193x 0-0010: :pac193x_prep_iio_channels: Active chip channels: 25
   [    0.778006] NET: Registered protocol family 10
   [    0.784929] Segment Routing with IPv6
   [    0.788875] sit: IPv6, IPv4 and MPLS over IPv4 tunneling driver
   [    0.795743] NET: Registered protocol family 17
   [    0.801191] hctosys: unable to open rtc device (rtc0)
   [    0.807774] Waiting for root device /dev/mmcblk0p3...
   [    0.858506] mmc0: mmc_select_hs200 failed, error -74
   [    0.865764] mmc0: new MMC card at address 0001
   [    0.872564] mmcblk0: mmc0:0001 DG4008 7.28 GiB
   [    0.878777] mmcblk0boot0: mmc0:0001 DG4008 partition 1 4.00 MiB
   [    0.886182] mmcblk0boot1: mmc0:0001 DG4008 partition 2 4.00 MiB
   [    0.892633] mmcblk0rpmb: mmc0:0001 DG4008 partition 3 4.00 MiB, chardev (247:0)
   [    0.919029] GPT:Primary header thinks Alt. header is not at the end of the disk.
   [    0.926448] GPT:2255841 != 15273599
   [    0.930019] GPT:Alternate GPT header not at the end of the disk.
   [    0.936029] GPT:2255841 != 15273599
   [    0.939583] GPT: Use GNU Parted to correct GPT errors.
   [    0.944800]  mmcblk0: p1 p2 p3
   [    0.966696] EXT4-fs (mmcblk0p3): INFO: recovery required on readonly filesystem
   [    0.974105] EXT4-fs (mmcblk0p3): write access will be enabled during recovery
   [    1.052362] random: fast init done
   [    1.057961] EXT4-fs (mmcblk0p3): recovery complete
   [    1.065734] EXT4-fs (mmcblk0p3): mounted filesystem with ordered data mode. Opts: (null)
   [    1.074002] VFS: Mounted root (ext4 filesystem) readonly on device 179:3.
   [    1.081654] Freeing unused kernel memory: 168K
   [    1.086108] This architecture does not have kernel memory protection.
   [    1.092629] Run /sbin/init as init process
   [    1.702217] systemd[1]: System time before build time, advancing clock.
   [    1.754192] systemd[1]: systemd 244.3+ running in system mode. (+PAM -AUDIT -SELINUX +IMA -APPARMOR -SMACK +SYSVINIT +UTMP -LIBCRYPTSETUP -GCRYPT -GNUTLS +ACL +XZ -LZ4 -SECCOMP +BLKID -ELFUTILS +KMOD -IDN2 -IDN -PCRE2 default-hierarchy=hybrid)
   [    1.776361] systemd[1]: Detected architecture riscv64.

   Welcome to OpenEmbedded nodistro.0!

   [    1.829651] systemd[1]: Set hostname to <icicle-kit-es>.
   [    2.648597] random: systemd: uninitialized urandom read (16 bytes read)
   [    2.657485] systemd[1]: Created slice system-getty.slice.
   [  OK  ] Created slice system-getty.slice.
   [    2.698779] random: systemd: uninitialized urandom read (16 bytes read)
   [    2.706317] systemd[1]: Created slice system-serial\x2dgetty.slice.
   [  OK  ] Created slice system-serial\x2dgetty.slice.
   [    2.748716] random: systemd: uninitialized urandom read (16 bytes read)
   [    2.756098] systemd[1]: Created slice User and Session Slice.
   [  OK  ] Created slice User and Session Slice.
   [    2.789065] systemd[1]: Started Dispatch Password Requests to Console Directory Watch.
   [  OK  ] Started Dispatch Password …ts to Console Directory Watch.
   [    2.828974] systemd[1]: Started Forward Password Requests to Wall Directory Watch.
   [  OK  ] Started Forward Password R…uests to Wall Directory Watch.
   [    2.869009] systemd[1]: Reached target Paths.
   [  OK  ] Reached target Paths.
   [    2.898808] systemd[1]: Reached target Remote File Systems.
   [  OK  ] Reached target Remote File Systems.
   [    2.938771] systemd[1]: Reached target Slices.
   [  OK  ] Reached target Slices.
   [    2.968754] systemd[1]: Reached target Swap.
   [  OK  ] Reached target Swap.
   [    2.999283] systemd[1]: Listening on initctl Compatibility Named Pipe.
   [  OK  ] Listening on initctl Compatibility Named Pipe.
   [    3.060458] systemd[1]: Condition check resulted in Journal Audit Socket being skipped.
   [    3.069826] systemd[1]: Listening on Journal Socket (/dev/log).
   [  OK  ] Listening on Journal Socket (/dev/log).
   [    3.109601] systemd[1]: Listening on Journal Socket.
   [  OK  ] Listening on Journal Socket.
   [    3.149868] systemd[1]: Listening on Network Service Netlink Socket.
   [  OK  ] Listening on Network Service Netlink Socket.
   [    3.189419] systemd[1]: Listening on udev Control Socket.
   [  OK  ] Listening on udev Control Socket.
   [    3.229179] systemd[1]: Listening on udev Kernel Socket.
   [  OK  ] Listening on udev Kernel Socket.
   [    3.269520] systemd[1]: Condition check resulted in Huge Pages File System being skipped.
   [    3.278477] systemd[1]: Condition check resulted in POSIX Message Queue File System being skipped.
   [    3.288200] systemd[1]: Condition check resulted in Kernel Debug File System being skipped.
   [    3.302570] systemd[1]: Mounting Temporary Directory (/tmp)...
            Mounting Temporary Directory (/tmp)...
   [    3.339226] systemd[1]: Condition check resulted in Create list of static device nodes for the current kernel being skipped.
   [    3.355883] systemd[1]: Starting File System Check on Root Device...
            Starting File System Check on Root Device...
   [    3.407220] systemd[1]: Starting Journal Service...
            Starting Journal Service...
   [    3.422441] systemd[1]: Condition check resulted in Load Kernel Modules being skipped.
   [    3.431770] systemd[1]: Condition check resulted in FUSE Control File System being skipped.
   [    3.446415] systemd[1]: Mounting Kernel Configuration File System...
            Mounting Kernel Configuration File System...
   [    3.458983] systemd[1]: Starting Apply Kernel Variables...
            Starting Apply Kernel Variables...
   [    3.471368] systemd[1]: Starting udev Coldplug all Devices...
            Starting udev Coldplug all Devices...
   [    3.491071] systemd[1]: Mounted Temporary Directory (/tmp).
   [  OK      3.498114] systemd[1]: Mounted Kernel Configuration File System.
   0m] Mounted Temporary Directory (/tmp).
   [  OK  ] Mounted Kernel Configuration File System.
   [    3.550853] systemd[1]: Started Apply Kernel Variables.
   [  OK      3.557535] systemd[1]: Started Journal Service.
   0m] Started Apply Kernel Variables.
   [  OK  ] Started Journal Service.
   [  OK  ] Started udev Coldplug all Devices.
   [  OK  ] Started File System Check on Root Device.
            Starting Remount Root and Kernel File Systems...
   [    8.133469] EXT4-fs (mmcblk0p3): re-mounted. Opts: (null)
   [  OK  ] Started Remount Root and Kernel File Systems.
            Starting Flush Journal to Persistent Storage...
   [    8.215327] systemd-journald[77]: Received client request to flush runtime journal.
            Starting Create Static Device Nodes in /dev...
   [  OK  ] Started Flush Journal to Persistent Storage.
   [  OK  ] Started Create Static Device Nodes in /dev.
   [  OK  ] Reached target Local File Systems (Pre).
            Mounting /var/volatile...
            Starting udev Kernel Device Manager...
   [  OK  ] Mounted /var/volatile.
            Starting Load/Save Random Seed...
   [  OK  ] Reached target Local File Systems.
            Starting Create Volatile Files and Directories...
   [  OK  ] Started udev Kernel Device Manager.
   [  OK  ] Started Create Volatile Files and Directories.
            Starting Network Time Synchronization...
            Starting Update UTMP about System Boot/Shutdown...
   [  OK  ] Started Update UTMP about System Boot/Shutdown.
   [  OK  ] Started Network Time Synchronization.
   [   11.618575] random: crng init done
   [   11.622007] random: 7 urandom warning(s) missed due to ratelimiting
   [  OK  ] Started Load/Save Random Seed.
   [  OK  ] Reached target System Initialization.
   [  OK  ] Started Daily Cleanup of Temporary Directories.
   [  OK  ] Reached target System Time Set.
   [  OK  ] Reached target System Time Synchronized.
   [  OK  ] Reached target Timers.
   [  OK  ] Listening on D-Bus System Message Bus Socket.
   [  OK  ] Listening on dropbear.socket.
   [  OK  ] Reached target Sockets.
   [  OK  ] Reached target Basic System.
   [  OK  ] Started D-Bus System Message Bus.
            Starting IPv6 Packet Filtering Framework...
            Starting IPv4 Packet Filtering Framework...
            Starting Login Service...
   [  OK  ] Started IPv6 Packet Filtering Framework.
   [  OK  ] Started IPv4 Packet Filtering Framework.
   [  OK  ] Reached target Network (Pre).
            Starting Network Service...
   [  OK  ] Started Login Service.
   [   12.602455] macb 20112000.ethernet eth0: PHY [20112000.ethernet-ffffffff:09] driver [Vitesse VSC8662] (irq=POLL)
   [   12.612795] macb 20112000.ethernet eth0: configuring for phy/sgmii link mode
   [   12.622153] pps pps0: new PPS source ptp0
   [  OK     12.626725] macb 20112000.ethernet: gem-ptp-timer ptp clock registered.
   0m] Started Network Service.
            Starting Network Name Resolution...
   [  OK  ] Started Network Name Resolution.
   [  OK  ] Reached target Network.
   [  OK  ] Reached target Host and Network Name Lookups.
   [  OK  ] Started Collectd.
   [  OK  ] Started Collectd.
            Starting Permit User Sessions...
   [  OK  ] Started Permit User Sessions.
   [  OK  ] Started Getty on tty1.
   [  OK  ] Started Serial Getty on ttyS0.
   [  OK  ] Reached target Login Prompts.
   [  OK  ] Reached target Multi-User System.
            Starting Update UTMP about System Runlevel Changes...
   [  OK  ] Started Update UTMP about System Runlevel Changes.

   OpenEmbedded nodistro.0 icicle-kit-es ttyS0

   icicle-kit-es login: [   15.795564] macb 20112000.ethernet eth0: Link is Up - 1Gbps/Full - flow control tx
   [   15.803306] IPv6: ADDRCONF(NETDEV_CHANGE): eth0: link becomes ready

   icicle-kit-es login: root
   root@icicle-kit-es:~#
