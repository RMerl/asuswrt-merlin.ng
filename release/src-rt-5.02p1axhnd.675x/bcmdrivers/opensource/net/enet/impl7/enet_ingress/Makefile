#
# Makefile for the enet_ingress test driver
#

NAME = bcm_enet_ingress

ifneq (,$(filter $(BRCM_CHIP),6858 6846 6856 6878))
obj-$(CONFIG_BCM_ENET) = $(NAME).o
endif

EXTRA_CFLAGS += -DTEST_INGRESS

ENET_DIR = $(BRCMDRIVERS_DIR)/opensource/net/enet/bcm9$(BRCM_CHIP)

$(NAME)-objs += enet_ingress.o
enet_objs = port.o port_types.o mux_index.o

EXTRA_CFLAGS += -I$(ENET_DIR) -I$(INC_BRCMSHARED_PUB_PATH)/drv -I$(SHARED_DIR)/opensource 

ifneq ($(strip $(CONFIG_BCM_RDPA)),)
EXTRA_CFLAGS += $(INC_RDP_FLAGS)
EXTRA_CFLAGS += -DRUNNER
enet_objs += runner.o runner_standalone.o
endif

ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_6858_6846_6856_6878_),)
RDP_RING = 1
endif

enet_objs += rdp_ring.o

EXTRA_CFLAGS += -DENET_INT_COALESCING_ENABLE -DENET_INTERRUPT_COALESCING_TIMEOUT_US=500 -DENET_INTERRUPT_COALESCING_MAX_PKT_CNT=32 

ifneq ($(strip $(CONFIG_BCM_FTTDP_G9991)),)
enet_objs += g9991.o
endif

ifneq ($(strip $(CONFIG_BCM_GPON)),)
EXTRA_CFLAGS += -DGPON
endif
ifneq ($(strip $(CONFIG_BCM_EPON)),)
EXTRA_CFLAGS += -DEPON
endif

-include $(TOPDIR)/Rules.make

EXTRA_CFLAGS += -Werror -Wall


$(NAME)-objs += $(enet_objs)

ifneq (,$(filter $(BRCM_CHIP),6858 6846 6856 6878))
SHARED_TARGETS = $(addprefix $(src)/,$(enet_objs:.o=.c))
endif
$(SHARED_TARGETS):
	ln -sf $(patsubst %.o, %.c,$(ENET_DIR)/$(notdir $@)) $(patsubst %.o, %.c,$@)

clean:
	rm -f core *.o *.a *.s *.ko .*.cmd .*.o.d Module.symvers modules.builtin modules.order *.mod.c
	rm -rf .tmp_versions
	find -type l -delete

.PHONY: clean 

