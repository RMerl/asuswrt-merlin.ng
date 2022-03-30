#
# Makefile for the optical wan detection
#

obj-$(CONFIG_BCM_PON_WAN_TYPE_AUTO_DETECT) := wantypedet.o 
wantypedet-objs += detect_wan_type.o
 
 
wantypedet-objs += wan_type_detection.o

ifneq ($(strip $(CONFIG_BCM_PMD)),)
    EXTRA_CFLAGS += -DCONFIG_BCM_PMD
endif

EXTRA_CFLAGS += $(INC_RDP_FLAGS) -Werror -Wall
-include $(TOPDIR)/Rules.make

clean:
	rm -f core *.o *.a *.s .*.cmd *.ko *.mod.c modules.*
