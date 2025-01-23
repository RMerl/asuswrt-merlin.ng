
obj-$(CONFIG_BCM_TMS) += nciTMSkmod.o
obj-$(CONFIG_BCM_TMS) += nciLservices.o
obj-$(CONFIG_BCM_TMS) += nciexLinuxETH.o

nciTMSkmod-objs := nciTMSkmod$(PRBM_EXT).o
nciLservices-objs := nciLservices$(PRBM_EXT).o
nciexLinuxETH-objs := nciexLinuxETH$(PRBM_EXT).o

clean:
	rm -f core *.o *.a *.s .*.cmd *.ko
