
                    RELEASE NOTES FOR TUXERA HFS+


--- Introduction ---

This file contains information regarding to the latest Tuxera HFS+ delivery. 
Tuxera recommends to read the entire file.

Table of content

 * Installation
 * Usage
 * Configurable Mount Options
 * Contact & Support Information


--- Installation ---

Load the Tuxera HFS+ kernel module, called thfsplus.ko, into the kernel:

	insmod thfsplus.ko

then the Tuxera HFS+ kernel driver is ready to be used.


--- Usage ---

Mount HFS+ volumes the following way:

	mount -t thfsplus device mountpoint

Example:

	mount -t thfsplus /dev/sda1 /mnt/usb


--- Configurable Mount Options ---

In addition to the generic mount options described by the manual page for the 
mount(8) command (e.g. ro, rw, sync, noatime, dirsync, etc, please see 'man 8 
mount' and 'man 5 fstab' for details), as well as the default mount options 
supported by the open source HFS+ driver described in its documentation (ref. 
Documentation/filesystems/hfsplus.txt), the Tuxera HFS+ filesystem driver 
supports the following mount options:

    fmask=
    Sets the file mode creation mask (Only the file permission bits of the mask 
    are used).
    Usage:
        -o fmask=0022

    dmask=
    Sets the directory mode creation mask (Only the permission bits of the mask 
    are used).
    Usage:
        -o dmask=0022

    allow_utime
    Specifies the permission required for setting the access/modified times of 
    a directory entry. By default, the permissions are derived from the 
    directory mode creation mask.
    Usage:
        -o allow_utime=0000

    utf8 (default)
    Use full Unicode support thus interpreting the HFS+ characters to be stored 
    in UTF-16 rather than UCS2 and assuming that the Linux side is using utf8.
    Usage (for enabling):
        -o utf8

    noutf8
    Disable use of full Unicode support thus interpreting the HFS+ characters 
    to be stored as UCS2 and assuming that the Linux side is using whatever the 
    default or specified NLS codepage is (see nls= mount option).

    nls=name
    Character set containing the definition of byte encoding used to represent 
    each Unicode character of file names. It is used for Unicode string 
    handling. To be sure you are not missing any files, it is advised to use 
    nls=utf8 which is capable of representing all Unicode characters or even 
    better to use the utf8 option (see above).
    Default = utf8
    Usage:
        -o nls=utf8

    nomode
    Gives read, write and execute permissions to all files and directories for 
    all users while retaining the original permissions, i.e. when the 
    filesystem is mounted without nomode mount option the original permissions 
    are set.


--- Contact & Support Information ---

Please send feedback and support queries to support@tuxera.com.

Besides describing your issue please provide the following information to help 
Tuxera's engineers address the issue:

  - detailed console error messages
  - kernel log by using the 'dmesg' command, or 'cat /proc/kmsg'
  - steps to reproduce the issue

Thank you.

Copyright (c) 2010-2022 Tuxera Inc. All rights reserved.
