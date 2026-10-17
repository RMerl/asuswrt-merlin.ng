.. SPDX-License-Identifier: GPL-2.0+

HiFive Unleashed
================

FU540-C000 RISC-V SoC
---------------------
The FU540-C000 is the world’s first 4+1 64-bit RISC-V SoC from SiFive.

The HiFive Unleashed development platform is based on FU540-C000 and capable
of running Linux.

Mainline support
----------------

The support for following drivers are already enabled:

1. SiFive UART Driver.
2. SiFive PRCI Driver for clock.
3. Cadence MACB ethernet driver for networking support.
4. SiFive SPI Driver.
5. MMC SPI Driver for MMC/SD support.

Booting from MMC using FSBL
---------------------------

Building
~~~~~~~~

1. Add the RISC-V toolchain to your PATH.
2. Setup ARCH & cross compilation environment variable:

.. code-block:: none

   export CROSS_COMPILE=<riscv64 toolchain prefix>

3. make sifive_fu540_defconfig
4. make

Flashing
~~~~~~~~

The current U-Boot port is supported in S-mode only and loaded from DRAM.

A prior stage M-mode firmware/bootloader (e.g OpenSBI) is required to
boot the u-boot.bin in S-mode and provide M-mode runtime services.

Currently, the u-boot.bin is used as a payload of the OpenSBI FW_PAYLOAD
firmware. We need to compile OpenSBI with below command:

.. code-block:: none

	make PLATFORM=generic FW_PAYLOAD_PATH=<path to u-boot-dtb.bin>

