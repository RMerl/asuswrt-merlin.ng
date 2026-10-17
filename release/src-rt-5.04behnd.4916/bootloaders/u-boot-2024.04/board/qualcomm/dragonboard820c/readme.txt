# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2017 Jorge Ramirez-Ortiz <jorge.ramirez-ortiz@linaro.org>

================================================================================
             What is working (enough to boot a distro from SD card)
================================================================================
   - UART
   - SD card
   - PSCI reset
   - Environment in EXT4 partition 1 in SD card (check defconfig for details)
         dont forget to insert the card in the SD slot before booting if you
         are going to make mods to the environment

================================================================================
                     Build & Run instructions
================================================================================

1) Install mkbootimg and dtbTool from Codeaurora:

   git://codeaurora.org/quic/kernel/skales
   commit 8492547e404e969262d9070dee9bdd15668bb70f worked for me.

2) Setup CROSS_COMPILE to aarch64 compiler or if you use ccache just do
   CROSS_COMPILE="ccache aarch64-linux-gnu-"

3) cd to the u-boot tree

  $ make dragonboard820c_config
  $ make -j `nproc`

4) generate fake, empty ramdisk (can have 0 bytes)

   $ touch rd

5) Generate qualcomm device tree table with dtbTool

   $ dtbTool -o dt.img arch/arm/dts

6) Generate Android boot image with mkbootimg:

   $ mkbootimg --kernel=u-boot-dtb.bin             \
               --output=u-boot.img                 \
               --dt=dt.img                         \
               --pagesize 4096                     \
               --base 0x80000000                   \
               --ramdisk=rd                        \
               --cmdline=""

7) Reboot the board into fastboot mode
   - plug the board micro-usb to your laptop usb host.
   - reboot the board with vol- button pressed

8) Boot the uboot image using fastboot

   $ fastboot boot u-boot.img

   or flash it to the UFS drive boot partition:

   $ fastboot flash boot u-boot.img
   $ fastboot reboot


================================================================================
      To boot a linux kernel from SDHCI with the ROOTFS on an NFS share:
================================================================================

1) create an EXT4 partition on the SD card (must be partition #1)

2) build the kernel image and dtb  (documented extensively somewhere else)

3) copy the drivers to the NFS partition (ie: 192.168.1.2 /exports/db820c-rootfs)

4) add the u-boot headers to the image:

    $ mkimage -A arm64                                     \
              -O linux                                     \
              -C none                                      \
              -T kernel                                    \
              -a 0x80080000                                \
              -e 0x80080000                                \
              -n Dragonboard820c                           \
              -d $kernel/arch/arm64/boot/Image             \
              uImage

5) copy the generated uImage and the device tree binary to the SD card EXT4
   partition

    $ cp uImage /mnt/boot/
    $ cp apq8096-db820c.dtb /mnt/boot/

6) on the SD card create /extlinux/extlinux.conf  as follows:

   default nfs
   prompt 1
   timeout 10

   LABEL nfs
      MENU NFS entry
      LINUX /uImage
      FDT /apq8096-db820c.dtb
      APPEND root=/dev/nfs rw                                         \
             nfsroot=192.168.1.2:/exports/db829c-rootfs,v3,tcp        \
             rootwait                                                 \
             ip=dhcp consoleblank=0                                   \
             console=tty0                                             \
             console=ttyMSM0,115200n8                                 \
             earlyprintk earlycon=msm_serial_dm,0x75b0000             \
             androidboot.bootdevice=624000.ufshc                      \
             androidboot.verifiedbootstate=orange                     \
             androidboot.ver0

7) remove the SD card from the laptop and insert it back to the db820 board.
   the SD card EXT4 partition#1 should contain:
      /uImage
      /apq8096-db820c.dtb
      /extlinux/extlinux.conf

8) reboot the db820 board

================================================================================
                    Successful boot sequence
================================================================================

