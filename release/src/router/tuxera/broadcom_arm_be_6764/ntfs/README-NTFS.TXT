

                   RELEASE NOTES FOR MICROSOFT NTFS BY TUXERA


--- Introduction ---

This file contains information regarding the latest Microsoft NTFS by Tuxera 
implementation. Tuxera recommends to read the entire file.

Table Of Content

 * Installation
 * Usage
 * Configurable Mount Options
 * Utilities
 * Benchmarking and Tuning
 * Contact & Support Information


--- Installation ---

Load the Microsoft NTFS by Tuxera kernel module, called tntfs.ko, into the 
kernel:

        insmod tntfs.ko

then the Tuxera NTFS kernel driver is ready to be used.


--- Usage ---

Mount NTFS volumes the following way:

        mount -t tntfs [-o options] device mountpoint

Please note that the file system type is 'tntfs'.

The Tuxera NTFS kernel driver can work in two main performance modes:

1. Normal mount mode: the kernel driver is doing transparent caching. This can 
significantly improve most different kind of workloads.

2. 'iostreaming' mount mode: besides transparent caching, this mode gives the 
highest possible read and write throughput performance and at the same time it 
has the lowest CPU usage.

Examples:

        mount -t tntfs device mountpoint
        mount -t tntfs -o iostreaming device mountpoint


--- Configurable Mount Options ---

This section provides details on the available mount options at (re)mount to 
tweak the behaviour of the NTFS driver. Default values for mount options depend 
on build options and are normally optimized by Tuxera for optimum performance 
and fail safety. If you want to ensure a particular mount option is in use, you 
need to specify it on the command line or ask Tuxera to make it the default for 
your build target(s). In addition to the generic mount options described by the 
manual page for the mount(8) command (e.g. ro, rw, sync, noatime, etc, please 
see 'man 8 mount' and 'man 5 fstab' for details), the Microsoft NTFS by Tuxera 
driver supports the following mount options:

    uid=, gid=, umask=
    They provide default owner, group, and access mode mask. These options work 
    as documented in mount(8). By default, the files and directories are owned 
    by root and everybody has read and write permissions, as well as browse 
    permission for directories. I.e. the mode on all files and directories is 
    by default rwxrwxrwx, a consequence of the default umask=0000.

    fmask=, dmask=
    Instead of specifying umask= which applies both to files and directories, 
    fmask applies only to files and dmask only to directories.

    utf8 (default)
    Use full Unicode support thus interpreting the NTFS characters to be stored 
    in UTF-16 rather than UCS2 and assuming that the Linux side is using UTF-8.

    noutf8
    Disable use of full unicode support thus interpreting the NTFS characters 
    to be stored as UCS2 and assuming that the Linux side is using whatever the 
    default or specified NLS codepage is (see nls= mount option).

    nls=name
    Character set containing the definition of byte encoding used to represent 
    each Unicode character of file names. It is used for Unicode string 
    handling. Note that most character sets contain insufficient characters to 
    represent all possible Unicode characters that can exist on NTFS. To be 
    sure you are not missing any files, it is advised to use nls=utf8 which is 
    capable of representing all Unicode characters or even better to use the 
    utf8 option (see above).
    Default = utf8
    Usage:
        -o nls=utf8

    iocharset=name
    Deprecated option. Still supported but please use utf8 or nls=name in the 
    future. See the utf8 and nls options above for details.

    show_sys_files
    If show_sys_files is specified, show the system files in directory 
    listings. Otherwise the default behavior is to hide the system files. Note 
    that even when show_sys_files is specified, "$MFT" will not be visible due 
    to some libc implementations. Further, note that irrespective of 
    show_sys_files, all files are accessible by name, i.e. you can always do 
    "ls -l $UpCase" for example to specifically show the system file containing 
    the Unicode upcase table.

    protected_system={show,ignore}
    If protected_system is specified or if protected_system=show is specified, 
    we shows all content. If protected_system=ignore is set instead, we skip 
    all content with 'system' and 'hidden' attribute.
    Default = show

    hidden={show,ignore}
    If hidden is specified or if hidden=show is specified, we show all content 
    with 'hidden' attribute set. If hidden=ignore is specified, we skip all 
    content with 'hidden' attribute.
    Default = show

    case_sensitive=<BOOL>
    If case_sensitive is specified, treat all file names as case sensitive and 
    create file names in the POSIX namespace. The default behavior is to treat 
    file names as case sensitive.

    disable_sparse
    If disable_sparse is specified, creation of sparse regions, i.e. holes, 
    inside files is disabled for the volume (for the duration of mount only). 
    By default, creation of sparse regions is enabled, which is consistent with 
    the behavior of traditional Unix filesystems.

    errors=opt
    What to do when critical filesystem errors are found. Following values can 
    be used for "opt":
    continue:
        DEFAULT, try to clean-up as much as possible, e.g. marking a corrupt 
        inode as bad so it is no longer accessed, and then continue.
    recover:
        recover as much as possible and continue operation. If read-only mount, 
        the recovery is done in memory only and not written to disk.

    ioblocksize=SIZE
    The preferred/optimal default I/O block size which the driver advertises to 
    user space via the stat, fstat and lstat system calls. The default I/O 
    block size is 131072 (128 kB).
    Default = 131072

    iostreaming (disabled by default)
    The iostreaming mount mode gives the highest possible read and write 
    throughput performance and at the same time it has the lowest CPU usage 
    during large file transfers.

    min_iostreaming_read_iosize=SIZE, min_iostreaming_write_iosize=SIZE
    The minimum block size when iostreaming starts is configurable for both 
    read and write via theses mount options. Optimal values are are chipset and 
    deployment specific, typically between 64kiB and 1 MiB.
    Default (read)  = 131072
    Default (write) = 65536

    sloppy
    If sloppy is specified, ignore unknown mount options. Otherwise the default 
    behavior is to abort mount if any unknown options are found.


