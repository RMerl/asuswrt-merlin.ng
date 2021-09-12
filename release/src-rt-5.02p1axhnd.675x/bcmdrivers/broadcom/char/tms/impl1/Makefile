
nciTMSkmod-objs += nciTMSkmod$(PRBM_EXT).o
nciLservices-objs += nciLservices$(PRBM_EXT).o
nciexLinuxETH-objs += nciexLinuxETH$(PRBM_EXT).o

obj-$(CONFIG_BCM_TMS) += nciTMSkmod.o
obj-$(CONFIG_BCM_TMS) += nciLservices.o
obj-$(CONFIG_BCM_TMS) += nciexLinuxETH.o

clean:
	rm -f core *.ko *.o *.a *.s

