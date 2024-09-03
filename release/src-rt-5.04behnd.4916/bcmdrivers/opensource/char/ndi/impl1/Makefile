obj-$(CONFIG_BCM_NDI) += ndi.o

ndi-objs := ndi_arp.o ndi_core.o ndi_dhcp.o ndi_netlink.o ndi_tables.o

ccflags-y += -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)
ccflags-y += -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD)
ccflags-y += -Werror -Wall
