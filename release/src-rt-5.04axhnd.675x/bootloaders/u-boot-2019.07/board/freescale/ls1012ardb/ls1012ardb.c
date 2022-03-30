// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2016 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <i2c.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/fsl_serdes.h>
#ifdef CONFIG_FSL_LS_PPA
#include <asm/arch/ppa.h>
#endif
#include <asm/arch/mmu.h>
#include <asm/arch/soc.h>
#include <hwconfig.h>
#include <ahci.h>
#include <mmc.h>
#include <scsi.h>
#include <fsl_esdhc.h>
#include <environment.h>
#include <fsl_mmdc.h>
#include <netdev.h>
#include <fsl_sec.h>

DECLARE_GLOBAL_DATA_PTR;

#define BOOT_FROM_UPPER_BANK	0x2
#define BOOT_FROM_LOWER_BANK	0x1

int checkboard(void)
{
#ifdef CONFIG_TARGET_LS1012ARDB
	u8 in1;

	puts("Board: LS1012ARDB ");

	/* Initialize i2c early for Serial flash bank information */
	i2c_set_bus_num(0);

	if (i2c_read(I2C_MUX_IO_ADDR, I2C_MUX_IO_1, 1, &in1, 1) < 0) {
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
		gd->ram_size = CONFIG_SYS_SDRAM_SIZE;

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

	gd->ram_size = CONFIG_SYS_SDRAM_SIZE;
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

#ifdef CONFIG_ENV_IS_NOWHERE
	gd->env_addr = (ulong)&default_environment[0];
#endif

#ifdef CONFIG_FSL_CAAM
	sec_init();
#endif

#ifdef CONFIG_FSL_LS_PPA
	ppa_init();
#endif
	return 0;
}

#ifdef CONFIG_TARGET_LS1012ARDB
int esdhc_status_fixup(void *blob, const char *compat)
{
	char esdhc1_path[] = "/soc/esdhc@1580000";
	bool sdhc2_en = false;
	u8 mux_sdhc2;
	u8 io = 0;

	i2c_set_bus_num(0);

	/* IO1[7:3] is the field of board revision info. */
	if (i2c_read(I2C_MUX_IO_ADDR, I2C_MUX_IO_1, 1, &io, 1) < 0) {
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
		if (i2c_read(I2C_MUX_IO_ADDR, I2C_MUX_IO_0, 1, &io, 1) < 0) {
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

int ft_board_setup(void *blob, bd_t *bd)
{
	arch_fixup_fdt(blob);

	ft_cpu_setup(blob, bd);

	return 0;
}

static int switch_to_bank1(void)
{
	u8 data;
	int ret;

	i2c_set_bus_num(0);

	data = 0xf4;
	ret = i2c_write(0x24, 0x3, 1, &data, 1);
	if (ret) {
		printf("i2c write error to chip : %u, addr : %u, data : %u\n",
		       0x24, 0x3, data);
	}

	return ret;
}

static int switch_to_bank2(void)
{
	u8 data;
	int ret;

	i2c_set_bus_num(0);

	data = 0xfc;
	ret = i2c_write(0x24, 0x7, 1, &data, 1);
	if (ret) {
		printf("i2c write error to chip : %u, addr : %u, data : %u\n",
		       0x24, 0x7, data);
		goto err;
	}

	data = 0xf5;
	ret = i2c_write(0x24, 0x3, 1, &data, 1);
	if (ret) {
		printf("i2c write error to chip : %u, addr : %u, data : %u\n",
		       0x24, 0x3, data);
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

static int flash_bank_cmd(cmd_tbl_t *cmdtp, int flag, int argc,
			  char * const argv[])
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