Format: Log Type - Time(microsec) - Message - Optional Info
Log Type: B - Since Boot(Power On Reset),  D - Delta,  S - Statistic
S - QC_IMAGE_VERSION_STRING=BOOT.XF.1.0-00301
S - IMAGE_VARIANT_STRING=M8996LAB
S - OEM_IMAGE_VERSION_STRING=crm-ubuntu68
S - Boot Interface: UFS
S - Secure Boot: Off
S - Boot Config @ 0x00076044 = 0x000001c9
S - JTAG ID @ 0x000760f4 = 0x4003e0e1
S - OEM ID @ 0x000760f8 = 0x00000000
S - Serial Number @ 0x00074138 = 0x2e8844ce
S - OEM Config Row 0 @ 0x00074188 = 0x0000000000000000
S - OEM Config Row 1 @ 0x00074190 = 0x0000000000000000
S - Feature Config Row 0 @ 0x000741a0 = 0x0050000010000100
S - Feature Config Row 1 @ 0x000741a8 = 0x00fff00001ffffff
S - Core 0 Frequency, 1228 MHz
B -         0 - PBL, Start
B -     10412 - bootable_media_detect_entry, Start
B -     47480 - bootable_media_detect_success, Start
B -     47481 - elf_loader_entry, Start
B -     49027 - auth_hash_seg_entry, Start
B -     49129 - auth_hash_seg_exit, Start
B -     82403 - elf_segs_hash_verify_entry, Start
B -     84905 - PBL, End
B -     86955 - SBL1, Start
B -    182969 - usb: hs_phy_nondrive_start
B -    183305 - usb: PLL lock success - 0x3
B -    186294 - usb: hs_phy_nondrive_finish
B -    190442 - boot_flash_init, Start
D -        30 - boot_flash_init, Delta
B -    197548 - sbl1_ddr_set_default_params, Start
D -        30 - sbl1_ddr_set_default_params, Delta
B -    205509 - boot_config_data_table_init, Start
D -    200659 - boot_config_data_table_init, Delta - (60 Bytes)
B -    410713 - CDT Version:3,Platform ID:24,Major ID:1,Minor ID:0,Subtype:0
B -    415410 - Image Load, Start
D -     22570 - PMIC Image Loaded, Delta - (37272 Bytes)
B -    437980 - pm_device_init, Start
B -    443744 - PON REASON:PM0:0x200000061 PM1:0x200000021
B -    480161 - PM_SET_VAL:Skip
D -     40016 - pm_device_init, Delta
B -    482083 - pm_driver_init, Start
D -      2928 - pm_driver_init, Delta
B -    488671 - pm_sbl_chg_init, Start
D -        91 - pm_sbl_chg_init, Delta
B -    495442 - vsense_init, Start
D -         0 - vsense_init, Delta
B -    505171 - Pre_DDR_clock_init, Start
D -       396 - Pre_DDR_clock_init, Delta
B -    509045 - ddr_initialize_device, Start
B -    512766 - 8996 v3.x detected, Max frequency = 1.8 GHz
B -    522373 - ddr_initialize_device, Delta
B -    522404 - DDR ID, Rank 0, Rank 1, 0x6, 0x300, 0x300
B -    526247 - Basic DDR tests done
B -    594994 - clock_init, Start
D -       274 - clock_init, Delta
B -    598349 - Image Load, Start
D -      4331 - QSEE Dev Config Image Loaded, Delta - (46008 Bytes)
B -    603808 - Image Load, Start
D -      5338 - APDP Image Loaded, Delta - (0 Bytes)
B -    612409 - usb: UFS Serial - 2f490ecf
B -    616801 - usb: fedl, vbus_low
B -    620431 - Image Load, Start
D -     55418 - QSEE Image Loaded, Delta - (1640572 Bytes)
B -    675849 - Image Load, Start
D -      2013 - SEC Image Loaded, Delta - (4096 Bytes)
B -    683413 - sbl1_efs_handle_cookies, Start
D -       457 - sbl1_efs_handle_cookies, Delta
B -    691892 - Image Load, Start
D -     14396 - QHEE Image Loaded, Delta - (254184 Bytes)
B -    706319 - Image Load, Start
D -     14061 - RPM Image Loaded, Delta - (223900 Bytes)
B -    721111 - Image Load, Start
D -      3233 - STI Image Loaded, Delta - (0 Bytes)
B -    727913 - Image Load, Start
D -     34709 - APPSBL Image Loaded, Delta - (748716 Bytes)
B -    762713 - SBL1, End
D -    680028 - SBL1, Delta
S - Flash Throughput, 94000 KB/s  (2959024 Bytes,  31250 us)
S - DDR Frequency, 1017 MHz
Android Bootloader - UART_DM Initialized!!!

