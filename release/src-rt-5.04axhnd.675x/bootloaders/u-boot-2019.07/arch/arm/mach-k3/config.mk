# SPDX-License-Identifier:	GPL-2.0+
#
# Copyright (C) 2017-2018 Texas Instruments Incorporated - http://www.ti.com/
#	Lokesh Vutla <lokeshvutla@ti.com>

ifdef CONFIG_SPL_BUILD

# Openssl is required to generate x509 certificate.
# Error out if openssl is not available.
ifeq ($(shell which openssl),)
$(error "No openssl in $(PATH), consider installing openssl")
endif

IMAGE_SIZE= $(shell cat $(obj)/u-boot-spl.bin | wc -c)
MAX_SIZE= $(shell printf "%d" $(CONFIG_SYS_K3_MAX_DOWNLODABLE_IMAGE_SIZE))

ifeq ($(CONFIG_SYS_K3_KEY), "")
KEY=""
# On HS use real key or warn if not available
ifeq ($(CONFIG_TI_SECURE_DEVICE),y)
ifneq ($(wildcard $(TI_SECURE_DEV_PKG)/keys/custMpk.pem),)
KEY=$(TI_SECURE_DEV_PKG)/keys/custMpk.pem
else
$(warning "WARNING: signing key not found. Random key will NOT work on HS hardware!")
endif
endif
else
KEY=$(patsubst "%",$(srctree)/%,$(CONFIG_SYS_K3_KEY))
endif

# tiboot3.bin is mandated by ROM and ROM only supports R5 boot.
# So restrict tiboot3.bin creation for CPU_V7R.
ifdef CONFIG_CPU_V7R
image_check: $(obj)/u-boot-spl.bin FORCE
	@if [ $(IMAGE_SIZE) -gt $(MAX_SIZE) ]; then			    \
		echo "===============================================" >&2; \
		echo "ERROR: Final Image too big. " >&2;		    \
		echo "$< size = $(IMAGE_SIZE), max size = $(MAX_SIZE)" >&2; \
		echo "===============================================" >&2; \
		exit 1;							    \
	fi

tiboot3.bin: image_check FORCE
	$(srctree)/tools/k3_gen_x509_cert.sh -c 16 -b $(obj)/u-boot-spl.bin \
				-o $@ -l $(CONFIG_SPL_TEXT_BASE) -k $(KEY)

ALL-y	+= tiboot3.bin
endif

ifdef CONFIG_ARM64
ifeq ($(CONFIG_TI_SECURE_DEVICE),y)
SPL_ITS := u-boot-spl-k3_HS.its
$(SPL_ITS): FORCE
	IS_HS=1 \
	$(srctree)/tools/k3_fit_atf.sh \
	$(patsubst %,$(obj)/dts/%.dtb,$(subst ",,$(CONFIG_SPL_OF_LIST))) > $@

ALL-y	+= tispl.bin_HS
else
SPL_ITS := u-boot-spl-k3.its
$(SPL_ITS): FORCE
	$(srctree)/tools/k3_fit_atf.sh \
	$(patsubst %,$(obj)/dts/%.dtb,$(subst ",,$(CONFIG_SPL_OF_LIST))) > $@

ALL-y	+= tispl.bin
endif
endif

else

ifeq ($(CONFIG_TI_SECURE_DEVICE),y)
ALL-y	+= u-boot.img_HS
else
ALL-y	+= u-boot.img
endif
endif

include $(srctree)/arch/arm/mach-k3/config_secure.mk
