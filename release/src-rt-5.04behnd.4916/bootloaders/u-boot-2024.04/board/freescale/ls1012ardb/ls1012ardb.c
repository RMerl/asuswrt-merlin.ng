// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2016 Freescale Semiconductor, Inc.
 * Copyright 2021 NXP
 */

#include <common.h>
#include <command.h>
#include <fdt_support.h>
#include <hang.h>
#include <i2c.h>
#include <asm/cache.h>
#include <init.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/fsl_serdes.h>
#include <asm/arch/mmu.h>
#include <asm/arch/soc.h>
#include <hwconfig.h>
#include <ahci.h>
#include <mmc.h>
#include <scsi.h>
#include <fsl_esdhc.h>
#include <env_internal.h>
#include <fsl_mmdc.h>
#include <netdev.h>
#include <net/pfe_eth/pfe/pfe_hw.h>

DECLARE_GLOBAL_DATA_PTR;

#define BOOT_FROM_UPPER_BANK	0x2
#define BOOT_FROM_LOWER_BANK	0x1

int checkboard(void)
{
#ifdef CONFIG_TARGET_LS1012ARDB
	u8 in1;
	int ret, bus_num = 0;

	puts("Board: LS1012ARDB ");

	/* Initialize i2c early for Serial flash bank information */
#if CONFIG_IS_ENABLED(DM_I2C)
	struct udevice *dev;

	ret = i2c_get_chip_for_busnum(bus_num, I2C_MUX_IO_ADDR,
				      1, &dev);
	if (ret) {
		printf("%s: Cannot find udev for a bus %d\n", __func__,
		       bus_num);
		return -ENXIO;
	}
	ret = dm_i2c_read(dev, I2C_MUX_IO_1, &in1, 1);
#else /* Non DM I2C support - will be removed */
	i2c_set_bus_num(bus_num);
	ret = i2c_read(I2C_MUX_IO_ADDR, I2C_MUX_IO_1, 1, &in1, 1);
#endif
	if (ret < 0) {
		printf("Error reading i2c boot information!\n");
		return 0; /* Don't want to hang() on this error */
	}

	puts("Version");
	switch (in1 & SW_REV_MASK) {
	case SW_REV_A:
		puts(": RevA");
		break;
	case SW_REV_B:
		puts(": RevB");
		break;
	case SW_REV_C:
		puts(": RevC");
		break;
	case SW_REV_C1:
		puts(": RevC1");
		break;
	case SW_REV_C2:
		puts(": RevC2");
		break;
	case SW_REV_D:
		puts(": RevD");
		break;
	case SW_REV_E:
		puts(": RevE");
		break;
	default:
		puts(": unknown");
		break;
	}

	printf(", boot from QSPI");
	if ((in1 & SW_BOOT_MASK) == SW_BOOT_EMU)
		puts(": emu\n");
	else if ((in1 & SW_BOOT_MASK) == SW_BOOT_BANK1)
		puts(": bank1\n");
	else if ((in1 & SW_BOOT_MASK) == SW_BOOT_BANK2)
		puts(": bank2\n");
	else
		puts("unknown\n");
#else

	puts("Board: LS1012A2G5RDB ");
#endif
	return 0;
}

#ifdef CONFIG_TFABOOT
int dram_init(void)
{
	gd->ram_size = tfa_get_dram_size();
	if (!gd->ram_size)
		gd->ram_size = CFG_SYS_SDRAM_SIZE;

	return 0;
}
#else
int dram_init(void)
{
#ifndef CONFIG_TFABOOT
	static const struct fsl_mmdc_info mparam = {
		0x05180000,	/* mdctl */
		0x00030035,	/* mdpdc */
		0x12554000,	/* mdotc */
		0xbabf7954,	/* mdcfg0 */
		0xdb328f64,	/* mdcfg1 */
		0x01ff00db,	/* mdcfg2 */
		0x00001680,	/* mdmisc */
		0x0f3c8000,	/* mdref */
		0x00002000,	/* mdrwd */
		0x00bf1023,	/* mdor */
		0x0000003f,	/* mdasp */
		0x0000022a,	/* mpodtctrl */
		0xa1390003,	/* mpzqhwctrl */
	};

	mmdc_init(&mparam);
#endif

	gd->ram_size = CFG_SYS_SDRAM_SIZE;
#if !defined(CONFIG_SPL) || defined(CONFIG_SPL_BUILD)
	/* This will break-before-make MMU for DDR */
	update_early_mmu_table();
#endif

	return 0;
}
#endif


