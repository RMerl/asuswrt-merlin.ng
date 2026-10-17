# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2000-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.

# A note on target vs host configuration:
#
# Host tools can be used across multiple targets, or different configurations
# of the same target. Thus, host tools must be able to handle any combination
# of target configurations. To prevent having different variations of the same
# tool, the tool build options may not depend on target configuration.
#
# Some linux distributions package these utilities as u-boot-tools, and it
# would be unmaintainable to have a different tool variation for each
# arch or configuration.
#
# A couple of simple rules:
#
# 1) Do not use target CONFIG_* options to enable or disable features in host
#    tools. Only use the configs from tools/Kconfig
# 2) It's okay to use target configs to disable building specific tools.
#    That's as long as the features of those tools aren't modified.
#

# Enable all the config-independent tools
ifneq ($(HOST_TOOLS_ALL),)
CONFIG_ARCH_KIRKWOOD = y
CONFIG_CMD_LOADS = y
CONFIG_CMD_NET = y
CONFIG_NETCONSOLE = y
CONFIG_ARCH_SUNXI = y
endif

OPENSSL_DIR=$(kbuild-dir)/../../../hostTools/openssl

subdir-$(HOST_TOOLS_ALL) += gdb

# Merge all the different vars for envcrc into one
ENVCRC-$(CONFIG_ENV_IS_IN_EEPROM) = y
ENVCRC-$(CONFIG_ENV_IS_IN_FLASH) = y
ENVCRC-$(CONFIG_ENV_IS_IN_ONENAND) = y
ENVCRC-$(CONFIG_ENV_IS_IN_NAND) = y
ENVCRC-$(CONFIG_ENV_IS_IN_NVRAM) = y
ENVCRC-$(CONFIG_ENV_IS_IN_SPI_FLASH) = y
BUILD_ENVCRC ?= $(ENVCRC-y)

hostprogs-$(CONFIG_SPL_GENERATE_ATMEL_PMECC_HEADER) += atmel_pmecc_params

hostprogs-$(CONFIG_VIDEO_LOGO) += bmp_logo
HOSTCFLAGS_bmp_logo.o := -pedantic

hostprogs-$(BUILD_ENVCRC) += envcrc
envcrc-objs := envcrc.o generated/lib/crc32.o generated/env/embedded.o generated/lib/sha1.o

hostprogs-$(CONFIG_CMD_NET) += gen_eth_addr
HOSTCFLAGS_gen_eth_addr.o := -pedantic

hostprogs-$(CONFIG_CMD_NET) += gen_ethaddr_crc
gen_ethaddr_crc-objs := gen_ethaddr_crc.o generated/lib/crc8.o
HOSTCFLAGS_gen_ethaddr_crc.o := -pedantic

hostprogs-$(CONFIG_CMD_LOADS) += img2srec
HOSTCFLAGS_img2srec.o := -pedantic

hostprogs-y += passwd_gen
passwd_gen-objs := passwd_gen.o generated/lib/sha256.o

hostprogs-y += swapenvimage
swapenvimage-objs := swapenvimage.o os_support.o generated/lib/crc32.o

hostprogs-y += mkenvimage
mkenvimage-objs := mkenvimage.o os_support.o generated/lib/crc32.o

hostprogs-y += dumpimage mkimage
hostprogs-$(CONFIG_TOOLS_LIBCRYPTO) += fit_info fit_check_sign
hostprogs-$(CONFIG_TOOLS_LIBCRYPTO) += fdt_add_pubkey

ifneq ($(CONFIG_CMD_BOOTEFI_SELFTEST)$(CONFIG_FWU_MDATA_GPT_BLK),)
hostprogs-y += file2include
endif

FIT_OBJS-y := fit_common.o fit_image.o image-host.o generated/boot/image-fit.o
FIT_SIG_OBJS-$(CONFIG_TOOLS_LIBCRYPTO) := image-sig-host.o generated/boot/image-fit-sig.o
FIT_CIPHER_OBJS-$(CONFIG_TOOLS_LIBCRYPTO) := generated/boot/image-cipher.o

# The following files are synced with upstream DTC.
# Use synced versions from scripts/dtc/libfdt/.
LIBFDT_OBJS := $(addprefix libfdt/, fdt.o fdt_ro.o fdt_wip.o fdt_sw.o fdt_rw.o \
		fdt_strerror.o fdt_empty_tree.o fdt_addresses.o fdt_overlay.o)

