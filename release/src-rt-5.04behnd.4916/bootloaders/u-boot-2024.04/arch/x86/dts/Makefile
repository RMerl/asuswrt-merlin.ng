# SPDX-License-Identifier: GPL-2.0+

dtb-y += bayleybay.dtb \
	cherryhill.dtb \
	chromebook_coral.dtb \
	chromebook_link.dtb \
	chromebox_panther.dtb \
	chromebook_samus.dtb \
	conga-qeval20-qa3-e3845.dtb \
	coreboot.dtb \
	cougarcanyon2.dtb \
	crownbay.dtb \
	dfi-bt700-q7x-151.dtb \
	edison.dtb \
	efi-x86_app.dtb \
	efi-x86_payload.dtb \
	galileo.dtb \
	minnowmax.dtb \
	qemu-x86_i440fx.dtb \
	qemu-x86_q35.dtb \
	theadorable-x86-dfi-bt700.dtb \
	slimbootloader.dtb \
	baytrail_som-db5800-som-6867.dtb

include $(srctree)/scripts/Makefile.dts

targets += $(dtb-y)

DTC_FLAGS += -R 4 -p $(if $(CONFIG_EFI_APP),0x8000,0x1000)

PHONY += dtbs
dtbs: $(addprefix $(obj)/, $(dtb-y))
	@:

clean-files := *.dtb