[0] BUILD_VERSION=
[0] BUILD_DATE=16:07:51 - Nov 17 2017
[0] welcome to lk
[10] platform_init()
[10] target_init()
[10] RPM GLink Init
[10] Opening RPM Glink Port success
[10] Opening SSR Glink Port success
[20] Glink Connection between APPS and RPM established
[20] Glink Connection between APPS and RPM established
[40] UFS init success
[80] Qseecom Init Done in Appsbl
[80] secure app region addr=0x86600000 size=0x2200000[90] TZ App region notif returned with status:0 addr:86600000 size:35651584
[100] TZ App log region register returned with status:0 addr:916d4000 size:4096
[100] Qseecom TZ Init Done in Appsbl
[120] Loading cmnlib done
[120] qseecom_start_app: Loading app keymaster for the first time
[150] <8>keymaster: "\"KEYMASTER Init \""
[160] Selected panel: none
Skip panel configuration
[160] pm8x41_get_is_cold_boot: cold boot
[170] boot_verifier: Device is in ORANGE boot state.
[180] Device is unlocked! Skipping verification...
[180] Loading (boot) image (348160): start
[190] Loading (boot) image (348160): done
[190] use_signed_kernel=1, is_unlocked=1, is_tampered=0.
[200] Your device has been unlocked and cant be trusted.
Wait for 5 seconds before proceeding

[5200] mdtp: mdtp_img loaded
[5210] mdtp: is_test_mode: test mode is set to 1
[5210] mdtp: read_metadata: SUCCESS
[5230] LK SEC APP Handle: 0x1
[5230] Return value from recv_data: 14
[5240] Return value from recv_data: 14
[5250] Return value from recv_data: 14
[5260] DTB Total entry: 1, DTB version: 3
[5260] Using DTB entry 0x00000123/00000000/0x00000018/0 for device 0x00000123/00030001/0x00010018/0
[5270] cmdline:  androidboot.bootdevice=624000.ufshc androidboot.verifiedbootstate=orange androidboot.veritymode=enforcing androidboot.serialno=2f490ecf androidboot.baseband=apq mdss_mdp.panel=0
[5290] Updating device tree: start
[5290] Updating device tree: done
[5290] Return value from recv_data: 14
[5300] RPM GLINK UnInit
[5300] Qseecom De-Init Done in Appsbl
[5300] booting linux @ 0x80080000, ramdisk @ 0x82200000 (0), tags/device tree @ 0x82000000
[5310] Jumping to kernel via monitor

U-Boot 2017.11-00145-ge895117 (Nov 29 2017 - 10:04:06 +0100)
Qualcomm-DragonBoard 820C

DRAM:  3 GiB
PSCI:  v1.0
MMC:   sdhci@74a4900: 0
In:    serial@75b0000
Out:   serial@75b0000
Err:   serial@75b0000
Net:   Net Initialization Skipped
No ethernet found.
Hit any key to stop autoboot:  0
switch to partitions #0, OK
mmc0 is current device
Scanning mmc 0:1...
Found /extlinux/extlinux.conf
Retrieving file: /extlinux/extlinux.conf
433 bytes read in 71 ms (5.9 KiB/s)
1:      nfs root

Retrieving file: /uImage
19397184 bytes read in 2024 ms (9.1 MiB/s)
append: root=/dev/nfs rw nfsroot=192.168.1.2:/db820c/rootfs,v3,tcp rootwait ip=dhcp consoleblank=0 console=tty0 console=ttyMSM0,115200n8 earlyprintk earlycon=msm_serial_dm,0x75b0000 androidboot.bootdevice=624000.ufshc androidboot.verifiedbootstate=orange androidboot.ver0

Retrieving file: /apq8096-db820c.dtb
38134 bytes read in 37 ms (1005.9 KiB/s)

## Booting kernel from Legacy Image at 95000000 ...
   Image Name:   Dragonboard820c
   Image Type:   AArch64 Linux Kernel Image (uncompressed)
   Data Size:    19397120 Bytes = 18.5 MiB
   Load Address: 80080000
   Entry Point:  80080000
   Verifying Checksum ... OK
## Flattened Device Tree blob at 93000000
   Booting using the fdt blob at 0x93000000
   Loading Kernel Image ... OK
   Using Device Tree in place at 0000000093000000, end 000000009300c4f5

Starting kernel ...

