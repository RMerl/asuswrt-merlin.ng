# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2004-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.

ifndef CONFIG_SPL_BUILD
# core command
obj-y += boot.o
obj-$(CONFIG_CMD_BOOTM) += bootm.o
obj-y += help.o
obj-y += panic.o
obj-y += version.o

# command
obj-$(CONFIG_CMD_ARMFFA) += armffa.o
obj-$(CONFIG_CMD_2048) += 2048.o
obj-$(CONFIG_CMD_ACPI) += acpi.o
obj-$(CONFIG_CMD_ADDRMAP) += addrmap.o
obj-$(CONFIG_CMD_AES) += aes.o
obj-$(CONFIG_CMD_AB_SELECT) += ab_select.o
obj-$(CONFIG_CMD_ADC) += adc.o
obj-$(CONFIG_CMD_ARMFLASH) += armflash.o
obj-$(CONFIG_BLK) += blk_common.o
obj-$(CONFIG_CMD_BOOTDEV) += bootdev.o
obj-$(CONFIG_CMD_BOOTFLOW) += bootflow.o
obj-$(CONFIG_CMD_BOOTMETH) += bootmeth.o
obj-$(CONFIG_CMD_SOURCE) += source.o
obj-$(CONFIG_CMD_BCB) += bcb.o
obj-$(CONFIG_CMD_BDI) += bdinfo.o
obj-$(CONFIG_CMD_BIND) += bind.o
obj-$(CONFIG_CMD_BINOP) += binop.o
obj-$(CONFIG_CMD_BLKMAP) += blkmap.o
obj-$(CONFIG_CMD_BLOBLIST) += bloblist.o
obj-$(CONFIG_CMD_BLOCK_CACHE) += blkcache.o
obj-$(CONFIG_CMD_BMP) += bmp.o
obj-$(CONFIG_CMD_BOOTCOUNT) += bootcount.o
obj-$(CONFIG_CMD_BOOTEFI) += bootefi.o
obj-$(CONFIG_CMD_BOOTMENU) += bootmenu.o
obj-$(CONFIG_CMD_BOOTSTAGE) += bootstage.o
obj-$(CONFIG_CMD_BOOTZ) += bootz.o
obj-$(CONFIG_CMD_BOOTI) += booti.o
obj-$(CONFIG_CMD_BTRFS) += btrfs.o
obj-$(CONFIG_CMD_BUTTON) += button.o
obj-$(CONFIG_CMD_CAT) += cat.o
obj-$(CONFIG_CMD_CACHE) += cache.o
obj-$(CONFIG_CMD_CBFS) += cbfs.o
obj-$(CONFIG_CMD_CEDIT) += cedit.o
obj-$(CONFIG_CMD_CLK) += clk.o
obj-$(CONFIG_CMD_CLS) += cls.o
obj-$(CONFIG_CMD_CONFIG) += config.o
obj-$(CONFIG_CMD_CONITRACE) += conitrace.o
obj-$(CONFIG_CMD_CONSOLE) += console.o
obj-$(CONFIG_CMD_CPU) += cpu.o
obj-$(CONFIG_CMD_DATE) += date.o
obj-$(CONFIG_CMD_DEMO) += demo.o
obj-$(CONFIG_CMD_DM) += dm.o
obj-$(CONFIG_CMD_SOUND) += sound.o
ifdef CONFIG_POST
obj-$(CONFIG_CMD_DIAG) += diag.o
endif
obj-$(CONFIG_CMD_ADTIMG) += adtimg.o
obj-$(CONFIG_CMD_ABOOTIMG) += abootimg.o
obj-$(CONFIG_CMD_CYCLIC) += cyclic.o
obj-$(CONFIG_CMD_EVENT) += event.o
obj-$(CONFIG_CMD_EXTENSION) += extension_board.o
obj-$(CONFIG_CMD_ECHO) += echo.o
obj-$(CONFIG_ENV_IS_IN_EEPROM) += eeprom.o
obj-$(CONFIG_CMD_EEPROM) += eeprom.o
obj-$(CONFIG_EFI) += efi.o efi_common.o
obj-$(CONFIG_CMD_EFIDEBUG) += efidebug.o efi_common.o
obj-$(CONFIG_CMD_EFICONFIG) += eficonfig.o
ifdef CONFIG_CMD_EFICONFIG
ifdef CONFIG_EFI_MM_COMM_TEE
obj-$(CONFIG_EFI_SECURE_BOOT) += eficonfig_sbkey.o
endif
endif
obj-$(CONFIG_CMD_ELF) += elf.o
obj-$(CONFIG_CMD_EROFS) += erofs.o
obj-$(CONFIG_HUSH_PARSER) += exit.o
obj-$(CONFIG_CMD_EXT4) += ext4.o
obj-$(CONFIG_CMD_EXT2) += ext2.o
obj-$(CONFIG_CMD_FAT) += fat.o
obj-$(CONFIG_CMD_FDT) += fdt.o
obj-$(CONFIG_CMD_SQUASHFS) += sqfs.o
obj-$(CONFIG_CMD_SELECT_FONT) += font.o
obj-$(CONFIG_CMD_FLASH) += flash.o
obj-$(CONFIG_CMD_FPGA) += fpga.o
obj-$(CONFIG_CMD_FPGAD) += fpgad.o
obj-$(CONFIG_CMD_FS_GENERIC) += fs.o
obj-$(CONFIG_CMD_FUSE) += fuse.o
obj-$(CONFIG_CMD_FWU_METADATA) += fwu_mdata.o
obj-$(CONFIG_CMD_GETTIME) += gettime.o
obj-$(CONFIG_CMD_GPIO) += gpio.o
obj-$(CONFIG_CMD_HISTORY) += history.o
obj-$(CONFIG_CMD_HVC) += smccc.o
obj-$(CONFIG_CMD_I2C) += i2c.o
obj-$(CONFIG_CMD_IOTRACE) += iotrace.o
obj-$(CONFIG_CMD_HASH) += hash.o
obj-$(CONFIG_CMD_IDE) += ide.o disk.o
obj-$(CONFIG_CMD_INI) += ini.o
obj-$(CONFIG_CMD_IRQ) += irq.o
obj-$(CONFIG_CMD_ITEST) += itest.o
obj-$(CONFIG_CMD_JFFS2) += jffs2.o
obj-$(CONFIG_CMD_CRAMFS) += cramfs.o
obj-$(CONFIG_LED_STATUS_CMD) += legacy_led.o
obj-$(CONFIG_CMD_LED) += led.o
obj-$(CONFIG_CMD_LICENSE) += license.o
obj-y += load.o
obj-$(CONFIG_CMD_LOG) += log.o
obj-$(CONFIG_CMD_LSBLK) += lsblk.o
obj-$(CONFIG_CMD_MD5SUM) += md5sum.o
obj-$(CONFIG_CMD_MEMORY) += mem.o
obj-$(CONFIG_CMD_IO) += io.o
obj-$(CONFIG_CMD_MII) += mii.o
obj-$(CONFIG_CMD_MISC) += misc.o
obj-$(CONFIG_CMD_MDIO) += mdio.o
obj-$(CONFIG_CMD_PAUSE) += pause.o
obj-$(CONFIG_CMD_SLEEP) += sleep.o
obj-$(CONFIG_CMD_MMC) += mmc.o
obj-$(CONFIG_CMD_OPTEE_RPMB) += optee_rpmb.o
obj-$(CONFIG_CMD_MP) += mp.o
obj-$(CONFIG_CMD_MTD) += mtd.o
obj-$(CONFIG_CMD_MTDPARTS) += mtdparts.o
obj-$(CONFIG_CMD_CLONE) += clone.o
ifneq ($(CONFIG_CMD_NAND)$(CONFIG_CMD_SF),)
obj-y += legacy-mtd-utils.o
endif
obj-$(CONFIG_CMD_MUX) += mux.o
obj-$(CONFIG_CMD_NAND) += nand.o
obj-$(CONFIG_CMD_NET) += net.o
obj-$(CONFIG_ENV_SUPPORT) += nvedit.o
obj-$(CONFIG_CMD_NVEDIT_EFI) += nvedit_efi.o
obj-$(CONFIG_CMD_ONENAND) += onenand.o
obj-$(CONFIG_CMD_OSD) += osd.o
obj-$(CONFIG_CMD_PART) += part.o
obj-$(CONFIG_CMD_PCAP) += pcap.o
ifdef CONFIG_PCI
obj-$(CONFIG_CMD_PCI) += pci.o
obj-$(CONFIG_CMD_PCI_MPS) += pci_mps.o
endif
obj-$(CONFIG_CMD_PINMUX) += pinmux.o
obj-$(CONFIG_CMD_PMC) += pmc.o
obj-$(CONFIG_CMD_PSTORE) += pstore.o
obj-$(CONFIG_CMD_PWM) += pwm.o
obj-$(CONFIG_CMD_PXE) += pxe.o
obj-$(CONFIG_CMD_WOL) += wol.o
obj-$(CONFIG_CMD_QFW) += qfw.o
obj-$(CONFIG_CMD_READ) += read.o
obj-$(CONFIG_CMD_WRITE) += read.o
obj-$(CONFIG_CMD_REGINFO) += reginfo.o
obj-$(CONFIG_CMD_REISER) += reiser.o
obj-$(CONFIG_CMD_REMOTEPROC) += remoteproc.o
obj-$(CONFIG_CMD_RNG) += rng.o
obj-$(CONFIG_CMD_KASLRSEED) += kaslrseed.o
obj-$(CONFIG_CMD_RKMTD) += rkmtd.o
obj-$(CONFIG_CMD_ROCKUSB) += rockusb.o
obj-$(CONFIG_CMD_RTC) += rtc.o
obj-$(CONFIG_SANDBOX) += host.o
obj-$(CONFIG_CMD_SATA) += sata.o
obj-$(CONFIG_CMD_NVME) += nvme.o
obj-$(CONFIG_SANDBOX) += sb.o
obj-$(CONFIG_CMD_SF) += sf.o
obj-$(CONFIG_CMD_SCMI) += scmi.o
obj-$(CONFIG_CMD_SCSI) += scsi.o disk.o
obj-$(CONFIG_CMD_SHA1SUM) += sha1sum.o
obj-$(CONFIG_CMD_SEAMA) += seama.o
obj-$(CONFIG_CMD_SETEXPR) += setexpr.o
obj-$(CONFIG_CMD_SETEXPR_FMT) += printf.o
obj-$(CONFIG_CMD_SPI) += spi.o
obj-$(CONFIG_CMD_STRINGS) += strings.o
obj-$(CONFIG_CMD_SMBIOS) += smbios.o
obj-$(CONFIG_CMD_SMC) += smccc.o
obj-$(CONFIG_CMD_SYSBOOT) += sysboot.o
obj-$(CONFIG_CMD_STACKPROTECTOR_TEST) += stackprot_test.o
obj-$(CONFIG_CMD_TEMPERATURE) += temperature.o
obj-$(CONFIG_CMD_TERMINAL) += terminal.o
obj-$(CONFIG_CMD_TIME) += time.o
obj-$(CONFIG_CMD_TIMER) += timer.o
obj-$(CONFIG_CMD_TRACE) += trace.o
obj-$(CONFIG_HUSH_PARSER) += test.o
obj-$(CONFIG_CMD_TPM) += tpm-common.o
obj-$(CONFIG_CMD_TPM_V1) += tpm-v1.o
obj-$(CONFIG_CMD_TPM_TEST) += tpm_test.o
obj-$(CONFIG_CMD_TPM_V2) += tpm-v2.o
obj-$(CONFIG_CMD_CROS_EC) += cros_ec.o
obj-$(CONFIG_CMD_TSI148) += tsi148.o
obj-$(CONFIG_CMD_UBI) += ubi.o
obj-$(CONFIG_CMD_UBIFS) += ubifs.o
obj-$(CONFIG_CMD_UNIVERSE) += universe.o
obj-$(CONFIG_CMD_UNLZ4) += unlz4.o
obj-$(CONFIG_CMD_UNZIP) += unzip.o
obj-$(CONFIG_CMD_VIRTIO) += virtio.o
obj-$(CONFIG_CMD_WDT) += wdt.o
obj-$(CONFIG_CMD_LZMADEC) += lzmadec.o
obj-$(CONFIG_CMD_UFS) += ufs.o
obj-$(CONFIG_CMD_USB) += usb.o disk.o
obj-$(CONFIG_CMD_VIDCONSOLE) += video.o

