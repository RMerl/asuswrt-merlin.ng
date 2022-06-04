
bcm_pcie_hcd-objs += bcm_pcie_hcd$(PRBM_EXT).o

obj-$(CONFIG_BCM_PCIE_HCD) += bcm_pcie_hcd.o

clean:
	rm -f core *.o *.a *.s .*.o .*.cmd *.ko modules.*
	rm -f arm/*.o arm/.*.cmd
	rm -f common/*.o common/.*.cmd
	rm -f mips/*.o mips/.*.cmd