[    0.000000] Booting Linux on physical CPU 0x0
[    0.000000] Linux version 4.11.3-30039-g5a922a1 (jramirez@igloo) (gcc version 6.3.1 20170404 (Linaro GCC 6.3-2017.05) ) #1 SMP PREEMPT Wed Oct 18 10:21:11 CEST 2017
[    0.000000] Boot CPU: AArch64 Processor [511f2112]
[    0.000000] earlycon: msm_serial_dm0 at MMIO 0x00000000075b0000 (options '')
[    0.000000] bootconsole [msm_serial_dm0] enabled
[    0.000000] efi: Getting EFI parameters from FDT:
[    0.000000] efi: UEFI not found.
[    0.000000] OF: reserved mem: OVERLAP DETECTED!
[    0.000000] adsp@8ea00000 (0x000000008ea00000--0x0000000090400000) overlaps with gpu@8f200000 (0x000000008f200000--0x000000008f300000)
[    0.000000] Reserved memory: created DMA memory pool at 0x000000008f200000, size 1 MiB
[    0.000000] OF: reserved mem: initialized node gpu@8f200000, compatible id shared-dma-pool
[    0.000000] Reserved memory: created DMA memory pool at 0x0000000090400000, size 8 MiB
[    0.000000] OF: reserved mem: initialized node venus@90400000, compatible id shared-dma-pool
[    0.000000] cma: Reserved 128 MiB at 0x00000000b8000000
[    0.000000] NUMA: No NUMA configuration found
[    0.000000] NUMA: Faking a node at [mem 0x0000000000000000-0x00000000bfffffff]
[    0.000000] NUMA: Adding memblock [0x80000000 - 0x857fffff] on node 0
[    0.000000] NUMA: Adding memblock [0x91800000 - 0xbfffffff] on node 0
[    0.000000] NUMA: Initmem setup node 0 [mem 0x80000000-0xbfffffff]
[    0.000000] NUMA: NODE_DATA [mem 0xb7fb6680-0xb7fb817f]
[    0.000000] Zone ranges:
[    0.000000]   DMA      [mem 0x0000000080000000-0x00000000bfffffff]
[    0.000000]   Normal   empty
[    0.000000] Movable zone start for each node
[    0.000000] Early memory node ranges
[    0.000000]   node   0: [mem 0x0000000080000000-0x00000000857fffff]
[    0.000000]   node   0: [mem 0x0000000091800000-0x00000000bfffffff]
[    0.000000] Initmem setup node 0 [mem 0x0000000080000000-0x00000000bfffffff]
[    0.000000] psci: probing for conduit method from DT.
[    0.000000] psci: PSCIv1.0 detected in firmware.
[    0.000000] psci: Using standard PSCI v0.2 function IDs
[    0.000000] psci: MIGRATE_INFO_TYPE not supported.
[    0.000000] percpu: Embedded 23 pages/cpu @ffff8000de9a3000 s57240 r8192 d28776 u94208
[    0.000000] pcpu-alloc: s57240 r8192 d28776 u94208 alloc=23*4096
[    0.000000] pcpu-alloc: [0] 0 [0] 1 [0] 2 [0] 3
[    0.000000] Detected PIPT I-cache on CPU0
[    0.000000] Built 1 zonelists in Node order, mobility grouping on.  Total pages: 720293
[    0.000000] Policy zone: Normal
[    0.000000] Kernel command line: root=/dev/nfs rw nfsroot=192.168.1.2:/db820c/rootfs,v3,tcp rootwait ip=dhcp consoleblank=0
console=tty0 console=ttyMSM0,115200n8 earlyprintk earlycon=msm_serial_dm,0x75b0000 androidboot.bootdevice=624000.ufshc androidboot.verifiedbootstate=orange a
ndroidboot.ver0
[    0.000000] PID hash table entries: 4096 (order: 3, 32768 bytes)
[    0.000000] software IO TLB [mem 0xd3fff000-0xd7fff000] (64MB) mapped at [ffff800053fff000-ffff800057ffefff]
[    0.000000] Memory: 2644172K/2926908K available (11196K kernel code, 1470K rwdata, 5132K rodata, 1088K init, 449K bss, 151664K reserved, 131072K cma-reser
ved)
[    0.000000] Virtual kernel memory layout:
[    0.000000]     modules : 0xffff000000000000 - 0xffff000008000000   (   128 MB)
[    0.000000]     vmalloc : 0xffff000008000000 - 0xffff7dffbfff0000   (129022 GB)
[    0.000000]       .text : 0xffff000008080000 - 0xffff000008b70000   ( 11200 KB)
[    0.000000]     .rodata : 0xffff000008b70000 - 0xffff000009080000   (  5184 KB)
[    0.000000]       .init : 0xffff000009080000 - 0xffff000009190000   (  1088 KB)
[    0.000000]       .data : 0xffff000009190000 - 0xffff0000092ffa00   (  1471 KB)
[    0.000000]        .bss : 0xffff0000092ffa00 - 0xffff00000937014c   (   450 KB)
[    0.000000]     fixed   : 0xffff7dfffe7fd000 - 0xffff7dfffec00000   (  4108 KB)
[    0.000000]     PCI I/O : 0xffff7dfffee00000 - 0xffff7dffffe00000   (    16 MB)
[    0.000000]     vmemmap : 0xffff7e0000000000 - 0xffff800000000000   (  2048 GB maximum)
[    0.000000]               0xffff7e0000000000 - 0xffff7e00037a93c0   (    55 MB actual)
[    0.000000]     memory  : 0xffff800000000000 - 0xffff8000dea4f000   (  3562 MB)
[    0.000000] SLUB: HWalign=128, Order=0-3, MinObjects=0, CPUs=4, Nodes=1
[    0.000000] Preemptible hierarchical RCU implementation.
[    0.000000]  Build-time adjustment of leaf fanout to 64.
[    0.000000]  RCU restricting CPUs from NR_CPUS=64 to nr_cpu_ids=4.
[    0.000000] RCU: Adjusting geometry for rcu_fanout_leaf=64, nr_cpu_ids=4
[    0.000000] NR_IRQS:64 nr_irqs:64 0
[    0.000000] GICv3: CPU0: found redistributor 0 region 0:0x0000000009c00000
[    0.000000] GICv2m: range[mem 0x09bd0000-0x09bd0fff], SPI[544:639]
[    0.000000] arm_arch_timer: Architected cp15 and mmio timer(s) running at 19.20MHz (virt/virt).
[    0.000000] clocksource: arch_sys_counter: mask: 0xffffffffffffff max_cycles: 0x46d987e47, max_idle_ns: 440795202767 ns
[    0.000002] sched_clock: 56 bits at 19MHz, resolution 52ns, wraps every 4398046511078ns

