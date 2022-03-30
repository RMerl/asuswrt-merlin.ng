obj-$(CONFIG_BCM_NAT46) += nat46.o
nat46-objs := nat46-netdev.o nat46-module.o nat46-core.o nat46-glue.o
CFLAGS_nat46.o := -DDEBUG

EXTRA_CFLAGS += -I$(SHARED_DIR)/opensource/include/bcm963xx
EXTRA_CFLAGS += -Werror -Wall
EXTRA_CFLAGS += -Wno-error=date-time

clean:
	rm -f core *.o *.a *.s .*.cmd *.ko modules.builtin modules.order