RSA_OBJS-$(CONFIG_TOOLS_LIBCRYPTO) := $(addprefix generated/lib/rsa/, \
					rsa-sign.o rsa-verify.o \
					rsa-mod-exp.o)

ECDSA_OBJS-$(CONFIG_TOOLS_LIBCRYPTO) := $(addprefix generated/lib/ecdsa/, ecdsa-libcrypto.o)

AES_OBJS-$(CONFIG_TOOLS_LIBCRYPTO) := $(addprefix generated/lib/aes/, \
					aes-encrypt.o aes-decrypt.o)

# Cryptographic helpers and image types that depend on openssl/libcrypto
LIBCRYPTO_OBJS-$(CONFIG_TOOLS_LIBCRYPTO) := \
			generated/lib/fdt-libcrypto.o \
			sunxi_toc0.o

KWB_IMAGE_OBJS-$(CONFIG_TOOLS_LIBCRYPTO) := kwbimage.o

ROCKCHIP_OBS = generated/lib/rc4.o rkcommon.o rkimage.o rksd.o rkspi.o

# common objs for dumpimage and mkimage
dumpimage-mkimage-objs := aisimage.o \
			atmelimage.o \
			$(FIT_OBJS-y) \
			$(FIT_SIG_OBJS-y) \
			$(FIT_CIPHER_OBJS-y) \
			generated/boot/fdt_region.o \
			generated/boot/bootm.o \
			generated/lib/crc32.o \
			default_image.o \
			generated/lib/fdtdec_common.o \
			generated/lib/fdtdec.o \
			generated/boot/image.o \
			generated/boot/image-host.o \
			imagetool.o \
			imximage.o \
			imx8image.o \
			imx8mimage.o \
			$(KWB_IMAGE_OBJS-y) \
			generated/lib/md5.o \
			lpc32xximage.o \
			mxsimage.o \
			omapimage.o \
			os_support.o \
			pblimage.o \
			pbl_crc32.o \
			renesas_spkgimage.o \
			sfspl.o \
			vybridimage.o \
			stm32image.o \
			$(ROCKCHIP_OBS) \
			socfpgaimage.o \
			sunxi_egon.o \
			generated/lib/crc16-ccitt.o \
			generated/lib/hash-checksum.o \
			generated/lib/sha1.o \
			generated/lib/sha256.o \
			generated/lib/sha512.o \
			generated/common/hash.o \
			ublimage.o \
			zynqimage.o \
			zynqmpimage.o \
			zynqmpbif.o \
			$(LIBCRYPTO_OBJS-y) \
			$(LIBFDT_OBJS) \
			gpimage.o \
			gpimage-common.o \
			mtk_image.o \
			mtk_nand_headers.o \
			$(ECDSA_OBJS-y) \
			$(RSA_OBJS-y) \
			$(AES_OBJS-y)

dumpimage-objs := $(dumpimage-mkimage-objs) dumpimage.o
mkimage-objs   := $(dumpimage-mkimage-objs) mkimage.o
fit_info-objs   := $(dumpimage-mkimage-objs) fit_info.o
fit_check_sign-objs   := $(dumpimage-mkimage-objs) fit_check_sign.o
fdt_add_pubkey-objs   := $(dumpimage-mkimage-objs) fdt_add_pubkey.o
file2include-objs := file2include.o

ifneq ($(CONFIG_MX23)$(CONFIG_MX28)$(CONFIG_TOOLS_LIBCRYPTO),)
# Add CFG_MXS into host CFLAGS, so we can check whether or not register
# the mxsimage support within tools/mxsimage.c .
HOSTCFLAGS_mxsimage.o += -DCFG_MXS
endif

ifdef CONFIG_TOOLS_LIBCRYPTO
# This affects include/image.h, but including the board config file
# is tricky, so manually define this options here.
HOST_EXTRACFLAGS	+= -DCONFIG_FIT_SIGNATURE -L$(OPENSSL_DIR)/obj/ -I$(OPENSSL_DIR)/src/include -I$(OPENSSL_DIR)/obj/include
HOST_EXTRACFLAGS	+= -DCONFIG_FIT_SIGNATURE_MAX_SIZE=0xffffffff
HOST_EXTRACFLAGS	+= -DCONFIG_FIT_CIPHER
endif

