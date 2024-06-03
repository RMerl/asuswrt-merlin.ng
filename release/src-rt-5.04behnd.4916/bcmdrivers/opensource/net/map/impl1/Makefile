obj-$(CONFIG_BCM_MAP) += nat46.o
nat46-objs := nat46-netdev.o nat46-module.o nat46-core.o nat46-glue.o
CFLAGS_nat46.o := -DDEBUG

# Git commit sourced from https://github.com/ayourtch/nat46.git
PKG_SOURCE_VERSION := 4c5beee

# Changes made by Broadcom since nat46 is included in devel
BCM_CHANGE_REVISION := 24

EXTRA_CFLAGS += -I$(SHARED_DIR)/opensource/include/bcm963xx
EXTRA_CFLAGS += -Werror -Wall
EXTRA_CFLAGS += -DNAT46_VERSION=\"$(PKG_SOURCE_VERSION).$(BCM_CHANGE_REVISION)\"

clean:
	rm -f core *.o *.a *.s .*.cmd *.ko modules.builtin modules.order
