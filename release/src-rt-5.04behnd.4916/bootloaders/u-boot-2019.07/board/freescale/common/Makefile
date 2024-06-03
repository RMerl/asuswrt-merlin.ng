# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.

MINIMAL=

ifdef CONFIG_SPL_BUILD
ifdef CONFIG_SPL_INIT_MINIMAL
MINIMAL=y
endif
endif

ifdef MINIMAL
# necessary to create built-in.o
obj- := __dummy__.o
else
obj-$(CONFIG_FSL_CADMUS)	+= cadmus.o
obj-$(CONFIG_FSL_VIA)		+= cds_via.o
obj-$(CONFIG_FMAN_ENET)	+= fman.o
obj-$(CONFIG_FSL_PIXIS)	+= pixis.o
ifndef CONFIG_SPL_BUILD
obj-$(CONFIG_FSL_NGPIXIS)	+= ngpixis.o
endif
obj-$(CONFIG_VID)		+= vid.o
obj-$(CONFIG_FSL_QIXIS)	+= qixis.o
obj-$(CONFIG_PQ_MDS_PIB)	+= pq-mds-pib.o
ifndef CONFIG_SPL_BUILD
obj-$(CONFIG_ID_EEPROM)	+= sys_eeprom.o
endif
obj-$(CONFIG_FSL_SGMII_RISER)	+= sgmii_riser.o
ifndef CONFIG_RAMBOOT_PBL
obj-$(CONFIG_FSL_FIXED_MMC_LOCATION)	+= sdhc_boot.o
endif

obj-$(CONFIG_FSL_DIU_CH7301)	+= diu_ch7301.o

ifdef CONFIG_ARM
obj-$(CONFIG_DEEP_SLEEP)		+= arm_sleep.o
else
obj-$(CONFIG_DEEP_SLEEP)		+= mpc85xx_sleep.o
endif

obj-$(CONFIG_FSL_DCU_SII9022A)    += dcu_sii9022a.o

obj-$(CONFIG_TARGET_MPC8541CDS)	+= cds_pci_ft.o
obj-$(CONFIG_TARGET_MPC8548CDS)	+= cds_pci_ft.o
obj-$(CONFIG_TARGET_MPC8555CDS)	+= cds_pci_ft.o

obj-$(CONFIG_TARGET_MPC8536DS)	+= ics307_clk.o
obj-$(CONFIG_TARGET_MPC8572DS)	+= ics307_clk.o
obj-$(CONFIG_TARGET_P1022DS)		+= ics307_clk.o
obj-$(CONFIG_P2020DS)		+= ics307_clk.o
obj-$(CONFIG_TARGET_P3041DS)		+= ics307_clk.o
obj-$(CONFIG_TARGET_P4080DS)		+= ics307_clk.o
obj-$(CONFIG_TARGET_P5020DS)		+= ics307_clk.o
obj-$(CONFIG_TARGET_P5040DS)		+= ics307_clk.o
obj-$(CONFIG_VSC_CROSSBAR)    += vsc3316_3308.o
obj-$(CONFIG_IDT8T49N222A)	+= idt8t49n222a_serdes_clk.o
obj-$(CONFIG_ZM7300)		+= zm7300.o
obj-$(CONFIG_POWER_PFUZE100)	+= pfuze.o
obj-$(CONFIG_DM_PMIC_PFUZE100)	+= pfuze.o
obj-$(CONFIG_POWER_MC34VR500)	+= mc34vr500.o

obj-$(CONFIG_LS102XA_STREAM_ID)	+= ls102xa_stream_id.o

obj-$(CONFIG_EMC2305)              += emc2305.o

# deal with common files for P-series corenet based devices
obj-$(CONFIG_TARGET_P2041RDB)	+= p_corenet/
obj-$(CONFIG_TARGET_P3041DS)	+= p_corenet/
obj-$(CONFIG_TARGET_P4080DS)	+= p_corenet/
obj-$(CONFIG_TARGET_P5020DS)	+= p_corenet/
obj-$(CONFIG_TARGET_P5040DS)	+= p_corenet/

obj-$(CONFIG_LAYERSCAPE_NS_ACCESS)	+= ns_access.o

ifdef CONFIG_SECURE_BOOT
obj-$(CONFIG_CMD_ESBC_VALIDATE) += fsl_validate.o cmd_esbc_validate.o
endif
obj-$(CONFIG_CHAIN_OF_TRUST) += fsl_chain_of_trust.o

endif