# MXSImage needs LibSSL
ifneq ($(CONFIG_MX23)$(CONFIG_MX28)$(CONFIG_ARMADA_38X)$(CONFIG_TOOLS_LIBCRYPTO),)
HOSTCFLAGS_kwbimage.o += \
	$(shell pkg-config --cflags libssl libcrypto 2> /dev/null || echo "")
HOSTLDLIBS_mkimage += \
	-L$(OPENSSL_DIR)/obj/ -lssl -lcrypto  -Wl,-rpath=$(OPENSSL_DIR)/obj/

# OS X deprecate openssl in favour of CommonCrypto, supress deprecation
# warnings on those systems
ifeq ($(HOSTOS),darwin)
HOSTCFLAGS_mxsimage.o += -Wno-deprecated-declarations
HOSTCFLAGS_image-sig.o += -Wno-deprecated-declarations
HOSTCFLAGS_rsa-sign.o += -Wno-deprecated-declarations
endif
endif

HOSTCFLAGS_fit_image.o += -DMKIMAGE_DTC=\"$(CONFIG_MKIMAGE_DTC_PATH)\"

HOSTLDLIBS_dumpimage := $(HOSTLDLIBS_mkimage)
HOSTLDLIBS_fit_info := $(HOSTLDLIBS_mkimage)
HOSTLDLIBS_fit_check_sign := $(HOSTLDLIBS_mkimage)
HOSTLDLIBS_fdt_add_pubkey := $(HOSTLDLIBS_mkimage)

hostprogs-$(CONFIG_EXYNOS5250) += mkexynosspl
hostprogs-$(CONFIG_EXYNOS5420) += mkexynosspl
HOSTCFLAGS_mkexynosspl.o := -pedantic

HOSTCFLAGS_kwboot.o += -pthread
HOSTLDLIBS_kwboot += -pthread
HOSTLDLIBS_kwboot += \
	$(shell pkg-config --libs tinfo 2> /dev/null || echo "-ltinfo")

ifdtool-objs := $(LIBFDT_OBJS) ifdtool.o
hostprogs-$(CONFIG_X86) += ifdtool

ifwitool-objs := ifwitool.o
hostprogs-$(CONFIG_X86)$(CONFIG_SANDBOX) += ifwitool

hostprogs-$(CONFIG_MX23) += mxsboot
hostprogs-$(CONFIG_MX28) += mxsboot
HOSTCFLAGS_mxsboot.o := -pedantic

hostprogs-$(CONFIG_ARCH_SUNXI) += mksunxiboot
hostprogs-$(CONFIG_ARCH_SUNXI) += sunxi-spl-image-builder
sunxi-spl-image-builder-objs := sunxi-spl-image-builder.o generated/lib/bch.o

hostprogs-$(CONFIG_NETCONSOLE) += ncb

hostprogs-$(CONFIG_ARCH_KIRKWOOD) += kwboot
hostprogs-$(CONFIG_ARCH_MVEBU) += kwboot

hostprogs-y += proftool
proftool-objs = proftool.o generated/lib/abuf.o

hostprogs-$(CONFIG_STATIC_RELA) += relocate-rela
hostprogs-$(CONFIG_RISCV) += prelink-riscv

hostprogs-$(CONFIG_ARCH_OCTEON) += update_octeon_header
update_octeon_header-objs := update_octeon_header.o generated/lib/crc32.o

hostprogs-y += fdtgrep
fdtgrep-objs += $(LIBFDT_OBJS) generated/boot/fdt_region.o fdtgrep.o

ifneq ($(TOOLS_ONLY),y)
hostprogs-y += spl_size_limit
endif

hostprogs-$(CONFIG_MIPS) += mips-relocs

hostprogs-$(CONFIG_ASN1_COMPILER)	+= asn1_compiler
HOSTCFLAGS_asn1_compiler.o = -idirafter $(srctree)/include

HOSTCFLAGS_mkeficapsule.o += \
	$(shell pkg-config --cflags gnutls 2> /dev/null || echo "")
HOSTCFLAGS_mkeficapsule.o += \
	$(shell pkg-config --cflags uuid 2> /dev/null || echo "")
