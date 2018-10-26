ifneq ($(strip $(BRCM_CHIP)),47189)
BSPOBJS += bcm63xx_ipc.o
ifdef SEC_ARCH
BSPOBJS += bcm63xx_sec.o
CFLAGS += -DBOARD_SEC_ARCH=$(SEC_ARCH)
ifeq ($(strip $(BLD_SEC_TK)),1)
BSPOBJS += bcm63xx_sec_tk.o bcm63xx_keystore.o
endif

endif
endif


