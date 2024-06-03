# File: bcmdrivers/broadcom/char/tcpspdtest/impl1/Makefile
#
# Makefile for the BCM63xx Tcp Speed Test driver.
#

obj-$(CONFIG_BCM_SPDTEST) += bcm_tcpspdtest.o
bcm_tcpspdtest-objs += tcpspdtest.o rnr_flow.o tcp_engine.o sock_mgr.o prtcl.o genl_msgs.o tcp_buff_pool.o
ifneq ($(strip $(CONFIG_BCM_XRDP)),)
bcm_tcpspdtest-objs += rnr_engine.o udp_engine.o
endif

# Uncomment to enable profiling (code coverage / hot-spot location / branch statistics)
#GCOV_PROFILE := y

EXTRA_CFLAGS += $(INC_RDP_FLAGS)
EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -Werror -Wfatal-errors

-include $(TOPDIR)/Rules.make

clean:
	rm -f core *.ko *.o *.a *.s


# Makefile trick: breaking the following into two rules allows
# the "%.o : %.c" rule (defined earlier by kbuild), to take precidence

%.o : %.o_tmp
	$(Q)mv $< $@

%.o_tmp : %.$(BRCM_CHIP).o_saved
	@echo "Using $<"
	$(Q)cp $< $@

