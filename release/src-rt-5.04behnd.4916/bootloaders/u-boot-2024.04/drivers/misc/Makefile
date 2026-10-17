# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2000-2007
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.

obj-$(CONFIG_$(SPL_TPL_)MISC) += misc-uclass.o
obj-$(CONFIG_$(SPL_TPL_)NVMEM) += nvmem.o

obj-$(CONFIG_$(SPL_TPL_)CROS_EC) += cros_ec.o
obj-$(CONFIG_$(SPL_TPL_)CROS_EC_SANDBOX) += cros_ec_sandbox.o
obj-$(CONFIG_$(SPL_TPL_)CROS_EC_LPC) += cros_ec_lpc.o

ifndef CONFIG_SPL_BUILD
obj-$(CONFIG_SANDBOX) += sandbox_adder.o
obj-$(CONFIG_CROS_EC_I2C) += cros_ec_i2c.o
obj-$(CONFIG_CROS_EC_SPI) += cros_ec_spi.o
obj-$(CONFIG_SANDBOX) += p2sb_sandbox.o p2sb_emul.o
obj-$(CONFIG_SANDBOX) += swap_case.o
endif

ifdef CONFIG_$(SPL_)DM_I2C
ifndef CONFIG_SPL_BUILD
obj-$(CONFIG_SANDBOX) += i2c_eeprom_emul.o
obj-$(CONFIG_USB_HUB_USB251XB) += usb251xb.o
endif
endif
ifdef CONFIG_SPL_OF_PLATDATA
ifdef CONFIG_SPL_BUILD
obj-$(CONFIG_SANDBOX) += spltest_sandbox.o
endif
endif
obj-$(CONFIG_ALTERA_SYSID) += altera_sysid.o
obj-$(CONFIG_ATSHA204A) += atsha204a-i2c.o
obj-$(CONFIG_CBMEM_CONSOLE) += cbmem_console.o
obj-$(CONFIG_DS4510)  += ds4510.o
obj-$(CONFIG_FSL_DEVICE_DISABLE) += fsl_devdis.o
obj-$(CONFIG_FSL_IFC) += fsl_ifc.o
obj-$(CONFIG_FSL_IIM) += fsl_iim.o
obj-$(CONFIG_FSL_SEC_MON) += fsl_sec_mon.o
obj-$(CONFIG_$(SPL_)FS_LOADER) += fs_loader.o
obj-$(CONFIG_GATEWORKS_SC) += gsc.o
obj-$(CONFIG_GDSYS_IOEP) += gdsys_ioep.o
obj-$(CONFIG_GDSYS_RXAUI_CTRL) += gdsys_rxaui_ctrl.o
obj-$(CONFIG_GDSYS_SOC) += gdsys_soc.o
obj-$(CONFIG_IRQ) += irq-uclass.o
obj-$(CONFIG_SANDBOX) += irq_sandbox.o irq_sandbox_test.o
obj-$(CONFIG_$(SPL_)I2C_EEPROM) += i2c_eeprom.o
obj-$(CONFIG_IHS_FPGA) += ihs_fpga.o
obj-$(CONFIG_IMX8) += imx8/
obj-$(CONFIG_IMX_ELE) += imx_ele/
obj-$(CONFIG_LED_STATUS) += status_led.o
obj-$(CONFIG_LED_STATUS_GPIO) += gpio_led.o
obj-$(CONFIG_MPC83XX_SERDES) += mpc83xx_serdes.o
obj-$(CONFIG_$(SPL_TPL_)LS2_SFP) += ls2_sfp.o
obj-$(CONFIG_$(SPL_)MXC_OCOTP) += mxc_ocotp.o
obj-$(CONFIG_MXS_OCOTP) += mxs_ocotp.o
obj-$(CONFIG_NPCM_OTP) += npcm_otp.o
obj-$(CONFIG_NPCM_HOST) += npcm_host_intf.o
obj-$(CONFIG_NUVOTON_NCT6102D) += nuvoton_nct6102d.o
obj-$(CONFIG_P2SB) += p2sb-uclass.o
obj-$(CONFIG_PCA9551_LED) += pca9551_led.o
obj-$(CONFIG_$(SPL_)PWRSEQ) += pwrseq-uclass.o
ifdef CONFIG_QFW
obj-y += qfw.o
obj-$(CONFIG_QFW_ACPI) += qfw_acpi.o
obj-$(CONFIG_QFW_PIO) += qfw_pio.o
obj-$(CONFIG_QFW_MMIO) += qfw_mmio.o
obj-$(CONFIG_QFW_SMBIOS) += qfw_smbios.o
obj-$(CONFIG_SANDBOX) += qfw_sandbox.o
endif
obj-$(CONFIG_ROCKCHIP_EFUSE) += rockchip-efuse.o
obj-$(CONFIG_ROCKCHIP_OTP) += rockchip-otp.o
obj-$(CONFIG_$(SPL_TPL_)ROCKCHIP_IODOMAIN) += rockchip-io-domain.o
obj-$(CONFIG_SANDBOX) += syscon_sandbox.o misc_sandbox.o
obj-$(CONFIG_SIFIVE_OTP) += sifive-otp.o
obj-$(CONFIG_SMSC_LPC47M) += smsc_lpc47m.o
obj-$(CONFIG_SMSC_SIO1007) += smsc_sio1007.o
obj-$(CONFIG_STM32MP_FUSE) += stm32mp_fuse.o
obj-$(CONFIG_STM32_RCC) += stm32_rcc.o
obj-$(CONFIG_SYS_DPAA_QBMAN) += fsl_portals.o
obj-$(CONFIG_TEGRA186_BPMP) += tegra186_bpmp.o
obj-$(CONFIG_TEGRA_CAR) += tegra_car.o
obj-$(CONFIG_TEST_DRV) += test_drv.o
obj-$(CONFIG_TWL4030_LED) += twl4030_led.o
obj-$(CONFIG_VEXPRESS_CONFIG) += vexpress_config.o
obj-$(CONFIG_WINBOND_W83627) += winbond_w83627.o
obj-$(CONFIG_JZ4780_EFUSE) += jz4780_efuse.o
obj-$(CONFIG_MICROCHIP_FLEXCOM) += microchip_flexcom.o
obj-$(CONFIG_K3_AVS0) += k3_avs.o
obj-$(CONFIG_ESM_K3) += k3_esm.o
obj-$(CONFIG_ESM_PMIC) += esm_pmic.o
obj-$(CONFIG_SL28CPLD) += sl28cpld.o
