

ifeq ($(strip $(CONFIG_BRCM_IKOS)),)

obj-$(CONFIG_BCM963138) += bcm_pinmux_impl1.o
obj-$(CONFIG_BCM963138) += bcm_led_impl1.o
obj-$(CONFIG_BCM963138) += bcm_gpio_impl1.o
obj-$(CONFIG_BCM963138) += bcm_misc_hw_init_impl1.o
obj-$(CONFIG_BCM963138) += bcm_otp_impl1.o

obj-$(CONFIG_BCM963148) += bcm_pinmux_impl1.o
obj-$(CONFIG_BCM963148) += bcm_led_impl1.o
obj-$(CONFIG_BCM963148) += bcm_gpio_impl1.o
obj-$(CONFIG_BCM963148) += bcm_misc_hw_init_impl1.o
obj-$(CONFIG_BCM963148) += bcm_otp_impl1.o

obj-$(CONFIG_BCM963381) += bcm_gpio_impl1.o
obj-$(CONFIG_BCM963381) += bcm_pinmux_impl1.o
obj-$(CONFIG_BCM963381) += bcm_led_impl1.o
obj-$(CONFIG_BCM963381) += bcm_misc_hw_init_impl2.o
obj-$(CONFIG_BCM963381) += bcm_otp_impl1.o

obj-$(CONFIG_BCM960333) += bcm_led_impl2.o
obj-$(CONFIG_BCM960333) += bcm_pinmux_impl2.o
obj-$(CONFIG_BCM960333) += bcm_gpio_impl2.o
obj-$(CONFIG_BCM960333) += bcm_misc_hw_init_impl4.o

obj-$(CONFIG_BCM96838) += bcm_misc_hw_init_impl3.o
obj-$(CONFIG_BCM96838) += wan_drv_impl1.o

obj-$(CONFIG_BCM96848) += bcm_gpio_impl1.o
obj-$(CONFIG_BCM96848) += bcm_pinmux_impl1.o
obj-$(CONFIG_BCM96848) += bcm_led_impl1.o
obj-$(CONFIG_BCM96848) += bcm_misc_hw_init_impl5.o
obj-$(CONFIG_BCM96848) += wan_drv_impl2.o
obj-$(CONFIG_BCM96848) += bcm_otp_impl1.o

obj-$(CONFIG_BCM96858) += bcm_gpio_impl1.o
obj-$(CONFIG_BCM96858) += bcm_led_impl1.o
obj-$(CONFIG_BCM96858) += bcm_otp_impl1.o
obj-$(CONFIG_BCM96858) += bcm_pinmux_impl1.o
obj-$(CONFIG_BCM96858) += bcm_misc_hw_init_impl6.o
obj-$(CONFIG_BCM96858) += bcm_ubus_impl1.o

obj-$(CONFIG_BCM94908) += bcm_gpio_impl1.o
obj-$(CONFIG_BCM94908) += bcm_led_impl1.o
obj-$(CONFIG_BCM94908) += bcm_pinmux_impl1.o
obj-$(CONFIG_BCM94908) += bcm_misc_hw_init_impl6.o
obj-$(CONFIG_BCM94908) += bcm_otp_impl1.o

obj-$(CONFIG_BCM947189) += bcm_gpio_impl3.o
obj-$(CONFIG_BCM947189) += bcm_led_impl3.o
obj-$(CONFIG_BCM947189) += bcm_misc_hw_init_impl7.o
obj-$(CONFIG_BCM947189) += bcm_bbsi.o

obj-$(CONFIG_BCM963158) += bcm_gpio_impl1.o
obj-$(CONFIG_BCM963158) += bcm_led_impl1.o
obj-$(CONFIG_BCM963158) += bcm_otp_impl1.o
obj-$(CONFIG_BCM963158) += bcm_pinmux_impl1.o
obj-$(CONFIG_BCM963158) += bcm_misc_hw_init_impl6.o
obj-$(CONFIG_BCM963158) += bcm_ubus_impl1.o

obj-$(CONFIG_BCM96846) += bcm_gpio_impl1.o
obj-$(CONFIG_BCM96846) += bcm_led_impl1.o
obj-$(CONFIG_BCM96846) += bcm_otp_impl1.o
obj-$(CONFIG_BCM96846) += bcm_pinmux_impl1.o
obj-$(CONFIG_BCM96846) += bcm_misc_hw_init_impl6.o
obj-$(CONFIG_BCM96846) += bcm_ubus_impl1.o

obj-$(CONFIG_BCM96856) += bcm_gpio_impl1.o
obj-$(CONFIG_BCM96856) += bcm_led_impl1.o
obj-$(CONFIG_BCM96856) += bcm_otp_impl1.o
obj-$(CONFIG_BCM96856) += bcm_misc_hw_init_impl6.o
obj-$(CONFIG_BCM96856) += bcm_pinmux_impl1.o
obj-$(CONFIG_BCM96856) += bcm_ubus_impl1.o

EXTRA_CFLAGS += -DCONFIG_BCM9$(BRCM_CHIP) -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD)  -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)

ifeq ($(findstring _$(strip $(BRCM_CHIP))_,_6838_6848_6858_63138_63148_4908_47189_6836_63158_6846_6856_),_$(strip $(BRCM_CHIP))_)
	EXTRA_CFLAGS += -I$(INC_BRCMSHARED_PUB_PATH)/drv/mdio
	EXTRA_CFLAGS += -I$(INC_BRCMSHARED_PUB_PATH)/drv/unimac
	EXTRA_CFLAGS += -I$(INC_BRCMSHARED_PUB_PATH)/drv/phys
	EXTRA_CFLAGS += -I$(INC_BRCMSHARED_PUB_PATH)/pmc
	EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PRIV_PATH)/bcm963xx
	EXTRA_CFLAGS += -I$(INC_BRCMSHARED_PRIV_PATH)/bcm963xx
	EXTRA_CFLAGS += $(INC_RDP_FLAGS)
endif 

ifeq ($(findstring _$(strip $(BRCM_CHIP))_,_6838_6848_6858_6836_63158_6846_6856_),_$(strip $(BRCM_CHIP))_)
obj-y += wan_drv_shared.o
endif

obj-y += bcm_nvram_data_impl.o

-include $(TOPDIR)/Rules.make

endif

