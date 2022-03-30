# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (c) 2013 Google, Inc

obj-$(CONFIG_UT_DM) += bus.o
obj-$(CONFIG_UT_DM) += nop.o
obj-$(CONFIG_UT_DM) += test-driver.o
obj-$(CONFIG_UT_DM) += test-fdt.o
obj-$(CONFIG_UT_DM) += test-main.o
obj-$(CONFIG_UT_DM) += test-uclass.o

# Tests for particular subsystems - when enabling driver model for a new
# subsystem you must add sandbox tests here.
obj-$(CONFIG_UT_DM) += core.o
ifneq ($(CONFIG_SANDBOX),)
obj-$(CONFIG_SOUND) += audio.o
obj-$(CONFIG_BLK) += blk.o
obj-$(CONFIG_BOARD) += board.o
obj-$(CONFIG_DM_BOOTCOUNT) += bootcount.o
obj-$(CONFIG_CLK) += clk.o
obj-$(CONFIG_DM_ETH) += eth.o
obj-$(CONFIG_FIRMWARE) += firmware.o
obj-$(CONFIG_DM_GPIO) += gpio.o
obj-$(CONFIG_DM_HWSPINLOCK) += hwspinlock.o
obj-$(CONFIG_DM_I2C) += i2c.o
obj-$(CONFIG_SOUND) += i2s.o
obj-$(CONFIG_LED) += led.o
obj-$(CONFIG_DM_MAILBOX) += mailbox.o
obj-$(CONFIG_DM_MMC) += mmc.o
obj-y += ofnode.o
obj-$(CONFIG_OSD) += osd.o
obj-$(CONFIG_DM_VIDEO) += panel.o
obj-$(CONFIG_DM_PCI) += pci.o
obj-$(CONFIG_PCH) += pch.o
obj-$(CONFIG_PHY) += phy.o
obj-$(CONFIG_POWER_DOMAIN) += power-domain.o
obj-$(CONFIG_DM_PWM) += pwm.o
obj-$(CONFIG_RAM) += ram.o
obj-y += regmap.o
obj-$(CONFIG_REMOTEPROC) += remoteproc.o
obj-$(CONFIG_DM_RESET) += reset.o
obj-$(CONFIG_SYSRESET) += sysreset.o
obj-$(CONFIG_DM_RTC) += rtc.o
obj-$(CONFIG_DM_SPI_FLASH) += sf.o
obj-$(CONFIG_SMEM) += smem.o
obj-$(CONFIG_DM_SPI) += spi.o
obj-y += syscon.o
obj-$(CONFIG_DM_USB) += usb.o
obj-$(CONFIG_DM_PMIC) += pmic.o
obj-$(CONFIG_DM_REGULATOR) += regulator.o
obj-$(CONFIG_TIMER) += timer.o
obj-$(CONFIG_DM_VIDEO) += video.o
obj-$(CONFIG_ADC) += adc.o
obj-$(CONFIG_SPMI) += spmi.o
obj-$(CONFIG_WDT) += wdt.o
obj-$(CONFIG_AXI) += axi.o
obj-$(CONFIG_MISC) += misc.o
obj-$(CONFIG_DM_SERIAL) += serial.o
obj-$(CONFIG_CPU) += cpu.o
obj-$(CONFIG_SOUND) += sound.o
obj-$(CONFIG_TEE) += tee.o
obj-$(CONFIG_VIRTIO_SANDBOX) += virtio.o
obj-$(CONFIG_DMA) += dma.o
endif
