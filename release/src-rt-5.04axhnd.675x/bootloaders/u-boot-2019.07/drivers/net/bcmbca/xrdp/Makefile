# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2019 Broadcom Ltd
#

CHIP_ID = $(patsubst "bcm%",%,$(CONFIG_SYS_SOC))
KBUILD_CPPFLAGS += -DCONFIG_BCM9$(CHIP_ID)
KBUILD_CPPFLAGS += -Werror

obj-y += \
	bcmbca_xrdp_api.o \
	access_logging.o \
	data_path_$(CHIP_ID).o

ifdef CONFIG_BCMBCA_XRDP_PON
KBUILD_CPPFLAGS += -DXRDP_SBPM -DGEN45
obj-y += rdp_drv_sbpm.o
else # CONFIG_BCMBCA_XRDP_DSL
KBUILD_CPPFLAGS += -DXRDP_DIRECT
endif
