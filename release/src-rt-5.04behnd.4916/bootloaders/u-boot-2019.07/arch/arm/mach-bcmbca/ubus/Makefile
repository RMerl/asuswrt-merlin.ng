BRCM_CHIP = $(patsubst "bcm%",%,$(CONFIG_SYS_SOC))
EXTRA_CFLAGS += -I$(srctree)/arch/$(ARCH)/mach-bcmbca/ubus/$(BRCM_CHIP)

obj-$(CONFIG_$(SPL_TPL_)BCMBCA_UBUS4) += $(if $(wildcard $(srctree)/arch/$(ARCH)/mach-bcmbca/ubus/bcm_ubus4.c),bcm_ubus4.o,bcm_ubus4_preb.o)
obj-$(CONFIG_$(SPL_TPL_)BCMBCA_UBUS4) += $(if $(wildcard $(srctree)/arch/$(ARCH)/mach-bcmbca/ubus/bcm_ubus4_dt.c),bcm_ubus4_dt.o,bcm_ubus4_dt_preb.o)
obj-$(CONFIG_$(SPL_TPL_)BCMBCA_UBUS4) += $(BRCM_CHIP)/bcm_ubus_tbl.o

$(obj)/bcm_ubus4_preb.o:
	cp $(srctree)/arch/$(ARCH)/mach-bcmbca/ubus/bcm_ubus4.o $(obj)/bcm_ubus4_preb.o

$(obj)/bcm_ubus4_dt_preb.o:
	cp $(srctree)/arch/$(ARCH)/mach-bcmbca/ubus/bcm_ubus4_dt.o $(obj)/bcm_ubus4_dt_preb.o