int board_early_init_f(void)
{
	fsl_lsch2_early_init_f();

	return 0;
}

int board_init(void)
{
	struct ccsr_cci400 *cci = (struct ccsr_cci400 *)(CONFIG_SYS_IMMR +
					CONFIG_SYS_CCI400_OFFSET);
	/*
	 * Set CCI-400 control override register to enable barrier
	 * transaction
	 */
	if (current_el() == 3)
		out_le32(&cci->ctrl_ord, CCI400_CTRLORD_EN_BARRIER);

#ifdef CONFIG_SYS_FSL_ERRATUM_A010315
	erratum_a010315();
#endif

	return 0;
}

#ifdef CONFIG_FSL_PFE
void board_quiesce_devices(void)
{
	pfe_command_stop(0, NULL);
}
#endif

#ifdef CONFIG_TARGET_LS1012ARDB
int esdhc_status_fixup(void *blob, const char *compat)
{
	char esdhc1_path[] = "/soc/esdhc@1580000";
	bool sdhc2_en = false;
	u8 mux_sdhc2;
	u8 io = 0;
	int ret, bus_num = 0;

#if CONFIG_IS_ENABLED(DM_I2C)
	struct udevice *dev;

	ret = i2c_get_chip_for_busnum(bus_num, I2C_MUX_IO_ADDR,
				      1, &dev);
	if (ret) {
		printf("%s: Cannot find udev for a bus %d\n", __func__,
		       bus_num);
		return -ENXIO;
	}
	ret = dm_i2c_read(dev, I2C_MUX_IO_1, &io, 1);
#else
	i2c_set_bus_num(bus_num);
	/* IO1[7:3] is the field of board revision info. */
	ret = i2c_read(I2C_MUX_IO_ADDR, I2C_MUX_IO_1, 1, &io, 1);
#endif
	if (ret < 0) {
		printf("Error reading i2c boot information!\n");
		return 0;
	}

	/* hwconfig method is used for RevD and later versions. */
	if ((io & SW_REV_MASK) <= SW_REV_D) {
#ifdef CONFIG_HWCONFIG
		if (hwconfig("esdhc1"))
			sdhc2_en = true;
#endif
	} else {
		/*
		 * The I2C IO-expander for mux select is used to control
		 * the muxing of various onboard interfaces.
		 *
		 * IO0[3:2] indicates SDHC2 interface demultiplexer
		 * select lines.
		 *	00 - SDIO wifi
		 *	01 - GPIO (to Arduino)
		 *	10 - eMMC Memory
		 *	11 - SPI
		 */
#if CONFIG_IS_ENABLED(DM_I2C)
		ret = dm_i2c_read(dev, I2C_MUX_IO_0, &io, 1);
#else
		ret = i2c_read(I2C_MUX_IO_ADDR, I2C_MUX_IO_0, 1, &io, 1);
#endif
		if (ret < 0) {
			printf("Error reading i2c boot information!\n");
			return 0;
		}

		mux_sdhc2 = (io & 0x0c) >> 2;
		/* Enable SDHC2 only when use SDIO wifi and eMMC */
		if (mux_sdhc2 == 2 || mux_sdhc2 == 0)
			sdhc2_en = true;
	}
	if (sdhc2_en)
		do_fixup_by_path(blob, esdhc1_path, "status", "okay",
				 sizeof("okay"), 1);
	else
		do_fixup_by_path(blob, esdhc1_path, "status", "disabled",
				 sizeof("disabled"), 1);
	return 0;
}
#endif

int ft_board_setup(void *blob, struct bd_info *bd)
{
	arch_fixup_fdt(blob);

	ft_cpu_setup(blob, bd);

	return 0;
}

