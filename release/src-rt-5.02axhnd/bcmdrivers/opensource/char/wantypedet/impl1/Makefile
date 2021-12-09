#
# Makefile for the optical wan detection
#

obj-$(CONFIG_BCM_WANTYPEDET) += wantypedet.o
wantypedet-objs += detect_wan_type.o
wantypedet-objs += detect_opt.o
ifeq ($(CONFIG_BCM_PON_WAN_TYPE_AUTO_DETECT),y)
wantypedet-objs += wan_type_detection.o
endif

EXTRA_CFLAGS += $(INC_RDP_FLAGS) -Werror -Wall

-include $(TOPDIR)/Rules.make

clean:
	rm -f core *.o *.a *.s .*.cmd *.ko *.mod.c modules.*
