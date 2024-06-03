# File: bcmdrivers/opensource/char/fpi/impl1/Makefile
#
# Makefile for the Flow Provisioning Interface module
#

bcm_fpi-objs := fpi.o
bcm_fpi_blog-objs := fpi_blog.o

obj-$(CONFIG_BCM_FPI) += bcm_fpi.o
obj-$(CONFIG_BCM_FPI_BLOG) += bcm_fpi_blog.o

EXTRA_CFLAGS += -I. -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BCMLIBS_PATH)

EXTRA_CFLAGS += -Werror -Wfatal-errors 

clean:
	rm -f core *.ko *.o *.a *.s