obj-$(CONFIG_CMD_FASTBOOT) += fastboot.o
obj-$(CONFIG_CMD_FS_UUID) += fs_uuid.o

obj-$(CONFIG_CMD_USB_MASS_STORAGE) += usb_mass_storage.o
obj-$(CONFIG_CMD_USB_SDP) += usb_gadget_sdp.o
obj-$(CONFIG_CMD_THOR_DOWNLOAD) += thordown.o
obj-$(CONFIG_CMD_VBE) += vbe.o
obj-$(CONFIG_CMD_XIMG) += ximg.o
obj-$(CONFIG_CMD_XXD) += xxd.o
obj-$(CONFIG_CMD_YAFFS2) += yaffs2.o
obj-$(CONFIG_CMD_SPL) += spl.o
obj-$(CONFIG_CMD_W1) += w1.o
obj-$(CONFIG_CMD_ZIP) += zip.o
obj-$(CONFIG_CMD_ZFS) += zfs.o

obj-$(CONFIG_CMD_DFU) += dfu.o
obj-$(CONFIG_CMD_GPT) += gpt.o
obj-$(CONFIG_CMD_MBR) += mbr.o
obj-$(CONFIG_CMD_ETHSW) += ethsw.o
obj-$(CONFIG_CMD_AXI) += axi.o
obj-$(CONFIG_CMD_PVBLOCK) += pvblock.o

