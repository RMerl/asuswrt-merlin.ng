# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2019 Broadcom Ltd
#


# ccflags-y += -save-temps

ifeq ($(FW_CHK_BOARDID),y)
EXTRA_CFLAGS += -DFW_CHK_BOARDID
endif

obj-$(CONFIG_BCMBCA_HTTPD) += bcmbca_net.o
obj-$(CONFIG_BCMBCA_HTTPD) += httpd.o
obj-$(CONFIG_BCMBCA_HTTPD) += uip.o
obj-$(CONFIG_BCMBCA_HTTPD) += uip_arch.o
obj-$(CONFIG_BCMBCA_HTTPD) += uip_arp.o


