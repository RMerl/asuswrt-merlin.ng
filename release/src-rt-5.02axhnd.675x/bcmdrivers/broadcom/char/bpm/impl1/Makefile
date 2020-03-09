# File: bcmdrivers/char/bpm/bcm963xx/Makefile
#
# Makefile for the BCM63xx BPM module
#

bcm_bpm-objs   := bpm.o
obj-$(CONFIG_BCM_BPM) += bcm_bpm.o

clean:
	rm -f core *.o *.a *.s

%.o:
	@echo "Using bpm.$(CONFIG_BCM_CHIP_NUMBER)_impl${CONFIG_BCM_WLAN_IMPL}.o_saved"
	$(Q)cp $(obj)/bpm.$(CONFIG_BCM_CHIP_NUMBER)_impl$(CONFIG_BCM_WLAN_IMPL).o_saved $(obj)/bpm.o
