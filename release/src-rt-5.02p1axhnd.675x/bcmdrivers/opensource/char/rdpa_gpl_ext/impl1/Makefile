
rdpa_gpl_ext-objs += rdpa_gpl_ext$(PRBM_EXT).o

obj-$(CONFIG_BCM_RDPA_GPL_EXT) += rdpa_gpl_ext.o

clean:
	rm -rf `find . -name '*.[o,d]'`
	rm -rf `find . -name '*~'`
	rm -rf `find . -name '*.ko'`
	rm -rf rdpa_gpl_ext.mod.c modules.order Module.symvers .tmp_versions
	rm -rf `find . -name '.*.cmd'`
	rm -rf rdpa

