# SPDX-License-Identifier: GPL-2.0+

obj-y	:= db-88f6281-bp.o
extra-y := kwbimage.cfg

quiet_cmd_sed = SED     $@
      cmd_sed = sed $(SEDFLAGS_$(@F)) $< >$(dir $<)$(@F)

SEDFLAGS_kwbimage.cfg = -e "s/^\#@BOOT_FROM.*/BOOT_FROM	$(if $(CONFIG_CMD_NAND),nand,spi)/"
$(src)/kwbimage.cfg: $(src)/kwbimage.cfg.in include/autoconf.mk \
		include/config/auto.conf
	$(call if_changed,sed)
