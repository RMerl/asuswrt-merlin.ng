obj-$(CONFIG_BCM_BCA_GPIO) += gpio-bcm-bca.o
obj-$(CONFIG_BCM_BCA_GPIO) += bcm_bca_periph_intr.o
EXTRA_CFLAGS += -Werror -Wfatal-errors

# periph intr APIs are originally included in bca extintr driver.
# when use this gpio driver replacing bca extintr driver, include
# the periph intr APIs
ifeq ($(CONFIG_BCM_BCA_EXTINTR),)
EXTRA_CFLAGS += -DPERIPH_INTR
endif

clean:
	rm -f core *.o *.a *.s .*.cmd *.ko
.PHONY: clean
