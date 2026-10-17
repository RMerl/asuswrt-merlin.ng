obj-$(CONFIG_BCM_FTPM_SMC_DRV) += ftpm_smc_drv.o
EXTRA_CFLAGS += -Werror -Wfatal-errors
EXTRA_CFLAGS += -I. 
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/broadcom/char/tpm/impl$(CONFIG_BCM_FTPM_SMC_DRV_IMPL)/$(BRCM_CHIP)

bcm_headers_install:
	echo BCMLIBS installing header
	echo $(INSTALL_HEADERS_WITH_CP) ./*.h $(INC_BCMDRIVER_PATH)/include/
	$(INSTALL_HEADERS_WITH_CP) ./*.h $(INC_BCMDRIVER_PATH)/include/

clean:
	rm -f core *.o *.a *.s .*.cmd *.ko
.PHONY: clean bcm_headers_install
