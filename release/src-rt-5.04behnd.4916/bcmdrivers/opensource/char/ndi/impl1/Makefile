obj-$(CONFIG_BCM_NDI) += ndi.o

ndi-objs := ndi_core.o ndi_tables.o ndi_netlink.o ndi_dhcp.o ndi_arp.o

ccflags-y += -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)
ccflags-y += -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD)
ccflags-y += -Werror -Wall
