include $(DTS_DIR)/dts/make.dsl

9$(CONFIG_BCM_CHIP_NUMBER).dtb := $(shell cp -f $(KERNEL_DIR)/include/generated/autoconf.h $(DTB_DIR)/linux_autoconf.h; \
	echo -e $(DSL_DEFS) > $(DTS_DIR)/dts/dslmemdef.h)

dtb-y := $(patsubst $(src)/%.dts,%.dtb, $(wildcard $(src)/*.dts))
always	:=  $(dtb-y)

.PHONY: dtbs_clean
dtbs_clean:
	rm -vf *.dtb .*.tmp .*.cmd linux_autoconf.h $(DTS_DIR)/dts/dslmemdef.h

