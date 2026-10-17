.. SPDX-License-Identifier: GPL-2.0+

Openpiton RISC-V SoC
====================

OpenPiton is an open source, manycore processor and research platform. It is a
tiled manycore framework scalable from one to 1/2 billion cores. It supports a
number of ISAs including RISC-V with its P-Mesh cache coherence protocol and
networks on chip. It is highly configurable in both core and uncore components.
OpenPiton has been verified in both ASIC and multiple Xilinx FPGA prototypes
running full-stack Debian linux.

RISC-V Standard Bootflow
------------------------

Currently, OpenPiton implements RISC-V standard bootflow in the following steps
mover.S -> u-boot-spl -> opensbi -> u-boot -> Linux
This board supports S-mode u-boot as well as M-mode SPL

Building OpenPition
-------------------

If you'd like to build OpenPiton, please go to OpenPiton github repo
(at https://github.com/PrincetonUniversity/openpiton) to build from the latest
changes

Building Images
---------------

SPL
~~~

1. Add the RISC-V toolchain to your PATH.
2. Setup ARCH & cross compilation environment variable:

.. code-block:: none

	export CROSS_COMPILE=<riscv64 toolchain prefix>
	export ARCH=riscv

3. make openpiton_riscv64_spl_defconfig
4. make

U-Boot
~~~~~~

1. Add the RISC-V toolchain to your PATH.
2. Setup ARCH & cross compilation environment variable:

.. code-block:: none

	export CROSS_COMPILE=<riscv64 toolchain prefix>
	export ARCH=riscv

3. make openpiton_riscv64_defconfig
4. make

opensbi
~~~~~~~

1. Add the RISC-V toolchain to your PATH.
2. Setup ARCH & cross compilation environment variable:

.. code-block:: none

	export CROSS_COMPILE=<riscv64 toolchain prefix>
	export ARCH=riscv

3. Go to OpenSBI directory
4. make PLATFORM=fpga/openpiton FW_PAYLOAD_PATH=<path to u-boot-nodtb.bin>

Using fw_payload.bin with Linux
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Put the generated fw_payload.bin into the /boot directory on the root filesystem,
plug in the SD card, then flash the bitstream. Linux will boot automatically.

Booting
-------
Once you plugin the sdcard and power up, you should see the U-Boot prompt.

Sample Dual-core Debian boot log from OpenPiton
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: none

	Trying to boot from MMC1

	OpenSBI v0.9-5-gd06cb61
	____                    _____ ____ _____
	/ __ \                  / ____|  _ \_   _|
	| |  | |_ __   ___ _ __ | (___ | |_) || |
	| |  | | '_ \ / _ \ '_ \ \___ \|  _ < | |
	| |__| | |_) |  __/ | | |____) | |_) || |_
	\____/| .__/ \___|_| |_|_____/|____/_____|
	| |
	|_|

	Platform Name             : OPENPITON RISC-V
	Platform Features         : timer,mfdeleg
	Platform HART Count       : 3
	Firmware Base             : 0x80000000
	Firmware Size             : 104 KB
	Runtime SBI Version       : 0.2

	Domain0 Name              : root
	Domain0 Boot HART         : 0
	Domain0 HARTs             : 0*,1*,2*
	Domain0 Region00          : 0x0000000080000000-0x000000008001ffff ()
	Domain0 Region01          : 0x0000000000000000-0xffffffffffffffff (R,W,X)
	Domain0 Next Address      : 0x0000000080200000
	Domain0 Next Arg1         : 0x0000000082200000
	Domain0 Next Mode         : S-mode
	Domain0 SysReset          : yes

	Boot HART ID              : 0
	Boot HART Domain          : root
	Boot HART ISA             : rv64imafdcsu
	Boot HART Features        : scounteren,mcounteren
	Boot HART PMP Count       : 0
	Boot HART PMP Granularity : 0
	Boot HART PMP Address Bits: 0
	Boot HART MHPM Count      : 0
	Boot HART MHPM Count      : 0
	Boot HART MIDELEG         : 0x0000000000000222
	Boot HART MEDELEG         : 0x000000000000b109


	U-Boot 2021.01+ (Jun 12 2021 - 10:31:34 +0800)

	DRAM:  1 GiB
	MMC:   sdhci@f000000000: 0 (eMMC)
	In:    uart@fff0c2c000
	Out:   uart@fff0c2c000
	Err:   uart@fff0c2c000
	Hit any key to stop autoboot:  0
	6492992 bytes read in 5310 ms (1.2 MiB/s)
	## Flattened Device Tree blob at 86000000
	Booting using the fdt blob at 0x86000000
	Loading Device Tree to 00000000bfffa000, end 00000000bffff007 ... OK

	Starting kernel ...

	[    0.000000] OF: fdt: Ignoring memory range 0x80000000 - 0x80200000
	[    0.000000] Linux version 5.6.0-rc4-gb9d34f7e294d-dirty
	[    0.000000] earlycon: sbi0 at I/O port 0x0 (options '')
	[    0.000000] printk: bootconsole [sbi0] enabled
	[    0.000000] Zone ranges:
	[    0.000000]   DMA32    [mem 0x0000000080200000-0x00000000bfffffff]
	[    0.000000]   Normal   empty
	[    0.000000] Movable zone start for each node
	[    0.000000] Early memory node ranges
	[    0.000000]   node   0: [mem 0x0000000080200000-0x00000000bfffffff]
	[    0.000000] Initmem setup node 0 [mem 0x0000000080200000-0x00000000bfffffff]
	[    0.000000] On node 0 totalpages: 261632
	[    0.000000]   DMA32 zone: 4088 pages used for memmap
	[    0.000000]   DMA32 zone: 0 pages reserved
	[    0.000000]   DMA32 zone: 261632 pages, LIFO batch:63
	[    0.000000] software IO TLB: mapped [mem 0xbaffa000-0xbeffa000] (64MB)
	[    0.000000] SBI specification v0.2 detected
	[    0.000000] SBI implementation ID=0x1 Version=0x9
	[    0.000000] SBI v0.2 TIME extension detected
	[    0.000000] SBI v0.2 IPI extension detected
	[    0.000000] SBI v0.2 RFENCE extension detected
	[    0.000000] SBI v0.2 HSM extension detected
	[    0.000000] elf_hwcap is 0x112d
	[    0.000000] percpu: Embedded 16 pages/cpu s25368 r8192 d31976 u65536
	[    0.000000] pcpu-alloc: s25368 r8192 d31976 u65536 alloc=16*4096
	[    0.000000] pcpu-alloc: [0] 0
	[    0.000000] Built 1 zonelists, mobility grouping on.  Total pages: 257544
	[    0.000000] Kernel command line: earlycon=sbi root=/dev/piton_sd1
	[    0.000000] Dentry cache hash table entries: 131072 (order: 8, 1048576 bytes, linear)
	[    0.000000] Inode-cache hash table entries: 65536 (order: 7, 524288 bytes, linear)
	[    0.000000] Sorting __ex_table...
	[    0.000000] mem auto-init: stack:off, heap alloc:off, heap free:off
	[    0.000000] Memory: 956252K/1046528K available (4357K kernel code, 286K rwdata, 1200K rodata, 168K init, 311K bss, 90276K re)
	[    0.000000] SLUB: HWalign=64, Order=0-3, MinObjects=0, CPUs=1, Nodes=1
	[    0.000000] rcu: Hierarchical RCU implementation.
	[    0.000000] rcu:     RCU restricting CPUs from NR_CPUS=8 to nr_cpu_ids=1.
	[    0.000000] rcu: RCU calculated value of scheduler-enlistment delay is 10 jiffies.
	[    0.000000] rcu: Adjusting geometry for rcu_fanout_leaf=16, nr_cpu_ids=1
	[    0.000000] NR_IRQS: 0, nr_irqs: 0, preallocated irqs: 0
	[    0.000000] plic: mapped 2 interrupts with 1 handlers for 2 contexts.
	[    0.000000] riscv_timer_init_dt: Registering clocksource cpuid [0] hartid [0]
	[    0.000000] clocksource: riscv_clocksource: mask: 0xffffffffffffffff max_cycles: 0x1ec037a6a, max_idle_ns: 7052723236599 ns
	[    0.000138] sched_clock: 64 bits at 520kHz, resolution 1919ns, wraps every 4398046510738ns
	[    0.009429] printk: console [hvc0] enabled
	[    0.009429] printk: console [hvc0] enabled
	[    0.017850] printk: bootconsole [sbi0] disabled
	[    0.017850] printk: bootconsole [sbi0] disabled
	[    0.028029] Calibrating delay loop (skipped), value calculated using timer frequency.. 1.04 BogoMIPS (lpj=5208)
	[    0.038753] pid_max: default: 32768 minimum: 301
	[    0.050248] Mount-cache hash table entries: 2048 (order: 2, 16384 bytes, linear)
	[    0.058661] Mountpoint-cache hash table entries: 2048 (order: 2, 16384 bytes, linear)
	[    0.069359] *** VALIDATE tmpfs ***
	[    0.089093] *** VALIDATE proc ***
	[    0.101135] *** VALIDATE cgroup ***
	[    0.105019] *** VALIDATE cgroup2 ***
	[    0.144310] rcu: Hierarchical SRCU implementation.
	[    0.162836] smp: Bringing up secondary CPUs ...
	[    0.167736] smp: Brought up 1 node, 1 CPU
	[    0.185982] devtmpfs: initialized
	[    0.216237] random: get_random_u32 called from bucket_table_alloc.isra.25+0x4e/0x15c with crng_init=0
	[    0.236026] clocksource: jiffies: mask: 0xffffffff max_cycles: 0xffffffff, max_idle_ns: 19112604462750000 ns
	[    0.246916] futex hash table entries: 256 (order: 2, 16384 bytes, linear)
	[    0.266994] NET: Registered protocol family 16
	[    0.763362] clocksource: Switched to clocksource riscv_clocksource
	[    0.770122] *** VALIDATE bpf ***
	[    0.782837] *** VALIDATE ramfs ***
	[    0.829997] NET: Registered protocol family 2
	[    0.853577] tcp_listen_portaddr_hash hash table entries: 512 (order: 1, 8192 bytes, linear)
	[    0.864085] TCP established hash table entries: 8192 (order: 4, 65536 bytes, linear)
	[    0.875373] TCP bind hash table entries: 8192 (order: 5, 131072 bytes, linear)
	[    0.887958] TCP: Hash tables configured (established 8192 bind 8192)
	[    0.902149] UDP hash table entries: 512 (order: 2, 16384 bytes, linear)
	[    0.909904] UDP-Lite hash table entries: 512 (order: 2, 16384 bytes, linear)
	[    0.924809] NET: Registered protocol family 1
	[    0.948605] RPC: Registered named UNIX socket transport module.
	[    0.956003] RPC: Registered udp transport module.
	[    0.961565] RPC: Registered tcp transport module.
	[    0.966432] RPC: Registered tcp NFSv4.1 backchannel transport module.
	[    0.987180] Initialise system trusted keyrings
	[    0.998953] workingset: timestamp_bits=46 max_order=18 bucket_order=0
	[    1.323977] *** VALIDATE nfs ***
	[    1.328520] *** VALIDATE nfs4 ***
	[    1.334422] NFS: Registering the id_resolver key type
	[    1.340148] Key type id_resolver registered
	[    1.345280] Key type id_legacy registered
	[    1.349820] nfs4filelayout_init: NFSv4 File Layout Driver Registering...
	[    1.357610] Installing knfsd (copyright (C) 1996 okir@monad.swb.de).
	[    1.866909] Key type asymmetric registered
	[    1.872460] Asymmetric key parser 'x509' registered
	[    1.878750] Block layer SCSI generic (bsg) driver version 0.4 loaded (major 254)
	[    1.887480] io scheduler mq-deadline registered
	[    1.892864] io scheduler kyber registered
	[    3.905595] Serial: 8250/16550 driver, 4 ports, IRQ sharing disabled
	[    3.954332] fff0c2c000.uart: ttyS0 at MMIO 0xfff0c2c000 (irq = 1, base_baud = 4166687) is a 16550
	[    4.254794] loop: module loaded
	[    4.258269] piton_sd:v1.0 Apr 26, 2019
	[    4.258269]
	[    4.265170] gpt partition table header:
	[    4.265283] signature: 5452415020494645
	[    4.269258] revision: 10000
	[    4.273746] size: 5c
	[    4.276659] crc_header: 26b42404
	[    4.278911] reserved: 0
	[    4.282730] current lba: 1
	[    4.285311] backup lda: 3b723ff
	[    4.288093] partition entries lba: 2
	[    4.291835] number partition entries: 80
	[    4.295529] size partition entries: 80
	[    9.473253]  piton_sd: piton_sd1
	[   10.099676] libphy: Fixed MDIO Bus: probed
	[   10.148782] NET: Registered protocol family 10
	[   10.183418] Segment Routing with IPv6
	[   10.189384] sit: IPv6, IPv4 and MPLS over IPv4 tunneling driver
	[   10.214449] NET: Registered protocol family 17
	[   10.227413] Key type dns_resolver registered
	[   10.240561] Loading compiled-in X.509 certificates
	[   10.465264] EXT4-fs (piton_sd1): mounted filesystem with ordered data mode. Opts: (null)
	[   10.475922] VFS: Mounted root (ext4 filesystem) readonly on device 254:1.
	[   10.551865] devtmpfs: mounted
	[   10.562744] Freeing unused kernel memory: 168K
	[   10.567450] This architecture does not have kernel memory protection.
	[   10.574688] Run /sbin/init as init process
	[   10.578916]   with arguments:
	[   10.582489]     /sbin/init
	[   10.585312]   with environment:
	[   10.588518]     HOME=/
	[   10.591459]     TERM=linux
	[   18.154373] systemd[1]: System time before build time, advancing clock.
	[   18.565415] systemd[1]: systemd 238 running in system mode. (+PAM +AUDIT +SELINUX +IMA +APPARMOR +SMACK +SYSVINIT +UTMP +LIB)
	[   18.596359] systemd[1]: Detected architecture riscv64.

	Welcome to Debian GNU/Linux buster/sid!

	[   18.797150] systemd[1]: Set hostname to <openpiton>.
	[   31.609244] random: systemd: uninitialized urandom read (16 bytes read)
	[   31.630366] systemd[1]: Listening on /dev/initctl Compatibility Named Pipe.
	[  OK  ] Listening on /dev/initctl Compatibility Named Pipe.
	[   31.674820] random: systemd: uninitialized urandom read (16 bytes read)
	[   31.806800] systemd[1]: Created slice system-serial\x2dgetty.slice.
	[  OK  ] Created slice system-serial\x2dgetty.slice.
	[   31.839855] random: systemd: uninitialized urandom read (16 bytes read)
	[   31.850670] systemd[1]: Reached target Slices.
	[  OK  ] Reached target Slices.
	[   32.128005] systemd[1]: Reached target Swap.
	[  OK  ] Reached target Swap.
	[   32.180337] systemd[1]: Listening on Journal Socket.
	[  OK  ] Listening on Journal Socket.
	[   32.416448] systemd[1]: Mounting Kernel Debug File System...
	Mounting Kernel Debug File System...
	[   32.937934] systemd[1]: Starting Remount Root and Kernel File Systems...
	Starting Remount Root and Kernel File Systems...
	[   33.117472] urandom_read: 4 callbacks suppressed
	[   33.117645] random: systemd: uninitialized urandom read (16 bytes read)
	[   33.214868] systemd[1]: Started Forward Password Requests to Wall Directory Watch.
	[  OK  ] Started Forward Password Requests to Wall Directory Watch.
	[   33.366745] random: systemd: uninitialized urandom read (16 bytes read)
	[   33.453262] systemd[1]: Listening on Journal Socket (/dev/log).
	[  OK  ] Listening on Journal Socket (/dev/log).
	[   33.627020] random: systemd: uninitialized urandom read (16 bytes read)
	[   34.029973] systemd[1]: Starting Load Kernel Modules...
	Starting Load Kernel Modules...
	[  OK  ] Created slice system-getty.slice.
	[  OK  ] Started Dispatch Password Requests to Console Directory Watch.
	[  OK  ] Reached target Local Encrypted Volumes.
	[  OK  ] Reached target Paths.
	[  OK  ] Reached target Remote File Systems.
	[  OK  ] Listening on udev Kernel Socket.
	[  OK  ] Listening on udev Control Socket.
	[  OK  ] Reached target Sockets.
	Starting udev Coldplug all Devices...
	Starting Journal Service...
	[   37.108761] systemd[1]: Starting Create Static Device Nodes in /dev...
	Starting Create Static Device Nodes in /dev...
	[   37.941929] systemd[1]: Mounted Kernel Debug File System.
	[  OK  ] Mounted Kernel Debug File System.
	[   38.463855] systemd[1]: Started Remount Root and Kernel File Systems.
	[  OK  ] Started Remount Root and Kernel File Systems.
	[   39.614728] systemd[1]: Started Load Kernel Modules.
	[  OK  ] Started Load Kernel Modules.
	[   40.794332] systemd[1]: Starting Apply Kernel Variables...
	Starting Apply Kernel Variables...
	[   41.928338] systemd[1]: Starting Load/Save Random Seed...
	Starting Load/Save Random Seed...
	[   43.494757] systemd[1]: Started Create Static Device Nodes in /dev.
	[  OK  ] Started Create Static Device Nodes in /dev.
	[   44.795372] systemd[1]: Starting udev Kernel Device Manager...
	Starting udev Kernel Device Manager...
	[   45.043065] systemd[1]: Reached target Local File Systems (Pre).
	[  OK  ] Reached target Local File Systems (Pre).
	[   45.224716] systemd[1]: Reached target Local File Systems.
	[  OK  ] Reached target Local File Systems.
	[   46.036491] systemd[1]: Started Apply Kernel Variables.
	[  OK  ] Started Apply Kernel Variables.
	[   46.947879] systemd[1]: Started Load/Save Random Seed.
	[  OK  ] Started Load/Save Random Seed.
	[   47.910242] systemd[1]: Starting Raise network interfaces...
	Starting Raise network interfaces...
	[   48.119915] systemd[1]: Started Journal Service.
	[  OK  ] Started Journal Service.
	Starting Flush Journal to Persistent Storage...
	[  OK  ] Started udev Kernel Device Manager.
	[   55.369915] systemd-journald[88]: Received request to flush runtime journal from PID 1
	[  OK  ] Started Flush Journal to Persistent Storage.
	Starting Create Volatile Files and Directories...
	[  OK  ] Started Raise network interfaces.
	[  OK  ] Reached target Network.
	[FAILED] Failed to start Create Volatile Files and Directories.
	See 'systemctl status systemd-tmpfiles-setup.service' for details.
	Starting Update UTMP about System Boot/Shutdown...
	[FAILED] Failed to start Network Time Synchronization.
	See 'systemctl status systemd-timesyncd.service' for details.
	[  OK  ] Reached target System Time Synchronized.
	[  OK  ] Stopped Network Time Synchronization.
	[  OK  ] Started udev Coldplug all Devices.
	[  OK  ] Found device /dev/hvc0.
	[  OK  ] Reached target System Initialization.
	[  OK  ] Reached target Basic System.
	[  OK  ] Started Regular background program processing daemon.
	[  OK  ] Started Daily Cleanup of Temporary Directories.
	Starting Permit User Sessions...
	[  OK  ] Started Daily apt download activities.
	[  OK  ] Started Daily apt upgrade and clean activities.
	[  OK  ] Reached target Timers.
	[  OK  ] Started Permit User Sessions.
	[  OK  ] Started Serial Getty on hvc0.
	[  OK  ] Reached target Login Prompts.
	[  OK  ] Reached target Multi-User System.
	[  OK  ] Reached target Graphical Interface.

	Debian GNU/Linux buster/sid openpiton hvc0

	openpiton login:
