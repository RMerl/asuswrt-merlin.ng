BRCM_CHIP = $(patsubst "bcm%",%,$(CONFIG_SYS_SOC))
EXTRA_CFLAGS += -I$(srctree)/arch/$(ARCH)/mach-bcmbca/ubus/$(BRCM_CHIP)

obj-$(CONFIG_$(SPL_TPL_)BCMBCA_UBUS4) +=	\
	bcm_ubus4.o					\
	bcm_ubus4_dt.o				\
	$(BRCM_CHIP)/bcm_ubus_tbl.o