More detailed description of steps required to build FW_PAYLOAD firmware
is beyond the scope of this document. Please refer OpenSBI documenation.
(Note: OpenSBI git repo is at https://github.com/riscv/opensbi.git)

Once the prior stage firmware/bootloader binary is generated, it should be
copied to the first partition of the sdcard.

.. code-block:: none

    sudo dd if=<prior_stage_firmware_binary> of=/dev/disk2s1 bs=1024

Booting
~~~~~~~

Once you plugin the sdcard and power up, you should see the U-Boot prompt.

Sample boot log from HiFive Unleashed board
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: none

   U-Boot 2019.07-00024-g350ff02f5b (Jul 22 2019 - 11:45:02 +0530)

   CPU:   rv64imafdc
   Model: SiFive HiFive Unleashed A00
   DRAM:  8 GiB
   MMC:   spi@10050000:mmc@0: 0
   In:    serial@10010000
   Out:   serial@10010000
   Err:   serial@10010000
   Net:   eth0: ethernet@10090000
   Hit any key to stop autoboot:  0
   => version
   U-Boot 2019.07-00024-g350ff02f5b (Jul 22 2019 - 11:45:02 +0530)

   riscv64-linux-gcc.br_real (Buildroot 2018.11-rc2-00003-ga0787e9) 8.2.0
   GNU ld (GNU Binutils) 2.31.1
   => mmc info
   Device: spi@10050000:mmc@0
   Manufacturer ID: 3
   OEM: 5344
   Name: SU08G
   Bus Speed: 20000000
   Mode: SD Legacy
   Rd Block Len: 512
   SD version 2.0
   High Capacity: Yes
   Capacity: 7.4 GiB
   Bus Width: 1-bit
   Erase Group Size: 512 Bytes
   => mmc part

   Partition Map for MMC device 0  --   Partition Type: EFI

   Part    Start LBA       End LBA         Name
           Attributes
           Type GUID
           Partition GUID
     1     0x00000800      0x000107ff      "bootloader"
           attrs:  0x0000000000000000
           type:   2e54b353-1271-4842-806f-e436d6af6985
           guid:   393bbd36-7111-491c-9869-ce24008f6403
     2     0x00040800      0x00ecdfde      ""
           attrs:  0x0000000000000000
           type:   0fc63daf-8483-4772-8e79-3d69d8477de4
           guid:   7fc9a949-5480-48c7-b623-04923080757f

Now you can configure your networking, tftp server and use tftp boot method to
load uImage.

.. code-block:: none

   => setenv ipaddr 10.206.7.133
   => setenv netmask 255.255.252.0
   => setenv serverip 10.206.4.143
   => setenv gateway 10.206.4.1

If you want to use a flat kernel image such as Image file

.. code-block:: none

   => tftpboot ${kernel_addr_r} /sifive/fu540/Image
   ethernet@10090000: PHY present at 0
   ethernet@10090000: Starting autonegotiation...
   ethernet@10090000: Autonegotiation complete
   ethernet@10090000: link up, 1000Mbps full-duplex (lpa: 0x3c00)
   Using ethernet@10090000 device
   TFTP from server 10.206.4.143; our IP address is 10.206.7.133
   Filename '/sifive/fu540/Image'.
   Load address: 0x84000000
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
            ##########################################
            1.2 MiB/s
   done
   Bytes transferred = 8867100 (874d1c hex)

Or if you want to use a compressed kernel image file such as Image.gz

.. code-block:: none

   => tftpboot ${kernel_addr_r} /sifive/fu540/Image.gz
   ethernet@10090000: PHY present at 0
   ethernet@10090000: Starting autonegotiation...
   ethernet@10090000: Autonegotiation complete
   ethernet@10090000: link up, 1000Mbps full-duplex (lpa: 0x3c00)
   Using ethernet@10090000 device
   TFTP from server 10.206.4.143; our IP address is 10.206.7.133
   Filename '/sifive/fu540/Image.gz'.
   Load address: 0x84000000
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
            ##########################################
            1.2 MiB/s
   done
   Bytes transferred = 4809458 (4962f2 hex)

By this time, correct kernel image is loaded and required environment variables
are set. You can proceed to load the ramdisk and device tree from the tftp server
as well.

.. code-block:: none

   => tftpboot ${ramdisk_addr_r} /sifive/fu540/uRamdisk
   ethernet@10090000: PHY present at 0
   ethernet@10090000: Starting autonegotiation...
   ethernet@10090000: Autonegotiation complete
   ethernet@10090000: link up, 1000Mbps full-duplex (lpa: 0x3c00)
   Using ethernet@10090000 device
   TFTP from server 10.206.4.143; our IP address is 10.206.7.133
   Filename '/sifive/fu540/uRamdisk'.
   Load address: 0x88300000
   Loading: #################################################################
            #################################################################
            #################################################################
            #################################################################
            #################################################################
            #################################################################
            #################################################################
            ##############
            418.9 KiB/s
   done
   Bytes transferred = 2398272 (249840 hex)
   => tftpboot ${fdt_addr_r} /sifive/fu540/hifive-unleashed-a00.dtb
   ethernet@10090000: PHY present at 0
   ethernet@10090000: Starting autonegotiation...
   ethernet@10090000: Autonegotiation complete
   ethernet@10090000: link up, 1000Mbps full-duplex (lpa: 0x7c00)
   Using ethernet@10090000 device
   TFTP from server 10.206.4.143; our IP address is 10.206.7.133
   Filename '/sifive/fu540/hifive-unleashed-a00.dtb'.
   Load address: 0x88000000
   Loading: ##
            1000 Bytes/s
   done
   Bytes transferred = 5614 (15ee hex)
   => setenv bootargs "root=/dev/ram rw console=ttySIF0 ip=dhcp earlycon=sbi"
   => booti ${kernel_addr_r} ${ramdisk_addr_r} ${fdt_addr_r}
   ## Loading init Ramdisk from Legacy Image at 88300000 ...
      Image Name:   Linux RootFS
      Image Type:   RISC-V Linux RAMDisk Image (uncompressed)
      Data Size:    2398208 Bytes = 2.3 MiB
      Load Address: 00000000
      Entry Point:  00000000
      Verifying Checksum ... OK
   ## Flattened Device Tree blob at 88000000
      Booting using the fdt blob at 0x88000000
      Using Device Tree in place at 0000000088000000, end 00000000880045ed

   Starting kernel ...

   [    0.000000] OF: fdt: Ignoring memory range 0x80000000 - 0x80200000
   [    0.000000] Linux version 5.3.0-rc1-00003-g460ac558152f (anup@anup-lab-machine) (gcc version 8.2.0 (Buildroot 2018.11-rc2-00003-ga0787e9)) #6 SMP Mon Jul 22 10:01:01 IST 2019
   [    0.000000] earlycon: sbi0 at I/O port 0x0 (options '')
   [    0.000000] printk: bootconsole [sbi0] enabled
   [    0.000000] Initial ramdisk at: 0x(____ptrval____) (2398208 bytes)
   [    0.000000] Zone ranges:
   [    0.000000]   DMA32    [mem 0x0000000080200000-0x00000000ffffffff]
   [    0.000000]   Normal   [mem 0x0000000100000000-0x000000027fffffff]
   [    0.000000] Movable zone start for each node
   [    0.000000] Early memory node ranges
   [    0.000000]   node   0: [mem 0x0000000080200000-0x000000027fffffff]
   [    0.000000] Initmem setup node 0 [mem 0x0000000080200000-0x000000027fffffff]
   [    0.000000] software IO TLB: mapped [mem 0xfbfff000-0xfffff000] (64MB)
   [    0.000000] CPU with hartid=0 is not available
   [    0.000000] CPU with hartid=0 is not available
   [    0.000000] elf_hwcap is 0x112d
   [    0.000000] percpu: Embedded 18 pages/cpu s34584 r8192 d30952 u73728
   [    0.000000] Built 1 zonelists, mobility grouping on.  Total pages: 2067975
   [    0.000000] Kernel command line: root=/dev/ram rw console=ttySIF0 ip=dhcp earlycon=sbi
   [    0.000000] Dentry cache hash table entries: 1048576 (order: 11, 8388608 bytes, linear)
   [    0.000000] Inode-cache hash table entries: 524288 (order: 10, 4194304 bytes, linear)
   [    0.000000] Sorting __ex_table...
   [    0.000000] mem auto-init: stack:off, heap alloc:off, heap free:off
   [    0.000000] Memory: 8182308K/8386560K available (5916K kernel code, 368K rwdata, 1840K rodata, 213K init, 304K bss, 204252K reserved, 0K cma-reserved)
   [    0.000000] SLUB: HWalign=64, Order=0-3, MinObjects=0, CPUs=4, Nodes=1
   [    0.000000] rcu: Hierarchical RCU implementation.
   [    0.000000] rcu:     RCU restricting CPUs from NR_CPUS=8 to nr_cpu_ids=4.
   [    0.000000] rcu: RCU calculated value of scheduler-enlistment delay is 25 jiffies.
   [    0.000000] rcu: Adjusting geometry for rcu_fanout_leaf=16, nr_cpu_ids=4
   [    0.000000] NR_IRQS: 0, nr_irqs: 0, preallocated irqs: 0
   [    0.000000] plic: mapped 53 interrupts with 4 handlers for 9 contexts.
   [    0.000000] riscv_timer_init_dt: Registering clocksource cpuid [0] hartid [1]
   [    0.000000] clocksource: riscv_clocksource: mask: 0xffffffffffffffff max_cycles: 0x1d854df40, max_idle_ns: 3526361616960 ns
   [    0.000006] sched_clock: 64 bits at 1000kHz, resolution 1000ns, wraps every 2199023255500ns
   [    0.008559] Console: colour dummy device 80x25
   [    0.012989] Calibrating delay loop (skipped), value calculated using timer frequency.. 2.00 BogoMIPS (lpj=4000)
   [    0.023104] pid_max: default: 32768 minimum: 301
   [    0.028273] Mount-cache hash table entries: 16384 (order: 5, 131072 bytes, linear)
   [    0.035765] Mountpoint-cache hash table entries: 16384 (order: 5, 131072 bytes, linear)
   [    0.045307] rcu: Hierarchical SRCU implementation.
   [    0.049875] smp: Bringing up secondary CPUs ...
   [    0.055729] smp: Brought up 1 node, 4 CPUs
   [    0.060599] devtmpfs: initialized
   [    0.064819] random: get_random_u32 called from bucket_table_alloc.isra.10+0x4e/0x160 with crng_init=0
   [    0.073720] clocksource: jiffies: mask: 0xffffffff max_cycles: 0xffffffff, max_idle_ns: 7645041785100000 ns
   [    0.083176] futex hash table entries: 1024 (order: 4, 65536 bytes, linear)
   [    0.090721] NET: Registered protocol family 16
   [    0.106319] vgaarb: loaded
   [    0.108670] SCSI subsystem initialized
   [    0.112515] usbcore: registered new interface driver usbfs
   [    0.117758] usbcore: registered new interface driver hub
   [    0.123167] usbcore: registered new device driver usb
   [    0.128905] clocksource: Switched to clocksource riscv_clocksource
   [    0.141239] NET: Registered protocol family 2
   [    0.145506] tcp_listen_portaddr_hash hash table entries: 4096 (order: 4, 65536 bytes, linear)
   [    0.153754] TCP established hash table entries: 65536 (order: 7, 524288 bytes, linear)
   [    0.163466] TCP bind hash table entries: 65536 (order: 8, 1048576 bytes, linear)
   [    0.173468] TCP: Hash tables configured (established 65536 bind 65536)
   [    0.179739] UDP hash table entries: 4096 (order: 5, 131072 bytes, linear)
   [    0.186627] UDP-Lite hash table entries: 4096 (order: 5, 131072 bytes, linear)
   [    0.194117] NET: Registered protocol family 1
   [    0.198417] RPC: Registered named UNIX socket transport module.
   [    0.203887] RPC: Registered udp transport module.
   [    0.208664] RPC: Registered tcp transport module.
   [    0.213429] RPC: Registered tcp NFSv4.1 backchannel transport module.
   [    0.219944] PCI: CLS 0 bytes, default 64
   [    0.224170] Unpacking initramfs...
   [    0.262347] Freeing initrd memory: 2336K
   [    0.266531] workingset: timestamp_bits=62 max_order=21 bucket_order=0
   [    0.280406] NFS: Registering the id_resolver key type
   [    0.284798] Key type id_resolver registered
   [    0.289048] Key type id_legacy registered
   [    0.293114] nfs4filelayout_init: NFSv4 File Layout Driver Registering...
   [    0.300262] NET: Registered protocol family 38
   [    0.304432] Block layer SCSI generic (bsg) driver version 0.4 loaded (major 254)
   [    0.311862] io scheduler mq-deadline registered
   [    0.316461] io scheduler kyber registered
   [    0.356421] Serial: 8250/16550 driver, 4 ports, IRQ sharing disabled
   [    0.363004] 10010000.serial: ttySIF0 at MMIO 0x10010000 (irq = 4, base_baud = 0) is a SiFive UART v0
   [    0.371468] printk: console [ttySIF0] enabled
   [    0.371468] printk: console [ttySIF0] enabled
   [    0.380223] printk: bootconsole [sbi0] disabled
   [    0.380223] printk: bootconsole [sbi0] disabled
   [    0.389589] 10011000.serial: ttySIF1 at MMIO 0x10011000 (irq = 1, base_baud = 0) is a SiFive UART v0
   [    0.398680] [drm] radeon kernel modesetting enabled.
   [    0.412395] loop: module loaded
   [    0.415214] sifive_spi 10040000.spi: mapped; irq=3, cs=1
   [    0.420628] sifive_spi 10050000.spi: mapped; irq=5, cs=1
   [    0.425897] libphy: Fixed MDIO Bus: probed
   [    0.429964] macb 10090000.ethernet: Registered clk switch 'sifive-gemgxl-mgmt'
   [    0.436743] macb: GEM doesn't support hardware ptp.
   [    0.441621] libphy: MACB_mii_bus: probed
   [    0.601316] Microsemi VSC8541 SyncE 10090000.ethernet-ffffffff:00: attached PHY driver [Microsemi VSC8541 SyncE] (mii_bus:phy_addr=10090000.ethernet-ffffffff:00, irq=POLL)
   [    0.615857] macb 10090000.ethernet eth0: Cadence GEM rev 0x10070109 at 0x10090000 irq 6 (70:b3:d5:92:f2:f3)
   [    0.625634] e1000e: Intel(R) PRO/1000 Network Driver - 3.2.6-k
   [    0.631381] e1000e: Copyright(c) 1999 - 2015 Intel Corporation.
   [    0.637382] ehci_hcd: USB 2.0 'Enhanced' Host Controller (EHCI) Driver
   [    0.643799] ehci-pci: EHCI PCI platform driver
   [    0.648261] ehci-platform: EHCI generic platform driver
   [    0.653497] ohci_hcd: USB 1.1 'Open' Host Controller (OHCI) Driver
   [    0.659599] ohci-pci: OHCI PCI platform driver
   [    0.664055] ohci-platform: OHCI generic platform driver
   [    0.669448] usbcore: registered new interface driver uas
   [    0.674575] usbcore: registered new interface driver usb-storage
   [    0.680642] mousedev: PS/2 mouse device common for all mice
   [    0.709493] mmc_spi spi1.0: SD/MMC host mmc0, no DMA, no WP, no poweroff, cd polling
   [    0.716615] usbcore: registered new interface driver usbhid
   [    0.722023] usbhid: USB HID core driver
   [    0.726738] NET: Registered protocol family 10
   [    0.731359] Segment Routing with IPv6
   [    0.734332] sit: IPv6, IPv4 and MPLS over IPv4 tunneling driver
   [    0.740687] NET: Registered protocol family 17
   [    0.744660] Key type dns_resolver registered
   [    0.806775] mmc0: host does not support reading read-only switch, assuming write-enable
   [    0.814020] mmc0: new SDHC card on SPI
   [    0.820137] mmcblk0: mmc0:0000 SU08G 7.40 GiB
   [    0.850220]  mmcblk0: p1 p2
   [    3.821524] macb 10090000.ethernet eth0: link up (1000/Full)
   [    3.828938] IPv6: ADDRCONF(NETDEV_CHANGE): eth0: link becomes ready
   [    3.848919] Sending DHCP requests .., OK
   [    6.252076] IP-Config: Got DHCP answer from 10.206.4.1, my address is 10.206.7.133
   [    6.259624] IP-Config: Complete:
   [    6.262831]      device=eth0, hwaddr=70:b3:d5:92:f2:f3, ipaddr=10.206.7.133, mask=255.255.252.0, gw=10.206.4.1
   [    6.272809]      host=dhcp-10-206-7-133, domain=sdcorp.global.sandisk.com, nis-domain=(none)
   [    6.281228]      bootserver=10.206.126.11, rootserver=10.206.126.11, rootpath=
   [    6.281232]      nameserver0=10.86.1.1, nameserver1=10.86.2.1
   [    6.294179]      ntpserver0=10.86.1.1, ntpserver1=10.86.2.1
   [    6.301026] Freeing unused kernel memory: 212K
   [    6.304683] This architecture does not have kernel memory protection.
   [    6.311121] Run /init as init process
              _  _
             | ||_|
             | | _ ____  _   _  _  _
             | || |  _ \| | | |\ \/ /
             | || | | | | |_| |/    \
             |_||_|_| |_|\____|\_/\_/

                  Busybox Rootfs

   Please press Enter to activate this console.
   / #

Booting from MMC using U-Boot SPL
---------------------------------

Building
~~~~~~~~

Before building U-Boot SPL, OpenSBI must be built first. OpenSBI can be
cloned and built for FU540 as below:

.. code-block:: console

	git clone https://github.com/riscv/opensbi.git
	cd opensbi
	make PLATFORM=generic
	export OPENSBI=<path to opensbi/build/platform/generic/firmware/fw_dynamic.bin>

Now build the U-Boot SPL and U-Boot proper

.. code-block:: console

	cd <U-Boot-dir>
	make sifive_fu540_defconfig
	make

This will generate spl/u-boot-spl.bin and FIT image (u-boot.itb)


Flashing
~~~~~~~~

ZSBL loads the U-Boot SPL (u-boot-spl.bin) from a partition with GUID type
5B193300-FC78-40CD-8002-E86C45580B47

U-Boot SPL expects a U-Boot FIT image (u-boot.itb) from a partition with GUID
type 2E54B353-1271-4842-806F-E436D6AF6985

FIT image (u-boot.itb) is a combination of fw_dynamic.bin, u-boot-nodtb.bin and
device tree blob (hifive-unleashed-a00.dtb)

Format the SD card (make sure the disk has GPT, otherwise use gdisk to switch)

.. code-block:: bash

	sudo sgdisk --clear \
	  --set-alignment=2 \
	  --new=1:34:2081 --change-name=1:loader1 --typecode=1:5B193300-FC78-40CD-8002-E86C45580B47 \
	  --new=2:2082:10273 --change-name=2:loader2 --typecode=2:2E54B353-1271-4842-806F-E436D6AF6985 \
	  --new=3:10274: --change-name=3:rootfs --typecode=3:0FC63DAF-8483-4772-8E79-3D69D8477DE4 \
	  /dev/sdX

Program the SD card

.. code-block:: bash

	sudo dd if=spl/u-boot-spl.bin of=/dev/sdX seek=34
	sudo dd if=u-boot.itb of=/dev/sdX seek=2082

Booting
~~~~~~~

Once you plugin the sdcard and power up, you should see the U-Boot prompt.

Sample boot log from HiFive Unleashed board
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: none

	U-Boot SPL 2020.04-rc2-00109-g63efc7e07e-dirty (Apr 30 2020 - 13:52:36 +0530)
	Trying to boot from MMC1


	U-Boot 2020.04-rc2-00109-g63efc7e07e-dirty (Apr 30 2020 - 13:52:36 +0530)

	CPU:   rv64imafdc
	Model: SiFive HiFive Unleashed A00
	DRAM:  8 GiB
	MMC:   spi@10050000:mmc@0: 0
	In:    serial@10010000
	Out:   serial@10010000
	Err:   serial@10010000
	Net:   eth0: ethernet@10090000
	Hit any key to stop autoboot:  0
	=> version
	U-Boot 2020.04-rc2-00109-g63efc7e07e-dirty (Apr 30 2020 - 13:52:36 +0530)

	riscv64-unknown-linux-gnu-gcc (crosstool-NG 1.24.0.37-3f461da) 9.2.0
	GNU ld (crosstool-NG 1.24.0.37-3f461da) 2.32
	=> mmc info
	Device: spi@10050000:mmc@0
	Manufacturer ID: 3
	OEM: 5344
	Name: SC16G
	Bus Speed: 20000000
	Mode: SD Legacy
	Rd Block Len: 512
	SD version 2.0
	High Capacity: Yes
	Capacity: 14.8 GiB
	Bus Width: 1-bit
	Erase Group Size: 512 Bytes
	=> mmc part

	Partition Map for MMC device 0  --   Partition Type: EFI

	Part    Start LBA       End LBA         Name
	Attributes
	Type GUID
	Partition GUID
	1     0x00000022      0x00000821      "loader1"
	attrs:  0x0000000000000000
	type:   5b193300-fc78-40cd-8002-e86c45580b47
	guid:   66e2b5d2-74db-4df8-ad6f-694b3617f87f
	2     0x00000822      0x00002821      "loader2"
	attrs:  0x0000000000000000
	type:   2e54b353-1271-4842-806f-e436d6af6985
	guid:   8befaeaf-bca0-435d-b002-e201f37c0a2f
	3     0x00002822      0x01dacbde      "rootfs"
	attrs:  0x0000000000000000
	type:   0fc63daf-8483-4772-8e79-3d69d8477de4
	type:   linux
	guid:   9faa81b6-39b1-4418-af5e-89c48f29c20d

Booting from SPI
----------------

Use Building steps from "Booting from MMC using U-Boot SPL" section.

Partition the SPI in Linux via mtdblock. (Require to boot the board in
SD boot mode by enabling MTD block in Linux)

Use prebuilt image from here [1], which support to partition the SPI flash.

.. code-block:: none

  # sgdisk --clear \
  > --set-alignment=2 \
  > --new=1:40:2087 --change-name=1:loader1 --typecode=1:5B193300-FC78-40CD-8002-E86C45580B47 \
  > --new=2:2088:10279 --change-name=2:loader2 --typecode=2:2E54B353-1271-4842-806F-E436D6AF6985 \
  > --new=3:10536:65494 --change-name=3:rootfs --typecode=3:0FC63DAF-8483-4772-8E79-3D69D8477DE4 \
  > /dev/mtdblock0

Program the SPI (Require to boot the board in SD boot mode)

Execute below steps on U-Boot proper,

.. code-block:: none

  tftpboot $kernel_addr_r u-boot-spl.bin
  sf erase 0x5000 $filesize
  sf write $kernel_addr_r 0x5000 $filesize

  tftpboot $kernel_addr_r u-boot.itb
  sf erase 0x105000 $filesize
  sf write $kernel_addr_r 0x105000 $filesize

Power off the board

Change DIP switches MSEL[3:0] are set to 0110

Power up the board.

[1] https://github.com/amarula/bsp-sifive
