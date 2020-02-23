BSPOBJS += bcm63xx_storage_dev.o bcm63xx_common.o
ifneq ($(strip $(BRCM_CHIP)),47189)
BSPOBJS += bcm63xx_ipc.o

ifdef SEC_ARCH
BSPOBJS += bcm63xx_sec.o
CFLAGS += -DBOARD_SEC_ARCH=$(SEC_ARCH)
endif

ifneq ($(strip $(CONFIG_CFE_FAILSAFE_BOOT)),)
BSPOBJS += bcm63xx_boot_state.o
endif

ifeq ($(strip $(BLD_SEC_TK)),1)
BSPOBJS += bcm63xx_sec_tk.o bcm63xx_keystore.o otp_map.o
endif

endif


