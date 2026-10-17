# SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
# ################################################################
# Copyright (c) Facebook, Inc.
# All rights reserved.
#
# This source code is licensed under both the BSD-style license (found in the
# LICENSE file in the root directory of this source tree) and the GPLv2 (found
# in the COPYING file in the root directory of this source tree).
# You may select, at your option, one of the above-listed licenses.
# ################################################################
obj-y += zstd_decompress.o
obj-y += zstd_common.o

ifeq ($(CONFIG_ZSTD_LIB_MINIFY),y)
ccflags-y += -DHUF_FORCE_DECOMPRESS_X1
ccflags-y += -DZSTD_FORCE_DECOMPRESS_SEQUENCES_SHORT
ccflags-y += -DZSTD_NO_INLINE
ccflags-y += -DZSTD_STRIP_ERROR_STRINGS
ccflags-y += -DDYNAMIC_BMI2=0
endif

zstd_decompress-y := \
		zstd_decompress_module.o \
		decompress/huf_decompress.o \
		decompress/zstd_ddict.o \
		decompress/zstd_decompress.o \
		decompress/zstd_decompress_block.o \
		zstd.o \

zstd_common-y := \
		zstd_common_module.o \
		common/debug.o \
		common/entropy_common.o \
		common/error_private.o \
		common/fse_decompress.o \
		common/zstd_common.o \
