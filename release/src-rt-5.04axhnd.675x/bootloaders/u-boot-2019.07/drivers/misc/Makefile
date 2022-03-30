# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2000-2007
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.

obj-$(CONFIG_MISC) += misc-uclass.o

obj-$(CONFIG_$(SPL_TPL_)CROS_EC) += cros_ec.o
obj-$(CONFIG_$(SPL_TPL_)CROS_EC_SANDBOX) += cros_ec_sandbox.o
obj-$(CONFIG_$(SPL_TPL_)CROS_EC_LPC) += cros_ec_lpc.o

ifndef CONFIG_SPL_BUILD
obj-$(CONFIG_CROS_EC_I2C) += cros_ec_i2c.o
obj-$(CONFIG_CROS_EC_SPI) += cros_ec_spi.o
endif

ifdef CONFIG_DM_I2C
ifndef CONFIG_SPL_BUILD
obj-$(CONFIG_SANDBOX) += i2c_eeprom_emul.o
endif
endif
ifdef CONFIG_SPL_OF_PLATDATA
ifdef CONFIG_SPL_BUILD
obj-$(CONFIG_SANDBOX) += spltest_sandbox.o
endif
endif
obj-$(CONFIG_ALI152X) += ali512x.o
obj-$(CONFIG_ALTERA_SYSID) += altera_sysid.o
obj-$(CONFIG_ATSHA204A) += atsha204a-i2c.o
obj-$(CONFIG_CBMEM_CONSOLE) += cbmem_console.o
obj-$(CONFIG_DS4510)  += ds4510.o
obj-$(CONFIG_FSL_DEVICE_DISABLE) += fsl_devdis.o
obj-$(CONFIG_FSL_IFC) += fsl_ifc.o
obj-$(CONFIG_FSL_IIM) += fsl_iim.o
obj-$(CONFIG_FSL_MC9SDZ60) += mc9sdz60.o
obj-$(CONFIG_FSL_SEC_MON) += fsl_sec_mon.o
obj-$(CONFIG_FS_LOADER) += fs_loader.o
obj-$(CONFIG_GDSYS_IOEP) += gdsys_ioep.o
obj-$(CONFIG_GDSYS_RXAUI_CTRL) += gdsys_rxaui_ctrl.o
obj-$(CONFIG_GDSYS_SOC) += gdsys_soc.o
obj-$(CONFIG_$(SPL_)I2C_EEPROM) += i2c_eeprom.o
obj-$(CONFIG_IHS_FPGA) += ihs_fpga.o
obj-$(CONFIG_IMX8) += imx8/
obj-$(CONFIG_LED_STATUS) += status_led.o
obj-$(CONFIG_LED_STATUS_GPIO) += gpio_led.o
obj-$(CONFIG_MPC83XX_SERDES) += mpc83xx_serdes.o
obj-$(CONFIG_MXC_OCOTP) += mxc_ocotp.o
obj-$(CONFIG_MXS_OCOTP) += mxs_ocotp.o
obj-$(CONFIG_NS87308) += ns87308.o
obj-$(CONFIG_NUVOTON_NCT6102D) += nuvoton_nct6102d.o
obj-$(CONFIG_PCA9551_LED) += pca9551_led.o
obj-$(CONFIG_$(SPL_)PWRSEQ) += pwrseq-uclass.o
obj-$(CONFIG_QFW) += qfw.o
obj-$(CONFIG_ROCKCHIP_EFUSE) += rockchip-efuse.o
obj-$(CONFIG_SANDBOX) += swap_case.o
obj-$(CONFIG_SANDBOX) += syscon_sandbox.o misc_sandbox.o
obj-$(CONFIG_SMSC_LPC47M) += smsc_lpc47m.o
obj-$(CONFIG_SMSC_SIO1007) += smsc_sio1007.o
obj-$(CONFIG_STM32MP_FUSE) += stm32mp_fuse.o
obj-$(CONFIG_STM32_RCC) += stm32_rcc.o
obj-$(CONFIG_SYS_DPAA_QBMAN) += fsl_portals.o
obj-$(CONFIG_TEGRA186_BPMP) += tegra186_bpmp.o
obj-$(CONFIG_TEGRA_CAR) += tegra_car.o
obj-$(CONFIG_TWL4030_LED) += twl4030_led.o
obj-$(CONFIG_VEXPRESS_CONFIG) += vexpress_config.o
obj-$(CONFIG_WINBOND_W83627) += winbond_w83627.o
obj-$(CONFIG_JZ4780_EFUSE) += jz4780_efuse.o