# Power
obj-$(CONFIG_CMD_PMIC) += pmic.o
obj-$(CONFIG_CMD_REGULATOR) += regulator.o

obj-$(CONFIG_CMD_BLOB) += blob.o

# Android Verified Boot 2.0
obj-$(CONFIG_CMD_AVB) += avb.o

# Foundries.IO SCP03
obj-$(CONFIG_CMD_SCP03) += scp03.o

obj-$(CONFIG_HUSH_SELECTABLE) += cli.o

obj-$(CONFIG_ARM) += arm/
obj-$(CONFIG_RISCV) += riscv/
obj-$(CONFIG_SANDBOX) += sandbox/
obj-$(CONFIG_X86) += x86/

# Meson
obj-$(CONFIG_CMD_MESON) += meson/

obj-$(CONFIG_ARCH_MVEBU) += mvebu/

# TI
obj-$(CONFIG_ARCH_KEYSTONE) += ti/
obj-$(CONFIG_ARCH_K3) += ti/
obj-$(CONFIG_ARCH_OMAP2PLUS) += ti/
endif # !CONFIG_SPL_BUILD

obj-$(CONFIG_$(SPL_)CMD_TLV_EEPROM) += tlv_eeprom.o

obj-$(CONFIG_CMD_BCM_EXT_UTILS) += broadcom/

