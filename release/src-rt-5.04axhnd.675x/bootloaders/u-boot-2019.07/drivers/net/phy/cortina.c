// SPDX-License-Identifier: GPL-2.0+
/*
 * Cortina CS4315/CS4340 10G PHY drivers
 *
 * Copyright 2014 Freescale Semiconductor, Inc.
 * Copyright 2018 NXP
 *
 */

#include <config.h>
#include <common.h>
#include <malloc.h>
#include <linux/ctype.h>
#include <linux/string.h>
#include <linux/err.h>
#include <phy.h>
#include <cortina.h>
#ifdef CONFIG_SYS_CORTINA_FW_IN_NAND
#include <nand.h>
#elif defined(CONFIG_SYS_CORTINA_FW_IN_SPIFLASH)
#include <spi_flash.h>
#elif defined(CONFIG_SYS_CORTINA_FW_IN_MMC)
#include <mmc.h>
#endif

#ifndef CONFIG_PHYLIB_10G
#error The Cortina PHY needs 10G support
#endif

#ifndef CORTINA_NO_FW_UPLOAD
struct cortina_reg_config cortina_reg_cfg[] = {
	/* CS4315_enable_sr_mode */
	{VILLA_GLOBAL_MSEQCLKCTRL, 0x8004},
	{VILLA_MSEQ_OPTIONS, 0xf},
	{VILLA_MSEQ_PC, 0x0},
	{VILLA_MSEQ_BANKSELECT,	   0x4},
	{VILLA_LINE_SDS_COMMON_SRX0_RX_CPA, 0x55},
	{VILLA_LINE_SDS_COMMON_SRX0_RX_LOOP_FILTER, 0x30},
	{VILLA_DSP_SDS_SERDES_SRX_DFE0_SELECT, 0x1},
	{VILLA_DSP_SDS_DSP_COEF_DFE0_SELECT, 0x2},
	{VILLA_LINE_SDS_COMMON_SRX0_RX_CPB, 0x2003},
	{VILLA_DSP_SDS_SERDES_SRX_FFE_DELAY_CTRL, 0xF047},
	{VILLA_MSEQ_ENABLE_MSB, 0x0000},
	{VILLA_MSEQ_SPARE21_LSB, 0x6},
	{VILLA_MSEQ_RESET_COUNT_LSB, 0x0},
	{VILLA_MSEQ_SPARE12_MSB, 0x0000},
	/*
	 * to invert the receiver path, uncomment the next line
	 * write (VILLA_MSEQ_SPARE12_MSB, 0x4000)
	 *
	 * SPARE2_LSB is used to configure the device while in sr mode to
	 * enable power savings and to use the optical module LOS signal.
	 * in power savings mode, the internal prbs checker can not be used.
	 * if the optical module LOS signal is used as an input to the micro
	 * code, then the micro code will wait until the optical module
	 * LOS = 0 before turning on the adaptive equalizer.
	 * Setting SPARE2_LSB bit 0 to 1 places the devie in power savings mode
	 * while setting bit 0 to 0 disables power savings mode.
	 * Setting SPARE2_LSB bit 2 to 0 configures the device to use the
	 * optical module LOS signal while setting bit 2 to 1 configures the
	 * device so that it will ignore the optical module LOS SPARE2_LSB = 0
	 */

	/* enable power savings, ignore optical module LOS */
	{VILLA_MSEQ_SPARE2_LSB, 0x5},

