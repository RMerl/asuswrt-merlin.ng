.. SPDX-License-Identifier: GPL-2.0+:

.. index::
   single: bootflow (command)

bootflow command
================

Synopsis
--------

::

    bootflow scan [-abelGH] [bootdev]
    bootflow list [-e]
    bootflow select [<num|name>]
    bootflow info [-ds]
    bootflow read
    bootflow boot
    bootflow cmdline [set|get|clear|delete|auto] <param> [<value>]
    bootfloe menu [-t]

Description
-----------

The `bootflow` command is used to manage bootflows. It can scan bootdevs to
locate bootflows, list them and boot them.

See :doc:`../../develop/bootstd` for more information.

Note that `CONFIG_BOOTSTD_FULL` (which enables `CONFIG_CMD_BOOTFLOW_FULL) must
be enabled to obtain full functionality with this command. Otherwise, it only
supports `bootflow scan` which scans and boots the first available bootflow.

bootflow scan
~~~~~~~~~~~~~

Scans for available bootflows, optionally booting the first valid one it finds.
This operates in two modes:

- If no bootdev is selected (see `bootdev select`) it scans bootflows one
  by one, extracting all the bootdevs from each
- If a bootdev is selected, it just scans that one bootflow

Flags are:

-a
    Collect all bootflows, even those that cannot be loaded. Normally if a file
    is not where it is expected, then the bootflow fails and so is dropped
    during the scan. With this option you can see why each bootflow would be
    dropped.

-b
    Boot each valid bootflow as it is scanned. Typically only the first bootflow
    matters, since by then the system boots in the OS and U-Boot is no-longer
    running. `bootflow scan -b` is a quick way to boot the first available OS.
    A valid bootflow is one that made it all the way to the `loaded` state.
    Note that if `-m` is provided as well, booting is delayed until the user
    selects a bootflow.

-e
    Used with -l to also show errors for each bootflow. The shows detailed error
    information for each bootflow that failed to make it to the `loaded` state.

-l
    List bootflows while scanning. This is helpful when you want to see what
    is happening during scanning. Use it with the `-b` flag to see which
    bootdev and bootflows are being tried.

-G
    Skip global bootmeths when scanning. By default these are tried first, but
    this flag disables them.

-H
    Don't use bootdev hunters. By default these are used before each boot
    priority or label is tried, to see if more bootdevs can be discovered, but
    this flag disables that process.

-m
    Show a menu of available bootflows for the user to select. When used with
    -b it then boots the one that was selected, if any.

The optional argument specifies a particular bootdev to scan. This can either be
the name of a bootdev or its sequence number (both shown with `bootdev list`).
Alternatively a convenience label can be used, like `mmc0`, which is the type of
device and an optional sequence number. Specifically, the label is the uclass of
the bootdev's parent followed by the sequence number of that parent. Sequence
numbers are typically set by aliases, so if you have 'mmc0' in your devicetree
alias section, then `mmc0` refers to the bootdev attached to that device.


bootflow list
~~~~~~~~~~~~~

Lists the previously scanned bootflows. You must use `bootflow scan` before this
to see anything.

If you scanned with -a and have bootflows with errors, -e can be used to show
those errors.

The list looks something like this:

===  ======  ======  ========  ====  ===============================   ================
Seq  Method  State   Uclass    Part  Name                              Filename
===  ======  ======  ========  ====  ===============================   ================
  0  distro  ready   mmc          2  mmc\@7e202000.bootdev.part_2      /boot/extlinux/extlinux.conf
  1  pxe     ready   ethernet     0  smsc95xx_eth.bootdev.0            rpi.pxe/extlinux/extlinux.conf
===  ======  ======  ========  ====  ===============================   ================

The fields are as follows:

Seq:
    Sequence number in the scan, used to reference the bootflow later

Method:
    The boot method (bootmeth) used to find the bootflow. Several methods are
    included in U-Boot.

State:
    Current state of the bootflow, indicating how far the bootdev got in
    obtaining a valid one. See :ref:`BootflowStates` for a list of states.

Uclass:
    Name of the media device's Uclass. This indicates the type of the parent
    device (e.g. MMC, Ethernet).

Part:
    Partition number being accesseed, numbered from 1. Normally a device will
    have a partition table with a small number of partitions. For devices
    without partition tables (e.g. network) this field is 0.

Name:
    Name of the bootflow. This is generated from the bootdev appended with
    the partition information

Filename:
    Name of the bootflow file. This indicates where the file is on the
    filesystem or network device.


bootflow select
~~~~~~~~~~~~~~~

Use this to select a particular bootflow. You can select it by the sequence
number or name, as shown in `bootflow list`.

Once a bootflow is selected, you can use `bootflow info` and `bootflow boot`.

If no bootflow name or number is provided, then any existing bootflow is
unselected.


bootflow info
~~~~~~~~~~~~~

This shows information on the current bootflow, with the format looking like
this:

=========  ===============================
Name       mmc\@7e202000.bootdev.part_2
Device     mmc\@7e202000.bootdev
Block dev  mmc\@7e202000.blk
Type       distro
Method:    extlinux
State      ready
Partition  2
Subdir     (none)
Filename   /extlinux/extlinux.conf
Buffer     3db7ad48
Size       232 (562 bytes)
FDT:       <NULL>
Error      0
=========  ===============================

Most of the information is the same as `bootflow list` above. The new fields
are:

Device
    Name of the bootdev

Block dev
    Name of the block device, if any. Network devices don't have a block device.

Subdir
    Subdirectory used for retrieving files. For network bootdevs this is the
    directory of the 'bootfile' parameter passed from DHCP. All file retrievals
    when booting are relative to this.

Buffer
    Buffer containing the bootflow file. You can use the :doc:`md` to look at
    it, or dump it with `bootflow info -d`.

Size
    Size of the bootflow file

FDT:
    Filename of the device tree, if supported. The EFI bootmeth uses this to
    remember the filename to load. If `<NULL>` then there is none.

Error
    Error number returned from scanning for the bootflow. This is 0 if the
    bootflow is in the 'loaded' state, or a negative error value on error. You
    can look up Linux error codes to find the meaning of the number.

Use the `-d` flag to dump out the contents of the bootfile file.

The `-s` flag shows any x86 setup block, instead of the above.


bootflow read
~~~~~~~~~~~~~

This reads any files related to the bootflow. Some bootflows with large files
avoid doing this when the bootflow is scanned, since it uses a lot of memory
and takes extra time. The files are then automatically read when `bootflow boot`
is used.

This command reads these files immediately. Typically this fills in the bootflow
`buf` property, which can be used to examine the bootflow.

Note that reading the files does not result in any extra parsing, nor loading of
images in the files. This is purely used to read in the data ready for
booting, or examination.


bootflow boot
~~~~~~~~~~~~~

This boots the current bootflow, reading any required files first.


bootflow cmdline
~~~~~~~~~~~~~~~~

Some bootmeths can obtain the OS command line since it is stored with the OS.
In that case, you can use `bootflow cmdline` to adjust this. The command line
is assumed to be in the format used by Linux, i.e. a space-separated set of
parameters with optional values, e.g. "noinitrd console=/dev/tty0".

To change or add a parameter, use::

    bootflow cmdline set <param> <value>

To clear a parameter value to empty you can use "" for the value, or use::

    bootflow cmdline clear <param>

To delete a parameter entirely, use::

    bootflow cmdline delete <param>

Automatic parameters are available in a very few cases. You can use these to
add parmeters where the value is known by U-Boot. For example::

    bootflow cmdline auto earlycon
    bootflow cmdline auto console

can be used to set the early console (or console) to a suitable value so that
output appears on the serial port. This is only supported by the 16550 serial
driver so far.

bootflow menu
~~~~~~~~~~~~~

This shows a menu with available bootflows. The user can select a particular
bootflow, which then becomes the current one.

The `-t` flag requests a text menu. Otherwise, if a display is available, a
graphical menu is shown.


Example
-------

Here is an example of scanning for bootflows, then listing them::

    U-Boot> bootflow scan -l
    Scanning for bootflows in all bootdevs
    Seq  Type         State   Uclass    Part  Name                      Filename
    ---  -----------  ------  --------  ----  ------------------------  ----------------
    Scanning bootdev 'mmc@7e202000.bootdev':
      0  distro       ready   mmc          2  mmc@7e202000.bootdev.p    /extlinux/extlinux.conf
    Scanning bootdev 'sdhci@7e300000.bootdev':
    Card did not respond to voltage select! : -110
    Scanning bootdev 'smsc95xx_eth.bootdev':
    Waiting for Ethernet connection... done.
    BOOTP broadcast 1
    DHCP client bound to address 192.168.4.30 (4 ms)
    Using smsc95xx_eth device
    TFTP from server 192.168.4.1; our IP address is 192.168.4.30
    Filename 'rpi.pxe/'.
    Load address: 0x200000
    Loading: *
    TFTP error: 'Is a directory' (0)
    Starting again

    missing environment variable: pxeuuid
    Retrieving file: rpi.pxe/pxelinux.cfg/01-b8-27-eb-a6-61-e1
    Waiting for Ethernet connection... done.
    Using smsc95xx_eth device
    TFTP from server 192.168.4.1; our IP address is 192.168.4.30
    Filename 'rpi.pxe/pxelinux.cfg/01-b8-27-eb-a6-61-e1'.
    Load address: 0x2500000
    Loading: ##################################################  566 Bytes
    	 45.9 KiB/s
    done
    Bytes transferred = 566 (236 hex)
      1  distro       ready   ethernet     0  smsc95xx_eth.bootdev.0 rpi.pxe/extlinux/extlinux.conf
    No more bootdevs
    ---  -----------  ------  --------  ----  ------------------------  ----------------
    (2 bootflows, 2 valid)
    U-Boot> bootflow l
    Showing all bootflows
    Seq  Type         State   Uclass    Part  Name                      Filename
    ---  -----------  ------  --------  ----  ------------------------  ----------------
      0  distro       ready   mmc          2  mmc@7e202000.bootdev.p    /extlinux/extlinux.conf
      1  pxe          ready   ethernet     0  smsc95xx_eth.bootdev.0     rpi.pxe/extlinux/extlinux.conf
    ---  -----------  ------  --------  ----  ------------------------  ----------------
    (2 bootflows, 2 valid)


The second one is then selected by name (we could instead use `bootflow sel 0`),
displayed and booted::

    U-Boot> bootflow info
    No bootflow selected
    U-Boot> bootflow sel mmc@7e202000.bootdev.part_2
    U-Boot> bootflow info
    Name:      mmc@7e202000.bootdev.part_2
    Device:    mmc@7e202000.bootdev
    Block dev: mmc@7e202000.blk
    Method:    distro
    State:     ready
    Partition: 2
    Subdir:    (none)
    Filename:  extlinux/extlinux.conf
    Buffer:    3db7ae88
    Size:      232 (562 bytes)
    OS:        Fedora-Workstation-armhfp-31-1.9 (5.3.7-301.fc31.armv7hl)
    Cmdline:   (none)
    Logo:      (none)
    FDT:       <NULL>
    Error:     0
    U-Boot> bootflow boot
    ** Booting bootflow 'smsc95xx_eth.bootdev.0'
    Ignoring unknown command: ui
    Ignoring malformed menu command:  autoboot
    Ignoring malformed menu command:  hidden
    Ignoring unknown command: totaltimeout
    1:	Fedora-Workstation-armhfp-31-1.9 (5.3.7-301.fc31.armv7hl)
    Retrieving file: rpi.pxe/initramfs-5.3.7-301.fc31.armv7hl.img
    get 2700000 rpi.pxe/initramfs-5.3.7-301.fc31.armv7hl.img
    Waiting for Ethernet connection... done.
    Using smsc95xx_eth device
    TFTP from server 192.168.4.1; our IP address is 192.168.4.30
    Filename 'rpi.pxe/initramfs-5.3.7-301.fc31.armv7hl.img'.
    Load address: 0x2700000
    Loading: ###################################T ###############  57.7 MiB
    	 1.9 MiB/s
    done
    Bytes transferred = 60498594 (39b22a2 hex)
    Retrieving file: rpi.pxe//vmlinuz-5.3.7-301.fc31.armv7hl
    get 80000 rpi.pxe//vmlinuz-5.3.7-301.fc31.armv7hl
    Waiting for Ethernet connection... done.
    Using smsc95xx_eth device
    TFTP from server 192.168.4.1; our IP address is 192.168.4.30
    Filename 'rpi.pxe//vmlinuz-5.3.7-301.fc31.armv7hl'.
    Load address: 0x80000
    Loading: ##################################################  7.2 MiB
    	 2.3 MiB/s
    done
    Bytes transferred = 7508480 (729200 hex)
    append: ro root=UUID=9732b35b-4cd5-458b-9b91-80f7047e0b8a rhgb quiet LANG=en_US.UTF-8 cma=192MB cma=256MB
    Retrieving file: rpi.pxe//dtb-5.3.7-301.fc31.armv7hl/bcm2837-rpi-3-b.dtb
    get 2600000 rpi.pxe//dtb-5.3.7-301.fc31.armv7hl/bcm2837-rpi-3-b.dtb
    Waiting for Ethernet connection... done.
    Using smsc95xx_eth device
    TFTP from server 192.168.4.1; our IP address is 192.168.4.30
    Filename 'rpi.pxe//dtb-5.3.7-301.fc31.armv7hl/bcm2837-rpi-3-b.dtb'.
    Load address: 0x2600000
    Loading: ##################################################  13.8 KiB
    	 764.6 KiB/s
    done
    Bytes transferred = 14102 (3716 hex)
    Kernel image @ 0x080000 [ 0x000000 - 0x729200 ]
    ## Flattened Device Tree blob at 02600000
       Booting using the fdt blob at 0x2600000
       Using Device Tree in place at 02600000, end 02606715

    Starting kernel ...

    [  OK  ] Started Show Plymouth Boot Screen.
    [  OK  ] Started Forward Password R…s to Plymouth Directory Watch.
    [  OK  ] Reached target Local Encrypted Volumes.
    [  OK  ] Reached target Paths.
    ....


Here we scan for bootflows and boot the first one found::

    U-Boot> bootflow scan -bl
    Scanning for bootflows in all bootdevs
    Seq  Method       State   Uclass    Part  Name                    Filename
    ---  -----------  ------  --------  ----  ----------------------  ----------------
    Scanning bootdev 'mmc@7e202000.bootdev':
      0  distro       ready   mmc          2  mmc@7e202000.bootdev.p  /extlinux/extlinux.conf
    ** Booting bootflow 'mmc@7e202000.bootdev.part_2'
    Ignoring unknown command: ui
    Ignoring malformed menu command:  autoboot
    Ignoring malformed menu command:  hidden
    Ignoring unknown command: totaltimeout
    1:	Fedora-KDE-armhfp-31-1.9 (5.3.7-301.fc31.armv7hl)
    Retrieving file: /initramfs-5.3.7-301.fc31.armv7hl.img
    getfile 2700000 /initramfs-5.3.7-301.fc31.armv7hl.img
    Retrieving file: /vmlinuz-5.3.7-301.fc31.armv7hl
    getfile 80000 /vmlinuz-5.3.7-301.fc31.armv7hl
    append: ro root=UUID=b8781f09-e2dd-4cb8-979b-7df5eeaaabea rhgb LANG=en_US.UTF-8 cma=192MB console=tty0 console=ttyS1,115200
    Retrieving file: /dtb-5.3.7-301.fc31.armv7hl/bcm2837-rpi-3-b.dtb
    getfile 2600000 /dtb-5.3.7-301.fc31.armv7hl/bcm2837-rpi-3-b.dtb
    Kernel image @ 0x080000 [ 0x000000 - 0x729200 ]
    ## Flattened Device Tree blob at 02600000
       Booting using the fdt blob at 0x2600000
       Using Device Tree in place at 02600000, end 02606715

    Starting kernel ...

    [    0.000000] Booting Linux on physical CPU 0x0


Here is am example using the -e flag to see all errors::

    U-Boot> bootflow scan -a
    Card did not respond to voltage select! : -110
    Waiting for Ethernet connection... done.
    BOOTP broadcast 1
    DHCP client bound to address 192.168.4.30 (4 ms)
    Using smsc95xx_eth device
    TFTP from server 192.168.4.1; our IP address is 192.168.4.30
    Filename 'rpi.pxe/'.
    Load address: 0x200000
    Loading: *
    TFTP error: 'Is a directory' (0)
    Starting again

    missing environment variable: pxeuuid
    Retrieving file: rpi.pxe/pxelinux.cfg/01-b8-27-eb-a6-61-e1
    Waiting for Ethernet connection... done.
    Using smsc95xx_eth device
    TFTP from server 192.168.4.1; our IP address is 192.168.4.30
    Filename 'rpi.pxe/pxelinux.cfg/01-b8-27-eb-a6-61-e1'.
    Load address: 0x2500000
    Loading: ##################################################  566 Bytes
    	 49.8 KiB/s
    done
    Bytes transferred = 566 (236 hex)
    U-Boot> bootflow l -e
    Showing all bootflows
    Seq  Type         State   Uclass    Part  Name                   Filename
    ---  -----------  ------  --------  ----  ---------------------  ----------------
      0  distro       fs      mmc          1  mmc@7e202000.bootdev.p /extlinux/extlinux.conf
         ** File not found, err=-2
      1  distro       ready   mmc          2  mmc@7e202000.bootdev.p /extlinux/extlinux.conf
      2  distro       fs      mmc          3  mmc@7e202000.bootdev.p /extlinux/extlinux.conf
         ** File not found, err=-1
      3  distro       media   mmc          0  mmc@7e202000.bootdev.p <NULL>
         ** No partition found, err=-2
      4  distro       media   mmc          0  mmc@7e202000.bootdev.p <NULL>
         ** No partition found, err=-2
      5  distro       media   mmc          0  mmc@7e202000.bootdev.p <NULL>
         ** No partition found, err=-2
      6  distro       media   mmc          0  mmc@7e202000.bootdev.p <NULL>
         ** No partition found, err=-2
      7  distro       media   mmc          0  mmc@7e202000.bootdev.p <NULL>
         ** No partition found, err=-2
      8  distro       media   mmc          0  mmc@7e202000.bootdev.p <NULL>
         ** No partition found, err=-2
      9  distro       media   mmc          0  mmc@7e202000.bootdev.p <NULL>
         ** No partition found, err=-2
      a  distro       media   mmc          0  mmc@7e202000.bootdev.p <NULL>
         ** No partition found, err=-2
      b  distro       media   mmc          0  mmc@7e202000.bootdev.p <NULL>
         ** No partition found, err=-2
      c  distro       media   mmc          0  mmc@7e202000.bootdev.p <NULL>
         ** No partition found, err=-2
      d  distro       media   mmc          0  mmc@7e202000.bootdev.p <NULL>
         ** No partition found, err=-2
      e  distro       media   mmc          0  mmc@7e202000.bootdev.p <NULL>
         ** No partition found, err=-2
      f  distro       media   mmc          0  mmc@7e202000.bootdev.p <NULL>
         ** No partition found, err=-2
     10  distro       media   mmc          0  mmc@7e202000.bootdev.p <NULL>
         ** No partition found, err=-2
     11  distro       media   mmc          0  mmc@7e202000.bootdev.p <NULL>
         ** No partition found, err=-2
     12  distro       media   mmc          0  mmc@7e202000.bootdev.p <NULL>
         ** No partition found, err=-2
     13  distro       media   mmc          0  mmc@7e202000.bootdev.p <NULL>
         ** No partition found, err=-2
     14  distro       ready   ethernet     0  smsc95xx_eth.bootdev.0 rpi.pxe/extlinux/extlinux.conf
    ---  -----------  ------  --------  ----  ---------------------  ----------------
    (21 bootflows, 2 valid)
    U-Boot>

Here is an example of booting ChromeOS, adjusting the console beforehand. Note that
the cmdline is word-wrapped here and some parts of the command line are elided::

    => bootfl list
    Showing all bootflows
    Seq  Method       State   Uclass    Part  Name                      Filename
    ---  -----------  ------  --------  ----  ------------------------  ----------------
    0  cros         ready   nvme         0  5.10.153-20434-g98da1eb2c <NULL>
    1  efi          ready   nvme         c  nvme#0.blk#1.bootdev.part efi/boot/bootia32.efi
    2  efi          ready   usb_mass_    2  usb_mass_storage.lun0.boo efi/boot/bootia32.efi
    ---  -----------  ------  --------  ----  ------------------------  ----------------
    (3 bootflows, 3 valid)
    => bootfl sel 0
    => bootfl inf
    Name:      5.10.153-20434-g98da1eb2cf9d (chrome-bot@chromeos-release-builder-us-central1-b-x32-12-xijx) #1 SMP PREEMPT Tue Jan 24 19:38:23 PST 2023
    Device:    nvme#0.blk#1.bootdev
    Block dev: nvme#0.blk#1
    Method:    cros
    State:     ready
    Partition: 0
    Subdir:    (none)
    Filename:  <NULL>
    Buffer:    737a1400
    Size:      c47000 (12873728 bytes)
    OS:        ChromeOS
    Cmdline:   console= loglevel=7 init=/sbin/init cros_secure drm.trace=0x106
        root=/dev/dm-0 rootwait ro dm_verity.error_behavior=3
        dm_verity.max_bios=-1 dm_verity.dev_wait=1
        dm="1 vroot none ro 1,0 6348800
          verity payload=PARTUUID=799c935b-ae62-d143-8493-816fa936eef7/PARTNROFF=1
          hashtree=PARTUUID=799c935b-ae62-d143-8493-816fa936eef7/PARTNROFF=1
          hashstart=6348800 alg=sha256
          root_hexdigest=78cc462cd45aecbcd49ca476587b4dee59aa1b00ba5ece58e2c29ec9acd914ab
          salt=8dec4dc80a75dd834a9b3175c674405e15b16a253fdfe05c79394ae5fd76f66a"
        noinitrd vt.global_cursor_default=0
        kern_guid=799c935b-ae62-d143-8493-816fa936eef7 add_efi_memmap
        noresume i915.modeset=1 ramoops.ecc=1 tpm_tis.force=0
        intel_pmc_core.warn_on_s0ix_failures=1 i915.enable_guc=3 i915.enable_dc=4
        xdomain=0 swiotlb=65536 intel_iommu=on i915.enable_psr=1
        usb-storage.quirks=13fe:6500:u
    X86 setup: 742e3400
    Logo:      (none)
    FDT:       <NULL>
    Error:     0
    => bootflow cmdline auto earlycon
    => bootflow cmd auto console
    => print bootargs
    bootargs=console=ttyS0,115200n8 loglevel=7 ...
        usb-storage.quirks=13fe:6500:u earlycon=uart8250,mmio32,0xfe03e000,115200n8
    => bootflow cmd del console
    => print bootargs
    bootargs=loglevel=7 ... earlycon=uart8250,mmio32,0xfe03e000,115200n8
    => bootfl boot
    ** Booting bootflow '5.10.153-20434-g98da1eb2cf9d (chrome-bot@chromeos-release-builder-us-central1-b-x32-12-xijx) #1 SMP PREEMPT Tue Jan 24 19:38:23 PST 2023' with cros
    Kernel command line: "loglevel=7 ... earlycon=uart8250,mmio32,0xfe03e000,115200n8"

    Starting kernel ...

    [    0.000000] Linux version 5.10.153-20434-g98da1eb2cf9d (chrome-bot@chromeos-release-builder-us-central1-b-x32-12-xijx) (Chromium OS 15.0_pre465103_p20220825-r4 clang version 15.0.0 (/var/tmp/portage/sys-devel/llvm-15.0_pre465103_p20220825-r4/work/llvm-15.0_pre465103_p20220825/clang db1978b67431ca3462ad8935bf662c15750b8252), LLD 15.0.0) #1 SMP PREEMPT Tue Jan 24 19:38:23 PST 2023
    [    0.000000] Command line: loglevel=7 ... usb-storage.quirks=13fe:6500:u earlycon=uart8250,mmio32,0xfe03e000,115200n8
    [    0.000000] x86/split lock detection: warning about user-space split_locks

This shows looking at x86 setup information::

    => bootfl sel 0
    => bootfl i -s
    Setup located at 77b56010:

    ACPI RSDP addr      : 0
    E820: 2 entries
            Addr        Size  Type
               0        1000  RAM
        fffff000        1000  Reserved
    Setup sectors       : 1e
    Root flags          : 1
    Sys size            : 63420
    RAM size            : 0
    Video mode          : ffff
    Root dev            : 0
    Boot flag           : 0
    Jump                : 66eb
    Header              : 53726448
                          Kernel V2
    Version             : 20d
    Real mode switch    : 0
    Start sys seg       : 1000
    Kernel version      : 38cc
       @00003acc:
    Type of loader      : ff
                          unknown
    Load flags          : 1
                        : loaded-high
    Setup move size     : 8000
    Code32 start        : 100000
    Ramdisk image       : 0
    Ramdisk size        : 0
    Bootsect kludge     : 0
    Heap end ptr        : 5160
    Ext loader ver      : 0
    Ext loader type     : 0
    Command line ptr    : 735000
    Initrd addr max     : 7fffffff
    Kernel alignment    : 200000
    Relocatable kernel  : 1
    Min alignment       : 15
                        : 200000
    Xload flags         : 3
                        : 64-bit-entry can-load-above-4gb
    Cmdline size        : 7ff
    Hardware subarch    : 0
    HW subarch data     : 0
    Payload offset      : 26e
    Payload length      : 612045
    Setup data          : 0
    Pref address        : 1000000
    Init size           : 1383000
    Handover offset     : 0

This shows reading a bootflow to examine the kernel::

    => bootfl i 0
    Name:
    Device:    emmc@1c,0.bootdev
    Block dev: emmc@1c,0.blk
    Method:    cros
    State:     ready
    Partition: 2
    Subdir:    (none)
    Filename:  <NULL>
    Buffer:    0
    Size:      63ee00 (6548992 bytes)
    OS:        ChromeOS
    Cmdline:   console= loglevel=7 init=/sbin/init cros_secure oops=panic panic=-1 root=PARTUUID=35c775e7-3735-d745-93e5-d9e0238f7ed0/PARTNROFF=1 rootwait rw dm_verity.error_behavior=3 dm_verity.max_bios=-1 dm_verity.dev_wait=0 dm="1 vroot none rw 1,0 3788800 verity payload=ROOT_DEV hashtree=HASH_DEV hashstart=3788800 alg=sha1 root_hexdigest=55052b629d3ac889f25a9583ea12cdcd3ea15ff8 salt=a2d4d9e574069f4fed5e3961b99054b7a4905414b60a25d89974a7334021165c" noinitrd vt.global_cursor_default=0 kern_guid=35c775e7-3735-d745-93e5-d9e0238f7ed0 add_efi_memmap boot=local noresume noswap i915.modeset=1 tpm_tis.force=1 tpm_tis.interrupts=0 nmi_watchdog=panic,lapic disablevmx=off
    X86 setup: 77b56010
    Logo:      (none)
    FDT:       <NULL>
    Error:     0

Note that `Buffer` is 0 so it has not be read yet. Using `bootflow read`::

    => bootfl read
    => bootfl info
    Name:
    Device:    emmc@1c,0.bootdev
    Block dev: emmc@1c,0.blk
    Method:    cros
    State:     ready
    Partition: 2
    Subdir:    (none)
    Filename:  <NULL>
    Buffer:    77b7e400
    Size:      63ee00 (6548992 bytes)
    OS:        ChromeOS
    Cmdline:   console= loglevel=7 init=/sbin/init cros_secure oops=panic panic=-1 root=PARTUUID=35c775e7-3735-d745-93e5-d9e0238f7ed0/PARTNROFF=1 rootwait rw dm_verity.error_behavior=3 dm_verity.max_bios=-1 dm_verity.dev_wait=0 dm="1 vroot none rw 1,0 3788800 verity payload=ROOT_DEV hashtree=HASH_DEV hashstart=3788800 alg=sha1 root_hexdigest=55052b629d3ac889f25a9583ea12cdcd3ea15ff8 salt=a2d4d9e574069f4fed5e3961b99054b7a4905414b60a25d89974a7334021165c" noinitrd vt.global_cursor_default=0 kern_guid=35c775e7-3735-d745-93e5-d9e0238f7ed0 add_efi_memmap boot=local noresume noswap i915.modeset=1 tpm_tis.force=1 tpm_tis.interrupts=0 nmi_watchdog=panic,lapic disablevmx=off
    X86 setup: 781b4400
    Logo:      (none)
    FDT:       <NULL>
    Error:     0

Now the buffer can be accessed::

    => md 77b7e400
    77b7e400: 1186f6fc 40000002 b8fa0c75 00000018  .......@u.......
    77b7e410: c08ed88e a68dd08e 000001e8 000000e8  ................
    77b7e420: ed815d00 00000021 62c280b8 89e80100  .]..!......b....
    77b7e430: 22f7e8c4 c0850061 22ec850f eb890061  ..."a......"a...
    77b7e440: 0230868b 01480000 21d0f7c3 00fb81c3  ..0...H....!....
    77b7e450: 7d010000 0000bb05 c3810100 00d4f000  ...}............
    77b7e460: 8130858d 85890061 00618132 3095010f  ..0.a...2.a....0
    77b7e470: 0f006181 c883e020 e0220f20 e000bb8d  .a.. ... .".....
    77b7e480: c0310062 001800b9 8dabf300 62e000bb  b.1............b
    77b7e490: 07878d00 89000010 00bb8d07 8d0062f0  .............b..
    77b7e4a0: 00100787 0004b900 07890000 00100005  ................
    77b7e4b0: 08c78300 8df37549 630000bb 0183b800  ....Iu.....c....
    77b7e4c0: 00b90000 89000008 00000507 c7830020  ............ ...
    77b7e4d0: f3754908 e000838d 220f0062 0080b9d8  .Iu.....b.."....
    77b7e4e0: 320fc000 08e8ba0f c031300f b8d0000f  ...2.....01.....
    77b7e4f0: 00000020 6ad8000f 00858d10 50000002   ......j.......P

This shows using a text menu to boot an OS::

    => bootflow scan
    => bootfl list
    => bootfl menu -t
    U-Boot    :    Boot Menu

    UP and DOWN to choose, ENTER to select

      >    0  mmc1        mmc1.bootdev.whole
           1  mmc1        Fedora-Workstation-armhfp-31-1.9 (5.3.7-301.fc31.armv7hl)
           2  mmc1        mmc1.bootdev.part_1
           3  mmc4        mmc4.bootdev.whole
           4  mmc4        Armbian
           5  mmc4        mmc4.bootdev.part_1
           6  mmc5        mmc5.bootdev.whole
           7  mmc5        ChromeOS
           8  mmc5        ChromeOS
    U-Boot    :    Boot Menu

    UP and DOWN to choose, ENTER to select

           0  mmc1        mmc1.bootdev.whole
      >    1  mmc1        Fedora-Workstation-armhfp-31-1.9 (5.3.7-301.fc31.armv7hl)
           2  mmc1        mmc1.bootdev.part_1
           3  mmc4        mmc4.bootdev.whole
           4  mmc4        Armbian
           5  mmc4        mmc4.bootdev.part_1
           6  mmc5        mmc5.bootdev.whole
           7  mmc5        ChromeOS
           8  mmc5        ChromeOS
    U-Boot    :    Boot Menu

    Selected: Fedora-Workstation-armhfp-31-1.9 (5.3.7-301.fc31.armv7hl)
    => bootfl boot
    ** Booting bootflow 'mmc1.bootdev.part_1' with extlinux
    Ignoring unknown command: ui
    Ignoring malformed menu command:  autoboot
    Ignoring malformed menu command:  hidden
    Ignoring unknown command: totaltimeout
    Fedora-Workstation-armhfp-31-1.9 Boot Options.
    1:	Fedora-Workstation-armhfp-31-1.9 (5.3.7-301.fc31.armv7hl)
    Enter choice: 1
    1:	Fedora-Workstation-armhfp-31-1.9 (5.3.7-301.fc31.armv7hl)
    Retrieving file: /vmlinuz-5.3.7-301.fc31.armv7hl
    Retrieving file: /initramfs-5.3.7-301.fc31.armv7hl.img
    append: ro root=UUID=9732b35b-4cd5-458b-9b91-80f7047e0b8a rhgb quiet LANG=en_US.UTF-8 cma=192MB cma=256MB
    Retrieving file: /dtb-5.3.7-301.fc31.armv7hl/sandbox.dtb
    ...


Return value
------------

On success `bootflow boot` normally boots into the Operating System and does not
return to U-Boot. If something about the U-Boot processing fails, then the
return value $? is 1. If the boot succeeds but for some reason the Operating
System returns, then $? is 0, indicating success.

For `bootflow menu` the return value is $? is 0 (true) if an option was choses,
else 1.

For other subcommands, the return value $? is always 0 (true).


.. BootflowStates_:
