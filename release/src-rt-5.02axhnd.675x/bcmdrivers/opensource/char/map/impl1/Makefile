#ivi-objs    :=  ivi_rule.o ivi_rule6.o ivi_map.o ivi_map_tcp.o ivi_xmit.o ivi_nf.o ivi_ioctl.o ivi_module.o ivi_portmap.o
ivi-objs    :=  ivi_rule.o ivi_rule6.o ivi_map.o ivi_xmit.o ivi_nf.o ivi_ioctl.o ivi_module.o ivi_portmap.o

obj-$(CONFIG_BCM_MAP) += ivi.o

EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -Werror -Wall

clean:
	rm -f core *.o *.a *.s .*.cmd *.ko