static int switch_to_bank1(void)
{
	u8 data = 0xf4, chip_addr = 0x24, offset_addr = 0x03;
	int ret, bus_num = 0;

#if CONFIG_IS_ENABLED(DM_I2C)
	struct udevice *dev;

	ret = i2c_get_chip_for_busnum(bus_num, chip_addr,
				      1, &dev);
	if (ret) {
		printf("%s: Cannot find udev for a bus %d\n", __func__,
		       bus_num);
		return -ENXIO;
	}
	/*
	 * --------------------------------------------------------------------
	 * |bus |I2C address|       Device     |          Notes               |
	 * --------------------------------------------------------------------
	 * |I2C1|0x24, 0x25,| IO expander (CFG,| Provides 16bits of General   |
	 * |    |0x26	    | RESET, and INT/  | Purpose parallel Input/Output|
	 * |    |           | KW41GPIO) - NXP  | (GPIO) expansion for the     |
	 * |    |           | PCAL9555AHF      | I2C bus                      |
	 * ----- --------------------------------------------------------------
	 * - mount three IO expander(PCAL9555AHF) on I2C1
	 *
	 * PCAL9555A device address
	 *           slave address
	 *  --------------------------------------
	 *  | 0 | 1 | 0 | 0 | A2 | A1 | A0 | R/W |
	 *  --------------------------------------
	 *  |     fixed     | hardware selectable|
	 *
	 * Output port 1(Pinter register bits = 0x03)
	 *
	 * P1_[7~0] = 0xf4
	 * P1_0 <---> CFG_MUX_QSPI_S0
	 * P1_1 <---> CFG_MUX_QSPI_S1
	 * CFG_MUX_QSPI_S[1:0] = 0b00
	 *
	 * QSPI chip-select demultiplexer select
	 * ---------------------------------------------------------------------
	 * CFG_MUX_QSPI_S1|CFG_MUX_QSPI_S0|              Values
	 * ---------------------------------------------------------------------
	 *    0           | 0            |CS routed to SPI memory bank1(default)
	 * ---------------------------------------------------------------------
	 *    0           | 1             |CS routed to SPI memory bank2
	 * ---------------------------------------------------------------------
	 *
	 */
	ret = dm_i2c_write(dev, offset_addr, &data, 1);
#else /* Non DM I2C support - will be removed */
	i2c_set_bus_num(bus_num);
	ret = i2c_write(chip_addr, offset_addr, 1, &data, 1);
#endif

	if (ret) {
		printf("i2c write error to chip : %u, addr : %u, data : %u\n",
		       chip_addr, offset_addr, data);
	}

	return ret;
}

static int switch_to_bank2(void)
{
	u8 data[2] = {0xfc, 0xf5}, offset_addr[2] = {0x7, 0x3};
	u8 chip_addr = 0x24;
	int ret, i, bus_num = 0;

#if CONFIG_IS_ENABLED(DM_I2C)
	struct udevice *dev;

	ret = i2c_get_chip_for_busnum(bus_num, chip_addr,
				      1, &dev);
	if (ret) {
		printf("%s: Cannot find udev for a bus %d\n", __func__,
		       bus_num);
		return -ENXIO;
	}
#else /* Non DM I2C support - will be removed */
	i2c_set_bus_num(bus_num);
#endif

	/*
	 * 1th step: config port 1
	 *	- the port 1 pin is enabled as an output
	 * 2th step: output port 1
	 *	- P1_[7:0] output 0xf5,
	 *	  then CFG_MUX_QSPI_S[1:0] equal to 0b01,
	 *	  CS routed to SPI memory bank2
	 */
	for (i = 0; i < sizeof(data); i++) {
#if CONFIG_IS_ENABLED(DM_I2C)
		ret = dm_i2c_write(dev, offset_addr[i], &data[i], 1);
#else /* Non DM I2C support - will be removed */
		ret = i2c_write(chip_addr, offset_addr[i], 1, &data[i], 1);
#endif
		if (ret) {
			printf("i2c write error to chip : %u, addr : %u, data : %u\n",
			       chip_addr, offset_addr[i], data[i]);
			goto err;
		}
	}

err:
	return ret;
}

static int convert_flash_bank(int bank)
{
	int ret = 0;

	switch (bank) {
	case BOOT_FROM_UPPER_BANK:
		ret = switch_to_bank2();
		break;
	case BOOT_FROM_LOWER_BANK:
		ret = switch_to_bank1();
		break;
	default:
		ret = CMD_RET_USAGE;
		break;
	};

	return ret;
}

static int flash_bank_cmd(struct cmd_tbl *cmdtp, int flag, int argc,
			  char *const argv[])
{
	if (argc != 2)
		return CMD_RET_USAGE;
	if (strcmp(argv[1], "1") == 0)
		convert_flash_bank(BOOT_FROM_LOWER_BANK);
	else if (strcmp(argv[1], "2") == 0)
		convert_flash_bank(BOOT_FROM_UPPER_BANK);
	else
		return CMD_RET_USAGE;

	return 0;
}

U_BOOT_CMD(
	boot_bank, 2, 0, flash_bank_cmd,
	"Flash bank Selection Control",
	"bank[1-lower bank/2-upper bank] (e.g. boot_bank 1)"
);