	{VILLA_MSEQ_SPARE7_LSB, 0x1e},
	{VILLA_MSEQ_BANKSELECT, 0x4},
	{VILLA_MSEQ_SPARE9_LSB, 0x2},
	{VILLA_MSEQ_SPARE3_LSB, 0x0F53},
	{VILLA_MSEQ_SPARE3_MSB, 0x2006},
	{VILLA_MSEQ_SPARE8_LSB, 0x3FF7},
	{VILLA_MSEQ_SPARE8_MSB, 0x0A46},
	{VILLA_MSEQ_COEF8_FFE0_LSB, 0xD500},
	{VILLA_MSEQ_COEF8_FFE1_LSB, 0x0200},
	{VILLA_MSEQ_COEF8_FFE2_LSB, 0xBA00},
	{VILLA_MSEQ_COEF8_FFE3_LSB, 0x0100},
	{VILLA_MSEQ_COEF8_FFE4_LSB, 0x0300},
	{VILLA_MSEQ_COEF8_FFE5_LSB, 0x0300},
	{VILLA_MSEQ_COEF8_DFE0_LSB, 0x0700},
	{VILLA_MSEQ_COEF8_DFE0N_LSB, 0x0E00},
	{VILLA_MSEQ_COEF8_DFE1_LSB, 0x0B00},
	{VILLA_DSP_SDS_DSP_COEF_LARGE_LEAK, 0x2},
	{VILLA_DSP_SDS_SERDES_SRX_DAC_ENABLEB_LSB, 0xD000},
	{VILLA_MSEQ_POWER_DOWN_LSB, 0xFFFF},
	{VILLA_MSEQ_POWER_DOWN_MSB, 0x0},
	{VILLA_MSEQ_CAL_RX_SLICER, 0x80},
	{VILLA_DSP_SDS_SERDES_SRX_DAC_BIAS_SELECT1_MSB, 0x3f},
	{VILLA_GLOBAL_MSEQCLKCTRL, 0x4},
	{VILLA_MSEQ_OPTIONS, 0x7},

	/* set up min value for ffe1 */
	{VILLA_MSEQ_COEF_INIT_SEL, 0x2},
	{VILLA_DSP_SDS_DSP_PRECODEDINITFFE21, 0x41},

	/* CS4315_sr_rx_pre_eq_set_4in */
	{VILLA_GLOBAL_MSEQCLKCTRL, 0x8004},
	{VILLA_MSEQ_OPTIONS, 0xf},
	{VILLA_MSEQ_BANKSELECT, 0x4},
	{VILLA_MSEQ_PC, 0x0},

	/* for lengths from 3.5 to 4.5inches */
	{VILLA_MSEQ_SERDES_PARAM_LSB, 0x0306},
	{VILLA_MSEQ_SPARE25_LSB, 0x0306},
	{VILLA_MSEQ_SPARE21_LSB, 0x2},
	{VILLA_MSEQ_SPARE23_LSB, 0x2},
	{VILLA_MSEQ_CAL_RX_DFE_EQ, 0x0},

	{VILLA_GLOBAL_MSEQCLKCTRL, 0x4},
	{VILLA_MSEQ_OPTIONS, 0x7},

	/* CS4315_rx_drive_4inch */
	/* for length  4inches */
	{VILLA_GLOBAL_VILLA2_COMPATIBLE, 0x0000},
	{VILLA_HOST_SDS_COMMON_STX0_TX_OUTPUT_CTRLA, 0x3023},
	{VILLA_LINE_SDS_COMMON_STX0_TX_OUTPUT_CTRLB, 0xc01E},

	/* CS4315_tx_drive_4inch */
	/* for length  4inches */
	{VILLA_GLOBAL_VILLA2_COMPATIBLE, 0x0000},
	{VILLA_LINE_SDS_COMMON_STX0_TX_OUTPUT_CTRLA, 0x3023},
	{VILLA_LINE_SDS_COMMON_STX0_TX_OUTPUT_CTRLB, 0xc01E},
};

