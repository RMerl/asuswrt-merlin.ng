
INTRODUCTION
============

The NTFS-3G driver is an open source, freely available read/write NTFS driver 
for Linux, FreeBSD, macOS, NetBSD, OpenIndiana, QNX and Haiku. It provides 
safe and fast handling of the Windows XP, Windows Server 2003, Windows 2000, 
Windows Vista, Windows Server 2008, Windows 7, Windows 8, Windows Server 2012,
Windows Server 2016, Windows 10 and Windows Server 2019 NTFS file systems.

The purpose of the project is to develop, quality assurance and support a 
trustable, featureful and high performance solution for hardware platforms 
and operating systems whose users need to reliably interoperate with NTFS. 
Besides this practical goal, the project also aims to explore the limits 
of the hybrid, kernel/user space filesystem driver approach, performance, 
reliability and feature richness per invested effort wise.

Besides the common file system features, NTFS-3G has support for file 
ownership and permissions, POSIX ACLs, junction points, extended attributes 
and creating internally compressed files (parameter files in the directory
.NTFS-3G may be required to enable them). The new compressed file formats
available in Windows 10 can also be read through a plugin. 

News, support answers, problem submission instructions, support and discussion 
forums, and other information are available on the project web site at

	https://github.com/tuxera/ntfs-3g/wiki

The project has been funded, supported and maintained since 2008 by Tuxera:

	https://tuxera.com


LICENSES
========

All the NTFS related components: the file system drivers, the ntfsprogs
utilities and the shared library libntfs-3g are distributed under the terms
of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version. See the included file COPYING.

The fuse-lite library is distributed under the terms of the GNU LGPLv2.
See the included file COPYING.LIB.


QUICK INSTALLATION
==================

Most distributions have an up-to-date NTFS-3G package ready for use, and
the recommended way is to install it.

If you need some specific customization, you can compile and install from
the released source code. Make sure you have the basic development tools
and the kernel includes the FUSE kernel module. Then unpack the source
tarball and type:  

	./configure
	make
	make install      # or 'sudo make install' if you aren't root.

Please note that NTFS-3G doesn't require the FUSE user space package any
more.

The list of options for building specific configurations is displayed by
typing :

	./configure --help

Below are a few specific options to ./configure :
	--disable-ntfsprogs : do not build the ntfsprogs tools,
	--enable-extras : build more ntfsprogs tools,
	--disable-plugins : disable support for plugins
	--enable-posix-acls : enable support for Posix ACLs
	--enable-xattr-mappings : enable system extended attributes mappings
	--with-fuse=external : use external fuse (overriding Linux default)

There are also a few make targets for building parts :
	make libntfs : only build the libntfs-3g library
	make libs : only build libntfs-3g (and libfuse-lite, if relevant)
	make drivers : only build drivers and libraries, without ntfsprogs
	make ntfsprogs : only build ntfsprogs and libntfs-3g, without drivers


USAGE
=====

If there was no error during installation then the NTFS volume can be
read-write mounted for everybody the following way as the root user 
(unmount the volume if it was already mounted, and replace /dev/sda1 
and /mnt/windows, if needed):

	mount -t ntfs-3g /dev/sda1 /mnt/windows
or
	ntfs-3g /dev/sda1 /mnt/windows

Please see the ntfs-3g manual page for more options and examples.

You can also make NTFS to be mounted during boot by putting the below 
line at the END(!) of the /etc/fstab file:

	/dev/sda1 /mnt/windows ntfs-3g defaults 0 0


TESTING WITHOUT INSTALLING
=========================

Newer versions of ntfs-3g can be tested without installing anything and
without disturbing an existing installation. Just configure and make as
shown previously. This will create the scripts ntfs-3g and lowntfs-3g
in the src directory, which you may activate for testing:

	./configure
	make

then, as root:
	src/ntfs-3g [-o mount-options] /dev/sda1 /mnt/windows

And, to end the test, unmount the usual way:
	umount /dev/sda1


NTFS UTILITIES
==============

The ntfsprogs directory includes utilities for doing all required tasks to
NTFS partitions. In general, just run a utility without any command line
options to display the version number and usage syntax.

The following utilities are so far implemented:

ntfsfix - Attempt to fix an NTFS partition and force Windows to check NTFS.

mkntfs - Format a partition with the NTFS filesystem.  See man 8 mkntfs for
command line options.

ntfslabel - Display/change the label of an NTFS partition.  See man 8 ntfslabel
for details.

ntfsundelete - Recover deleted files from an NTFS volume.  See man 8
ntfsundelete for more details.

ntfsresize - Resize NTFS volumes.  See man 8 ntfsresize for details.

ntfsclone - Efficiently create/restore an image of an NTFS partition.  See
man 8 ntfsclone for details.

ntfscluster - Locate the owner of any given sector or cluster on an NTFS
partition.  See man 8 ntfscluster for details.

ntfsinfo - Show some information about an NTFS partition or one of the files
or directories within it.  See man 8 ntfsinfo for details.

ntfsrecover - Recover updates committed by Windows but interrupted before
being synced.

ntfsls - List information about files in a directory residing on an NTFS
partition.  See man 8 ntfsls for details.

ntfscat - Concatenate files and print their contents on the standard output.

ntfscp - Overwrite files on an NTFS partition.

ntfssecaudit - Audit the security metadata.

ntfsusermap - Assistance for building a user mapping file.
