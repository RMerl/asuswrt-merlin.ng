# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2000-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.

#ccflags-y += -DDEBUG

ifeq ($(FW_CHK_BOARDID),y)
EXTRA_CFLAGS += -DFW_CHK_BOARDID
endif

obj-y += checksum.o
obj-$(CONFIG_NET)	+= $(if $(wildcard $(srctree)/net/arp.c),arp.o,arp_preb.o)
obj-$(CONFIG_CMD_BOOTP) += bootp.o
obj-$(CONFIG_CMD_CDP)  += cdp.o
obj-$(CONFIG_CMD_DNS)  += dns.o
ifdef CONFIG_DM_ETH
obj-$(CONFIG_NET)      += eth-uclass.o
else
obj-$(CONFIG_NET)      += eth_legacy.o
endif
obj-$(CONFIG_NET)      += eth_common.o
obj-$(CONFIG_CMD_LINK_LOCAL) += link_local.o
obj-$(CONFIG_NET)	+= $(if $(wildcard $(srctree)/net/net.c),net.o,net_preb.o)
obj-$(CONFIG_CMD_NFS)  += nfs.o
obj-$(CONFIG_CMD_PING)	+= $(if $(wildcard $(srctree)/net/ping.c),ping.o,ping_preb.o)
obj-$(CONFIG_CMD_RARP) += rarp.o
obj-$(CONFIG_CMD_SNTP) += sntp.o
obj-$(CONFIG_CMD_TFTPBOOT) += $(if $(wildcard $(srctree)/net/tftp.c),tftp.o,tftp_preb.o)
obj-$(CONFIG_UDP_FUNCTION_FASTBOOT)  += fastboot.o
obj-$(CONFIG_CMD_WOL)  += wol.o

# Disable this warning as it is triggered by:
# sprintf(buf, index ? "foo%d" : "foo", index)
# and this is intentional usage.
CFLAGS_eth_common.o += -Wno-format-extra-args

$(obj)/arp_preb.o:
	cp $(srctree)/net/arp.o $(obj)/arp_preb.o

$(obj)/net_preb.o:
	cp $(srctree)/net/net.o $(obj)/net_preb.o

$(obj)/ping_preb.o:
	cp $(srctree)/net/ping.o $(obj)/ping_preb.o

$(obj)/tftp_preb.o:
	cp $(srctree)/net/tftp.o $(obj)/tftp_preb.o