--- Utilities ---

The software package may include additional utilities like:

  - mkntfs: speed optimized, low memory footprint, NTFS volume size independent 
  NTFS 'format' utility.

  - ntfsck: check and repair NTFS.

  - ntfslabel: show or change the label on an NTFS file system.

  - ntfsdebug: collect NTFS metadata for debug, analyses, troubleshooting 
  purposes.

Please see their usage in the relevant utility manual page, and/or type 
'utility_name --help' for a brief introduction.

Tuxera developed and supports many additional NTFS related utilities, used by 
millions of installation which may be useful for your usage scenario. These 
include but not limited to backup, restore, defragment, resize, compare, 
undelete, copy, list, wipe, dump. Please let us know.


--- Benchmarking and Tuning ---

A benchmark utility, called tuxera.bench, is included in the package. 
Tuxera.bench is a high-performance benchmark, similar to 'iozone'. The major 
difference is that while 'iozone' can run for hours, tuxera.bench can get the 
same or very similar results in a few minutes.

Tuxera.bench is able to determine the cached and effective read and write 
speeds using different record/block sizes in a very short time. The benchmark 
tool can be used to find the optimal I/O buffer size and alignment giving the 
highest throughput on a platform to tune the performance critical applications.

Please see 'tuxera.bench --help' for the options. Typically the default buffer 
alignment, shown by the previous command, gives the best performance.

Usage: 'cd' into a file system mounted by a file system driver to be tested 
then issue the following command

        path/to/tuxera.bench

Benchmark the following cases:

  1. Tuxera NTFS kernel driver in normal mount mode
  2. Tuxera NTFS kernel driver in 'iostreaming' mount mode
  3. Other Linux file system, e.g. ext2, ext3, ext4, XFS, FAT32

Please make sure the tests are done using the same disk and the same partition 
with the different file systems, otherwise the comparison can be misleading.

Please also note that lower capacity flash devices (USB pen drives, SD, MMC, 
CF, etc cards) can have a significantly lower speed limit than the maximum 
speed achievable by the file system drivers.

Example outputs:

Tuxera NTFS, ARM 300MHz, USB 2.0
*===================================================================*
|       |                  I/O Block Performance                    |
|       |-----------------------------------------------------------|
| Buffer|      Write (MB/sec)   |           Read (MB/sec)           |
|  Size |-----------------------|-----------------------------------|
|       |   Cached  | Effective |   Cached  | Effective |   Device  |
*===================================================================*
  16384      26.62       23.88      120.71       27.60       25.92
  32768      27.87       21.01      122.62       27.48       25.68
  65536      27.74       24.88      124.32       27.95       25.00
 131072      27.84       25.00      123.57       24.75       24.86
 262144      25.67       23.48      124.17       24.47       25.96
 524288      27.86       24.74      124.26       23.55       26.07
1048576      27.44       24.10      125.02       24.68       25.17
*===================================================================*
|                        I/O Metadata Performance                   |
*===================================================================*
  real io/sec:     306876
cached io/sec:    1103370

Explanation of the outputs:

'Cached Write Performance' means the performance what applications and users 
experience if lots of free RAM is available but no guarantee that the data is 
on the disk.

'Effective Write Performance' means that the written data is on the disk. This 
is more relevant for sustained write (large amount of data).

'Cached Read Performance' typically shows the memory bandwidth. Caching read 
data can greatly help small reads and short-term rereads.

'Effective Read Performance' means non-cached read performance from the disk 
via the file system.

'Device Read Performance' means non-cached read performance from the disk 
without file system involvement. This is the physical performance limit of the 
hardware, firmwares and kernel device drivers. Effective file system read 
performance is rarely higher than this values.


--- Contact & Support Information ---

Please send feedback and support queries to support@tuxera.com.

Besides describing your issue please provide the following information to help 
Tuxera's engineers address the issue:

  - detailed console error messages
  - kernel log by using the 'dmesg' command, or 'cat /proc/kmsg'
  - steps to reproduce the issue

Thank you.

Copyright (c) 2008-2024 Tuxera Inc. All Rights Reserved.