void cs4340_upload_firmware(struct phy_device *phydev)
{
	char line_temp[0x50] = {0};
	char reg_addr[0x50] = {0};
	char reg_data[0x50] = {0};
	int i, line_cnt = 0, column_cnt = 0;
	struct cortina_reg_config fw_temp;
	char *addr = NULL;

#if defined(CONFIG_SYS_CORTINA_FW_IN_NOR) || \
	defined(CONFIG_SYS_CORTINA_FW_IN_REMOTE)

	addr = (char *)CONFIG_CORTINA_FW_ADDR;
#elif defined(CONFIG_SYS_CORTINA_FW_IN_NAND)
	int ret;
	size_t fw_length = CONFIG_CORTINA_FW_LENGTH;

	addr = malloc(CONFIG_CORTINA_FW_LENGTH);
	ret = nand_read(get_nand_dev_by_index(0),
			(loff_t)CONFIG_CORTINA_FW_ADDR,
			&fw_length, (u_char *)addr);
	if (ret == -EUCLEAN) {
		printf("NAND read of Cortina firmware at 0x%x failed %d\n",
		       CONFIG_CORTINA_FW_ADDR, ret);
	}
#elif defined(CONFIG_SYS_CORTINA_FW_IN_SPIFLASH)
	int ret;
	struct spi_flash *ucode_flash;

	addr = malloc(CONFIG_CORTINA_FW_LENGTH);
	ucode_flash = spi_flash_probe(CONFIG_ENV_SPI_BUS, CONFIG_ENV_SPI_CS,
				CONFIG_ENV_SPI_MAX_HZ, CONFIG_ENV_SPI_MODE);
	if (!ucode_flash) {
		puts("SF: probe for Cortina ucode failed\n");
	} else {
		ret = spi_flash_read(ucode_flash, CONFIG_CORTINA_FW_ADDR,
				     CONFIG_CORTINA_FW_LENGTH, addr);
		if (ret)
			puts("SF: read for Cortina ucode failed\n");
		spi_flash_free(ucode_flash);
	}
#elif defined(CONFIG_SYS_CORTINA_FW_IN_MMC)
	int dev = CONFIG_SYS_MMC_ENV_DEV;
	u32 cnt = CONFIG_CORTINA_FW_LENGTH / 512;
	u32 blk = CONFIG_CORTINA_FW_ADDR / 512;
	struct mmc *mmc = find_mmc_device(CONFIG_SYS_MMC_ENV_DEV);

	if (!mmc) {
		puts("Failed to find MMC device for Cortina ucode\n");
	} else {
		addr = malloc(CONFIG_CORTINA_FW_LENGTH);
		printf("MMC read: dev # %u, block # %u, count %u ...\n",
		       dev, blk, cnt);
		mmc_init(mmc);
		(void)mmc->block_dev.block_read(&mmc->block_dev, blk, cnt,
						addr);
	}
#endif

	while (*addr != 'Q') {
		i = 0;

		while (*addr != 0x0a) {
			line_temp[i++] = *addr++;
			if (0x50 < i) {
				printf("Not found Cortina PHY ucode at 0x%p\n",
				       (char *)CONFIG_CORTINA_FW_ADDR);
				return;
			}
		}

		addr++;  /* skip '\n' */
		line_cnt++;
		column_cnt = i;
		line_temp[column_cnt] = '\0';

		if (CONFIG_CORTINA_FW_LENGTH < line_cnt)
			return;

		for (i = 0; i < column_cnt; i++) {
			if (isspace(line_temp[i++]))
				break;
		}

		memcpy(reg_addr, line_temp, i);
		memcpy(reg_data, &line_temp[i], column_cnt - i);
		strim(reg_addr);
		strim(reg_data);
		fw_temp.reg_addr = (simple_strtoul(reg_addr, NULL, 0)) & 0xffff;
		fw_temp.reg_value = (simple_strtoul(reg_data, NULL, 0)) &
				     0xffff;
		phy_write(phydev, 0x00, fw_temp.reg_addr, fw_temp.reg_value);
	}
}
#endif

int cs4340_phy_init(struct phy_device *phydev)
{
#ifndef CORTINA_NO_FW_UPLOAD
	int timeout = 100;  /* 100ms */
#endif
	int reg_value;

	/*
	 * Cortina phy has provision to store
	 * phy firmware in attached dedicated EEPROM.
	 * Boards designed with EEPROM attached to Cortina
	 * does not require FW upload.
	 */
#ifndef CORTINA_NO_FW_UPLOAD
	/* step1: BIST test */
	phy_write(phydev, 0x00, VILLA_GLOBAL_MSEQCLKCTRL,     0x0004);
	phy_write(phydev, 0x00, VILLA_GLOBAL_LINE_SOFT_RESET, 0x0000);
	phy_write(phydev, 0x00, VILLA_GLOBAL_BIST_CONTROL,    0x0001);
	while (--timeout) {
		reg_value = phy_read(phydev, 0x00, VILLA_GLOBAL_BIST_STATUS);
		if (reg_value & mseq_edc_bist_done) {
			if (0 == (reg_value & mseq_edc_bist_fail))
				break;
		}
		udelay(1000);
	}

	if (!timeout) {
		printf("%s BIST mseq_edc_bist_done timeout!\n", __func__);
		return -1;
	}

	/* setp2: upload ucode */
	cs4340_upload_firmware(phydev);
#endif
	reg_value = phy_read(phydev, 0x00, VILLA_GLOBAL_DWNLD_CHECKSUM_STATUS);
	if (reg_value) {
		debug("%s checksum status failed.\n", __func__);
		return -1;
	}

	return 0;
}

