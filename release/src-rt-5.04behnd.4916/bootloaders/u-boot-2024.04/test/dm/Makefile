# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (c) 2013 Google, Inc
# Copyright 2022-2023 Arm Limited and/or its affiliates <open-source-office@arm.com>

obj-$(CONFIG_UT_DM) += test-dm.o

# Tests for particular subsystems - when enabling driver model for a new
# subsystem you must add sandbox tests here.
ifeq ($(CONFIG_SPL_BUILD),y)
obj-$(CONFIG_SPL_OF_PLATDATA) += of_platdata.o
else
obj-$(CONFIG_UT_DM) += bus.o
obj-$(CONFIG_UT_DM) += test-driver.o
obj-$(CONFIG_UT_DM) += test-fdt.o
obj-$(CONFIG_UT_DM) += test-uclass.o

obj-$(CONFIG_UT_DM) += core.o
obj-$(CONFIG_UT_DM) += read.o
obj-$(CONFIG_UT_DM) += phys2bus.o
ifeq ($(CONFIG_NVMXIP_QSPI)$(CONFIG_SANDBOX64),yy)
obj-y += nvmxip.o
endif

ifneq ($(CONFIG_SANDBOX),)
ifeq ($(CONFIG_ACPIGEN),y)
obj-y += acpi.o
obj-y += acpigen.o
obj-y += acpi_dp.o
obj-(CONFIG_DM_GPIO) += gpio.o
obj-y += irq.o
endif
obj-$(CONFIG_ADC) += adc.o
obj-$(CONFIG_SOUND) += audio.o
obj-$(CONFIG_AXI) += axi.o
obj-$(CONFIG_BLK) += blk.o
obj-$(CONFIG_BLKMAP) += blkmap.o
obj-$(CONFIG_BUTTON) += button.o
obj-$(CONFIG_DM_BOOTCOUNT) += bootcount.o
obj-$(CONFIG_DM_REBOOT_MODE) += reboot-mode.o
obj-$(CONFIG_CLK) += clk.o clk_ccf.o
obj-$(CONFIG_CPU) += cpu.o
obj-$(CONFIG_CROS_EC) += cros_ec.o
obj-$(CONFIG_PWM_CROS_EC) += cros_ec_pwm.o
obj-$(CONFIG_$(SPL_TPL_)DEVRES) += devres.o
obj-$(CONFIG_DMA) += dma.o
obj-$(CONFIG_VIDEO_MIPI_DSI) += dsi_host.o
obj-$(CONFIG_DM_DSA) += dsa.o
obj-$(CONFIG_ECDSA_VERIFY) += ecdsa.o
obj-$(CONFIG_EFI_MEDIA_SANDBOX) += efi_media.o
obj-$(CONFIG_DM_ETH) += eth.o
obj-$(CONFIG_EXTCON) += extcon.o
ifneq ($(CONFIG_EFI_PARTITION),)
obj-$(CONFIG_FASTBOOT_FLASH_MMC) += fastboot.o
endif
obj-$(CONFIG_FIRMWARE) += firmware.o
obj-$(CONFIG_DM_FPGA) += fpga.o
obj-$(CONFIG_FWU_MDATA_GPT_BLK) += fwu_mdata.o
obj-$(CONFIG_SANDBOX) += host.o
obj-$(CONFIG_DM_HWSPINLOCK) += hwspinlock.o
obj-$(CONFIG_DM_I2C) += i2c.o
obj-$(CONFIG_SOUND) += i2s.o
obj-$(CONFIG_CLK_K210_SET_RATE) += k210_pll.o
obj-$(CONFIG_IOMMU) += iommu.o
obj-$(CONFIG_LED) += led.o
obj-$(CONFIG_DM_MAILBOX) += mailbox.o
obj-$(CONFIG_DM_MDIO) += mdio.o
obj-$(CONFIG_DM_MDIO_MUX) += mdio_mux.o
obj-$(CONFIG_MEMORY) += memory.o
obj-$(CONFIG_MISC) += misc.o
obj-$(CONFIG_DM_MMC) += mmc.o
obj-$(CONFIG_CMD_MUX) += mux-cmd.o
obj-$(CONFIG_MULTIPLEXER) += mux-emul.o
obj-$(CONFIG_MUX_MMIO) += mux-mmio.o
obj-y += fdtdec.o
obj-$(CONFIG_MTD_RAW_NAND) += nand.o
obj-$(CONFIG_UT_DM) += nop.o
obj-y += ofnode.o
obj-y += ofread.o
obj-y += of_extra.o
obj-$(CONFIG_OSD) += osd.o
obj-$(CONFIG_VIDEO) += panel.o
obj-$(CONFIG_EFI_PARTITION) += part.o
obj-$(CONFIG_PCI) += pci.o
obj-$(CONFIG_P2SB) += p2sb.o
obj-$(CONFIG_PCI_ENDPOINT) += pci_ep.o
obj-$(CONFIG_PCH) += pch.o
obj-$(CONFIG_PHY) += phy.o
ifneq ($(CONFIG_PINMUX),)
obj-$(CONFIG_PINCONF) += pinmux.o
endif
obj-$(CONFIG_POWER_DOMAIN) += power-domain.o
obj-$(CONFIG_ACPI_PMC) += pmc.o
obj-$(CONFIG_DM_PMIC) += pmic.o
obj-$(CONFIG_DM_PWM) += pwm.o
obj-$(CONFIG_ARM_FFA_TRANSPORT) += ffa.o
obj-$(CONFIG_QFW) += qfw.o
obj-$(CONFIG_RAM) += ram.o
obj-y += regmap.o
obj-$(CONFIG_REMOTEPROC) += remoteproc.o
obj-$(CONFIG_DM_RESET) += reset.o
obj-$(CONFIG_SYSRESET) += sysreset.o
obj-$(CONFIG_DM_REGULATOR) += regulator.o
obj-$(CONFIG_CMD_RKMTD) += rkmtd.o
obj-$(CONFIG_DM_RNG) += rng.o
obj-$(CONFIG_DM_RTC) += rtc.o
obj-$(CONFIG_SCMI_FIRMWARE) += scmi.o
obj-$(CONFIG_SCSI) += scsi.o
obj-$(CONFIG_DM_SERIAL) += serial.o
obj-$(CONFIG_DM_SPI_FLASH) += sf.o
obj-$(CONFIG_SIMPLE_BUS) += simple-bus.o
obj-$(CONFIG_SIMPLE_PM_BUS) += simple-pm-bus.o
obj-$(CONFIG_SMEM) += smem.o
obj-$(CONFIG_SOC_DEVICE) += soc.o
obj-$(CONFIG_SOUND) += sound.o
obj-$(CONFIG_DM_SPI) += spi.o
obj-$(CONFIG_SPMI) += spmi.o
obj-y += syscon.o
obj-$(CONFIG_RESET_SYSCON) += syscon-reset.o
obj-$(CONFIG_SM) += sm.o
obj-$(CONFIG_SYSINFO) += sysinfo.o
obj-$(CONFIG_SYSINFO_GPIO) += sysinfo-gpio.o
obj-$(CONFIG_UT_DM) += tag.o
obj-$(CONFIG_TEE) += tee.o
obj-$(CONFIG_TIMER) += timer.o
obj-$(CONFIG_TPM_V2) += tpm.o
obj-$(CONFIG_DM_USB) += usb.o
obj-$(CONFIG_DM_RNG) += rng.o
obj-$(CONFIG_VIDEO) += video.o
ifeq ($(CONFIG_VIRTIO_SANDBOX),y)
obj-y += virtio.o
obj-$(CONFIG_VIRTIO_RNG) += virtio_device.o
obj-$(CONFIG_VIRTIO_RNG) += virtio_rng.o
endif
ifeq ($(CONFIG_WDT_GPIO)$(CONFIG_WDT_SANDBOX),yy)
obj-y += wdt.o
endif
endif
endif # !SPL
