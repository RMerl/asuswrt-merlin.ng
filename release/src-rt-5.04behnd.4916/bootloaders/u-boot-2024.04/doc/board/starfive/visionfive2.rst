.. SPDX-License-Identifier: GPL-2.0+

StarFive VisionFive2
====================

JH7110 RISC-V SoC
-----------------

The JH7110 is 4+1 64-bit RISC-V SoC from StarFive.

The StarFive VisionFive2 development platform is based on JH7110 and capable
of running Linux.

Mainline support
----------------

The support for following drivers are already enabled:

1. ns16550 UART Driver.
2. StarFive JH7110 clock Driver.
3. StarFive JH7110 reset Driver.
4. Cadence QSPI controller Driver.
5. MMC SPI Driver for MMC/SD support.
6. PLDA PCIE controller driver.
7. On-board VL805 PCIE-USB controller driver.

Booting from MMC using U-Boot SPL
---------------------------------

The current U-Boot port is supported in S-mode only and loaded from DRAM.

A prior stage M-mode firmware/bootloader (e.g OpenSBI) is required to
boot the u-boot.itb in S-mode and provide M-mode runtime services.

Currently, the u-boot.itb is used as a dynamic of the OpenSBI FW_DYNAMIC
firmware with the latest.

Building
~~~~~~~~

1. Add the RISC-V toolchain to your PATH.
2. Setup ARCH & cross compilation environment variable:

.. code-block:: none

   export CROSS_COMPILE=<riscv64 toolchain prefix>

Before building U-Boot SPL, OpenSBI must be built first. OpenSBI can be
cloned and built for JH7110 as below:

.. code-block:: console

	git clone https://github.com/riscv/opensbi.git
	cd opensbi
	make PLATFORM=generic FW_TEXT_START=0x40000000 FW_OPTIONS=0

The VisionFive 2 support for OpenSBI was introduced after the v1.2 release.

