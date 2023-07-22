include ../../.config

DEDUPTX_OBJS := deDupTx.o

obj-$(CONFIG_NETFILTER_ROUTER_BOOST) += $(DEDUPTX_OBJS)


#ifneq ($(CONFIG_BCM_CHIP_NUMBER),)
# CFLAGS_MODULE += -DHAVE_BRCM_FC
# SRCBASE := ../..
# CFLAGS_MODULE += -I$(SRCBASE)/shared/opensource/include/bcm963xx
# CFLAGS_MODULE += -I$(SRCBASE)/bcmdrivers/broadcom/include/bcm963xx
# CFLAGS_MODULE += -I$(SRCBASE)/bcmdrivers/opensource/include/bcm963xx
#else
# CFLAGS_MODULE += -DNO_BRCM_FC
#endif

CFLAGS_MODULE += -DBUILD_NAME=\"$(BUILD_NAME)\"

clean-files += *.o .*.o.cmd
