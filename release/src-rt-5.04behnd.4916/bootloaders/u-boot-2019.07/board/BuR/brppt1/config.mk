#
# Copyright (C) 2018 Hannes Schmelzer <oe5hpm@oevsv.at> -
# B&R Industrial Automation GmbH - http://www.br-automation.com
#
# SPDX-License-Identifier:	GPL-2.0+
#

hw-platform-y :=$(shell echo $(CONFIG_DEFAULT_DEVICE_TREE) | sed -e 's/am335x-//')

payload_off :=$(shell printf "%d" $(CONFIG_SYS_SPI_U_BOOT_OFFS))

quiet_cmd_prodbin = PRODBIN $@ $(payload_off)
cmd_prodbin =								\
	dd if=/dev/zero ibs=1M count=2 2>/dev/null | tr "\000" "\377" >$@ && \
	dd conv=notrunc bs=1 if=MLO.byteswap of=$@ seek=0 2>/dev/null && \
	dd bs=1 if=u-boot-dtb.img of=$@ seek=$(payload_off) 2>/dev/null

quiet_cmd_prodzip = SAPZIP  $@
cmd_prodzip =					\
	test -d misc && rm -r misc;		\
	mkdir misc &&				\
	cp MLO.byteswap misc/ &&		\
	cp spl/u-boot-spl.bin misc/ &&		\
	cp u-boot-dtb.img misc/ &&		\
	zip -9 -r $@ misc/* >/dev/null $<

ifeq ($(hw-platform-y),brppt1-spi)
ALL-y += $(hw-platform-y)_prog.bin
ALL-y += $(hw-platform-y)_prod.zip
endif

$(hw-platform-y)_prog.bin: u-boot-dtb.img spl/u-boot-spl.bin
	$(call if_changed,prodbin)

$(hw-platform-y)_prod.zip: $(hw-platform-y)_prog.bin
	$(call if_changed,prodzip)
