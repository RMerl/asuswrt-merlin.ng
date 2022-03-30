#
# Makefile for the bcm bootstate driver
#

obj-$(CONFIG_BCM_BOOTSTATE) += bcm_bootstate.o

EXTRA_CFLAGS += -I. -I$(src)/../include/
EXTRA_CFLAGS += -Werror -Wfatal-errors

clean:
	rm -f core *.ko *.o *.a *.s