int cs4340_config(struct phy_device *phydev)
{
	cs4340_phy_init(phydev);
	return 0;
}

int cs4340_probe(struct phy_device *phydev)
{
	phydev->flags = PHY_FLAG_BROKEN_RESET;
	return 0;
}

int cs4340_startup(struct phy_device *phydev)
{
	phydev->link = 1;

	/* For now just lie and say it's 10G all the time */
	phydev->speed = SPEED_10000;
	phydev->duplex = DUPLEX_FULL;
	return 0;
}

int cs4223_phy_init(struct phy_device *phydev)
{
	int reg_value;

	reg_value = phy_read(phydev, 0x00, CS4223_EEPROM_STATUS);
	if (!(reg_value & CS4223_EEPROM_FIRMWARE_LOADDONE)) {
		printf("%s CS4223 Firmware not present in EERPOM\n", __func__);
		return -ENOSYS;
	}

	return 0;
}

int cs4223_config(struct phy_device *phydev)
{
	return cs4223_phy_init(phydev);
}

int cs4223_probe(struct phy_device *phydev)
{
	phydev->flags = PHY_FLAG_BROKEN_RESET;
	return 0;
}

int cs4223_startup(struct phy_device *phydev)
{
	phydev->link = 1;
	phydev->speed = SPEED_10000;
	phydev->duplex = DUPLEX_FULL;
	return 0;
}

struct phy_driver cs4340_driver = {
	.name = "Cortina CS4315/CS4340",
	.uid = PHY_UID_CS4340,
	.mask = 0xfffffff0,
	.features = PHY_10G_FEATURES,
	.mmds = (MDIO_DEVS_PMAPMD | MDIO_DEVS_PCS |
		 MDIO_DEVS_PHYXS | MDIO_DEVS_AN |
		 MDIO_DEVS_VEND1 | MDIO_DEVS_VEND2),
	.config = &cs4340_config,
	.probe	= &cs4340_probe,
	.startup = &cs4340_startup,
	.shutdown = &gen10g_shutdown,
};

struct phy_driver cs4223_driver = {
	.name = "Cortina CS4223",
	.uid = PHY_UID_CS4223,
	.mask = 0x0ffff00f,
	.features = PHY_10G_FEATURES,
	.mmds = (MDIO_DEVS_PMAPMD | MDIO_DEVS_PCS |
		 MDIO_DEVS_AN),
	.config = &cs4223_config,
	.probe	= &cs4223_probe,
	.startup = &cs4223_startup,
	.shutdown = &gen10g_shutdown,
};

int phy_cortina_init(void)
{
	phy_register(&cs4340_driver);
	phy_register(&cs4223_driver);
	return 0;
}

int get_phy_id(struct mii_dev *bus, int addr, int devad, u32 *phy_id)
{
	int phy_reg;

	/* Cortina PHY has non-standard offset of PHY ID registers */
	phy_reg = bus->read(bus, addr, 0, VILLA_GLOBAL_CHIP_ID_LSB);
	if (phy_reg < 0)
		return -EIO;
	*phy_id = (phy_reg & 0xffff) << 16;

	phy_reg = bus->read(bus, addr, 0, VILLA_GLOBAL_CHIP_ID_MSB);
	if (phy_reg < 0)
		return -EIO;
	*phy_id |= (phy_reg & 0xffff);

	if ((*phy_id == PHY_UID_CS4340) || (*phy_id == PHY_UID_CS4223))
		return 0;

	/*
	 * If Cortina PHY not detected,
	 * try generic way to find PHY ID registers
	 */
	phy_reg = bus->read(bus, addr, devad, MII_PHYSID1);
	if (phy_reg < 0)
		return -EIO;
	*phy_id = (phy_reg & 0xffff) << 16;

	phy_reg = bus->read(bus, addr, devad, MII_PHYSID2);
	if (phy_reg < 0)
		return -EIO;
	*phy_id |= (phy_reg & 0xffff);

	return 0;
}
