obj-$(CONFIG_BCM_BPM) += bcm_bpm.o

bcm_bpm-objs += bcm_bpm_dep.o

$(obj)/bcm_bpm_dep.o:
	cp $(obj)/bcm_bpm.o $(obj)/bcm_bpm_dep.o

	rm -f core *.ko *.o *.a *.s