filechk_data_gz = (echo "static const char data_gz[] ="; cat $< | scripts/bin2c; echo ";")

filechk_data_size = \
	(echo "static const size_t data_size = "; \
	cat $< | wc -c; echo ";")

# "config" command
$(obj)/config.o: $(obj)/config_data_gz.h $(obj)/config_data_size.h

targets += config_data.gz
$(obj)/config_data.gz: $(KCONFIG_CONFIG) FORCE
	$(call if_changed,gzip)

targets += config_data_gz.h
$(obj)/config_data_gz.h: $(obj)/config_data.gz FORCE
	$(call filechk,data_gz)

targets += config_data_size.h
$(obj)/config_data_size.h: $(KCONFIG_CONFIG) FORCE
	$(call filechk,data_size)

# "license" command
$(obj)/license.o: $(obj)/license_data_gz.h $(obj)/license_data_size.h

targets += license_data.gz
$(obj)/license_data.gz: $(srctree)/Licenses/gpl-2.0.txt FORCE
	$(call if_changed,gzip)

targets += license_data_gz.h
$(obj)/license_data_gz.h: $(obj)/license_data.gz FORCE
	$(call filechk,data_gz)

targets += license_data_size.h
$(obj)/license_data_size.h: $(srctree)/Licenses/gpl-2.0.txt FORCE
	$(call filechk,data_size)

CFLAGS_ethsw.o := -Wno-enum-conversion