More detailed description of steps required to build FW_DYNAMIC firmware
is beyond the scope of this document. Please refer OpenSBI documenation.
(Note: OpenSBI git repo is at https://github.com/riscv/opensbi.git)

Now build the U-Boot SPL and U-Boot proper

.. code-block:: console

	cd <U-Boot-dir>
	make starfive_visionfive2_defconfig
	make OPENSBI=$(opensbi_dir)/opensbi/build/platform/generic/firmware/fw_dynamic.bin

This will generate the U-Boot SPL image (spl/u-boot-spl.bin.normal.out) as well
as the FIT image (u-boot.itb) with OpenSBI and U-Boot.

Flashing
~~~~~~~~

The device firmware loads U-Boot SPL (u-boot-spl.bin.normal.out) from the
partition with type GUID 2E54B353-1271-4842-806F-E436D6AF6985. You are free
to choose any partition number.

With the default configuration U-Boot SPL loads the U-Boot FIT image
(u-boot.itb) from partition 2 (CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION=0x2).
When formatting it is recommended to use GUID
BC13C2FF-59E6-4262-A352-B275FD6F7172 for this partition.

The FIT image (u-boot.itb) is a combination of OpenSBI's fw_dynamic.bin,
u-boot-nodtb.bin and the device tree blob
(jh7110-starfive-visionfive-2-v1.3b.dtb or
jh7110-starfive-visionfive-2-v1.2a.dtb).

Format the SD card (make sure the disk has GPT, otherwise use gdisk to switch)

.. code-block:: bash

	sudo sgdisk --clear \
	  --set-alignment=2 \
	  --new=1:4096:8191 --change-name=1:spl --typecode=1:2E54B353-1271-4842-806F-E436D6AF6985\
	  --new=2:8192:16383 --change-name=2:uboot --typecode=2:BC13C2FF-59E6-4262-A352-B275FD6F7172  \
	  --new=3:16384:1654784 --change-name=3:system --typecode=3:EBD0A0A2-B9E5-4433-87C0-68B6B72699C7 \
	  /dev/sdb

Program the SD card

.. code-block:: bash

	sudo dd if=u-boot-spl.bin.normal.out of=/dev/sdb1
	sudo dd if=u-boot.itb of=/dev/sdb2

	sudo mount /dev/sdb3 /mnt/
	sudo cp u-boot-spl.bin.normal.out /mnt/
	sudo cp u-boot.itb /mnt/
	sudo cp Image.gz /mnt/
	sudo cp initramfs.cpio.gz /mnt/
	sudo cp jh7110-starfive-visionfive-2.dtb /mnt/
	sudo umount /mnt

Booting
~~~~~~~

The board provides the DIP switches MSEL[1:0] to select the boot device.
To select booting from SD-card set the DIP switches MSEL[1:0] to 10.

Once you plugin the sdcard and power up, you should see the U-Boot prompt.

Sample boot log from StarFive VisionFive2 board
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: none


	U-Boot SPL 2023.04-rc2-00055-gfc43b9c51a-dirty (Mar 02 2023 - 10:51:39 +0800)
	DDR version: dc2e84f0.
	Trying to boot from MMC2

	OpenSBI v1.2-80-g4b28afc
	  ____                    _____ ____ _____
	 / __ \                  / ____|  _ \_   _|
	| |  | |_ __   ___ _ __ | (___ | |_) || |
	| |  | | '_ \ / _ \ '_ \ \___ \|  _ < | |
	| |__| | |_) |  __/ | | |____) | |_) || |_
	 \____/| .__/ \___|_| |_|_____/|____/_____|
	       | |
	       |_|

	Platform Name             : StarFive VisionFive 2 v1.3B
	Platform Features         : medeleg
	Platform HART Count       : 5
	Platform IPI Device       : aclint-mswi
	Platform Timer Device     : aclint-mtimer @ 4000000Hz
	Platform Console Device   : uart8250
	Platform HSM Device       : ---
	Platform PMU Device       : ---
	Platform Reboot Device    : ---
	Platform Shutdown Device  : ---
	Platform Suspend Device   : ---
	Firmware Base             : 0x40000000
	Firmware Size             : 264 KB
	Firmware RW Offset        : 0x20000
	Runtime SBI Version       : 1.0

	Domain0 Name              : root
	Domain0 Boot HART         : 2
	Domain0 HARTs             : 0*,1*,2*,3*,4*
	Domain0 Region00          : 0x0000000002000000-0x000000000200ffff M: (I,R,W) S/U: ()
	Domain0 Region01          : 0x0000000040000000-0x000000004001ffff M: (R,X) S/U: ()
	Domain0 Region02          : 0x0000000040000000-0x000000004007ffff M: (R,W) S/U: ()
	Domain0 Region03          : 0x0000000000000000-0xffffffffffffffff M: (R,W,X) S/U: (R,W,X)
	Domain0 Next Address      : 0x0000000040200000
	Domain0 Next Arg1         : 0x0000000040287970
	Domain0 Next Mode         : S-mode
	Domain0 SysReset          : yes
	Domain0 SysSuspend        : yes

	Boot HART ID              : 2
	Boot HART Domain          : root
	Boot HART Priv Version    : v1.11
	Boot HART Base ISA        : rv64imafdcbx
	Boot HART ISA Extensions  : none
	Boot HART PMP Count       : 8
	Boot HART PMP Granularity : 4096
	Boot HART PMP Address Bits: 34
	Boot HART MHPM Count      : 2
	Boot HART MIDELEG         : 0x0000000000000222
	Boot HART MEDELEG         : 0x000000000000b109


	U-Boot 2023.04-rc2-00055-gfc43b9c51a-dirty (Mar 02 2023 - 10:51:39 +0800)

	CPU:   rv64imac_zba_zbb
	Model: StarFive VisionFive 2 v1.3B
	DRAM:  8 GiB
	Core:  107 devices, 18 uclasses, devicetree: separate
	MMC:   mmc@16010000: 0, mmc@16020000: 1
	Loading Environment from nowhere... OK
	In:    serial@10000000
	Out:   serial@10000000
	Err:   serial@10000000
	Net:   No ethernet found.
	Working FDT set to ff74a340
	Hit any key to stop autoboot:  0
	StarFive #
	StarFive # version
	U-Boot 2023.04-rc2-00055-gfc43b9c51a-dirty (Mar 02 2023 - 10:51:39 +0800)

	riscv64-buildroot-linux-gnu-gcc.br_real (Buildroot VF2_515_v1.0.0_rc4) 10.3.0
	GNU ld (GNU Binutils) 2.36.1
	StarFive #
	StarFive # mmc dev 1
	switch to partitions #0, OK
	mmc1 is current device
	StarFive # mmc info
	Device: mmc@16020000
	Manufacturer ID: 9f
	OEM: 5449
	Name: SD64G
	Bus Speed: 50000000
	Mode: SD High Speed (50MHz)
	Rd Block Len: 512
	SD version 3.0
	High Capacity: Yes
	Capacity: 58.3 GiB
	Bus Width: 4-bit
	Erase Group Size: 512 Bytes
	StarFive #
	StarFive # mmc part

	Partition Map for MMC device 1  --   Partition Type: EFI

	Part    Start LBA       End LBA         Name
			Attributes
			Type GUID
			Partition GUID
	1     0x00001000      0x00001fff      "spl"
			attrs:  0x0000000000000000
			type:   2e54b353-1271-4842-806f-e436d6af6985
					(2e54b353-1271-4842-806f-e436d6af6985)
			guid:   d5ee2056-3020-475b-9a33-25b4257c9f12
	2     0x00002000      0x00003fff      "uboot"
			attrs:  0x0000000000000000
			type:   bc13c2ff-59e6-4262-a352-b275fd6f7172
					(bc13c2ff-59e6-4262-a352-b275fd6f7172)
			guid:   379ab7fe-fd0c-4149-b758-960c1cbfc0cc
	3     0x00004000      0x00194000      "system"
			attrs:  0x0000000000000000
			type:   ebd0a0a2-b9e5-4433-87c0-68b6b72699c7
					(data)
			guid:   539a6df9-4655-4953-8541-733ca36eb1db
	StarFive #
	StarFive # fatls mmc 1:3
	6429424   Image.gz
	717705   u-boot.itb
	125437   u-boot-spl.bin.normal.out
	152848495   initramfs.cpio.gz
		11285   jh7110-starfive-visionfive-2-v1.3b.dtb

	5 file(s), 0 dir(s)

	StarFive # fatload mmc 1:3 ${kernel_addr_r} Image.gz
	6429424 bytes read in 394 ms (15.6 MiB/s)
	StarFive # fatload mmc 1:3 ${fdt_addr_r} jh7110-starfive-visionfive-2.dtb
	11285 bytes read in 5 ms (2.2 MiB/s)
	StarFive # fatload mmc 1:3 ${ramdisk_addr_r} initramfs.cpio.gz
	152848495 bytes read in 9271 ms (15.7 MiB/s)
	StarFive # booti ${kernel_addr_r} ${ramdisk_addr_r}:${filesize} ${fdt_addr_r}
	Uncompressing Kernel Image
	## Flattened Device Tree blob at 46000000
	Booting using the fdt blob at 0x46000000
	Working FDT set to 46000000
	Loading Ramdisk to f5579000, end fe73d86f ... OK
	Loading Device Tree to 00000000f5573000, end 00000000f5578c14 ... OK
	Working FDT set to f5573000

	Starting kernel ...


	] Linux version 6.2.0-starfive-00026-g11934a315b67 (wyh@wyh-VirtualBox) (riscv64-linux-gnu-gcc (Ubuntu 7.5.0-3ubuntu1~18.04) 7.5.0, GNU ld (GNU Binutils for Ubuntu) 2.30) #1 SMP Thu Mar  2 14:51:36 CST 2023
	[    0.000000] OF: fdt: Ignoring memory range 0x40000000 - 0x40200000
	[    0.000000] Machine model: StarFive VisionFive 2 v1.3B
	[    0.000000] efi: UEFI not found.
	[    0.000000] Zone ranges:
	[    0.000000]   DMA32    [mem 0x0000000040200000-0x00000000ffffffff]
	[    0.000000]   Normal   [mem 0x0000000100000000-0x000000013fffffff]
	[    0.000000] Movable zone start for each node
	[    0.000000] Early memory node ranges
	[    0.000000]   node   0: [mem 0x0000000040200000-0x000000013fffffff]
	[    0.000000] Initmem setup node 0 [mem 0x0000000040200000-0x000000013fffffff]
	[    0.000000] On node 0, zone DMA32: 512 pages in unavailable ranges
	[    0.000000] SBI specification v1.0 detected
	[    0.000000] SBI implementation ID=0x1 Version=0x10002
	[    0.000000] SBI TIME extension detected
	[    0.000000] SBI IPI extension detected
	[    0.000000] SBI RFENCE extension detected
	[    0.000000] SBI HSM extension detected
	[    0.000000] CPU with hartid=0 is not available
	[    0.000000] CPU with hartid=0 is not available
	[    0.000000] CPU with hartid=0 is not available
	[    0.000000] riscv: base ISA extensions acdfim
	[    0.000000] riscv: ELF capabilities acdfim
	[    0.000000] percpu: Embedded 18 pages/cpu s35960 r8192 d29576 u73728
	[    0.000000] pcpu-alloc: s35960 r8192 d29576 u73728 alloc=18*4096
	[    0.000000] pcpu-alloc: [0] 0 [0] 1 [0] 2 [0] 3
	[    0.000000] Built 1 zonelists, mobility grouping on.  Total pages: 1031688
	[    0.000000] Kernel command line: console=ttyS0,115200 debug rootwait earlycon=sbi
	[    0.000000] Dentry cache hash table entries: 524288 (order: 10, 4194304 bytes, linear)
	[    0.000000] Inode-cache hash table entries: 262144 (order: 9, 2097152 bytes, linear)
	[    0.000000] mem auto-init: stack:off, heap alloc:off, heap free:off
	[    0.000000] software IO TLB: area num 4.
	[    0.000000] software IO TLB: mapped [mem 0x00000000f1573000-0x00000000f5573000] (64MB)
	[    0.000000] Virtual kernel memory layout:
	[    0.000000]       fixmap : 0xffffffc6fee00000 - 0xffffffc6ff000000   (2048 kB)
	[    0.000000]       pci io : 0xffffffc6ff000000 - 0xffffffc700000000   (  16 MB)
	[    0.000000]      vmemmap : 0xffffffc700000000 - 0xffffffc800000000   (4096 MB)
	[    0.000000]      vmalloc : 0xffffffc800000000 - 0xffffffd800000000   (  64 GB)
	[    0.000000]      modules : 0xffffffff0136a000 - 0xffffffff80000000   (2028 MB)
	[    0.000000]       lowmem : 0xffffffd800000000 - 0xffffffd8ffe00000   (4094 MB)
	[    0.000000]       kernel : 0xffffffff80000000 - 0xffffffffffffffff   (2047 MB)
	[    0.000000] Memory: 3867604K/4192256K available (8012K kernel code, 4919K rwdata, 4096K rodata, 2190K init, 476K bss, 324652K reserved, 0K cma-reserved)
	[    0.000000] SLUB: HWalign=64, Order=0-3, MinObjects=0, CPUs=4, Nodes=1
	[    0.000000] rcu: Hierarchical RCU implementation.
	[    0.000000] rcu:     RCU restricting CPUs from NR_CPUS=64 to nr_cpu_ids=4.
	[    0.000000] rcu:     RCU debug extended QS entry/exit.
	[    0.000000]  Tracing variant of Tasks RCU enabled.
	[    0.000000] rcu: RCU calculated value of scheduler-enlistment delay is 25 jiffies.
	[    0.000000] rcu: Adjusting geometry for rcu_fanout_leaf=16, nr_cpu_ids=4
	[    0.000000] NR_IRQS: 64, nr_irqs: 64, preallocated irqs: 0
	[    0.000000] CPU with hartid=0 is not available
	[    0.000000] riscv-intc: unable to find hart id for /cpus/cpu@0/interrupt-controller
	[    0.000000] riscv-intc: 64 local interrupts mapped
	[    0.000000] plic: interrupt-controller@c000000: mapped 136 interrupts with 4 handlers for 9 contexts.
	[    0.000000] rcu: srcu_init: Setting srcu_struct sizes based on contention.
	[    0.000000] riscv-timer: riscv_timer_init_dt: Registering clocksource cpuid [0] hartid [4]
	[    0.000000] clocksource: riscv_clocksource: mask: 0xffffffffffffffff max_cycles: 0x1d854df40, max_idle_ns: 881590404240 ns
	[    0.000003] sched_clock: 64 bits at 4MHz, resolution 250ns, wraps every 2199023255500ns
	[    0.000437] Console: colour dummy device 80x25
	[    0.000568] Calibrating delay loop (skipped), value calculated using timer frequency.. 8.00 BogoMIPS (lpj=16000)
	[    0.000602] pid_max: default: 32768 minimum: 301
	[    0.000752] LSM: initializing lsm=capability,integrity
	[    0.001071] Mount-cache hash table entries: 8192 (order: 4, 65536 bytes, linear)
	[    0.001189] Mountpoint-cache hash table entries: 8192 (order: 4, 65536 bytes, linear)
	[    0.004201] CPU node for /cpus/cpu@0 exist but the possible cpu range is :0-3
	[    0.007426] cblist_init_generic: Setting adjustable number of callback queues.
	[    0.007457] cblist_init_generic: Setting shift to 2 and lim to 1.
	[    0.007875] riscv: ELF compat mode unsupported
	[    0.007902] ASID allocator disabled (0 bits)
	[    0.008405] rcu: Hierarchical SRCU implementation.
	[    0.008426] rcu:     Max phase no-delay instances is 1000.
	[    0.009247] EFI services will not be available.
	[    0.010738] smp: Bringing up secondary CPUs ...
	[    0.018358] smp: Brought up 1 node, 4 CPUs
	[    0.021776] devtmpfs: initialized
	[    0.027337] clocksource: jiffies: mask: 0xffffffff max_cycles: 0xffffffff, max_idle_ns: 7645041785100000 ns
	[    0.027389] futex hash table entries: 1024 (order: 4, 65536 bytes, linear)
	[    0.027888] pinctrl core: initialized pinctrl subsystem
	[    0.029881] NET: Registered PF_NETLINK/PF_ROUTE protocol family
	[    0.030401] audit: initializing netlink subsys (disabled)
	[    0.031041] audit: type=2000 audit(0.028:1): state=initialized audit_enabled=0 res=1
	[    0.031943] cpuidle: using governor menu
	[    0.043011] HugeTLB: registered 2.00 MiB page size, pre-allocated 0 pages
	[    0.043033] HugeTLB: 0 KiB vmemmap can be freed for a 2.00 MiB page
	[    0.044943] iommu: Default domain type: Translated
	[    0.044965] iommu: DMA domain TLB invalidation policy: strict mode
	[    0.046089] SCSI subsystem initialized
	[    0.046733] libata version 3.00 loaded.
	[    0.047231] usbcore: registered new interface driver usbfs
	[    0.047315] usbcore: registered new interface driver hub
	[    0.047420] usbcore: registered new device driver usb
	[    0.049770] vgaarb: loaded
	[    0.050277] clocksource: Switched to clocksource riscv_clocksource
	[    0.084690] NET: Registered PF_INET protocol family
	[    0.085561] IP idents hash table entries: 65536 (order: 7, 524288 bytes, linear)
	[    0.093010] tcp_listen_portaddr_hash hash table entries: 2048 (order: 4, 65536 bytes, linear)
	[    0.093152] Table-perturb hash table entries: 65536 (order: 6, 262144 bytes, linear)
	[    0.093224] TCP established hash table entries: 32768 (order: 6, 262144 bytes, linear)
	[    0.093821] TCP bind hash table entries: 32768 (order: 9, 2097152 bytes, linear)
	[    0.117880] TCP: Hash tables configured (established 32768 bind 32768)
	[    0.118500] UDP hash table entries: 2048 (order: 5, 196608 bytes, linear)
	[    0.118881] UDP-Lite hash table entries: 2048 (order: 5, 196608 bytes, linear)
	[    0.119675] NET: Registered PF_UNIX/PF_LOCAL protocol family
	[    0.121749] RPC: Registered named UNIX socket transport module.
	[    0.121776] RPC: Registered udp transport module.
	[    0.121784] RPC: Registered tcp transport module.
	[    0.121791] RPC: Registered tcp NFSv4.1 backchannel transport module.
	[    0.121816] PCI: CLS 0 bytes, default 64
	[    0.124101] Unpacking initramfs...
	[    0.125468] workingset: timestamp_bits=46 max_order=20 bucket_order=0
	[    0.128372] NFS: Registering the id_resolver key type
	[    0.128498] Key type id_resolver registered
	[    0.128525] Key type id_legacy registered
	[    0.128625] nfs4filelayout_init: NFSv4 File Layout Driver Registering...
	[    0.128649] nfs4flexfilelayout_init: NFSv4 Flexfile Layout Driver Registering...
	[    0.129358] 9p: Installing v9fs 9p2000 file system support
	[    0.130179] NET: Registered PF_ALG protocol family
	[    0.130499] Block layer SCSI generic (bsg) driver version 0.4 loaded (major 247)
	[    0.130544] io scheduler mq-deadline registered
	[    0.130556] io scheduler kyber registered
	[    0.416754] Serial: 8250/16550 driver, 4 ports, IRQ sharing disabled
	[    0.420857] SuperH (H)SCI(F) driver initialized
	[    0.443735] loop: module loaded
	[    0.448605] e1000e: Intel(R) PRO/1000 Network Driver
	[    0.448627] e1000e: Copyright(c) 1999 - 2015 Intel Corporation.
	[    0.450716] usbcore: registered new interface driver uas
	[    0.450832] usbcore: registered new interface driver usb-storage
	[    0.451638] mousedev: PS/2 mouse device common for all mice
	[    0.453465] sdhci: Secure Digital Host Controller Interface driver
	[    0.453487] sdhci: Copyright(c) Pierre Ossman
	[    0.453584] sdhci-pltfm: SDHCI platform and OF driver helper
	[    0.454140] usbcore: registered new interface driver usbhid
	[    0.454174] usbhid: USB HID core driver
	[    0.454833] riscv-pmu-sbi: SBI PMU extension is available
	[    0.454920] riscv-pmu-sbi: 16 firmware and 4 hardware counters
	[    0.454942] riscv-pmu-sbi: Perf sampling/filtering is not supported as sscof extension is not available
	[    0.457071] NET: Registered PF_INET6 protocol family
	[    0.460627] Segment Routing with IPv6
	[    0.460821] In-situ OAM (IOAM) with IPv6
	[    0.461005] sit: IPv6, IPv4 and MPLS over IPv4 tunneling driver
	[    0.462712] NET: Registered PF_PACKET protocol family
	[    0.462933] 9pnet: Installing 9P2000 support
	[    0.463141] Key type dns_resolver registered
	[    0.463168] start plist test
	[    0.469261] end plist test
	[    0.506774] debug_vm_pgtable: [debug_vm_pgtable         ]: Validating architecture page table helpers
	[    0.553683] gpio gpiochip0: Static allocation of GPIO base is deprecated, use dynamic allocation.
	[    0.554741] starfive-jh7110-sys-pinctrl 13040000.pinctrl: StarFive GPIO chip registered 64 GPIOs
	[    0.555900] gpio gpiochip1: Static allocation of GPIO base is deprecated, use dynamic allocation.
	[    0.556772] starfive-jh7110-aon-pinctrl 17020000.pinctrl: StarFive GPIO chip registered 4 GPIOs
	[    0.559454] printk: console [ttyS0] disabled
	[    0.579948] 10000000.serial: ttyS0 at MMIO 0x10000000 (irq = 3, base_baud = 1500000) is a 16550A
	[    0.580082] printk: console [ttyS0] enabled
	[   13.642680] Freeing initrd memory: 149264K
	[   13.651051] Freeing unused kernel image (initmem) memory: 2188K
	[   13.666431] Run /init as init process
	[   13.670116]   with arguments:
	[   13.673168]     /init
	[   13.675488]   with environment:
	[   13.678668]     HOME=/
	[   13.681038]     TERM=linux
	Starting syslogd: OK
	Starting klogd: OK
	Running sysctl: OK
	Populating /dev using udev: [   14.145944] udevd[93]: starting version 3.2.10
	[   15.214287] random: crng init done
	[   15.240816] udevd[94]: starting eudev-3.2.10
	done
	Saving random seed: OK
	Starting system message bus: dbus[122]: Unknown username "pulse" in message bus configuration file
	done
	Starting rpcbind: OK
	Starting iptables: OK
	Starting bluetoothd: OK
	Starting network: Waiting for interface eth0 to appear............... timeout!
	run-parts: /etc/network/if-pre-up.d/wait_iface: exit status 1
	FAIL
	Starting dropbear sshd: OK
	Starting NFS statd: OK
	Starting NFS services: OK
	Starting NFS daemon: rpc.nfsd: Unable to access /proc/fs/nfsd errno 2 (No such file or directory).
	Please try, as root, 'mount -t nfsd nfsd /proc/fs/nfsd' and then restart rpc.nfsd to correct the problem
	FAIL
	Starting NFS mountd: OK
	Starting DHCP server: FAIL

	Welcome to Buildroot
	buildroot login:

Booting from SPI
----------------

Use Building steps from "Booting from MMC using U-Boot SPL" section.

Partition the SPI in Linux via mtdblock. (Require to boot the board in
SD boot mode by enabling MTD block in Linux)

Use prebuilt image from here [1], which support to partition the SPI flash.


Program the SPI (Require to boot the board in SD boot mode)

Execute below steps on U-Boot proper,

.. code-block:: none

  sf probe
  fatload mmc 1:3 $kernel_addr_r u-boot.itb
  sf update $kernel_addr_r 0x100000 $filesize

  fatload mmc 1:3 $kernel_addr_r u-boot-spl.bin.normal.out
  sf update $kernel_addr_r 0x0 $filesize


Power off the board

Change DIP switches MSEL[1:0] are set to 00, select the boot mode to flash

Power up the board.
