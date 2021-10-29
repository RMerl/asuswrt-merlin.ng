#
# Makefile for the optical wan detection
#
ifneq ($(PREBUILT_EXTRAMOD),1)
obj-$(CONFIG_BCM_OPTICALDET) += opticaldet.o
opticaldet-objs += detect_dev.o
opticaldet-objs += detect_dev_trx_data.o
opticaldet-objs += trx_activation.o
else
obj-$(CONFIG_BCM_OPTICALDET) += opticaldet$(PRBM_EXT).o
endif

EXTRA_CFLAGS += $(INC_RDP_FLAGS) -Werror -Wall

-include $(TOPDIR)/Rules.make

clean:
	rm -f core *.o *.a *.s .*.cmd *.ko *.mod.c modules.*
