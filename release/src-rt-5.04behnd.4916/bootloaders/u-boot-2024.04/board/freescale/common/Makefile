# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.

MINIMAL=

ifdef CONFIG_SPL_BUILD
ifndef CONFIG_TPL_BUILD
ifdef CONFIG_SPL_INIT_MINIMAL
MINIMAL=y
endif
endif
endif

ifdef MINIMAL
# necessary to create built-in.o
obj- := __dummy__.o
else
# include i2c_common.o once if either VID or FSL_USE_PCA9547_MUX
I2C_COMMON=
ifdef CONFIG_VID
I2C_COMMON=y
endif
ifdef CONFIG_FSL_USE_PCA9547_MUX
I2C_COMMON=y
endif

obj-$(CONFIG_FSL_CADMUS)	+= cadmus.o
obj-$(CONFIG_FSL_VIA)		+= cds_via.o
obj-$(CONFIG_FMAN_ENET)	+= fman.o
ifndef CONFIG_SPL_BUILD
obj-$(CONFIG_FSL_NGPIXIS)	+= ngpixis.o
endif
obj-$(I2C_COMMON)		+= i2c_common.o
obj-$(CONFIG_FSL_USE_PCA9547_MUX)		+= i2c_mux.o
obj-$(CONFIG_$(SPL_)VID)	+= vid.o
obj-$(CONFIG_FSL_QIXIS)	+= qixis.o
ifndef CONFIG_SPL_BUILD
obj-$(CONFIG_ID_EEPROM)	+= sys_eeprom.o
endif
ifndef CONFIG_RAMBOOT_PBL
obj-$(CONFIG_FSL_FIXED_MMC_LOCATION)	+= sdhc_boot.o
endif

ifdef CONFIG_ARM
obj-$(CONFIG_DEEP_SLEEP)		+= arm_sleep.o
else
obj-$(CONFIG_DEEP_SLEEP)		+= mpc85xx_sleep.o
endif

obj-$(CONFIG_TARGET_MPC8548CDS)	+= cds_pci_ft.o

obj-$(CONFIG_TARGET_P3041DS)		+= ics307_clk.o
obj-$(CONFIG_TARGET_P4080DS)		+= ics307_clk.o
obj-$(CONFIG_TARGET_P5040DS)		+= ics307_clk.o
ifeq ($(CONFIG_$(SPL_)POWER_LEGACY),y)
obj-$(CONFIG_POWER_PFUZE100)	+= pfuze.o
endif
obj-$(CONFIG_DM_PMIC_PFUZE100)	+= pfuze.o
obj-$(CONFIG_POWER_MC34VR500)	+= mc34vr500.o
ifneq (,$(filter $(SOC), imx8ulp imx9))
obj-y				+= mmc.o
endif

obj-$(CONFIG_LS102XA_STREAM_ID)	+= ls102xa_stream_id.o

obj-$(CONFIG_EMC2305)              += emc2305.o

# deal with common files for P-series corenet based devices
obj-$(CONFIG_TARGET_P2041RDB)	+= p_corenet/
obj-$(CONFIG_TARGET_P3041DS)	+= p_corenet/
obj-$(CONFIG_TARGET_P4080DS)	+= p_corenet/
obj-$(CONFIG_TARGET_P5040DS)	+= p_corenet/

obj-$(CONFIG_LAYERSCAPE_NS_ACCESS)	+= ns_access.o

ifdef CONFIG_NXP_ESBC
obj-$(CONFIG_CMD_ESBC_VALIDATE) += fsl_validate.o cmd_esbc_validate.o
endif
obj-$(CONFIG_CHAIN_OF_TRUST) += fsl_chain_of_trust.o

endif