HOSTLDLIBS_mkeficapsule += \
	$(shell pkg-config --libs gnutls 2> /dev/null || echo "-lgnutls")
HOSTLDLIBS_mkeficapsule += \
	$(shell pkg-config --libs uuid 2> /dev/null || echo "-luuid")
hostprogs-$(CONFIG_TOOLS_MKEFICAPSULE) += mkeficapsule

mkfwumdata-objs := mkfwumdata.o generated/lib/crc32.o
HOSTLDLIBS_mkfwumdata += -luuid
hostprogs-$(CONFIG_TOOLS_MKFWUMDATA) += mkfwumdata

# We build some files with extra pedantic flags to try to minimize things
# that won't build on some weird host compiler -- though there are lots of
# exceptions for files that aren't complaint.
HOSTCFLAGS_crc32.o := -pedantic
HOSTCFLAGS_crc8.o := -pedantic
HOSTCFLAGS_md5.o := -pedantic
HOSTCFLAGS_sha1.o := -pedantic
HOSTCFLAGS_sha256.o := -pedantic
HOSTCFLAGS_sha512.o := -pedantic -DCONFIG_SHA512 -DCONFIG_SHA384

quiet_cmd_wrap = WRAP    $@
cmd_wrap = echo "\#include <../$(patsubst $(obj)/generated/%,%,$@)>" >$@

$(obj)/generated/%.c:
	$(call cmd,wrap)

clean-dirs := generated

always := $(hostprogs-y)

# Host tool to dump the currently configured default environment,
# build it on demand, i.e. not add it to 'always'.
hostprogs-y += printinitialenv

# Generated LCD/video logo
LOGO_H = $(objtree)/include/bmp_logo.h
LOGO_DATA_H = $(objtree)/include/bmp_logo_data.h
LOGO-$(CONFIG_VIDEO_LOGO) += $(LOGO_H)
LOGO-$(CONFIG_VIDEO_LOGO) += $(LOGO_DATA_H)

# Generic logo
ifeq ($(LOGO_BMP),)
LOGO_BMP= $(srctree)/$(src)/logos/denx.bmp

# Use board logo and fallback to vendor
ifneq ($(wildcard $(srctree)/$(src)/logos/$(BOARD).bmp),)
LOGO_BMP= $(srctree)/$(src)/logos/$(BOARD).bmp
else
ifneq ($(wildcard $(srctree)/$(src)/logos/$(VENDOR).bmp),)
LOGO_BMP= $(srctree)/$(src)/logos/$(VENDOR).bmp
endif
endif

endif # !LOGO_BMP

#
# Use native tools and options
# Define __KERNEL_STRICT_NAMES to prevent typedef overlaps
# Define _GNU_SOURCE to obtain the getline prototype from stdio.h
#
HOST_EXTRACFLAGS += -include $(srctree)/include/compiler.h \
		$(patsubst -I%,-idirafter%, $(filter -I%, $(UBOOTINCLUDE))) \
		-I$(srctree)/scripts/dtc/libfdt \
		-I$(srctree)/tools \
		-DUSE_HOSTCC \
		-D__KERNEL_STRICT_NAMES \
		-D_GNU_SOURCE

__build:	$(LOGO-y)

$(LOGO_H):	$(obj)/bmp_logo $(LOGO_BMP)
	$(obj)/bmp_logo --gen-info $(LOGO_BMP) > $@

ifeq ($(CONFIG_VIDEO),y)
$(LOGO_DATA_H):	$(obj)/bmp_logo $(LOGO_BMP)
	$(obj)/bmp_logo --gen-bmp $(LOGO_BMP) > $@
else
$(LOGO_DATA_H):	$(obj)/bmp_logo $(LOGO_BMP)
	$(obj)/bmp_logo --gen-data $(LOGO_BMP) > $@
#endif
endif

# Let clean descend into subdirs
subdir- += env

ifneq ($(CROSS_BUILD_TOOLS),)
override HOSTCC = $(CC)
override HOSTCFLAGS = $(CFLAGS)

quiet_cmd_crosstools_strip = STRIP   $^
      cmd_crosstools_strip = $(STRIP) $^; touch $@
$(obj)/.strip: $(call objectify,$(filter $(always),$(hostprogs-y)))
	$(call cmd,crosstools_strip)

always += .strip
endif
clean-files += .strip
