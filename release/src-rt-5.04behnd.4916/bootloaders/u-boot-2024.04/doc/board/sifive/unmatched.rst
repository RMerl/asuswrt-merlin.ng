.. SPDX-License-Identifier: GPL-2.0+

HiFive Unmatched
================

FU740-C000 RISC-V SoC
---------------------
The FU740-C000 is a 4+1 64-bit RISC-V core SoC from SiFive.

The HiFive Unmatched development platform is based on FU740-C000 and capable
of running Linux.

Mainline support
----------------
The support for following drivers are already enabled:

1. SiFive UART Driver.
2. SiFive PRCI Driver for clock.
3. Cadence MACB ethernet driver for networking support.
4. SiFive SPI Driver.
5. MMC SPI Driver for MMC/SD support.

Booting from micro SD card using U-Boot SPL
-------------------------------------------

Booting from an SD card requires that the boot mode selection DIP switches
MSEL[3:0] are set to 1011.

Building
--------

Before building U-Boot SPL, OpenSBI must be built first. OpenSBI can be
cloned and built for FU740 as below:

.. code-block:: console

	git clone https://github.com/riscv/opensbi.git
	cd opensbi
	make PLATFORM=generic
	export OPENSBI=<path to opensbi/build/platform/generic/firmware/fw_dynamic.bin>

Now build the U-Boot SPL and U-Boot proper

.. code-block:: console

	cd <U-Boot-dir>
	make sifive_unmatched_defconfig
	make

This will generate spl/u-boot-spl.bin and u-boot.itb


Flashing
--------

ZSBL loads the U-Boot SPL (u-boot-spl.bin) from a partition with GUID type
5B193300-FC78-40CD-8002-E86C45580B47

With the default configuration U-Boot SPL expects u-boot.itb starting at sector
2082 (CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR=0x822). It is recommended to use a
partition with type GUID 2E54B353-1271-4842-806F-E436D6AF6985 for storing
main U-Boot.

u-boot.itb is a combination of fw_dynamic.bin, u-boot-nodtb.bin and
device tree blob (hifive-unmatched-a00.dtb)

Format the SD card (make sure the disk has GPT, otherwise use gdisk to switch)

.. code-block:: bash

	sudo sgdisk -g --clear -a 1 \
	  --new=1:34:2081         --change-name=1:spl --typecode=1:5B193300-FC78-40CD-8002-E86C45580B47 \
	  --new=2:2082:10273      --change-name=2:uboot  --typecode=2:2E54B353-1271-4842-806F-E436D6AF6985 \
	  --new=3:16384:282623    --change-name=3:boot --typecode=3:0x0700 \
	  --new=4:286720:13918207 --change-name=4:root --typecode=4:0x8300 \
	  /dev/sdX

Copy linux Image.gz and hifive-unmatched-a00.dtb to boot partition

.. code-block:: bash

	sudo mkfs.vfat /dev/sdX3
	sudo mkfs.ext4 /dev/sdX4

	sudo mount /dev/sdX3 /media/sdX3
	sudo cp Image.gz hifive-unmatched-a00.dtb /media/sdX3/

Program the SD card

.. code-block:: bash

	sudo dd if=spl/u-boot-spl.bin of=/dev/sdX seek=34
	sudo dd if=u-boot.itb of=/dev/sdX seek=2082

Booting
-------
Once you plugin the sdcard and power up, you should see the U-Boot prompt.


Loading the kernel and dtb

.. code-block:: none

	fatload mmc 0:3 ${kernel_addr_r} Image.gz
	fatload mmc 0:3 ${fdt_addr_r} hifive-unmatched-a00.dtb
	booti ${kernel_addr_r} - ${fdt_addr_r}


Sample boot log from HiFive Unmatched board
-------------------------------------------

