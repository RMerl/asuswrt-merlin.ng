# SPDX-License-Identifier:	GPL-2.0+
#
# Copyright (C) 2019 Hannes Schmelzer <oe5hpm@oevsv.at> -
# B&R Industrial Automation GmbH - http://www.br-automation.com
#

hw-platform-y :=$(shell echo $(CONFIG_DEFAULT_DEVICE_TREE) | sed -e 's/imx6dl-//')

payload_off :=$(shell printf "%d" $(CONFIG_SYS_SPI_U_BOOT_OFFS))

quiet_cmd_prodbin = PRODBIN $@ $(payload_off)
cmd_prodbin =								\
	dd if=/dev/zero ibs=1M count=2 2>/dev/null | tr "\000" "\377" >$@ && \
	dd conv=notrunc bs=1 if=SPL of=$@ seek=1024 2>/dev/null && \
	dd bs=1 if=u-boot-dtb.img of=$@ seek=$(payload_off) 2>/dev/null

quiet_cmd_prodzip = SAPZIP  $@
cmd_prodzip =					\
	test -d misc && rm -r misc;		\
	mkdir misc &&				\
	cp SPL misc/ &&				\
	cp u-boot-dtb.img misc/ &&		\
	zip -9 -r $@ misc/* >/dev/null $<

ifeq ($(hw-platform-y),brppt2)
ifneq ($(CONFIG_SPL_BUILD),y)
INPUTS-y += $(hw-platform-y)_prog.bin
INPUTS-y += $(hw-platform-y)_prod.zip
endif
endif

$(hw-platform-y)_prog.bin: u-boot-dtb.img spl SPL
	$(call if_changed,prodbin)

$(hw-platform-y)_prod.zip: $(hw-platform-y)_prog.bin
	$(call if_changed,prodzip)
