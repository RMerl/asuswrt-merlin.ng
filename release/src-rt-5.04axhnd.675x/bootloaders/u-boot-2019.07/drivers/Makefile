# SPDX-License-Identifier: GPL-2.0+

obj-$(CONFIG_$(SPL_TPL_)CLK) += clk/
obj-$(CONFIG_$(SPL_TPL_)DM) += core/
obj-$(CONFIG_$(SPL_TPL_)DFU) += dfu/
obj-$(CONFIG_$(SPL_TPL_)GPIO_SUPPORT) += gpio/
obj-$(CONFIG_$(SPL_TPL_)DRIVERS_MISC_SUPPORT) += misc/ sysreset/ firmware/
obj-$(CONFIG_$(SPL_TPL_)I2C_SUPPORT) += i2c/
obj-$(CONFIG_$(SPL_TPL_)INPUT) += input/
obj-$(CONFIG_$(SPL_TPL_)LED) += led/
obj-$(CONFIG_$(SPL_TPL_)MMC_SUPPORT) += mmc/
obj-$(CONFIG_$(SPL_TPL_)NAND_SUPPORT) += mtd/nand/raw/
obj-$(CONFIG_$(SPL_TPL_)PCH_SUPPORT) += pch/
obj-$(CONFIG_$(SPL_TPL_)PCI) += pci/
obj-$(CONFIG_$(SPL_TPL_)PHY) += phy/
obj-$(CONFIG_$(SPL_TPL_)PINCTRL) += pinctrl/
obj-$(CONFIG_$(SPL_TPL_)RAM) += ram/
obj-$(CONFIG_$(SPL_TPL_)RTC_SUPPORT) += rtc/
obj-$(CONFIG_$(SPL_TPL_)SERIAL_SUPPORT) += serial/
obj-$(CONFIG_$(SPL_TPL_)SPI_FLASH_SUPPORT) += mtd/spi/
obj-$(CONFIG_$(SPL_TPL_)SPI_SUPPORT) += spi/
obj-$(CONFIG_$(SPL_TPL_)TIMER) += timer/
obj-$(CONFIG_$(SPL_TPL_)VIRTIO) += virtio/
obj-$(CONFIG_$(SPL_)DM_MAILBOX) += mailbox/
obj-$(CONFIG_$(SPL_)REMOTEPROC) += remoteproc/
obj-$(CONFIG_$(SPL_TPL_)TPM) += tpm/
obj-$(CONFIG_$(SPL_TPL_)UBI) += mtd/ubispl/
obj-$(CONFIG_$(SPL_TPL_)CRYPTO_SUPPORT) += crypto/

ifndef CONFIG_TPL_BUILD
ifdef CONFIG_SPL_BUILD

obj-$(CONFIG_SPL_BOOTCOUNT_LIMIT) += bootcount/
obj-$(CONFIG_SPL_CPU_SUPPORT) += cpu/
obj-$(CONFIG_SPL_MPC8XXX_INIT_DDR_SUPPORT) += ddr/fsl/
obj-$(CONFIG_ARMADA_38X) += ddr/marvell/a38x/
obj-$(CONFIG_ARMADA_XP) += ddr/marvell/axp/
obj-$(CONFIG_$(SPL_)ALTERA_SDRAM) += ddr/altera/
obj-$(CONFIG_ARCH_IMX8M) += ddr/imx/imx8m/
obj-$(CONFIG_SPL_POWER_SUPPORT) += power/ power/pmic/
obj-$(CONFIG_SPL_POWER_SUPPORT) += power/regulator/
obj-$(CONFIG_SPL_POWER_DOMAIN) += power/domain/
obj-$(CONFIG_SPL_DM_RESET) += reset/
obj-$(CONFIG_SPL_MTD_SUPPORT) += mtd/
obj-$(CONFIG_SPL_ONENAND_SUPPORT) += mtd/onenand/
obj-$(CONFIG_SPL_DMA_SUPPORT) += dma/
obj-$(CONFIG_SPL_ETH_SUPPORT) += net/
obj-$(CONFIG_SPL_ETH_SUPPORT) += net/phy/
obj-$(CONFIG_SPL_USB_ETHER) += net/phy/
obj-$(CONFIG_SPL_MUSB_NEW_SUPPORT) += usb/musb-new/
obj-$(CONFIG_SPL_USB_GADGET) += usb/gadget/
obj-$(CONFIG_SPL_USB_GADGET) += usb/common/
obj-$(CONFIG_SPL_USB_GADGET) += usb/gadget/udc/
obj-$(CONFIG_SPL_WATCHDOG_SUPPORT) += watchdog/
obj-$(CONFIG_SPL_USB_HOST_SUPPORT) += usb/host/
obj-$(CONFIG_OMAP_USB_PHY) += usb/phy/
obj-$(CONFIG_SPL_SATA_SUPPORT) += ata/ scsi/
obj-$(CONFIG_HAVE_BLOCK_DEVICE) += block/
obj-$(CONFIG_SPL_FPGA_SUPPORT) += fpga/
obj-$(CONFIG_SPL_THERMAL) += thermal/

endif
else
obj-$(CONFIG_HAVE_BLOCK_DEVICE) += block/
endif

ifdef CONFIG_TPL_BUILD
obj-$(CONFIG_TPL_MTD_SUPPORT) += mtd/
obj-$(CONFIG_TPL_MPC8XXX_INIT_DDR_SUPPORT) += ddr/fsl/

endif

ifeq ($(CONFIG_SPL_BUILD)$(CONFIG_TPL_BUILD),)

obj-y += adc/
obj-y += ata/
obj-$(CONFIG_DM_DEMO) += demo/
obj-$(CONFIG_BIOSEMU) += bios_emulator/
obj-y += block/
obj-y += board/
obj-$(CONFIG_BOOTCOUNT_LIMIT) += bootcount/
obj-y += cache/
obj-$(CONFIG_CPU) += cpu/
obj-y += crypto/
obj-$(CONFIG_FASTBOOT) += fastboot/
obj-y += firmware/
obj-$(CONFIG_FPGA) += fpga/
obj-y += misc/
obj-$(CONFIG_MMC) += mmc/
obj-$(CONFIG_NVME) += nvme/
obj-y += pcmcia/
obj-y += dfu/
obj-$(CONFIG_PCH) += pch/
obj-y += phy/allwinner/
obj-y += phy/marvell/
obj-y += rtc/
obj-y += scsi/
obj-y += sound/
obj-y += spmi/
obj-y += sysreset/
obj-y += video/
obj-y += watchdog/
obj-$(CONFIG_QE) += qe/
obj-$(CONFIG_U_QE) += qe/
obj-y += mailbox/
obj-y += memory/
obj-y += pwm/
obj-y += reset/
obj-y += input/
# SOC specific infrastructure drivers.
obj-y += smem/
obj-y += soc/
obj-y += thermal/
obj-$(CONFIG_TEE) += tee/
obj-y += axi/
obj-$(CONFIG_W1) += w1/
obj-$(CONFIG_W1_EEPROM) += w1-eeprom/

obj-$(CONFIG_MACH_PIC32) += ddr/microchip/
obj-$(CONFIG_DM_HWSPINLOCK) += hwspinlock/
endif
