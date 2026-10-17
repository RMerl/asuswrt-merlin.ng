obj-$(CONFIG_BCM_KQOE) += kqoe.o

kqoe-objs := kqoe_core.o kqoe_dscp.o kqoe_netlink.o

kqoe-objs-$(CONFIG_BCM_ARCHER) += hw/archer.o
kqoe-objs-$(CONFIG_BCM_RDPA) += hw/runner.o
kqoe-objs += $(kqoe-objs-m) $(kqoe-objs-y)

ccflags-y += -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD) \
	     -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD) \
	     $(INC_RDP_FLAGS)
ccflags-y += -Werror -Wall