[....]


Some kernel information:

root@linaro-developer:~# cat /proc/cpuinfo
processor       : 0
BogoMIPS        : 38.40
Features        : fp asimd evtstrm aes pmull sha1 sha2 crc32 cpuid
CPU implementer : 0x51
CPU architecture: 8
CPU variant     : 0x1
CPU part        : 0x211
CPU revision    : 2

processor       : 1
BogoMIPS        : 38.40
Features        : fp asimd evtstrm aes pmull sha1 sha2 crc32 cpuid
CPU implementer : 0x51
CPU architecture: 8
CPU variant     : 0x1
CPU part        : 0x211
CPU revision    : 2

processor       : 2
BogoMIPS        : 38.40
Features        : fp asimd evtstrm aes pmull sha1 sha2 crc32 cpuid
CPU implementer : 0x51
CPU architecture: 8
CPU variant     : 0x1
CPU part        : 0x205
CPU revision    : 2

processor       : 3
BogoMIPS        : 38.40
Features        : fp asimd evtstrm aes pmull sha1 sha2 crc32 cpuid
CPU implementer : 0x51
CPU architecture: 8
CPU variant     : 0x1
CPU part        : 0x205
CPU revision    : 2

root@linaro-developer:~# uname -a
Linux linaro-developer 4.11.3-30039-g5a922a1 #1 SMP PREEMPT Wed Oct 18 10:21:11 CEST 2017 aarch64 GNU/Linux

root@linaro-developer:~# cat /proc/cmdline
root=/dev/nfs rw nfsroot=192.168.1.2:/db820c/rootfs,v3,tcp rootwait ip=dhcp consoleblank=0 console=tty0 console=ttyMSM0,115200n8 earlyprintk earlycon=msm_serial_dm,0x75b0000 androidboot.bootdevice=624000.ufshc androidboot.verifiedbootstate=orange androidboot.ver0

root@linaro-developer:~# cat /proc/meminfo
MemTotal:        2776332 kB
MemFree:         2593696 kB
MemAvailable:    2561432 kB
Buffers:               0 kB
Cached:            94744 kB
SwapCached:            0 kB
Active:            43888 kB
Inactive:          72972 kB
Active(anon):      22968 kB
Inactive(anon):    24616 kB
Active(file):      20920 kB
Inactive(file):    48356 kB
Unevictable:           0 kB
Mlocked:               0 kB
SwapTotal:             0 kB
SwapFree:              0 kB
Dirty:                 0 kB
Writeback:             0 kB
AnonPages:         22120 kB
Mapped:            29284 kB
Shmem:             25468 kB
Slab:              32876 kB
SReclaimable:      12924 kB
SUnreclaim:        19952 kB
KernelStack:        2144 kB
PageTables:          928 kB
NFS_Unstable:          0 kB
Bounce:                0 kB
WritebackTmp:          0 kB
CommitLimit:     1388164 kB
Committed_AS:     204192 kB
VmallocTotal:   135290290112 kB
VmallocUsed:           0 kB
VmallocChunk:          0 kB
AnonHugePages:      2048 kB
ShmemHugePages:        0 kB
ShmemPmdMapped:        0 kB
CmaTotal:         131072 kB
CmaFree:          130356 kB
HugePages_Total:       0
HugePages_Free:        0
HugePages_Rsvd:        0
HugePages_Surp:        0
Hugepagesize:       2048 kB
