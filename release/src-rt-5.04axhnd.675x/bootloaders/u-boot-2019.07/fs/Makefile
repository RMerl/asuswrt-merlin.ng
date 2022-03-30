# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2000-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
# Copyright (c) 2012, NVIDIA CORPORATION.  All rights reserved.

ifdef CONFIG_SPL_BUILD
obj-$(CONFIG_FS_LOADER) += fs.o
obj-$(CONFIG_SPL_FS_FAT) += fat/
obj-$(CONFIG_SPL_FS_EXT4) += ext4/
else
obj-y				+= fs.o

obj-$(CONFIG_FS_BTRFS) += btrfs/
obj-$(CONFIG_FS_CBFS) += cbfs/
obj-$(CONFIG_CMD_CRAMFS) += cramfs/
obj-$(CONFIG_FS_EXT4) += ext4/
obj-$(CONFIG_FS_FAT) += fat/
obj-$(CONFIG_FS_JFFS2) += jffs2/
obj-$(CONFIG_CMD_REISER) += reiserfs/
obj-$(CONFIG_SANDBOX) += sandbox/
obj-$(CONFIG_CMD_UBIFS) += ubifs/
obj-$(CONFIG_YAFFS2) += yaffs2/
obj-$(CONFIG_CMD_ZFS) += zfs/
endif
obj-y += fs_internal.o
