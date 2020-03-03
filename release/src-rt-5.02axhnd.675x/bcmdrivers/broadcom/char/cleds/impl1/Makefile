

obj-$(CONFIG_BCM_CLEDS_PROC_DRIVER) += bcm_cleds.o

EXTRA_CFLAGS += -I. -I$(src)/../include/ -I$(src)/../../../../../bcmdrivers/opensource/include/bcm963xx/ -I$(src)/../../../../../shared/opensource/include/bcm963xx/

EXTRA_CFLAGS += -Werror -Wfatal-errors 

clean:
	rm -f core *.ko *.o *.a *.s