.. code-block:: none

	U-Boot SPL 2021.04-rc4-00009-g7d70643cc3-dirty (Mar 16 2021 - 18:03:14 +0800)
	Trying to boot from MMC1

	U-Boot 2021.04-rc4-00009-g7d70643cc3-dirty (Mar 16 2021 - 18:03:14 +0800)

	CPU:   rv64imafdc
	Model: SiFive HiFive Unmatched A00
	DRAM:  16 GiB
	MMC:   spi@10050000:mmc@0: 0
	In:    serial@10010000
	Out:   serial@10010000
	Err:   serial@10010000
	Model: SiFive HiFive Unmatched A00
	Net:
	Error: ethernet@10090000 address not set.
	No ethernet found.

	Hit any key to stop autoboot:  0
	PCIe Link up, Gen1

	Device 0: Vendor: 0x126f Rev: S1111A0L Prod: AA000000000000001995
		    Type: Hard Disk
		    Capacity: 488386.3 MB = 476.9 GB (1000215216 x 512)
	... is now current device
	Scanning nvme 0:1...
	libfdt fdt_check_header(): FDT_ERR_BADMAGIC
	Scanning disk mmc@0.blk...
	** Unrecognized filesystem type **
	** Unrecognized filesystem type **
	Scanning disk nvme#0.blk#0...
	Found 8 disks
	No EFI system partition

	Error: ethernet@10090000 address not set.
	BootOrder not defined
	EFI boot manager: Cannot load any image
	starting USB...
	Bus xhci_pci: Register 4000840 NbrPorts 4
	Starting the controller
	USB XHCI 1.00
	scanning bus xhci_pci for devices... 3 USB Device(s) found
	       scanning usb for storage devices... 0 Storage Device(s) found

	Device 0: unknown device
	switch to partitions #0, OK
	mmc0 is current device
	Scanning mmc 0:3...
	Found /extlinux/extlinux.conf
	Retrieving file: /extlinux/extlinux.conf
	205 bytes read in 9 ms (21.5 KiB/s)
	1:      OpenEmbedded-SiFive-HiFive-Unmatched
	Retrieving file: /Image.gz
	7225919 bytes read in 4734 ms (1.5 MiB/s)
	append: root=/dev/mmcblk0p4 rootfstype=ext4 rootwait console=ttySIF0,115200 earlycon=sbi
	Retrieving file: /hifive-unmatched-a00.dtb
	10445 bytes read in 13 ms (784.2 KiB/s)
	   Uncompressing Kernel Image
	Moving Image from 0x84000000 to 0x80200000, end=81629000
	## Flattened Device Tree blob at 88000000
	   Booting using the fdt blob at 0x88000000
	   Using Device Tree in place at 0000000088000000, end 00000000880058cc

	Starting kernel ...

	[    0.000000] Linux version 5.10.15 (oe-user@oe-host) (riscv64-oe-linux-gcc (GCC) 10.2.0, GNU ld (GNU Binutils) 2.35.0.201
	[    0.000000] OF: fdt: Ignoring memory range 0x80000000 - 0x80200000
	[    0.000000] earlycon: sbi0 at I/O port 0x0 (options '')
	[    0.000000] printk: bootconsole [sbi0] enabled
	[    0.000000] efi: UEFI not found.
	[    0.000000] Zone ranges:
	[    0.000000]   DMA32    [mem 0x0000000080200000-0x00000000ffffffff]
	[    0.000000]   Normal   [mem 0x0000000100000000-0x000000027fffffff]
	[    0.000000] Movable zone start for each node
	[    0.000000] Early memory node ranges
	[    0.000000]   node   0: [mem 0x0000000080200000-0x000000027fffffff]
	[    0.000000] Zeroed struct page in unavailable ranges: 512 pages
	[    0.000000] Initmem setup node 0 [mem 0x0000000080200000-0x000000027fffffff]
	[    0.000000] software IO TLB: mapped [mem 0x00000000fbfff000-0x00000000fffff000] (64MB)
	[    0.000000] SBI specification v0.3 detected
	[    0.000000] SBI implementation ID=0x1 Version=0x9
	[    0.000000] SBI v0.2 TIME extension detected
	[    0.000000] SBI v0.2 IPI extension detected
	[    0.000000] SBI v0.2 RFENCE extension detected
	[    0.000000] SBI v0.2 HSM extension detected
	[    0.000000] CPU with hartid=0 is not available
	[    0.000000] CPU with hartid=0 is not available
	[    0.000000] riscv: ISA extensions acdfim
	[    0.000000] riscv: ELF capabilities acdfim
	[    0.000000] percpu: Embedded 26 pages/cpu s66904 r8192 d31400 u106496
	[    0.000000] Built 1 zonelists, mobility grouping on.  Total pages: 2067975
	[    0.000000] Kernel command line: root=/dev/mmcblk0p4 rootfstype=ext4 rootwait console=ttySIF0,115200 earlycon=sbi
	[    0.000000] Dentry cache hash table entries: 1048576 (order: 11, 8388608 bytes, linear)
	[    0.000000] Inode-cache hash table entries: 524288 (order: 10, 4194304 bytes, linear)
	[    0.000000] Sorting __ex_table...
	[    0.000000] mem auto-init: stack:off, heap alloc:off, heap free:off
	[    0.000000] Memory: 8155880K/8386560K available (8490K kernel code, 5515K rwdata, 4096K rodata, 285K init, 383K bss, 23)
	[    0.000000] SLUB: HWalign=64, Order=0-3, MinObjects=0, CPUs=4, Nodes=1
	[    0.000000] rcu: Hierarchical RCU implementation.
	[    0.000000] rcu:     RCU restricting CPUs from NR_CPUS=8 to nr_cpu_ids=4.
	[    0.000000]  Tracing variant of Tasks RCU enabled.
	[    0.000000] rcu: RCU calculated value of scheduler-enlistment delay is 25 jiffies.
	[    0.000000] rcu: Adjusting geometry for rcu_fanout_leaf=16, nr_cpu_ids=4
	[    0.000000] NR_IRQS: 64, nr_irqs: 64, preallocated irqs: 0
	[    0.000000] CPU with hartid=0 is not available
	[    0.000000] riscv-intc: unable to find hart id for /cpus/cpu@0/interrupt-controller
	[    0.000000] riscv-intc: 64 local interrupts mapped
	[    0.000000] plic: interrupt-controller@c000000: mapped 69 interrupts with 4 handlers for 9 contexts.
	[    0.000000] random: get_random_bytes called from 0xffffffe000002a6a with crng_init=0
	[    0.000000] riscv_timer_init_dt: Registering clocksource cpuid [0] hartid [1]
	[    0.000000] clocksource: riscv_clocksource: mask: 0xffffffffffffffff max_cycles: 0x1d854df40, max_idle_ns: 352636161696s
	[    0.000007] sched_clock: 64 bits at 1000kHz, resolution 1000ns, wraps every 2199023255500ns
	[    0.008626] Console: colour dummy device 80x25
	[    0.013049] Calibrating delay loop (skipped), value calculated using timer frequency.. 2.00 BogoMIPS (lpj=4000)
	[    0.023115] pid_max: default: 32768 minimum: 301
	[    0.028423] Mount-cache hash table entries: 16384 (order: 5, 131072 bytes, linear)
	[    0.035919] Mountpoint-cache hash table entries: 16384 (order: 5, 131072 bytes, linear)
	[    0.045957] rcu: Hierarchical SRCU implementation.
	[    0.050393] EFI services will not be available.
	[    0.055132] smp: Bringing up secondary CPUs ...
	[    0.061824] smp: Brought up 1 node, 4 CPUs
	[    0.067458] devtmpfs: initialized
	[    0.072700] clocksource: jiffies: mask: 0xffffffff max_cycles: 0xffffffff, max_idle_ns: 7645041785100000 ns
	[    0.081789] futex hash table entries: 1024 (order: 4, 65536 bytes, linear)
	[    0.089738] NET: Registered protocol family 16
	[    0.093999] thermal_sys: Registered thermal governor 'step_wise'
	[    0.109208] iommu: Default domain type: Translated
	[    0.119694] vgaarb: loaded
	[    0.122571] SCSI subsystem initialized
	[    0.126499] usbcore: registered new interface driver usbfs
	[    0.131686] usbcore: registered new interface driver hub
	[    0.137071] usbcore: registered new device driver usb
	[    0.142286] EDAC MC: Ver: 3.0.0
	[    0.145760] Advanced Linux Sound Architecture Driver Initialized.
	[    0.152205] clocksource: Switched to clocksource riscv_clocksource
	[    1.046286] VFS: Disk quotas dquot_6.6.0
	[    1.049651] VFS: Dquot-cache hash table entries: 512 (order 0, 4096 bytes)
	[    1.062844] NET: Registered protocol family 2
	[    1.067172] tcp_listen_portaddr_hash hash table entries: 4096 (order: 4, 65536 bytes, linear)
	[    1.075455] TCP established hash table entries: 65536 (order: 7, 524288 bytes, linear)
	[    1.085428] TCP bind hash table entries: 65536 (order: 8, 1048576 bytes, linear)
	[    1.096548] TCP: Hash tables configured (established 65536 bind 65536)
	[    1.103043] UDP hash table entries: 4096 (order: 5, 131072 bytes, linear)
	[    1.109879] UDP-Lite hash table entries: 4096 (order: 5, 131072 bytes, linear)
	[    1.117413] NET: Registered protocol family 1
	[    1.121881] RPC: Registered named UNIX socket transport module.
	[    1.127139] RPC: Registered udp transport module.
	[    1.131901] RPC: Registered tcp transport module.
	[    1.136677] RPC: Registered tcp NFSv4.1 backchannel transport module.
	[    1.143194] PCI: CLS 0 bytes, default 64
	[    1.148359] Initialise system trusted keyrings
	[    1.152364] workingset: timestamp_bits=62 max_order=21 bucket_order=0
	[    1.165382] NFS: Registering the id_resolver key type
	[    1.169781] Key type id_resolver registered
	[    1.174011] Key type id_legacy registered
	[    1.178179] nfs4filelayout_init: NFSv4 File Layout Driver Registering...
	[    1.184874] Installing knfsd (copyright (C) 1996 okir@monad.swb.de).
	[    1.192453] 9p: Installing v9fs 9p2000 file system support
	[    1.198116] NET: Registered protocol family 38
	[    1.201886] Key type asymmetric registered
	[    1.206046] Asymmetric key parser 'x509' registered
	[    1.211029] Block layer SCSI generic (bsg) driver version 0.4 loaded (major 252)
	[    1.218468] io scheduler mq-deadline registered
	[    1.223072] io scheduler kyber registered
	[    1.228803] shpchp: Standard Hot Plug PCI Controller Driver version: 0.4
	[    1.235017] fu740-pcie e00000000.pcie: FPGA PCIE PROBE
	[    1.281706] fu740-pcie e00000000.pcie: PCIE-PERSTN is GPIO 504
	[    1.286922] fu740-pcie e00000000.pcie: PWREN is GPIO 501
	[    1.292377] fu740-pcie e00000000.pcie: host bridge /soc/pcie@e00000000 ranges:
	[    1.299603] fu740-pcie e00000000.pcie:       IO 0x0060080000..0x006008ffff -> 0x0060080000
	[    1.307922] fu740-pcie e00000000.pcie:      MEM 0x0060090000..0x0070ffffff -> 0x0060090000
	[    1.316244] fu740-pcie e00000000.pcie:      MEM 0x2000000000..0x3fffffffff -> 0x2000000000
	[    1.432223] fu740-pcie e00000000.pcie: PWREN enabling
	[    1.436607] fu740-pcie e00000000.pcie: PWREN valid
	[    1.560226] fu740-pcie e00000000.pcie: invalid resource
	[    1.664802] fu740-pcie e00000000.pcie: Link up
	[    1.768582] fu740-pcie e00000000.pcie: Link up
	[    1.872369] fu740-pcie e00000000.pcie: Link up
	[    1.876116] fu740-pcie e00000000.pcie: Link up, Gen3
	[    1.881352] fu740-pcie e00000000.pcie: PCI host bridge to bus 0000:00
	[    1.887700] pci_bus 0000:00: root bus resource [bus 00-ff]
	[    1.893247] pci_bus 0000:00: root bus resource [io  0x0000-0xffff] (bus address [0x60080000-0x6008ffff])
	[    1.902807] pci_bus 0000:00: root bus resource [mem 0x60090000-0x70ffffff]
	[    1.909748] pci_bus 0000:00: root bus resource [mem 0x2000000000-0x3fffffffff pref]
	[    1.917517] pci 0000:00:00.0: [f15e:0000] type 01 class 0x060400
	[    1.923569] pci 0000:00:00.0: reg 0x10: [mem 0x00000000-0x000fffff]
	[    1.929902] pci 0000:00:00.0: reg 0x38: [mem 0x00000000-0x0000ffff pref]
	[    1.936723] pci 0000:00:00.0: supports D1
	[    1.940755] pci 0000:00:00.0: PME# supported from D0 D1 D3hot
	[    1.947619] pci 0000:01:00.0: [1b21:2824] type 01 class 0x060400
	[    1.953052] pci 0000:01:00.0: enabling Extended Tags
	[    1.958165] pci 0000:01:00.0: PME# supported from D0 D3hot D3cold
	[    1.976890] pci 0000:01:00.0: bridge configuration invalid ([bus 00-00]), reconfiguring
	[    1.984425] pci 0000:02:00.0: [1b21:2824] type 01 class 0x060400
	[    1.990396] pci 0000:02:00.0: enabling Extended Tags
	[    1.995509] pci 0000:02:00.0: PME# supported from D0 D3hot D3cold
	[    2.001938] pci 0000:02:02.0: [1b21:2824] type 01 class 0x060400
	[    2.007682] pci 0000:02:02.0: enabling Extended Tags
	[    2.012793] pci 0000:02:02.0: PME# supported from D0 D3hot D3cold
	[    2.019167] pci 0000:02:03.0: [1b21:2824] type 01 class 0x060400
	[    2.024966] pci 0000:02:03.0: enabling Extended Tags
	[    2.030075] pci 0000:02:03.0: PME# supported from D0 D3hot D3cold
	[    2.036468] pci 0000:02:04.0: [1b21:2824] type 01 class 0x060400
	[    2.042250] pci 0000:02:04.0: enabling Extended Tags
	[    2.047359] pci 0000:02:04.0: PME# supported from D0 D3hot D3cold
	[    2.053811] pci 0000:02:08.0: [1b21:2824] type 01 class 0x060400
	[    2.059534] pci 0000:02:08.0: enabling Extended Tags
	[    2.064647] pci 0000:02:08.0: PME# supported from D0 D3hot D3cold
	[    2.071499] pci 0000:02:00.0: bridge configuration invalid ([bus 00-00]), reconfiguring
	[    2.078837] pci 0000:02:02.0: bridge configuration invalid ([bus 00-00]), reconfiguring
	[    2.086911] pci 0000:02:03.0: bridge configuration invalid ([bus 00-00]), reconfiguring
	[    2.094987] pci 0000:02:04.0: bridge configuration invalid ([bus 00-00]), reconfiguring
	[    2.103075] pci 0000:02:08.0: bridge configuration invalid ([bus 00-00]), reconfiguring
	[    2.111901] pci_bus 0000:03: busn_res: [bus 03-ff] end is updated to 03
	[    2.118031] pci 0000:04:00.0: [1b21:1142] type 00 class 0x0c0330
	[    2.123968] pci 0000:04:00.0: reg 0x10: [mem 0x00000000-0x00007fff 64bit]
	[    2.131038] pci 0000:04:00.0: PME# supported from D3cold
	[    2.148888] pci_bus 0000:04: busn_res: [bus 04-ff] end is updated to 04
	[    2.155588] pci_bus 0000:05: busn_res: [bus 05-ff] end is updated to 05
	[    2.162286] pci_bus 0000:06: busn_res: [bus 06-ff] end is updated to 06
	[    2.168408] pci 0000:07:00.0: [126f:2263] type 00 class 0x010802
	[    2.174351] pci 0000:07:00.0: reg 0x10: [mem 0x00000000-0x00003fff 64bit]
	[    2.192890] pci_bus 0000:07: busn_res: [bus 07-ff] end is updated to 07
	[    2.198837] pci_bus 0000:02: busn_res: [bus 02-ff] end is updated to 07
	[    2.205522] pci_bus 0000:01: busn_res: [bus 01-ff] end is updated to 07
	[    2.212241] pci 0000:00:00.0: BAR 0: assigned [mem 0x60100000-0x601fffff]
	[    2.219067] pci 0000:00:00.0: BAR 14: assigned [mem 0x60200000-0x603fffff]
	[    2.226010] pci 0000:00:00.0: BAR 6: assigned [mem 0x60090000-0x6009ffff pref]
	[    2.233308] pci 0000:01:00.0: BAR 14: assigned [mem 0x60200000-0x603fffff]
	[    2.240259] pci 0000:02:02.0: BAR 14: assigned [mem 0x60200000-0x602fffff]
	[    2.247203] pci 0000:02:08.0: BAR 14: assigned [mem 0x60300000-0x603fffff]
	[    2.254150] pci 0000:02:00.0: PCI bridge to [bus 03]
	[    2.259217] pci 0000:04:00.0: BAR 0: assigned [mem 0x60200000-0x60207fff 64bit]
	[    2.266594] pci 0000:02:02.0: PCI bridge to [bus 04]
	[    2.271615] pci 0000:02:02.0:   bridge window [mem 0x60200000-0x602fffff]
	[    2.278485] pci 0000:02:03.0: PCI bridge to [bus 05]
	[    2.283529] pci 0000:02:04.0: PCI bridge to [bus 06]
	[    2.288572] pci 0000:07:00.0: BAR 0: assigned [mem 0x60300000-0x60303fff 64bit]
	[    2.295952] pci 0000:02:08.0: PCI bridge to [bus 07]
	[    2.300973] pci 0000:02:08.0:   bridge window [mem 0x60300000-0x603fffff]
	[    2.307842] pci 0000:01:00.0: PCI bridge to [bus 02-07]
	[    2.313133] pci 0000:01:00.0:   bridge window [mem 0x60200000-0x603fffff]
	[    2.320009] pci 0000:00:00.0: PCI bridge to [bus 01-07]
	[    2.325288] pci 0000:00:00.0:   bridge window [mem 0x60200000-0x603fffff]
	[    2.332808] pcieport 0000:00:00.0: AER: enabled with IRQ 51
	[    2.337946] pcieport 0000:01:00.0: enabling device (0000 -> 0002)
	[    2.344786] pcieport 0000:02:02.0: enabling device (0000 -> 0002)
	[    2.351328] pcieport 0000:02:08.0: enabling device (0000 -> 0002)
	[    2.357091] pci 0000:04:00.0: enabling device (0000 -> 0002)
	[    2.362751] switchtec: loaded.
	[    2.365933] L2CACHE: DataError @ 0x00000003.00964470
	[    2.365992] L2CACHE: No. of Banks in the cache: 4
	[    2.375414] L2CACHE: No. of ways per bank: 16
	[    2.379846] L2CACHE: Sets per bank: 512
	[    2.383751] L2CACHE: Bytes per cache block: 64
	[    2.388267] L2CACHE: Index of the largest way enabled: 15
	[    2.434865] Serial: 8250/16550 driver, 4 ports, IRQ sharing disabled
	[    2.441695] 10010000.serial: ttySIF0 at MMIO 0x10010000 (irq = 1, base_baud = 115200) is a SiFive UART v0
	[    2.450625] printk: console [ttySIF0] enabled
	[    2.450625] printk: console [ttySIF0] enabled
	[    2.459360] printk: bootconsole [sbi0] disabled
	[    2.459360] printk: bootconsole [sbi0] disabled
	[    2.468824] 10011000.serial: ttySIF1 at MMIO 0x10011000 (irq = 2, base_baud = 115200) is a SiFive UART v0
	[    2.493853] loop: module loaded
	[    2.526475] nvme nvme0: pci function 0000:07:00.0
	[    2.530852] nvme 0000:07:00.0: enabling device (0000 -> 0002)
	[    2.537716] Rounding down aligned max_sectors from 4294967295 to 4294967288
	[    2.544470] db_root: cannot open: /etc/target
	[    2.545926] nvme nvme0: allocated 64 MiB host memory buffer.
	[    2.549020] sifive_spi 10040000.spi: mapped; irq=4, cs=1
	[    2.559941] spi-nor spi0.0: is25wp256 (32768 Kbytes)
	[    2.566431] sifive_spi 10050000.spi: mapped; irq=6, cs=1
	[    2.566707] nvme nvme0: 4/0/0 default/read/poll queues
	[    2.571935] libphy: Fixed MDIO Bus: probed
	[    2.580950] macb 10090000.ethernet: Registered clk switch 'sifive-gemgxl-mgmt'
	[    2.587536] macb 10090000.ethernet: invalid hw address, using random
	[    2.588100]  nvme0n1: p1 p2
	[    2.593875] BEU: Load or Store TILINK BUS ERR occurred
	[    2.594342] libphy: MACB_mii_bus: probed
	[    2.599312] macb 10090000.ethernet eth0: Cadence GEM rev 0x10070109 at 0x10090000 irq 7 (5e:57:b8:ab:24:4a)
	[    2.615501] e1000e: Intel(R) PRO/1000 Network Driver
	[    2.620251] e1000e: Copyright(c) 1999 - 2015 Intel Corporation.
	[    2.626463] ehci_hcd: USB 2.0 'Enhanced' Host Controller (EHCI) Driver
	[    2.632684] ehci-pci: EHCI PCI platform driver
	[    2.637144] ohci_hcd: USB 1.1 'Open' Host Controller (OHCI) Driver
	[    2.643273] ohci-pci: OHCI PCI platform driver
	[    2.647731] uhci_hcd: USB Universal Host Controller Interface driver
	[    2.654315] xhci_hcd 0000:04:00.0: xHCI Host Controller
	[    2.659450] xhci_hcd 0000:04:00.0: new USB bus registered, assigned bus number 1
	[    2.807373] xhci_hcd 0000:04:00.0: hcc params 0x0200e081 hci version 0x100 quirks 0x0000000010000410
	[    2.816609] usb usb1: New USB device found, idVendor=1d6b, idProduct=0002, bcdDevice= 5.10
	[    2.824115] usb usb1: New USB device strings: Mfr=3, Product=2, SerialNumber=1
	[    2.831312] usb usb1: Product: xHCI Host Controller
	[    2.836174] usb usb1: Manufacturer: Linux 5.10.15 xhci-hcd
	[    2.841652] usb usb1: SerialNumber: 0000:04:00.0
	[    2.846639] hub 1-0:1.0: USB hub found
	[    2.850037] hub 1-0:1.0: 2 ports detected
	[    2.854306] xhci_hcd 0000:04:00.0: xHCI Host Controller
	[    2.859335] xhci_hcd 0000:04:00.0: new USB bus registered, assigned bus number 2
	[    2.866599] xhci_hcd 0000:04:00.0: Host supports USB 3.0 SuperSpeed
	[    2.873638] usb usb2: We don't know the algorithms for LPM for this host, disabling LPM.
	[    2.881074] usb usb2: New USB device found, idVendor=1d6b, idProduct=0003, bcdDevice= 5.10
	[    2.889212] usb usb2: New USB device strings: Mfr=3, Product=2, SerialNumber=1
	[    2.896422] usb usb2: Product: xHCI Host Controller
	[    2.901282] usb usb2: Manufacturer: Linux 5.10.15 xhci-hcd
	[    2.906752] usb usb2: SerialNumber: 0000:04:00.0
	[    2.911671] hub 2-0:1.0: USB hub found
	[    2.915130] hub 2-0:1.0: 2 ports detected
	[    2.919486] usbcore: registered new interface driver usb-storage
	[    2.925212] usbcore: registered new interface driver usbserial_generic
	[    2.931620] usbserial: USB Serial support registered for generic
	[    2.937771] mousedev: PS/2 mouse device common for all mice
	[    2.943220] usbcore: registered new interface driver usbtouchscreen
	[    2.949466] i2c /dev entries driver
	[    2.954218] lm90 0-004c: supply vcc not found, using dummy regulator
	[    2.961629] EDAC DEVICE0: Giving out device to module Sifive ECC Manager controller sifive_edac.0: DEV sifive_edac.0 (I)
	[    2.997874] mmc_spi spi1.0: SD/MMC host mmc0, no DMA, no WP, no poweroff, cd polling
	[    3.005138] ledtrig-cpu: registered to indicate activity on CPUs
	[    3.010980] usbcore: registered new interface driver usbhid
	[    3.016407] usbhid: USB HID core driver
	[    3.020540] usbcore: registered new interface driver snd-usb-audio
	[    3.027209] NET: Registered protocol family 10
	[    3.031878] Segment Routing with IPv6
	[    3.034864] sit: IPv6, IPv4 and MPLS over IPv4 tunneling driver
	[    3.041232] NET: Registered protocol family 17
	[    3.045324] 9pnet: Installing 9P2000 support
	[    3.049397] Key type dns_resolver registered
	[    3.053786] Loading compiled-in X.509 certificates
	[    3.059729] ALSA device list:
	[    3.061943]   No soundcards found.
	[    3.066057] Waiting for root device /dev/mmcblk0p4...
	[    3.077319] mmc0: host does not support reading read-only switch, assuming write-enable
	[    3.084564] mmc0: new SDHC card on SPI
	[    3.089699] mmcblk0: mmc0:0000 SD32G 29.7 GiB
	[    3.126488] GPT:Primary header thinks Alt. header is not at the end of the disk.
	[    3.133144] GPT:13918241 != 62333951
	[    3.136679] GPT:Alternate GPT header not at the end of the disk.
	[    3.142673] GPT:13918241 != 62333951
	[    3.146231] GPT: Use GNU Parted to correct GPT errors.
	[    3.151398]  mmcblk0: p1 p2 p3 p4
	[    3.212226] usb 1-2: new high-speed USB device number 2 using xhci_hcd
	[    3.258310] EXT4-fs (mmcblk0p4): INFO: recovery required on readonly filesystem
	[    3.264855] EXT4-fs (mmcblk0p4): write access will be enabled during recovery
	[    3.458247] usb 1-2: New USB device found, idVendor=174c, idProduct=2074, bcdDevice= 0.01
	[    3.465662] usb 1-2: New USB device strings: Mfr=2, Product=3, SerialNumber=1
	[    3.472775] usb 1-2: Product: AS2107
	[    3.476336] usb 1-2: Manufacturer: ASMedia
	[    3.480419] usb 1-2: SerialNumber: USB2.0 Hub
	[    3.533583] EXT4-fs (mmcblk0p4): recovery complete
	[    3.543756] EXT4-fs (mmcblk0p4): mounted filesystem with ordered data mode. Opts: (null)
	[    3.551132] VFS: Mounted root (ext4 filesystem) readonly on device 179:4.
	[    3.554682] hub 1-2:1.0: USB hub found
	[    3.561105] devtmpfs: mounted
	[    3.561778] hub 1-2:1.0: 4 ports detected
	[    3.565546] Freeing unused kernel memory: 284K
	[    3.572964] Kernel memory protection not selected by kernel config.
	[    3.579225] Run /sbin/init as init process
	[    3.613136] usb 2-2: new SuperSpeed Gen 1 USB device number 2 using xhci_hcd
	[    3.643539] usb 2-2: New USB device found, idVendor=174c, idProduct=3074, bcdDevice= 0.01
	[    3.650948] usb 2-2: New USB device strings: Mfr=2, Product=3, SerialNumber=1
	[    3.658072] usb 2-2: Product: AS2107
	[    3.661630] usb 2-2: Manufacturer: ASMedia
	[    3.665709] usb 2-2: SerialNumber: USB2.0 Hub
	[    3.762380] hub 2-2:1.0: USB hub found
	[    3.766074] hub 2-2:1.0: 4 ports detected
	[    7.487226] systemd[1]: System time before build time, advancing clock.
	[    7.788093] systemd[1]: systemd 247.2+ running in system mode. (+PAM -AUDIT -SELINUX +IMA -APPARMOR -SMACK +SYSVINIT +U)
	[    7.809694] systemd[1]: Detected architecture riscv64.

	Welcome to OpenEmbedded nodistro.0!

	[    7.832648] systemd[1]: Set hostname to <unmatched>.
	[    9.397499] systemd[1]: Queued start job for default target Multi-User System.
	[    9.408518] random: systemd: uninitialized urandom read (16 bytes read)
	[    9.429329] systemd[1]: Created slice system-getty.slice.
	[  OK  ] Created slice system-getty.slice.
	[    9.440400] random: systemd: uninitialized urandom read (16 bytes read)
	[    9.447086] systemd[1]: Created slice system-modprobe.slice.
	[  OK  ] Created slice system-modprobe.slice.
	[    9.458480] random: systemd: uninitialized urandom read (16 bytes read)
	[    9.465436] systemd[1]: Created slice system-serial\x2dgetty.slice.
	[  OK  ] Created slice system-serial\x2dgetty.slice.
	[    9.478594] systemd[1]: Created slice User and Session Slice.
	[  OK  ] Created slice User and Session Slice.
	[    9.490225] systemd[1]: Started Dispatch Password Requests to Console Directory Watch.
	[  OK  ] Started Dispatch Password ��…ts to Console Directory Watch.
	[    9.506407] systemd[1]: Started Forward Password Requests to Wall Directory Watch.
	[  OK  ] Started Forward Password R��…uests to Wall Directory Watch.
	[    9.522312] systemd[1]: Reached target Paths.
	[  OK  ] Reached target Paths.
	[    9.531078] systemd[1]: Reached target Remote File Systems.
	[  OK  ] Reached target Remote File Systems.
	[    9.542855] systemd[1]: Reached target Slices.
	[  OK  ] Reached target Slices.
	[    9.552712] systemd[1]: Reached target Swap.
	[  OK  ] Reached target Swap.
	[    9.561566] systemd[1]: Listening on initctl Compatibility Named Pipe.
	[  OK  ] Listening on initctl Compatibility Named Pipe.
	[    9.578686] systemd[1]: Condition check resulted in Journal Audit Socket being skipped.
	[    9.586545] systemd[1]: Listening on Journal Socket (/dev/log).
	[  OK  ] Listening on Journal Socket (/dev/log).

	[snip]

	[  OK  ] Reached target System Time Synchronized.
	[  OK  ] Reached target Timers.
	[  OK  ] Listening on D-Bus System Message Bus Socket.
	[  OK  ] Reached target Sockets.
	[  OK  ] Reached target Basic System.
	[  OK  ] Started D-Bus System Message Bus.
		 Starting User Login Management...
		 Starting Permit User Sessions...
	[  OK  ] Started Xinetd A Powerful Replacement For Inetd.
	[  OK  ] Finished Permit User Sessions.
	[  OK  ] Started Getty on tty1.
	[  OK  ] Started Serial Getty on hvc0.
	[  OK  ] Started Serial Getty on ttySIF0.
	[  OK  ] Reached target Login Prompts.
	[  OK  ] Started User Login Management.
	[  OK  ] Reached target Multi-User System.
		 Starting Update UTMP about System Runlevel Changes...
	[  OK  ] Finished Update UTMP about System Runlevel Changes.

	OpenEmbedded nodistro.0 unmatched hvc0

	unmatched login:
	OpenEmbedded nodistro.0 unmatched ttySIF0

	unmatched login:


Booting from SPI
----------------

Use Building steps from "Booting from uSD using U-Boot SPL" section.

Partition the SPI in Linux via mtdblock.  The partition types here are
"HiFive Unleashed FSBL", "HiFive Unleashed BBL", and "U-Boot environment"
for partitions one through three respectively.

.. code-block:: none

	sgdisk --clear -a 1 \
	    --new=1:40:2087     --change-name=1:spl   --typecode=1:5B193300-FC78-40CD-8002-E86C45580B47 \
	    --new=2:2088:10279  --change-name=2:uboot --typecode=2:2E54B353-1271-4842-806F-E436D6AF6985 \
	    --new=3:10280:10535 --change-name=3:env   --typecode=3:3DE21764-95BD-54BD-A5C3-4ABE786F38A8 \
	    /dev/mtdblock0

Write U-Boot SPL and U-Boot to their partitions.

.. code-block:: none

	dd if=spl/u-boot-spl.bin of=/dev/mtdblock0 bs=4096 seek=5 conv=sync
	dd if=u-boot.itb  of=/dev/mtdblock0 bs=4096 seek=261 conv=sync

Power off the board.

Change DIP switches MSEL[3:0] to 0110.

Power up the board.
